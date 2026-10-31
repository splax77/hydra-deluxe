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
// names each field, its class, its dump line and how it is read (its kind),
// and holds the enum words. This file sends each field's raw value and
// derives nothing from them: which row is highlighted is worked out in
// frida_tools.highlighted_row_text, its one home.
//
// The player screen is found a different way from the other menus. The
// bridge's Il2Cpp.gc.choose asks il2cpp's liveness walk for the objects of a
// class, and that walk starts only from static fields
// (livenessCalculationFromStatics). Live step LS-K3 read "0 live" for
// PlayerSelectionMenu while the game called the screen's methods every frame,
// so no chain of fields from a static leads to it; Unity's native scene holds
// it. So watchPlayerScreens hooks one per-frame method of the class and
// records each object it is called on, and the read uses that record.

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
const PLAYER_SCREEN = "PlayerSelectionMenu";
const MENU_CLASSES = ["PlayerProfileMenu", "MainMenu", "SongSelect", PLAYER_SCREEN];
// The player screen's per-frame method: `public bool ʸʳʼʹʸʽʾˀʴʼʷ()`, an
// instance method of PlayerSelectionMenu at dump.cs line 73000 of build 6142,
// whose code no other class shares. The method watch of LS-K3 saw it called
// once a frame while the screen ran. tests/test_frida_tools.py checks it
// against the dump's facts.
const PLAYER_SCREEN_FRAME_RVA = 0x32F590;
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

// One entry of menu_fields.json, as this file uses it. menu_fields.json's
// "about" says what each kind means; an entry with no kind is an enum.
interface MenuField {
    key: string;
    class: string;
    field: string;
    kind?: string;
}
const FIELDS = MENU_FIELDS.fields as MenuField[];
const ENUM_KIND = "enum";

// An enum value's number.
function enumNumber(value: Il2Cpp.ValueType): number {
    return value.field<number>(ENUM_VALUE_FIELD).value;
}

// An array's elements, each turned into a plain value by `each`, or null for
// a null array. Reading the elements goes through the bridge's Il2Cpp.Array,
// which on its first use in this process finds where elements start by
// building a string of its own and calling System.String.ToCharArray on it.
// That call touches no game object, and it is the bridge's, not this file's.
function arrayValues<T extends Il2Cpp.Field.Type, R>(array: Il2Cpp.Array<T>, each: (element: T) => R): R[] | null {
    if (array.isNull()) {
        return null;
    }
    const values: R[] = [];
    for (const element of array) {
        values.push(each(element));
    }
    return values;
}

// One field's raw value on one menu, read the way its kind says, or the error
// text when the read failed. Il2Cpp.Object.field looks a name up through the
// class and its parents (il2cpp_class_get_field_from_name walks the parent
// chain), so a field declared on BaseMenu is found on a PlayerSelectionMenu
// by the same call.
function readField(menu: Il2Cpp.Object, f: MenuField): unknown {
    try {
        switch (f.kind ?? ENUM_KIND) {
            case ENUM_KIND:
                return enumNumber(menu.field<Il2Cpp.ValueType>(f.field).value);
            case "int":
            case "bool":
                return menu.field<number | boolean>(f.field).value;
            case "string_array":
                return arrayValues(menu.field<Il2Cpp.Array<Il2Cpp.String>>(f.field).value, text);
            case "enum_array":
                return arrayValues(menu.field<Il2Cpp.Array<Il2Cpp.ValueType>>(f.field).value, enumNumber);
            default:
                return `menu.ts reads no kind named ${f.kind}`;
        }
    } catch (e) {
        return `error: ${e}`;
    }
}

function findClass(image: Il2Cpp.Image, name: string): Il2Cpp.Class | string {
    const klass = image.tryClass(name);
    return klass === null ? `${name} not found` : klass;
}

// ---- the player-screen record ----------------------------------------------
// The PlayerSelectionMenu objects the per-frame hook has been called on, by
// address. Only the hook adds to it; only seenPlayerScreens takes from it.
const playerScreens = new Map<string, NativePointer>();
// Why the hook is not in, or null once it is.
let playerScreenHook: string | null = "the player-screen hook has not been placed yet";

