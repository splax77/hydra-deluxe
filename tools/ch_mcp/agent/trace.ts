// The hit-method tracer: hooks every method of the drum engine class and of
// its base class, and counts the calls, so the method that runs once per note
// can be picked out (plan 2026-10-28, Task D).
//
// It reads and traces only. Every hook is an Interceptor.attach with an
// onEnter that reads its arguments; nothing changes an argument or a return
// value, writes game memory or calls a game method. The no-writes check in
// tests/test_frida_attach.py keeps it that way.
//
// frida_attach.run_agent posts the start message. trace_hits.py adds the
// settings below to it, receives every message this agent sends, and ends the
// run on the signal ends_run reads.
//
// Two modes:
//   count  hooks every method, keeps a call count per method and the
//          arguments of the first ARG_SAMPLE_LIMIT calls, and sends the whole
//          table once at the end.
//   args   hooks only the methods the settings name and sends every call with
//          its arguments and GameManager.songTime at that moment.
// Both end when GameManager.isSongOver turns true, or just before the cap.

import "frida-il2cpp-bridge";
import { findGameManager, GAME_MANAGER } from "./gamemanager.js";

// How many calls per method keep their arguments in count mode (plan
// 2026-10-28, Task D).
const ARG_SAMPLE_LIMIT = 200;

// How often the end condition is checked: the clock sampling interval the
// user approved (plan 2026-10-28, question 5). The agent also stops one such
// interval before the cap, so its table reaches the runner before the runner
// detaches at the cap.
const END_POLL_MS = 250;

// The class flag for "sealed" (ECMA-335 partition II, TypeAttributes).
const TYPE_ATTRIBUTE_SEALED = 0x100;

interface Settings {
    cap_s: number;
    mode: "count" | "args";
    engine_class: string;
    engine_ctor_rva: number;
    hit_time_offset: number;
    methods: string[];
}

type Arg = number | boolean | string | null;
type Decoder = (ctx: X64CpuContext, args: InvocationArguments) => Arg;

interface Entry {
    names: string[];
    owner: "engine" | "base";
    rva: string;
    params: string[];
    count: number;
    args: Arg[][];
}

interface Target {
    address: NativePointer;
    entry: Entry;
    decoders: Decoder[];
}

// ---- reading one argument -------------------------------------------------
// Windows x64 passes the first four arguments by position in rcx, rdx, r8 and
// r9, or in xmm0 to xmm3 when the argument is a float or a double. Frida's
// args[i] reads the integer register or the stack slot; a float in the first
// four slots is only in the xmm register, read from the CPU context.

function xmmOf(ctx: X64CpuContext, slot: number): ArrayBuffer | undefined {
    return [ctx.xmm0, ctx.xmm1, ctx.xmm2, ctx.xmm3][slot];
}

function bitsOf(p: NativePointer): bigint {
    return BigInt(p.toString());
}

function floatDecoder(slot: number, double: boolean): Decoder {
    return (ctx, args) => {
        let buf: ArrayBuffer | undefined;
        if (slot < 4) {
            buf = xmmOf(ctx, slot);
        } else {
            buf = new BigUint64Array([bitsOf(args[slot])]).buffer;
        }
        if (buf === undefined) {
            return "xmm unreadable";
        }
        return double ? new Float64Array(buf, 0, 1)[0] : new Float32Array(buf, 0, 1)[0];
    };
}

