// The launch agent: loaded into a game that bot_run started paused, before
// GameAssembly.dll is mapped. On `arm` it watches for that module; the moment
// the game maps it, and before the game has used it, this agent changes one
// byte of the command-line player loader in memory so the loader builds a
// human player, not the bot. The game file on disk is never written.
//
// The plan is docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md, "The
// launch" and question 1; the user allowed this one-byte patch there
// (question 6). The decision itself, both byte checks, the write and the
// read-only hook on the loader's entry, is applyPatch in launch_core.ts. This
// file is the glue between it and Frida.
//
// It holds no address of its own: the module name, every RVA and every byte
// string come in the arm message, from tools/ch_mcp/launch_patch.json. It
// needs no il2cpp bridge, because it runs before il2cpp starts.
//
// frida_attach.LiveAgent posts each message with an id and takes only the
// reply carrying the same id. Every message gets exactly one reply: its
// answer, or an error. The agent never sends a message unasked.

import { applyPatch, readPatch, waitingStatus } from "./launch_core.js";
import type { Io, Patch, Status } from "./launch_core.js";

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
function patchModule(patch: Patch, module: Module): void {
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
    };
    status = applyPatch(patch, io, { base: base.toString(), size: module.size }, Date.now);
}

// Frida calls onAdded for every module already loaded, then for each new one
// as it is mapped. `defer` moves the patch out of that call to setImmediate
// (the plan, question 1, says when a run sets it).
function arm(m: Message): object {
    if (armed !== null) {
        throw new Error("this agent is already armed; a launch arms once");
    }
    const patch = readPatch(m.patch);
    const modules = Process.enumerateModules().map((x) => x.name);
    armed = patch;
    status = waitingStatus();
    observer = Process.attachModuleObserver({
        onAdded(module) {
            if (seen || module.name !== patch.module) {
                return;
            }
            seen = true;
            if (patch.defer) {
                setImmediate(() => patchModule(patch, module));
            } else {
                patchModule(patch, module);
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
