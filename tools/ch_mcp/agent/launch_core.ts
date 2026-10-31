// The launch agent's decision, with no Frida in it, so plain Node can run it
// (tests/test_launch_core.py). launch.ts is the glue: it hands this file the
// game's memory as an Io and answers the messages. The plan is
// docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md, "The launch".
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

// The game's memory as applyPatch sees it. Each RVA is the text from the
// patch, relative to the module's base; launch.ts adds the base.
export interface Io {
    readBytes(rva: string, n: number): number[];
    writeBytes(rva: string, bytes: number[]): void;
    hookEntry(rva: string, onEnter: () => void): void;
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
    error: string | null;
}

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

// The status from the arm until the module appears.
export function waitingStatus(): Status {
    return {
        state: "waiting",
        before: null,
        after: null,
        entry_before: null,
        module: null,
        loader_runs: 0,
        bytes_at_first_run: null,
        patched_at_ms: null,
        error: null,
    };
}

// Called by the entry hook each time the loader starts: counts the run and,
// on the first, reads the patch site as the loader is about to find it.
function loaderRan(status: Status, patch: Patch, io: Io): void {
    status.loader_runs += 1;
    if (status.loader_runs !== 1) {
        return;
    }
    try {
        status.bytes_at_first_run = formatBytes(
            io.readBytes(patch.patch_rva, parseBytes(patch.expect)!.length));
    } catch (e) {
        status.error = `reading the patch site at the loader's first run failed: ${e}`;
    }
}

// The whole decision, made once when the module appears. Both byte checks
// must pass before anything is written; then the write, its read-back, and
// only after those the read-only hook on the loader's entry. Any difference
// or failure ends in `refused` with the reason in `error`, and the caller
// kills the game on a refusal (bot_run.launch_human).
export function applyPatch(patch: Patch, io: Io, module: ModuleSeen,
                           now: () => number): Status {
    const status = waitingStatus();
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
            status.error = `the game's bytes are not the ones launch_patch.json expects: ` +
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
            status.error = `the write did not take: the patch site at ${patch.patch_rva} ` +
                `reads ${status.after} after writing ${patch.write}`;
            return status;
        }
        status.patched_at_ms = now();
        io.hookEntry(patch.loader_rva, () => loaderRan(status, patch, io));
        status.state = "patched";
    } catch (e) {
        status.state = "refused";
        status.error = `patching ${patch.module} failed: ${e}`;
    }
    return status;
}
