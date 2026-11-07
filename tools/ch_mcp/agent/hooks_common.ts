// The hooking code the trace agent (trace.ts) and the method-watch agent
// (menuhooks.ts) share: which code addresses a class's methods run at, and how
// each argument of a hooked call is read back into a plain value.
//
// It only reads. A hook built from these targets is an Interceptor.attach
// whose onEnter reads its arguments; nothing here changes an argument, a
// return value or game memory.

import "frida-il2cpp-bridge";

export type Arg = number | boolean | string | null;
export type Decoder = (ctx: X64CpuContext, args: InvocationArguments) => Arg;

// How a reference argument is read. The default gives the class name of the
// object it points to; menuhooks.ts passes one that reads a few classes by
// field name instead.
export type ObjectReader = (obj: Il2Cpp.Object) => Arg;

export interface Entry {
    names: string[];
    owner: string;
    rva: string;
    params: string[];
    count: number;
    args: Arg[][];
}

export interface Target {
    address: NativePointer;
    entry: Entry;
    decoders: Decoder[];
}

const classNameOf: ObjectReader = (obj) => obj.class.name;

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

export function decoderFor(type: Il2Cpp.Type, slot: number,
                           readObject: ObjectReader = classNameOf): Decoder {
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
    // A reference: null, or what readObject makes of the object it points to.
    return (_c, a) => {
        const p = a[slot];
        if (p.isNull()) {
            return null;
        }
        try {
            return readObject(new Il2Cpp.Object(p));
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

function decodersFor(method: Il2Cpp.Method, readObject: ObjectReader): Decoder[] {
    let slot = returnsThroughPointer(method) ? 1 : 0;
    if (!method.isStatic) {
        slot += 1; // `this`
    }
    return method.parameters.map((p, i) => decoderFor(p.type, slot + i, readObject));
}

// ---- hooking ----------------------------------------------------------------

// One target per code address: several methods of these classes share one
// body, and a hook on that address sees all of them.
export function targetsOf(klass: Il2Cpp.Class, owner: string,
                          wanted: string[] | null, byAddress: Map<string, Target>,
                          noCode: string[], readObject: ObjectReader = classNameOf) {
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
            decoders: decodersFor(m, readObject),
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

export function decode(t: Target, ctx: CpuContext, args: InvocationArguments): Arg[] {
    const x64 = ctx as X64CpuContext;
    return t.decoders.map((d) => {
        try {
            return d(x64, args);
        } catch (e) {
            return "unreadable";
        }
    });
}
