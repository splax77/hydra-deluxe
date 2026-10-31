// The menu read: which menus exist, whether each one's Online menu is on
// screen, and which song is selected. state.ts adds it to every reply as the
// `menu` block; frida_tools.menu_lines prints it and menu_tools decides from
// it. This file names no menu state of its own: it sends what it read.
//
// It reads fields by name. It makes one kind of game method call, the getters
// in ALLOWED_CALLS, because whether a menu is on screen is not a field. The
// user allowed method calls on 2026-10-30 (ruling 2 of the plan
// 2026-10-30-ch-mcp-star-power-and-song-start.md);
// test_agent_calls_only_named_methods in tests/test_frida_attach.py checks
// that every call in every agent is named here.

import "frida-il2cpp-bridge";

// The game methods an agent may call, by name. The test above reads this list
// from this file's text, so it stays one literal list.
export const ALLOWED_CALLS = ["get_activeInHierarchy"];

// The image and classes the read uses, as the Il2CppDumper dump of build 6142
// names them (dump.cs: SongSelect line 20595, MainMenu 42723,
// PlayerSelectionMenu 72822, GlobalVariables 55801, SongEntry 11073).
const ASSEMBLY_NAME = "CloneHero";
const MENU_CLASSES = ["MainMenu", "SongSelect", "PlayerSelectionMenu"];
// The menus that hold an `onlineMenu` GameObject.
const ONLINE_MENU_HOLDERS = new Set(["MainMenu", "SongSelect"]);
const SONG_SELECT = "SongSelect";
const GLOBALS_CLASS = "GlobalVariables";

// The same field gamemanager.ts reads to tell a destroyed Unity object from a
// live one; see NATIVE_OBJECT_FIELD there.
const NATIVE_OBJECT_FIELD = "m_CachedPtr";

function isDestroyed(obj: Il2Cpp.Object): boolean {
    return obj.field<NativePointer>(NATIVE_OBJECT_FIELD).value.isNull();
}

function text(s: Il2Cpp.String): string | null {
    return s.isNull() ? null : s.content;
}

// A SongEntry's folder and chart name, or null for no entry.
function songOf(entry: Il2Cpp.Object): { folderPath: string | null; chartName: string | null } | null {
    if (entry.isNull()) {
        return null;
    }
    return {
        folderPath: text(entry.field<Il2Cpp.String>("folderPath").value),
        chartName: text(entry.field<Il2Cpp.String>("chartName").value),
    };
}

// Whether one menu's `onlineMenu` object is active in the hierarchy: true or
// false from the getter, null when the field holds no live object, or the
// error text when the call failed.
function onlineMenuActive(menu: Il2Cpp.Object): boolean | null | string {
    const online = menu.field<Il2Cpp.Object>("onlineMenu").value;
    if (online.isNull() || isDestroyed(online)) {
        return null;
    }
    try {
        return online.method<boolean>("get_activeInHierarchy").invoke();
    } catch (e) {
        return `error: ${e}`;
    }
}

function findClass(image: Il2Cpp.Image, name: string): Il2Cpp.Class | string {
    const klass = image.tryClass(name);
    return klass === null ? `${name} not found` : klass;
}

// One menu class: how many live and destroyed instances the collector holds,
// and for the menus that hold one, each live instance's Online menu read.
function readClass(image: Il2Cpp.Image, name: string, live: Map<string, Il2Cpp.Object[]>) {
    const klass = findClass(image, name);
    if (typeof klass === "string") {
        return { found: false, reason: klass };
    }
    const held = Il2Cpp.gc.choose(klass);
    const alive = held.filter((obj) => !isDestroyed(obj));
    live.set(name, alive);
    const entry: { [key: string]: unknown } = {
        found: true,
        live: alive.length,
        destroyed: held.length - alive.length,
    };
    if (ONLINE_MENU_HOLDERS.has(name)) {
        entry.onlineMenuActive = alive.map(onlineMenuActive);
    }
    return entry;
}

function readGlobals(image: Il2Cpp.Image) {
    const klass = findClass(image, GLOBALS_CLASS);
    if (typeof klass === "string") {
        return { found: false, reason: klass };
    }
    const gv = klass.field<Il2Cpp.Object>("instance").value;
    if (gv.isNull() || isDestroyed(gv)) {
        return { found: false, reason: `no live ${GLOBALS_CLASS}.instance` };
    }
    const entry = songOf(gv.field<Il2Cpp.Object>("currentSongEntry").value);
    return {
        found: true,
        currentSongFolder: entry === null ? null : entry.folderPath,
        gameMode: gv.field<Il2Cpp.ValueType>("gameMode").value.field<number>("value__").value,
        playerCount: gv.field<number>("playerCount").value,
    };
}

// The menu block. Call it inside Il2Cpp.perform. Like lookUpGameManager, each
// menu class costs one walk of the whole heap.
export function readMenu() {
    const assembly = Il2Cpp.domain.tryAssembly(ASSEMBLY_NAME);
    if (assembly === null) {
        return { error: `${ASSEMBLY_NAME} assembly not found` };
    }
    const image = assembly.image;
    const live = new Map<string, Il2Cpp.Object[]>();
    const classes: { [name: string]: unknown } = {};
    for (const name of MENU_CLASSES) {
        classes[name] = readClass(image, name, live);
    }
    const songSelects = live.get(SONG_SELECT) ?? [];
    const selectedSong = songSelects.length === 0
        ? null
        : songOf(songSelects[0].field<Il2Cpp.Object>("currentSong").value);
    return { classes, selectedSong, globals: readGlobals(image) };
}