function decoderFor(type: Il2Cpp.Type, slot: number): Decoder {
    const E = Il2Cpp.Type.Enum;
    if (type.isByReference) {
        const name = type.name;
        return () => `ref ${name}`;
    }
    const kind = type.enumValue;
    switch (kind) {
        case E.BOOLEAN:
            return (_c, a) => (a[slot].toInt32() & 0xff) !== 0;
        case E.BYTE:
            return (_c, a) => (a[slot].toInt32() << 24) >> 24;
        case E.UBYTE:
            return (_c, a) => a[slot].toInt32() & 0xff;
        case E.SHORT:
            return (_c, a) => (a[slot].toInt32() << 16) >> 16;
        case E.USHORT:
        case E.CHAR:
            return (_c, a) => a[slot].toInt32() & 0xffff;
        case E.INT:
            return (_c, a) => a[slot].toInt32();
        case E.UINT:
            return (_c, a) => a[slot].toUInt32();
        case E.LONG:
        case E.NINT:
            return (_c, a) => Number(BigInt.asIntN(64, bitsOf(a[slot])));
        case E.ULONG:
        case E.NUINT:
            return (_c, a) => Number(BigInt.asUintN(64, bitsOf(a[slot])));
        case E.FLOAT:
            return floatDecoder(slot, false);
        case E.DOUBLE:
            return floatDecoder(slot, true);
    }
    const klass = type.class;
    if (klass.isEnum) {
        return (_c, a) => a[slot].toInt32();
    }
    if (klass.isValueType) {
        const name = type.name;
        return () => `<${name}>`;
    }
    // A reference: null, or the class name of the object it points to.
    return (_c, a) => {
        const p = a[slot];
        if (p.isNull()) {
            return null;
        }
        try {
            return new Il2Cpp.Object(p).class.name;
        } catch (e) {
            return "unreadable object";
        }
    };
}

// A value type wider than a register comes back through a hidden pointer
// passed first, which moves every other argument one slot along.
function returnsThroughPointer(method: Il2Cpp.Method): boolean {
    const rt = method.returnType;
    if (rt.isByReference || rt.enumValue === Il2Cpp.Type.Enum.VOID) {
        return false;
    }
    const klass = rt.class;
    if (!klass.isValueType || klass.isEnum || rt.isPrimitive) {
        return false;
    }
    return ![1, 2, 4, 8].includes(klass.valueTypeSize);
}

function decodersFor(method: Il2Cpp.Method): Decoder[] {
    let slot = returnsThroughPointer(method) ? 1 : 0;
    if (!method.isStatic) {
        slot += 1; // `this`
    }
    return method.parameters.map((p, i) => decoderFor(p.type, slot + i));
}

// ---- finding the engine -----------------------------------------------------

function findEngine(s: Settings): { klass: Il2Cpp.Class; image: string } | string {
    const found: { klass: Il2Cpp.Class; image: string }[] = [];
    for (const assembly of Il2Cpp.domain.assemblies) {
        const klass = assembly.image.tryClass(s.engine_class);
        if (klass !== null) {
            found.push({ klass, image: assembly.name });
        }
    }
    if (found.length !== 1) {
        return `engine class ${s.engine_class} found in ${found.length} images`;
    }
    return found[0];
}

// The shape the dump gives the engine class, checked so a class that only
// shares the name is never traced.
function shapeOf(klass: Il2Cpp.Class, s: Settings) {
    const parent = klass.parent;
    const ctorRva = ptr(s.engine_ctor_rva);
    return {
        sealed: (klass.flags & TYPE_ATTRIBUTE_SEALED) !== 0,
        generic_base: parent !== null && parent.isInflated,
        hit_time_double: klass.fields.some((f) => !f.isStatic
            && f.offset === s.hit_time_offset
            && f.type.enumValue === Il2Cpp.Type.Enum.DOUBLE),
        ctor_rva: klass.methods.some((m) => m.name === ".ctor"
            && !m.virtualAddress.isNull()
            && m.relativeVirtualAddress.equals(ctorRva)),
    };
}

// ---- hooking ----------------------------------------------------------------

// One target per code address: several methods of these classes share one
// body, and a hook on that address sees all of them.
function targetsOf(klass: Il2Cpp.Class, owner: "engine" | "base",
                   wanted: string[] | null, byAddress: Map<string, Target>,
                   noCode: string[]) {
    for (const m of klass.methods) {
        if (wanted !== null && !wanted.includes(m.name)) {
            continue;
        }
        const address = m.virtualAddress;
        if (address.isNull()) {
            noCode.push(`${owner} ${m.name}`);
            continue;
        }
        const key = address.toString();
        const known = byAddress.get(key);
        if (known !== undefined) {
            known.entry.names.push(m.name);
            continue;
        }
        byAddress.set(key, {
            address,
            decoders: decodersFor(m),
            entry: {
                names: [m.name],
                owner,
                rva: m.relativeVirtualAddress.toString(),
                params: m.parameters.map((p) => p.type.name),
                count: 0,
                args: [],
            },
        });
    }
}

