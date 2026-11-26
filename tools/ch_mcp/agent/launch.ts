// The launch agent: loaded into a game that bot_run started paused, before
// GameAssembly.dll is mapped. On `arm` it watches for that module; the moment
// the game maps it, and before the game has used it, this agent changes one
// byte of the command-line player loader in memory so the loader builds a
// human player, not the bot. The game file on disk is never written.
//
// The plan is docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md), "The
// launch" and "1. When the loader runs, and how the byte is 0 before it does";
// the user allowed this one-byte patch there ("Questions for the user",
// question 6). The decision itself, both byte checks, the write and the
// hook on the loader's entry, is applyPatch in launch_core.ts. When ch_start
// arms a song speed, that hook also calls the game's int setter once, at the
// loader's first run (setSpeed in launch_core.ts); callGame below is the one
// place the call is made. This file is the glue between it and Frida.
//
// It holds no address of its own: the module name, every RVA and every byte
// string come in the arm message, from tools/ch_mcp/launch_patch.json. It
// needs no il2cpp bridge, because it runs before il2cpp starts.
//
// frida_attach.LiveAgent posts each message with an id and takes only the
// reply carrying the same id. Every message gets exactly one reply: its
// answer, or an error. The agent never sends a message unasked.
//
// An `error` key marks an agent fault: LiveAgent raises on it, as on every
// other agent's. So a status never carries one; a refusal is a normal reply
// with `state: "refused"` and its `reason` (launch_core.ts).

import { applyPatch, readPatch, readSpeed, waitingStatus } from "./launch_core.js";
import type { GameCall, Io, Patch, SpeedPlan, Status } from "./launch_core.js";

// The game functions this agent may call, by their names in
// launch_patch.json's `calls` (tests/test_frida_attach.py pins the two to
// each other). callGame refuses any other.
const ALLOWED_CALLS = ["int_setting_set"];

// The one place this agent calls a game function: the setter launch_core.ts
// asks for at the loader's first run, when a speed was armed.
function callGame(base: NativePointer, fn: GameCall, args: (string | number)[]): void {
    if (!ALLOWED_CALLS.includes(fn.name)) {
        throw new Error(`${fn.name} is not on launch.ts's list of calls`);
    }
    const target = new NativeFunction(base.add(fn.rva), fn.returns as NativeFunctionReturnType,
                                      fn.args as NativeFunctionArgumentType[]);
    const values = fn.args.map((kind, i) => kind === "pointer" ? ptr(args[i] as string) : args[i]);
    (target as unknown as (...a: unknown[]) => unknown)(...values);
}

interface Message {
    type: string;
    id: number;
    [key: string]: unknown;
}

// Set by the one arm this agent accepts.
let armed: Patch | null = null;
let status: Status | null = null;
// The observer calls back for every module, and for the named one only once
// is wanted.
let seen = false;
// Kept for the agent's life, so the observer is never collected while the
// game is still starting.
let observer: ModuleObserver | null = null;

// The only function that changes the game's code. The observer's callback
// and its deferred branch both call it, and nothing else does
// (tests/test_frida_attach.py checks this).
function patchModule(patch: Patch, module: Module, speed: SpeedPlan | null): void {
    const base = module.base;
    const io: Io = {
        readBytes(rva, n) {
            const buf = base.add(rva).readByteArray(n);
            if (buf === null) {
                throw new Error(`nothing could be read at ${rva}`);
            }
            return Array.from(new Uint8Array(buf));
        },
        writeBytes(rva, bytes) {
            Memory.patchCode(base.add(rva), bytes.length, (code) => {
                code.writeByteArray(bytes);
            });
        },
        hookEntry(rva, onEnter) {
            Interceptor.attach(base.add(rva), {
                onEnter() {
                    onEnter();
                },
            });
        },
        readSlot(rva) {
            return base.add(rva).readPointer().toString();
        },
        readPointer(at, offset) {
            return ptr(at).add(offset).readPointer().toString();
        },
        readInt(at, offset) {
            return ptr(at).add(offset).readS32();
        },
        call(fn, args) {
            callGame(base, fn, args);
        },
    };
    status = applyPatch(patch, io, { base: base.toString(), size: module.size }, Date.now, speed);
}

// Frida calls onAdded for every module already loaded, then for each new one
// as it is mapped. `defer` moves the patch out of that call to setImmediate
// (the plan's "Questions for the user", question 2, says when a run sets it).
function arm(m: Message): object {
    if (armed !== null) {
        throw new Error("this agent is already armed; a launch arms once");
    }
    const patch = readPatch(m.patch);
    // A speed only when ch_start asked for one (bot_run.launch_human).
    const speed = readSpeed(m.patch, m.song_speed);
    const modules = Process.enumerateModules().map((x) => x.name);
    armed = patch;
    status = waitingStatus(speed);
    observer = Process.attachModuleObserver({
        onAdded(module) {
            if (seen || module.name !== patch.module) {
                return;
            }
            seen = true;
            if (patch.defer) {
                setImmediate(() => patchModule(patch, module, speed));
            } else {
                patchModule(patch, module, speed);
            }
        },
    });
    // The state as it stands after the observer's first calls: `waiting` on
    // a paused game, which has not mapped the module yet.
    return { state: status.state, modules };
}

// `read` is the first message LiveAgent.start posts; it and `status` both
// answer with the status, which is `idle` until the arm.
function current(): object {
    if (status === null) {
        return { state: "idle" };
    }
    return { ...status };
}

function answer(m: Message): object {
    switch (m.type) {
        case "read":
        case "status":
            return current();
        case "arm":
            return arm(m);
        default:
            throw new Error(`unknown message type ${m.type}`);
    }
}

// recv hands over one message per registration, so the handler registers
// itself again once its reply is sent. Messages that arrive meanwhile wait in
// Frida's queue and are answered in order.
function listen() {
    recv((message: Message) => {
        const id = message.id;
        try {
            send({ ...answer(message), id });
        } catch (e) {
            send({ error: `${message.type} failed: ${e}`, id });
        }
        listen();
    });
}

listen();
