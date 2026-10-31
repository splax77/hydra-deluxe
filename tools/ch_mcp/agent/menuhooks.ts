// The method-watch agent: hooks the menu and song-start methods that
// tools/ch_mcp/start_candidates.json names, and sends every call with its
// arguments, so a live run can show which method starts a song (the plan
// 2026-10-30-ch-mcp-star-power-and-song-start.md, task T7 and live step LS-M).
//
// It reads and traces only. Every hook is an Interceptor.attach whose onEnter
// reads its arguments; nothing changes an argument or a return value, writes
// game memory or calls a game method.
//
// method_watch.py builds the class list from start_candidates.json and posts
// it in the start message, then runs this agent through
// frida_attach.run_agent. The run ends when method_watch stops it or its cap
// passes: either way the runner detaches, which removes every hook. The agent
// itself keeps no timer.
//
// Messages, in order:
//   {hooked, classes, missing, no_code, not_hooked, skipped}
//                                                    once, after hooking
//   {call, class, names, rva, args, songTime, t_ms}  once per call
//   {error}                                          instead, when nothing
//                                                    could be hooked

import "frida-il2cpp-bridge";
import { lookUpGameManager } from "./gamemanager.js";
import { Arg, decode, ObjectReader, Target, targetsOf } from "./hooks_common.js";

// One class to watch, as method_watch.watch_settings writes it.
interface WatchClass {
    label: string;      // the class name the replies use
    full_name: string;  // namespace and name, as the image looks it up
    assembly: string | null;  // the one assembly to look in, or null for every one
    hook: "all" | "listed" | "by_name";
    rvas: number[];     // for "listed": the methods' RVAs
    names: string[];    // for "by_name": the method names, every overload
    skip_rvas: number[];  // for "all": addresses not to hook (method_watch.py's shared_rvas)
}

// One address an "all" class left unhooked, as the start reply names it.
interface Skipped {
    class: string;
    names: string[];
    rva: string;
}

interface Settings {
    cap_s: number;
    classes: WatchClass[];
}

// What a reference argument becomes in a call message. A song entry is sent
// as its folder, read by field name, so the reply names the song; a string
// as its text, so a scene load names its scene. Anything else is its class
// name, as the trace agent sends it.
const SONG_ENTRY_CLASS = "SongEntry";
const SONG_ENTRY_FOLDER_FIELD = "folderPath";
const STRING_CLASS = "System.String";

const readArgObject: ObjectReader = (obj) => {
    const klass = obj.class;
    if (klass.type.name === STRING_CLASS) {
        return new Il2Cpp.String(obj.handle).content;
    }
    if (klass.name === SONG_ENTRY_CLASS) {
        return obj.field<Il2Cpp.String>(SONG_ENTRY_FOLDER_FIELD).value.content;
    }
    return klass.name;
};

// The class in the one assembly its entry names. With no assembly named, the
// class in every image, so one that only shares its name with another is
// refused rather than guessed at (trace.ts's findEngine does the same).
function findClass(fullName: string, assemblyName: string | null): Il2Cpp.Class | string {
    if (assemblyName !== null) {
        const assembly = Il2Cpp.domain.tryAssembly(assemblyName);
        if (assembly === null) {
            return `assembly ${assemblyName} not found`;
        }
        const klass = assembly.image.tryClass(fullName);
        return klass === null ? `class ${fullName} not found in assembly ${assemblyName}` : klass;
    }
    const found: Il2Cpp.Class[] = [];
    for (const assembly of Il2Cpp.domain.assemblies) {
        const klass = assembly.image.tryClass(fullName);
        if (klass !== null) {
            found.push(klass);
        }
    }
    if (found.length !== 1) {
        return `class ${fullName} found in ${found.length} images`;
    }
    return found[0];
}

function rvaText(rva: number): string {
    return ptr(rva).toString();
}