function decode(t: Target, ctx: CpuContext, args: InvocationArguments): Arg[] {
    const x64 = ctx as X64CpuContext;
    return t.decoders.map((d) => {
        try {
            return d(x64, args);
        } catch (e) {
            return "unreadable";
        }
    });
}

function run(s: Settings, startMs: number) {
    const engine = findEngine(s);
    if (typeof engine === "string") {
        send({ error: engine });
        return;
    }
    const klass = engine.klass;
    const parent = klass.parent;
    const shape = shapeOf(klass, s);
    const about = {
        name: klass.name,
        image: engine.image,
        parent: parent === null ? null : parent.type.name,
        shape,
    };
    const failed = Object.entries(shape).filter(([, ok]) => !ok).map(([k]) => k);
    if (failed.length > 0 || parent === null) {
        send({ error: `engine shape check failed: ${failed.join(", ")}`, engine: about });
        return;
    }

    const found = findGameManager();
    if (typeof found === "string") {
        send({ error: found });
        return;
    }
    const songTime = found.gm.field<number>("songTime");
    const songOver = found.gm.field<boolean>("isSongOver");

    const wanted = s.mode === "args" ? s.methods : null;
    const byAddress = new Map<string, Target>();
    const noCode: string[] = [];
    targetsOf(klass, "engine", wanted, byAddress, noCode);
    targetsOf(parent, "base", wanted, byAddress, noCode);
    const targets = [...byAddress.values()];
    if (wanted !== null) {
        const hooked = new Set(targets.flatMap((t) => t.entry.names));
        const missing = wanted.filter((n) => !hooked.has(n));
        if (missing.length > 0) {
            send({ error: `methods not found on the engine or its base: ${missing.join(", ")}`,
                   engine: about, no_code: noCode });
            return;
        }
    }

    send({ engine: about, hooked: targets.length, no_code: noCode });

    let calls = 0;
    const listeners: InvocationListener[] = [];
    const notHooked: string[] = [];
    for (const t of targets) {
        try {
            listeners.push(Interceptor.attach(t.address, {
                onEnter(args) {
                    t.entry.count += 1;
                    calls += 1;
                    if (s.mode === "args") {
                        send({ call: t.entry.names[0], names: t.entry.names,
                               songTime: songTime.value,
                               args: decode(t, this.context, args) });
                    } else if (t.entry.args.length < ARG_SAMPLE_LIMIT) {
                        t.entry.args.push(decode(t, this.context, args));
                    }
                },
            }));
        } catch (e) {
            notHooked.push(`${t.entry.names.join("/")}: ${e}`);
        }
    }

    const stopMs = s.cap_s * 1000 - END_POLL_MS;
    let wasOver = songOver.value;
    const finish = (reason: string) => {
        clearInterval(timer);
        for (const l of listeners) {
            l.detach();
        }
        if (s.mode === "args") {
            send({ done: reason, mode: s.mode, calls, not_hooked: notHooked });
        } else {
            send({ done: reason, mode: s.mode, calls, not_hooked: notHooked,
                   no_code: noCode, methods: targets.map((t) => t.entry) });
        }
    };
    const timer = setInterval(() => {
        try {
            const over = songOver.value;
            if (over && !wasOver) {
                finish("song over");
                return;
            }
            wasOver = over;
            if (Date.now() - startMs >= stopMs) {
                finish("cap");
            }
        } catch (e) {
            clearInterval(timer);
            for (const l of listeners) {
                l.detach();
            }
            send({ error: `reading ${GAME_MANAGER} failed: ${e}` });
        }
    }, END_POLL_MS);
}

recv("start", (message: Settings) => {
    // The cap counts from the start message, not from when the hooks are in:
    // the runner's cap starts there, and the table has to reach it first.
    const startMs = Date.now();
    Il2Cpp.perform(() => run(message, startMs)).catch((e) => {
        send({ error: `Il2Cpp.perform failed: ${e}` });
    });
});
