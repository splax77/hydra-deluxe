// The launch agent's decision, with no Frida in it, so plain Node can run it
// (tests/test_launch_core.py). launch.ts is the glue: it hands this file the
// game's memory as an Io and answers the messages. The plan is
// docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md), "The launch".
//
// It holds no address and no byte of its own. Every RVA and every byte string
// comes in the arm message, from tools/ch_mcp/launch_patch.json, which owns
// them.
//
// Only type annotations and interfaces here: Node runs a .ts file by erasing
// its types, and refuses syntax that needs compiling (enums, namespaces).

// The fields of launch_patch.json this file reads; the rest ride along unread.
export interface Patch {
    module: string;
    loader_rva: string;
    entry_expect: string;
    patch_rva: string;
    expect: string;
    write: string;
    defer: boolean;
}

// One game function the agent may call, as launch_patch.json's `calls` lists
// it. launch.ts calls it only when its name is on launch.ts's own list.
export interface GameCall {
    name: string;
    rva: string;
    returns: string;
    args: string[];
}

// The song speed to set at the loader's first run, and where the setting is:
// launch_patch.json's `song_speed` block, with the armed value and the call.
export interface SpeedPlan {
    value: number;
    typeinfo_rva: string;
    statics_offset: string;
    object_offset: string;
    value_offset: string;
    min_offset: string;
    max_offset: string;
    min: number;
    max: number;
    setter: GameCall;
}

// The game's memory as applyPatch sees it. Each RVA is the text from the
// patch, relative to the module's base; launch.ts adds the base. A pointer
// is its hex text (zero for null), so this file needs no Frida type.
export interface Io {
    readBytes(rva: string, n: number): number[];
    writeBytes(rva: string, bytes: number[]): void;
    hookEntry(rva: string, onEnter: () => void): void;
    // The pointer stored at an RVA of the module.
    readSlot(rva: string): string;
    // The pointer, or the 32-bit integer, stored at `offset` from `at`.
    readPointer(at: string, offset: string): string;
    readInt(at: string, offset: string): number;
    // Calls one game function with arguments in its `args` order: a pointer
    // as its hex text, an int as a number.
    call(fn: GameCall, args: (string | number)[]): void;
}

export interface ModuleSeen {
    base: string;
    size: number;
}

// The status reply. `before` and `entry_before` are what the patch site and
// the loader's entry held when the module appeared; `after` is the patch site
// read back after the write. `loader_runs` counts the loader's calls since the
// hook was placed, and `bytes_at_first_run` is the patch site as the first of
// them found it. `patched_at_ms` is the agent's Date.now() when the write was
// read back, so the caller can set it against its own clock.
export interface Status {
    state: "waiting" | "patched" | "refused";
    before: string | null;
    after: string | null;
    entry_before: string | null;
    module: ModuleSeen | null;
    loader_runs: number;
    bytes_at_first_run: string | null;
    patched_at_ms: number | null;
    reason: string | null;
    // Only when a speed was armed, so a launch with none replies as before.
    speed?: SpeedStatus;
}

// The song speed's part of the status. `state` is "armed" until the
// loader's first run, then "set" or "refused". `min` and `max` are the
// object's own limits as read, `before` and `after` its value around the
// one setter call.
export interface SpeedStatus {
    value: number;
    state: "armed" | "set" | "refused";
    object: string | null;
    min: number | null;
    max: number | null;
    before: number | null;
    after: number | null;
}

// The one name of launch_patch.json's `calls` the speed uses.
const SETTER_CALL = "int_setting_set";

const RVA = /^0x[0-9A-Fa-f]+$/;
const BYTE = /^[0-9A-Fa-f]{2}$/;

// "BA 01 00 00 00" to [186, 1, 0, 0, 0]; null when the text is not that form.
function parseBytes(text: string): number[] | null {
    const words = text.trim().split(/\s+/);
    if (!words.every((w) => BYTE.test(w))) {
        return null;
    }
    return words.map((w) => parseInt(w, 16));
}

export function formatBytes(bytes: number[]): string {
    return bytes.map((b) => b.toString(16).toUpperCase().padStart(2, "0")).join(" ");
}

function same(a: number[], b: number[]): boolean {
    return a.length === b.length && a.every((v, i) => v === b[i]);
}

function needText(p: Record<string, unknown>, key: string): string {
    const v = p[key];
    if (typeof v !== "string" || v === "") {
        throw new Error(`the patch needs a text "${key}"`);
    }
    return v;
}

function needRva(p: Record<string, unknown>, key: string): string {
    const v = needText(p, key);
    if (!RVA.test(v)) {
        throw new Error(`the patch's "${key}" is not a hex RVA: ${v}`);
    }
    return v;
}

