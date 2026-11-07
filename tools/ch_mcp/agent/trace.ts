// The hit-method tracer: hooks every method of the drum engine class and of
// its base class, and counts the calls, so the method that runs once per note
// can be picked out (plan 2026-10-28, Task D).
//
// This agent only reads and traces: every hook is an Interceptor.attach with
// an onEnter that reads its arguments, and nothing here changes an argument or
// a return value. What an agent may change in the game, and the checks that
// hold it to that, are in docs/agents/clone-hero-mcp.md, "Safety rules".
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
// Both end at the first of three stops:
//   song over  GameManager.isSongOver turns true.
//   song cap   the song time played reaches the settings' song_cap_s. It is
//              counted in songTime from the poll where songTime first moves,
//              so a pause before or during the song does not use it up.
//   cap        the wall-clock cap is about to pass: the agent stops
//              END_POLL_MS before the runner's cap_s, counted from the start
//              message, so a song that never starts or stays paused still
//              ends. trace_hits.py owns both caps' numbers.

import "frida-il2cpp-bridge";
import { findGameManager, GAME_MANAGER } from "./gamemanager.js";
import { decode, targetsOf, Target } from "./hooks_common.js";

// How many calls per method keep their arguments in count mode (plan
// 2026-10-28, Task D).
const ARG_SAMPLE_LIMIT = 200;

// How often the end condition is checked: the clock sampling interval the
// user approved (plan 2026-10-28, question 5). The agent also stops one such
// interval before the wall-clock cap, so its table reaches the runner before
// the runner detaches at that cap.
const END_POLL_MS = 250;

// The class flag for "sealed" (ECMA-335 partition II, TypeAttributes).
const TYPE_ATTRIBUTE_SEALED = 0x100;

interface Settings {
    cap_s: number;       // the runner's wall-clock cap
    song_cap_s: number;  // the song cap, in song time played
    mode: "count" | "args";
    engine_class: string;
    engine_ctor_rva: number;
    hit_time_offset: number;
    methods: string[];
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
// targetsOf, decode and the argument decoders live in hooks_common.ts, which
// menuhooks.ts shares.

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
    // The song cap counts from the first poll whose songTime differs from
    // this first reading; until then the song has not moved.
    const firstSongTime = songTime.value;
    let movedFrom: number | null = null;
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
            const songNow = songTime.value;
            if (movedFrom === null && songNow !== firstSongTime) {
                movedFrom = songNow;
            }
            if (movedFrom !== null && songNow - movedFrom >= s.song_cap_s) {
                finish("song cap");
                return;
            }
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
    // The wall-clock cap counts from the start message, not from when the
    // hooks are in: the runner's cap starts there, and the table has to reach
    // it first.
    const startMs = Date.now();
    Il2Cpp.perform(() => run(message, startMs)).catch((e) => {
        send({ error: `Il2Cpp.perform failed: ${e}` });
    });
});