// Hook the player screen's per-frame method, once, as the state agent loads.
// Call it inside Il2Cpp.perform. The hook only records the object the method
// runs on: it walks no heap, reads no field and calls no game method, so it
// costs the game one map write a frame.
export function watchPlayerScreens(): void {
    const assembly = Il2Cpp.domain.tryAssembly(ASSEMBLY_NAME);
    if (assembly === null) {
        playerScreenHook = `${ASSEMBLY_NAME} assembly not found`;
        return;
    }
    const klass = findClass(assembly.image, PLAYER_SCREEN);
    if (typeof klass === "string") {
        playerScreenHook = klass;
        return;
    }
    const rva = ptr(PLAYER_SCREEN_FRAME_RVA).toString();
    const method = klass.methods.find(
        (m) => !m.isStatic && m.relativeVirtualAddress.toString() === rva);
    if (method === undefined) {
        playerScreenHook = `${PLAYER_SCREEN} has no instance method at RVA ${rva}`;
        return;
    }
    try {
        Interceptor.attach(method.virtualAddress, {
            onEnter(args) {
                // An instance method gets its object first.
                playerScreens.set(args[0].toString(), args[0]);
            },
        });
    } catch (e) {
        playerScreenHook = `hooking ${PLAYER_SCREEN} at RVA ${rva} failed: ${e}`;
        return;
    }
    playerScreenHook = null;
}

// What the read says when the Il2Cpp.perform around watchPlayerScreens failed.
export function playerScreenWatchFailed(e: unknown): void {
    playerScreenHook = `placing the player-screen hook failed: ${e}`;
}

// Whether a recorded address still holds a live PlayerSelectionMenu. The
// check reads the object's class word first and its native-object field only
// when the word names PlayerSelectionMenu, so freed and reused memory is read
// once, one word, and never as a menu. Memory the process has given back
// throws; that counts as gone.
function isLivePlayerScreen(klass: Il2Cpp.Class, obj: Il2Cpp.Object): boolean {
    try {
        return obj.class.handle.equals(klass.handle) && !isDestroyed(obj);
    } catch {
        return false;
    }
}

// The recorded player screens that are still alive, and how many were not.
// One that is destroyed, or whose memory no longer holds a PlayerSelectionMenu
// (the collector freed it and reused the space), is counted as destroyed and
// forgotten, so a later read never touches that memory again.
function seenPlayerScreens(klass: Il2Cpp.Class): Instances | string {
    if (playerScreenHook !== null) {
        return playerScreenHook;
    }
    const alive: Il2Cpp.Object[] = [];
    let destroyed = 0;
    for (const [key, p] of playerScreens) {
        const obj = new Il2Cpp.Object(p);
        if (isLivePlayerScreen(klass, obj)) {
            alive.push(obj);
        } else {
            playerScreens.delete(key);
            destroyed += 1;
        }
    }
    return { alive, destroyed };
}

// ---- reading the menus -----------------------------------------------------

interface Instances {
    alive: Il2Cpp.Object[];
    destroyed: number;
}

// One menu class's live instances and how many destroyed ones were skipped:
// the player screen's from the hook's record, every other class's from the
// collector, which walks the whole heap once per class.
function instancesOf(klass: Il2Cpp.Class, name: string): Instances | string {
    if (name === PLAYER_SCREEN) {
        return seenPlayerScreens(klass);
    }
    const held = Il2Cpp.gc.choose(klass);
    const alive = held.filter((obj) => !isDestroyed(obj));
    return { alive, destroyed: held.length - alive.length };
}

// One menu class: how many live and destroyed instances it has; each live
// instance's active read; for the menus that hold one, each live instance's
// Online menu read; and each field menu_fields.json names for this class, one
// value per live instance.
function readClass(image: Il2Cpp.Image, name: string, live: Map<string, Il2Cpp.Object[]>) {
    const klass = findClass(image, name);
    if (typeof klass === "string") {
        return { found: false, reason: klass };
    }
    const instances = instancesOf(klass, name);
    if (typeof instances === "string") {
        return { found: false, reason: instances };
    }
    const { alive, destroyed } = instances;
    live.set(name, alive);
    const entry: { [key: string]: unknown } = {
        found: true,
        live: alive.length,
        destroyed,
        active: alive.map(menuActive),
    };
    if (ONLINE_MENU_HOLDERS.has(name)) {
        entry.onlineMenuActive = alive.map(onlineMenuActive);
    }
    const fields: { [key: string]: unknown } = {};
    for (const f of FIELDS) {
        if (f.class === name) {
            fields[f.key] = alive.map((obj) => readField(obj, f));
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
// menu class read through the collector costs one walk of the whole heap
// (instancesOf says which classes are).
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