function needBytes(p: Record<string, unknown>, key: string): string {
    const v = needText(p, key);
    if (parseBytes(v) === null) {
        throw new Error(`the patch's "${key}" is not bytes written as two hex digits each: ${v}`);
    }
    return v;
}

// The arm message's patch, checked field by field. Throws naming the first
// field that is missing or malformed.
export function readPatch(raw: unknown): Patch {
    if (typeof raw !== "object" || raw === null || Array.isArray(raw)) {
        throw new Error(`arm needs a "patch" object, got ${JSON.stringify(raw)}`);
    }
    const p = raw as Record<string, unknown>;
    const patch: Patch = {
        module: needText(p, "module"),
        loader_rva: needRva(p, "loader_rva"),
        entry_expect: needBytes(p, "entry_expect"),
        patch_rva: needRva(p, "patch_rva"),
        expect: needBytes(p, "expect"),
        write: needBytes(p, "write"),
        defer: false,
    };
    if (typeof p.defer !== "boolean") {
        throw new Error(`the patch needs a true or false "defer"`);
    }
    patch.defer = p.defer;
    // A write of another length would overwrite bytes nobody checked.
    if (parseBytes(patch.write)!.length !== parseBytes(patch.expect)!.length) {
        throw new Error(`the patch's "write" (${patch.write}) is not the length of its "expect" (${patch.expect})`);
    }
    return patch;
}

function needInt(p: Record<string, unknown>, key: string): number {
    const v = p[key];
    if (typeof v !== "number" || !Number.isInteger(v)) {
        throw new Error(`the song speed needs a whole number "${key}", got ${JSON.stringify(v)}`);
    }
    return v;
}

// The armed song speed with launch_patch.json's `song_speed` block and its
// setter from `calls`, checked field by field; null when no speed was armed
// (`value` null or absent). Throws naming the first field that is missing or
// malformed, or a value outside the block's own limits.
export function readSpeed(raw: unknown, value: unknown): SpeedPlan | null {
    if (value === null || value === undefined) {
        return null;
    }
    const p = raw as Record<string, unknown>;
    const block = p.song_speed;
    if (typeof block !== "object" || block === null || Array.isArray(block)) {
        throw new Error(`a song speed was armed, but the patch has no "song_speed" object`);
    }
    const b = block as Record<string, unknown>;
    const calls = Array.isArray(p.calls) ? p.calls as Record<string, unknown>[] : [];
    const found = calls.filter((c) => typeof c === "object" && c !== null && c.name === SETTER_CALL);
    if (found.length !== 1) {
        throw new Error(`the patch's "calls" must list "${SETTER_CALL}" once, found ${found.length}`);
    }
    const c = found[0];
    const args = c.args;
    if (!Array.isArray(args) || !args.every((a) => a === "pointer" || a === "int")) {
        throw new Error(`the call "${SETTER_CALL}" needs "args" of "pointer" and "int" only`);
    }
    const setter: GameCall = {
        name: SETTER_CALL,
        rva: needRva(c, "rva"),
        returns: needText(c, "returns"),
        args: args as string[],
    };
    const plan: SpeedPlan = {
        value: 0,
        typeinfo_rva: needRva(b, "typeinfo_rva"),
        statics_offset: needRva(b, "statics_offset"),
        object_offset: needRva(b, "object_offset"),
        value_offset: needRva(b, "value_offset"),
        min_offset: needRva(b, "min_offset"),
        max_offset: needRva(b, "max_offset"),
        min: needInt(b, "min"),
        max: needInt(b, "max"),
        setter,
    };
    if (typeof value !== "number" || !Number.isInteger(value) ||
        value < plan.min || value > plan.max) {
        throw new Error(`the song speed ${JSON.stringify(value)} is not a whole number from ` +
                        `${plan.min} to ${plan.max}`);
    }
    plan.value = value;
    return plan;
}

// The status from the arm until the module appears, with the speed part
// when a speed was armed.
export function waitingStatus(speed: SpeedPlan | null = null): Status {
    const status: Status = {
        state: "waiting",
        before: null,
        after: null,
        entry_before: null,
        module: null,
        loader_runs: 0,
        bytes_at_first_run: null,
        patched_at_ms: null,
        reason: null,
    };
    if (speed !== null) {
        status.speed = {
            value: speed.value, state: "armed", object: null,
            min: null, max: null, before: null, after: null,
        };
    }
    return status;
}

// A pointer read as null: its text parses to zero.
function isNull(p: string): boolean {
    return Number.parseInt(p, 16) === 0;
}

// The text a null pointer argument is passed as.
const NULL_POINTER = "0";

