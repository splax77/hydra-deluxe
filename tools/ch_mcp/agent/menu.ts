// The menu read: which menus exist, whether each live one is on screen,
// whether each one's Online menu is on screen, the player screen's fields, and
// which song is selected. state.ts adds it to every reply as the `menu` block;
// frida_tools.menu_lines prints it and menu_tools decides from it. This file
// names no menu state of its own: it sends what it read.
//
// It reads fields by name. It calls the game's getters in ALLOWED_CALLS,
// because whether an object is on screen is not a field. The user allowed
// method calls on 2026-10-30 (ruling 2 of the plan
// 2026-10-30-ch-mcp-star-power-and-song-start.md);
// test_agent_calls_only_named_methods in tests/test_frida_attach.py checks
// that every call in every agent is named in that agent's list.
//
// The player screen's fields are private and their names obfuscated, so they
// come from tools/ch_mcp/menu_fields.json, bundled in at build time; that file
// names each field, its class and its dump line, and holds the enum words.

import "frida-il2cpp-bridge";
import { isDestroyed } from "./gamemanager.js";
import MENU_FIELDS from "../menu_fields.json";

// The game methods this agent may call, by name. The test above reads this
// list from this file's text, so it stays one literal list. The dump of build
// 6142 has both: Component.get_gameObject at dump.cs line 694561 and
// GameObject.get_activeInHierarchy at line 695181.
export const ALLOWED_CALLS = ["get_activeInHierarchy", "get_gameObject"];

// The image and classes the read uses, as the Il2CppDumper dump of build 6142
// names them (dump.cs: SongSelect line 20595, MainMenu 42723,
// PlayerProfileMenu 45647, PlayerSelectionMenu 72822, GlobalVariables 55801,
// SongEntry 11073).
const ASSEMBLY_NAME = "CloneHero";
const MENU_CLASSES = ["PlayerProfileMenu", "MainMenu", "SongSelect", "PlayerSelectionMenu"];
// The menus that hold an `onlineMenu` GameObject.
const ONLINE_MENU_HOLDERS = new Set(["MainMenu", "SongSelect"]);
const SONG_SELECT = "SongSelect";
const GLOBALS_CLASS = "GlobalVariables";
// An enum's one instance field, which holds its number.
const ENUM_VALUE_FIELD = "value__";

// The one place this agent runs a game method. Every method handed in was
// looked up by a literal name from ALLOWED_CALLS, which the test checks at
// each lookup.
function call<T extends Il2Cpp.Method.ReturnType>(method: Il2Cpp.Method<T>): T {
    return method.invoke();
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

// Whether a GameObject is active in the hierarchy: true or false from the
// getter, null when there is no live object, or the error text when the call
// failed.
function activeInHierarchy(gameObject: Il2Cpp.Object): boolean | null | string {
    if (gameObject.isNull() || isDestroyed(gameObject)) {
        return null;
    }
    try {
        return call(gameObject.method<boolean>("get_activeInHierarchy"));
    } catch (e) {
        return `error: ${e}`;
    }
}

// Whether one live menu's own GameObject is active in the hierarchy, read the
// same way, or the error text when the getter for the GameObject failed.
function menuActive(menu: Il2Cpp.Object): boolean | null | string {
    let gameObject: Il2Cpp.Object;
    try {
        gameObject = call(menu.method<Il2Cpp.Object>("get_gameObject"));
    } catch (e) {
        return `error: ${e}`;
    }
    return activeInHierarchy(gameObject);
}

// Whether one menu's `onlineMenu` object is active in the hierarchy.
function onlineMenuActive(menu: Il2Cpp.Object): boolean | null | string {
    return activeInHierarchy(menu.field<Il2Cpp.Object>("onlineMenu").value);
}

// One enum field's number on one menu, or the error text when the read failed.
function enumField(menu: Il2Cpp.Object, name: string): number | string {
    try {
        return menu.field<Il2Cpp.ValueType>(name).value.field<number>(ENUM_VALUE_FIELD).value;
    } catch (e) {
        return `error: ${e}`;
    }
}

function findClass(image: Il2Cpp.Image, name: string): Il2Cpp.Class | string {
    const klass = image.tryClass(name);
    return klass === null ? `${name} not found` : klass;
}

// One menu class: how many live and destroyed instances the collector holds;
// each live instance's active read; for the menus that hold one, each live
// instance's Online menu read; and each field menu_fields.json names for this
// class, one value per live instance.
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
        active: alive.map(menuActive),
    };
    if (ONLINE_MENU_HOLDERS.has(name)) {
        entry.onlineMenuActive = alive.map(onlineMenuActive);
    }
    const fields: { [key: string]: unknown } = {};
    for (const f of MENU_FIELDS.fields) {
        if (f.class === name) {
            fields[f.key] = alive.map((obj) => enumField(obj, f.field));
        }
    }
    if (Object.keys(fields).length > 0) {
        entry.fields = fields;
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
        gameMode: gv.field<Il2Cpp.ValueType>("gameMode").value.field<number>(ENUM_VALUE_FIELD).value,
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