// The targets of one class, added to `byAddress` (one target per code
// address, across every class). Returns what could not be found.
function watchClass(c: WatchClass, byAddress: Map<string, Target>, noCode: string[],
                    skipped: Skipped[]): { hooked: number; missing: string[] } {
    const klass = findClass(c.full_name, c.assembly);
    if (typeof klass === "string") {
        return { hooked: 0, missing: [klass] };
    }
    const missing: string[] = [];
    let wanted: string[] | null = null;
    if (c.hook === "listed") {
        const listed = new Set(c.rvas.map(rvaText));
        wanted = klass.methods
            .filter((m) => listed.has(m.relativeVirtualAddress.toString()))
            .map((m) => m.name);
        const present = new Set(klass.methods.map((m) => m.relativeVirtualAddress.toString()));
        for (const rva of c.rvas) {
            if (!present.has(rvaText(rva))) {
                missing.push(`${c.label} has no method at RVA ${rvaText(rva)}`);
            }
        }
    } else if (c.hook === "by_name") {
        wanted = c.names;
        const present = new Set(klass.methods.map((m) => m.name));
        for (const name of c.names) {
            if (!present.has(name)) {
                missing.push(`${c.label} has no method named ${name}`);
            }
        }
    }
    const own = new Map<string, Target>();
    targetsOf(klass, c.label, wanted, own, noCode, readArgObject);
    const skip = new Set(c.skip_rvas.map(rvaText));
    let hooked = 0;
    for (const [key, t] of own) {
        // A listed class hooks only the listed addresses, not an overload
        // that shares a listed method's name.
        if (c.hook === "listed" && !c.rvas.map(rvaText).includes(t.entry.rva)) {
            continue;
        }
        // Code other classes run too would bury this class's calls.
        if (skip.has(t.entry.rva)) {
            skipped.push({ class: c.label, names: t.entry.names, rva: t.entry.rva });
            continue;
        }
        const known = byAddress.get(key);
        if (known !== undefined) {
            known.entry.names.push(...t.entry.names);
            continue;
        }
        byAddress.set(key, t);
        hooked += 1;
    }
    return { hooked, missing };
}

// The song clock, when a GameManager is live as the watch starts. It is
// looked up once, here: each lookup walks the whole heap, so the hooks never
// do it. A watch started in the menu sends null; t_ms orders the calls.
function songClock(): (() => Arg) {
    const found = lookUpGameManager();
    if (!("gm" in found)) {
        return () => null;
    }
    const field = found.gm.field<number>("songTime");
    return () => {
        try {
            return field.value;
        } catch (e) {
            return null;
        }
    };
}

function run(s: Settings, startMs: number) {
    const byAddress = new Map<string, Target>();
    const noCode: string[] = [];
    const missing: string[] = [];
    const skipped: Skipped[] = [];
    const classes: { class: string; hooked: number }[] = [];
    for (const c of s.classes) {
        const result = watchClass(c, byAddress, noCode, skipped);
        classes.push({ class: c.label, hooked: result.hooked });
        missing.push(...result.missing);
    }
    const targets = [...byAddress.values()];
    if (targets.length === 0) {
        send({ error: "no method of the watched classes could be hooked",
               classes, missing, no_code: noCode, skipped });
        return;
    }
    const songTime = songClock();

    const notHooked: string[] = [];
    let hooked = 0;
    for (const t of targets) {
        try {
            Interceptor.attach(t.address, {
                onEnter(args) {
                    send({ call: t.entry.names[0], class: t.entry.owner,
                           names: t.entry.names, rva: t.entry.rva,
                           args: decode(t, this.context, args),
                           songTime: songTime(), t_ms: Date.now() - startMs });
                },
            });
            hooked += 1;
        } catch (e) {
            notHooked.push(`${t.entry.owner} ${t.entry.names.join("/")}: ${e}`);
        }
    }
    send({ hooked, classes, missing, no_code: noCode, not_hooked: notHooked, skipped });
}

recv("start", (message: Settings) => {
    // t_ms counts from the start message, the moment the runner's cap starts.
    const startMs = Date.now();
    Il2Cpp.perform(() => run(message, startMs)).catch((e) => {
        send({ error: `Il2Cpp.perform failed: ${e}` });
    });
});