// The speed's refusal: the status says why, and the setter is not called
// after it.
function refuseSpeed(status: Status, why: string): void {
    status.speed!.state = "refused";
    status.state = "refused";
    status.reason = `the song speed was not set: ${why}`;
}

// The one setter call, made at the loader's first run: find the object,
// check its limits against the armed ones, call the setter once, read the
// value back. Each failure refuses before anything later runs.
function setSpeed(status: Status, plan: SpeedPlan, io: Io): void {
    const speed = status.speed!;
    try {
        const klass = io.readSlot(plan.typeinfo_rva);
        if (isNull(klass)) {
            return refuseSpeed(status, `the settings class's TypeInfo slot at ${plan.typeinfo_rva} is null`);
        }
        const statics = io.readPointer(klass, plan.statics_offset);
        if (isNull(statics)) {
            return refuseSpeed(status, `the settings class's statics (+${plan.statics_offset}) are null`);
        }
        const object = io.readPointer(statics, plan.object_offset);
        if (isNull(object)) {
            return refuseSpeed(status, `the song_speed object (statics +${plan.object_offset}) is null`);
        }
        speed.object = object;
        speed.min = io.readInt(object, plan.min_offset);
        speed.max = io.readInt(object, plan.max_offset);
        if (speed.min !== plan.min || speed.max !== plan.max) {
            return refuseSpeed(status, `the object at ${object} holds limits ${speed.min} to ` +
                               `${speed.max}, not the ${plan.min} to ${plan.max} launch_patch.json names`);
        }
        speed.before = io.readInt(object, plan.value_offset);
        const args = plan.setter.args.map((kind, i) =>
            i === 0 ? object : kind === "int" ? plan.value : NULL_POINTER);
        io.call(plan.setter, args);
        speed.after = io.readInt(object, plan.value_offset);
        if (speed.after !== plan.value) {
            return refuseSpeed(status, `the setter was called with ${plan.value} and the ` +
                               `object reads ${speed.after} after it`);
        }
        speed.state = "set";
    } catch (e) {
        refuseSpeed(status, `${e}`);
    }
}

// Called by the entry hook each time the loader starts: counts the run and,
// on the first, reads the patch site as the loader is about to find it and
// sets the armed speed, if any.
function loaderRan(status: Status, patch: Patch, io: Io, speed: SpeedPlan | null): void {
    status.loader_runs += 1;
    if (status.loader_runs !== 1) {
        return;
    }
    try {
        status.bytes_at_first_run = formatBytes(
            io.readBytes(patch.patch_rva, parseBytes(patch.expect)!.length));
    } catch (e) {
        status.reason = `reading the patch site at the loader's first run failed: ${e}`;
    }
    if (speed !== null) {
        setSpeed(status, speed, io);
    }
}

// The whole decision, made once when the module appears. Both byte checks
// must pass before anything is written; then the write, its read-back, and
// only after those the hook on the loader's entry. Any difference
// or failure ends in `refused` with its `reason`, and the caller
// kills the game on a refusal (bot_run.launch_human). With a `speed`, the
// hook also sets it at the loader's first run (setSpeed).
export function applyPatch(patch: Patch, io: Io, module: ModuleSeen,
                           now: () => number, speed: SpeedPlan | null = null): Status {
    const status = waitingStatus(speed);
    status.module = module;
    const entryExpect = parseBytes(patch.entry_expect)!;
    const expect = parseBytes(patch.expect)!;
    const write = parseBytes(patch.write)!;
    try {
        const entry = io.readBytes(patch.loader_rva, entryExpect.length);
        const before = io.readBytes(patch.patch_rva, expect.length);
        status.entry_before = formatBytes(entry);
        status.before = formatBytes(before);
        if (!same(entry, entryExpect) || !same(before, expect)) {
            status.state = "refused";
            status.reason = `the game's bytes are not the ones launch_patch.json expects: ` +
                `the loader's entry at ${patch.loader_rva} reads ${status.entry_before} ` +
                `(expected ${patch.entry_expect}), the patch site at ${patch.patch_rva} ` +
                `reads ${status.before} (expected ${patch.expect}); nothing was written`;
            return status;
        }
        io.writeBytes(patch.patch_rva, write);
        const after = io.readBytes(patch.patch_rva, write.length);
        status.after = formatBytes(after);
        if (!same(after, write)) {
            status.state = "refused";
            status.reason = `the write did not take: the patch site at ${patch.patch_rva} ` +
                `reads ${status.after} after writing ${patch.write}`;
            return status;
        }
        status.patched_at_ms = now();
        io.hookEntry(patch.loader_rva, () => loaderRan(status, patch, io, speed));
        status.state = "patched";
    } catch (e) {
        status.state = "refused";
        status.reason = `patching ${patch.module} failed: ${e}`;
    }
    return status;
}
