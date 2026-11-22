// The kit agent: a virtual drum kit inside the game. It builds one Rewired
// custom controller from the game's own drum definition, hands it to one
// player with one of the game's maps loaded by category and layout, presses
// its pads and buttons on request, and takes it away again. It sends no key presses and never touches
// the window, so the game can stay minimized and unfocused. While the kit is
// set up, Rewired's keyboard and mouse devices are switched off, so the user's
// own keys and mouse reach no player; teardown switches each back.
//
// Before the kit it builds the placeholder controllers the setup message
// names, which only use up ids, so the kit lands on the id the game grades
// hit strength for; tools/ch_mcp/kit_tools.py says which id and checks it.
// A placeholder is never given to a player, and teardown destroys each one.
//
// The plan is docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md)
// ("One agent for the kit"). The user allowed method calls and field sets
// where a feature needs them (D272 rulings 2 and 5).
//
// Every game method it calls is a literal name in ALLOWED_CALLS, and every
// name there is listed with its dump line in tools/ch_mcp/kit_methods.json;
// tests/test_frida_attach.py checks both. It reads fields by name.
//
// It holds none of the kit's numbers. The definition id, the tags, the player,
// the map's category and layout, the element ids and values, the hold in
// frames, the rewrite flag, the thread and the press cap all come in the messages; tools/ch_mcp/kit_tools.py
// owns them and checks the map this agent reads back.
//
// frida_attach.LiveAgent posts each message with an id and takes only the
// reply carrying the same id. Every message gets exactly one reply: its
// answer, or an error. Messages are answered one at a time, in order.
//
// The block "in-game timing" below holds the play messages (play, retime,
// stop_play) and frame_probe. They hook GameManager.Update, the one
// method this agent hooks rather than calls. D339 item 4 lets that block read
// the song clock and the score at the addresses the play message gives; every
// other read here stays by name. Play events go to Python with no id.

import "frida-il2cpp-bridge";
import { GAME_MANAGER } from "./gamemanager.js";
import { PressSchedule } from "./press_schedule.js";
import type { PlayConfig, Step } from "./press_schedule.js";

// The game methods this agent may call, by name. The test above reads this
// list from this file's text, so it stays one literal list.
export const ALLOWED_CALLS = [
    "get_configuration", "get_controllers", "get_players",
    "get_ignoreInputWhenAppNotInFocus", "set_ignoreInputWhenAppNotInFocus",
    "get_Keyboard", "get_Mouse", "set_enabled",
    "GetFirstCustomControllerWithTag", "CreateCustomController", "DestroyCustomController",
    "get_AllPlayers", "get_id", "get_name",
    "get_hasKeyboard", "set_hasKeyboard", "get_hasMouse", "set_hasMouse",
    "AddController", "RemoveController",
    "LoadMap", "GetMaps",
    "get_categoryId", "get_layoutId", "get_enabled", "get_AllMaps",
    "get_actionId", "get_actionDescriptiveName", "get_elementIdentifierId",
    "get_elementIdentifierName", "get_elementType", "get_axisContribution",
    "SetAxisValueById", "SetButtonValueById",
    "get_Count", "get_Item",
    "get_frameCount", "get_isFocused",
    "GetButton", "GetAnyButton",
    "Update",
    "add_InputSourceUpdateEvent", "remove_InputSourceUpdateEvent", "ʵʺʴʻˀʺˀˁʶʻʷ",
];

// The images and classes, as the Il2CppDumper dump of build 6142 names them;
// kit_methods.json gives each one's dump line.
const REWIRED = "Rewired_Core";
const UNITY_CORE = "UnityEngine.CoreModule";
const GAME = "CloneHero";
const STRIKE = "StrikeCore";
// The engine's base class, scrambled; docs/clone-hero/code/names-6142.tsv
// maps it and its frame update to our names.
const ENGINE_BASE = "ʿʶʴˀʴʾʵʵʶʳʲ";

// How often the press loop wakes to read the frame count and, when the
// message asks for it, write the values again: the probe's 2 ms (the plan,
// "One agent for the kit").
const TICK_MS = 2;

interface Message {
    type: string;
    id: number;
    [key: string]: unknown;
}

// How many true reads a watch reply lists; the rest are only counted (the
// plan, task K7).
const WATCH_LIST_MAX = 20;

// One element of a press: an axis takes a number, a button takes true.
interface Element {
    id: number;
    value: number | boolean;
}

interface PlayerRecord {
    id: number;
    name: string | null;
    player: Il2Cpp.Object;
    helper: Il2Cpp.Object;
    hasKeyboard: boolean;
    hasMouse: boolean;
}

// What setup found before it changed anything, so teardown can put it back.
// Set by the first setup of this agent and kept until teardown.
interface Recorded {
    tag: string;
    placeholderTags: string[];
    switchWas: boolean;
    keyboardWas: boolean;
    mouseWas: boolean;
    players: PlayerRecord[];
    placeholders: Placeholder[];
    kit: Il2Cpp.Object | null;
    target: PlayerRecord | null;
}

// A placeholder controller that exists in the game, with the tag it has.
interface Placeholder {
    tag: string;
    controller: Il2Cpp.Object;
}

let recorded: Recorded | null = null;
// Every element a press wrote, by id, with whether it is a button, so
// teardown can clear them.
const touched = new Map<number, boolean>();

function rewired(name: string): Il2Cpp.Class {
    return Il2Cpp.domain.assembly(REWIRED).image.class(name);
}

function unity(name: string): Il2Cpp.Class {
    return Il2Cpp.domain.assembly(UNITY_CORE).image.class(name);
}

function text(s: Il2Cpp.String): string | null {
    return s.isNull() ? null : s.content;
}

function isNothing(o: Il2Cpp.Object | null): boolean {
    return o === null || o.isNull();
}

// An enum value as its number, whichever form the bridge hands back.
function enumNumber(v: unknown): number | string {
    if (typeof v === "number") {
        return v;
    }
    if (v instanceof Il2Cpp.ValueType) {
        return v.field<number>("value__").value;
    }
    return String(v);
}

// One argument as the bridge's .invoke would hand it to native code: the
// bridge's own Il2Cpp.toFridaValue, which its index.d.ts leaves out as
// internal. For a call this agent makes by hand (setup's LoadMap).
const bridgeToNative = (Il2Cpp as unknown as {
    toFridaValue?: (v: unknown) => NativeFunctionArgumentValue;
}).toFridaValue;

// A game function called by hand, its argument list built at run time.
type ByHandCall = (...args: NativeFunctionArgumentValue[]) => NativeFunctionReturnValue;

function bridgeValue(v: unknown): NativeFunctionArgumentValue {
    if (typeof bridgeToNative !== "function") {
        throw new Error("frida-il2cpp-bridge has no Il2Cpp.toFridaValue to marshal a by-hand call");
    }
    return bridgeToNative(v);
}

// The list's items, through the list's own count and indexer.
function items(list: Il2Cpp.Object): Il2Cpp.Object[] {
    const out: Il2Cpp.Object[] = [];
    if (isNothing(list)) {
        return out;
    }
    const count = list.method<number>("get_Count").invoke();
    for (let i = 0; i < count; i++) {
        out.push(list.method<Il2Cpp.Object>("get_Item", 1).invoke(i));
    }
    return out;
}

function config(): Il2Cpp.Object {
    return rewired("Rewired.ReInput").method<Il2Cpp.Object>("get_configuration").invoke();
}

function controllers(): Il2Cpp.Object {
    return rewired("Rewired.ReInput").method<Il2Cpp.Object>("get_controllers").invoke();
}

function ignoresUnfocused(cfg: Il2Cpp.Object): boolean {
    return cfg.method<boolean>("get_ignoreInputWhenAppNotInFocus").invoke();
}

function setIgnoresUnfocused(cfg: Il2Cpp.Object, value: boolean): void {
    cfg.method("set_ignoreInputWhenAppNotInFocus", 1).invoke(value);
}

// The two Rewired devices a run switches off, each a Rewired.Controller whose
// enabled flag says whether it reads input at all, for every player.
type Device = "keyboard" | "mouse";

function device(which: Device): Il2Cpp.Object {
    const ctl = controllers();
    const d = which === "keyboard"
        ? ctl.method<Il2Cpp.Object>("get_Keyboard").invoke()
        : ctl.method<Il2Cpp.Object>("get_Mouse").invoke();
    if (isNothing(d)) {
        throw new Error(`ReInput.controllers has no ${which} device`);
    }
    return d;
}

function deviceOn(which: Device): boolean {
    return device(which).method<boolean>("get_enabled").invoke();
}

function setDeviceOn(which: Device, value: boolean): void {
    device(which).method("set_enabled", 1).invoke(value);
}

function customType(): Il2Cpp.Field.Type {
    return rewired("Rewired.ControllerType").field("Custom").value;
}

function frameCount(): number {
    return unity("UnityEngine.Time").method<number>("get_frameCount").invoke();
}

// Every Rewired player, the System player included: it holds its own
// keyboard map.
function allPlayers(): PlayerRecord[] {
    const helper = rewired("Rewired.ReInput").method<Il2Cpp.Object>("get_players").invoke();
    const list = helper.method<Il2Cpp.Object>("get_AllPlayers").invoke();
    return items(list).map((p) => {
        const pc = p.field<Il2Cpp.Object>("controllers").value;
        return {
            id: p.method<number>("get_id").invoke(),
            name: text(p.method<Il2Cpp.String>("get_name").invoke()),
            player: p,
            helper: pc,
            hasKeyboard: pc.method<boolean>("get_hasKeyboard").invoke(),
            hasMouse: pc.method<boolean>("get_hasMouse").invoke(),
        };
    });
}

function setFlags(pc: Il2Cpp.Object, keyboard: boolean, mouse: boolean): void {
    pc.method("set_hasKeyboard", 1).invoke(keyboard);
    pc.method("set_hasMouse", 1).invoke(mouse);
}

function playerLine(p: PlayerRecord) {
    return { id: p.id, name: p.name, has_keyboard: p.hasKeyboard, has_mouse: p.hasMouse };
}

function findTagged(tag: string): Il2Cpp.Object | null {
    const found = controllers().method<Il2Cpp.Object>("GetFirstCustomControllerWithTag", 1)
        .invoke(Il2Cpp.string(tag));
    return isNothing(found) ? null : found;
}

function kitId(kit: Il2Cpp.Object): number {
    return kit.field<number>("id").value;
}

function createController(definitionId: number, tag: string): Il2Cpp.Object {
    const made = controllers().method<Il2Cpp.Object>("CreateCustomController", 2)
        .overload("System.Int32", "System.String").invoke(definitionId, Il2Cpp.string(tag));
    if (isNothing(made)) {
        throw new Error(`CreateCustomController(${definitionId}, "${tag}") returned nothing`);
    }
    return made;
}

// One teardown check for one controller: destroys it and says whether the
// game agreed. The id is read first, so the line can name it afterwards.
function destroyController(what: string, controller: Il2Cpp.Object): Check {
    const id = kitId(controller);
    const done = controllers().method<boolean>("DestroyCustomController", 1).invoke(controller);
    return { what, ok: done, detail: `controller ${id}: DestroyCustomController returned ${done}` };
}

// Records each placeholder that exists, by its tag, and, when `make` is true,
// creates the ones that do not. Each goes in the record the moment it is
// known, so a teardown after a create that threw destroys the ones before it.
// A tag already recorded is left alone. Returns how many it created.
function recordPlaceholders(rec: Recorded, definitionId: number, make: boolean): number {
    let created = 0;
    for (const tag of rec.placeholderTags) {
        if (rec.placeholders.some((p) => p.tag === tag)) {
            continue;
        }
        let controller = findTagged(tag);
        if (controller === null && make) {
            controller = createController(definitionId, tag);
            created += 1;
        }
        if (controller !== null) {
            rec.placeholders.push({ tag, controller });
        }
    }
    return created;
}

// The kit's maps on the player, each with every binding it holds.
function readMaps(pc: Il2Cpp.Object, kit: Il2Cpp.Object) {
    const maps = pc.field<Il2Cpp.Object>("maps").value;
    const list = maps.method<Il2Cpp.Object>("GetMaps", 2)
        .overload("Rewired.ControllerType", "System.Int32").invoke(customType(), kitId(kit));
    return items(list).map((m) => ({
        category_id: m.method<number>("get_categoryId").invoke(),
        layout_id: m.method<number>("get_layoutId").invoke(),
        enabled: m.method<boolean>("get_enabled").invoke(),
        bindings: items(m.method<Il2Cpp.Object>("get_AllMaps").invoke()).map((a) => ({
            element_id: a.method<number>("get_elementIdentifierId").invoke(),
            element_name: text(a.method<Il2Cpp.String>("get_elementIdentifierName").invoke()),
            element_type: enumNumber(a.method("get_elementType").invoke()),
            action_id: a.method<number>("get_actionId").invoke(),
            action_name: text(a.method<Il2Cpp.String>("get_actionDescriptiveName").invoke()),
            axis_contribution: enumNumber(a.method("get_axisContribution").invoke()),
        })),
    }));
}

function needString(m: Message, key: string): string {
    const v = m[key];
    if (typeof v !== "string" || v === "") {
        throw new Error(`${m.type} needs a text "${key}"`);
    }
    return v;
}

function stringList(m: Message, key: string): string[] {
    const v = m[key];
    if (!Array.isArray(v) || !v.every((s) => typeof s === "string" && s !== "")) {
        throw new Error(`${m.type} needs a list of texts "${key}"`);
    }
    return v;
}

function needInteger(m: Message, key: string): number {
    const v = m[key];
    if (typeof v !== "number" || !Number.isInteger(v) || v < 1) {
        throw new Error(`${m.type} needs a whole number "${key}" of 1 or more`);
    }
    return v;
}

// Runs on the main thread. Records the switch, Rewired's keyboard and mouse
// devices and every player's keyboard and mouse flags before it changes
// anything, then turns the switch off, switches both devices off, takes the
// keyboard and mouse from every player, finds the kit or builds the
// placeholders and then the kit, gives the kit to the named player and loads
// the one map the message names by category and layout, enabled. A kit found
// by its tag gets no new placeholders: its id is already set. It loads that
// map by name rather than the player's default maps, because the System
// player lists no default map for a custom controller
// (docs/clone-hero/07-internals.md, IN-76). The record goes in `recorded`
// before the first change, and each placeholder, the kit and its player go in
// it as soon as each is known, so a teardown after a setup that threw
// part-way still puts everything back.
function setup(m: Message) {
    const tag = needString(m, "tag");
    const placeholderTags = stringList(m, "placeholder_tags");
    const definitionId = needInteger(m, "definition_id");
    const playerName = needString(m, "player");
    const categoryId = needInteger(m, "category_id");
    const layoutId = needInteger(m, "layout_id");
    const cfg = config();
    const players = allPlayers();
    // A second setup keeps the first one's record: by then the flags read are
    // the ones the first setup set, not the user's.
    const first = recorded === null;
    if (recorded === null) {
        recorded = { tag, placeholderTags, switchWas: ignoresUnfocused(cfg),
                     keyboardWas: deviceOn("keyboard"), mouseWas: deviceOn("mouse"), players,
                     placeholders: [], kit: null, target: null };
    }
    const rec = recorded;
    setIgnoresUnfocused(cfg, false);
    setDeviceOn("keyboard", false);
    setDeviceOn("mouse", false);
    for (const p of players) {
        setFlags(p.helper, false, false);
    }
    let kit = findTagged(tag);
    const created = kit === null;
    const placeholdersCreated = recordPlaceholders(rec, definitionId, created);
    if (kit === null) {
        kit = createController(definitionId, tag);
    }
    rec.kit = kit;
    const target = players.find((p) => p.name === playerName);
    if (target === undefined) {
        throw new Error(`no player named ${playerName}; players: ${players.map((p) => p.name).join(", ")}`);
    }
    rec.target = target;
    target.helper.method("AddController", 2)
        .overload("Rewired.Controller", "System.Boolean").invoke(kit, true);
    // startEnabled is true: kit_tools.py's checks count enabled maps only.
    const maps = target.helper.field<Il2Cpp.Object>("maps").value;
    const loadMap = maps.method("LoadMap", 5)
        .overload("Rewired.ControllerType", "System.Int32", "System.Int32", "System.Int32",
                  "System.Boolean");
    // Called by hand, not through .invoke: this call must also pass the
    // method's own pointer as its last argument, which .invoke leaves out
    // here and the game's stub then overwrites past the arguments
    // (docs/clone-hero/07-internals.md, IN-76). Everything else is the
    // bridge's own: its types for each parameter and its marshalling of
    // each value.
    if (loadMap.virtualAddress.isNull()) {
        throw new Error("LoadMap has no code address");
    }
    const loadMapTypes: NativeFunctionArgumentType[] =
        ["pointer", ...loadMap.parameters.map((p) => p.type.fridaAlias), "pointer"];
    const loadMapCall = new NativeFunction(loadMap.virtualAddress,
        loadMap.returnType.fridaAlias as NativeFunctionReturnType, loadMapTypes) as unknown as ByHandCall;
    loadMapCall(maps.handle,
        ...[customType(), kitId(kit), categoryId, layoutId, true].map(bridgeValue),
        loadMap.handle);
    return {
        first_setup: first,
        switch_was: rec.switchWas,
        switch_now: ignoresUnfocused(cfg),
        keyboard_was: rec.keyboardWas,
        keyboard_now: deviceOn("keyboard"),
        mouse_was: rec.mouseWas,
        mouse_now: deviceOn("mouse"),
        players_was: rec.players.map(playerLine),
        players_now: allPlayers().map(playerLine),
        placeholders: rec.placeholders.map((p) => ({ id: kitId(p.controller), tag: p.tag })),
        placeholders_created: placeholdersCreated,
        kit: { id: kitId(kit), tag, created, player: playerName },
        maps: readMaps(target.helper, kit),
    };
}

function elementsOf(m: Message): Element[] {
    const raw = m.elements;
    if (!Array.isArray(raw) || raw.length === 0) {
        throw new Error("press needs a list of elements");
    }
    return raw.map((e) => {
        const ok = typeof e === "object" && e !== null && Number.isInteger(e.id)
            && (typeof e.value === "number" || e.value === true);
        if (!ok) {
            throw new Error(`a press element is {id, value} with a number or true: ${JSON.stringify(e)}`);
        }
        return { id: e.id, value: e.value };
    });
}

// Writes each element's value, or its rest value when `release` is set.
function write(kit: Il2Cpp.Object, elements: Element[], release: boolean): void {
    for (const e of elements) {
        if (typeof e.value === "boolean") {
            kit.method("SetButtonValueById", 2).invoke(e.id, release ? false : e.value);
        } else {
            kit.method("SetAxisValueById", 2).invoke(e.id, release ? 0 : e.value);
        }
    }
}

function sleep(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

// The press's optional "read": the game action names to read back on the
// kit's player, or none.
function readNames(m: Message): string[] {
    const raw = m.read;
    if (raw === undefined || raw === null) {
        return [];
    }
    if (!Array.isArray(raw) || !raw.every((n) => typeof n === "string" && n !== "")) {
        throw new Error(`a press "read" is a list of action names: ${JSON.stringify(raw)}`);
    }
    return raw;
}

// What the player's GetButton reads for one action.
function button(player: Il2Cpp.Object, action: string): boolean {
    return player.method<boolean>("GetButton", 1).overload("System.String")
        .invoke(Il2Cpp.string(action));
}

// What one player's GetAnyButton reads: the watch's read, and the frame
// probe's.
function anyButton(player: Il2Cpp.Object): boolean {
    return player.method<boolean>("GetAnyButton", 0).invoke();
}

// Reads each action named in `names` on `player` at `frame`, and adds a line
// to `seen` for every action whose value differs from its last read.
function readBack(player: Il2Cpp.Object, names: string[], frame: number,
                  last: Map<string, boolean>, seen: object[]): void {
    for (const action of names) {
        const value = button(player, action);
        if (last.get(action) !== value) {
            last.set(action, value);
            seen.push({ frame, action, value });
        }
    }
}

// One group of a press: its elements, and the frame and time it went down
// and came up (-1 until then).
interface Group {
    elements: Element[];
    downFrame: number;
    downMs: number;
    upFrame: number;
    upMs: number;
}

// The press's groups: its own elements first, then each group of its
// optional "then" list, each {elements}. A group carries no time of its own:
// the play route's rule (a later group goes down the frame after the group
// it follows, press_schedule.ts) holds here too.
function groupsOf(m: Message): Group[] {
    const fresh = (elements: Element[]): Group =>
        ({ elements, downFrame: -1, downMs: -1, upFrame: -1, upMs: -1 });
    const groups = [fresh(elementsOf(m))];
    const raw = m.then;
    if (raw === undefined || raw === null) {
        return groups;
    }
    if (!Array.isArray(raw)) {
        throw new Error(`a press "then" is a list of {elements}: ${JSON.stringify(raw)}`);
    }
    for (const g of raw) {
        if (typeof g !== "object" || g === null || g.after_ms !== undefined) {
            throw new Error(`a press "then" group is {elements}, with no "after_ms": `
                + JSON.stringify(g));
        }
        groups.push(fresh(elementsOf({ type: m.type, id: m.id, elements: g.elements })));
    }
    return groups;
}

// Sets the first group's elements, then on each tick puts down the next
// later group once the frame count has moved past the down frame of the
// group before it, so each lands one frame after the one before. Each group is held until the
// frame count has moved on by the hold from its own down, writing the values
// again each tick when `rewrite` is true. A group whose hold has passed comes
// up in that tick while another group is still to go down or held; the last
// ones come up in the release, which clears every element and runs even when
// the wait fails. Each step runs on the main thread when `on_main` is true,
// else on Frida's own thread. With a "read" list, each step also reads those
// actions on the kit's player, and the reply's `seen` holds each change. A
// press with later groups replies with each group's frames and times in
// `groups`.
async function press(m: Message) {
    const rec = recorded;
    if (rec === null || rec.kit === null) {
        throw new Error("no kit: send setup first");
    }
    const kit = rec.kit;
    const groups = groupsOf(m);
    const first = groups[0];
    const hold = needInteger(m, "hold_frames");
    const capS = m.cap_s;
    if (typeof capS !== "number" || !(capS > 0)) {
        throw new Error('press needs a "cap_s" above 0');
    }
    const rewrite = m.rewrite === true;
    const onMain = m.on_main === true;
    const where: "main" | undefined = onMain ? "main" : undefined;
    const names = readNames(m);
    const reader = names.length === 0 ? null : (rec.target === null ? null : rec.target.player);
    if (names.length > 0 && reader === null) {
        throw new Error("press has a read list but setup recorded no player to read it on");
    }
    const last = new Map<string, boolean>();
    const seen: object[] = [];
    // One step's reads: none without a read list.
    const look = (frame: number) => {
        if (reader !== null) {
            readBack(reader, names, frame, last, seen);
        }
    };
    for (const g of groups) {
        for (const e of g.elements) {
            touched.set(e.id, typeof e.value === "boolean");
        }
    }
    const start = Date.now();
    let upFrame = -1;
    let failure: Error | null = null;
    // A group is held once it is down and the frame count has moved on by
    // the hold from its own down.
    const held = (g: Group, frame: number) => g.downFrame >= 0 && frame >= g.downFrame + hold;
    // The down write sits inside the try too: a write that fails half-way has
    // already put some elements down, and the release below clears them all.
    try {
        first.downFrame = await settled(() => {
            write(kit, first.elements, false);
            const f = frameCount();
            look(f);
            return f;
        }, where);
        first.downMs = Date.now();
        let frame = first.downFrame;
        while (!groups.every((g) => held(g, frame))) {
            if (Date.now() - start >= capS * 1000) {
                failure = new Error(`the hold of ${hold} frames did not pass within the cap of `
                    + `${capS} s: down at frame ${first.downFrame}, last frame seen ${frame}`);
                break;
            }
            await sleep(TICK_MS);
            frame = await settled(() => {
                const f = frameCount();
                const t = Date.now();
                for (let k = 0; k < groups.length; ++k) {
                    const g = groups[k];
                    // The first group is already down; a later one waits for
                    // a frame past the down frame of the group before it.
                    const before = k > 0 ? groups[k - 1] : null;
                    if (before !== null && g.downFrame < 0 && before.downFrame >= 0
                        && f > before.downFrame) {
                        write(kit, g.elements, false);
                        g.downFrame = f;
                        g.downMs = t;
                    } else if (rewrite && g.downFrame >= 0 && g.upFrame < 0) {
                        write(kit, g.elements, false);
                    }
                }
                if (!groups.every((g) => held(g, f))) {
                    for (const g of groups) {
                        if (g.upFrame < 0 && held(g, f)) {
                            write(kit, g.elements, true);
                            g.upFrame = f;
                            g.upMs = t;
                        }
                    }
                }
                look(f);
                return f;
            }, where);
        }
    } catch (e) {
        failure = e instanceof Error ? e : new Error(`${e}`);
    }
    // The release runs on every path: after the hold, after a failed wait and
    // at the cap. It clears every group, and its read is the last one the
    // reply carries.
    try {
        upFrame = await settled(() => {
            for (const g of groups) {
                write(kit, g.elements, true);
            }
            const f = frameCount();
            look(f);
            return f;
        }, where);
        const upMs = Date.now();
        for (const g of groups) {
            if (g.upFrame < 0) {
                g.upFrame = upFrame;
                g.upMs = upMs;
            }
        }
    } catch (e) {
        throw new Error(`${failure === null ? "" : `${failure.message}; and `}the release failed, `
            + `so a pad may still be down (send teardown): ${e}`);
    }
    if (failure !== null) {
        throw new Error(`${failure.message}; released at frame ${upFrame}`);
    }
    const reply: { [key: string]: unknown } = {
        down_frame: first.downFrame, up_frame: first.upFrame, down_ms: first.downMs,
        up_ms: first.upMs, hold_frames: hold, rewrite, on_main: onMain,
    };
    if (groups.length > 1) {
        reply.groups = groups.map((g) => ({
            down_frame: g.downFrame, up_frame: g.upFrame, down_ms: g.downMs, up_ms: g.upMs,
        }));
    }
    if (names.length > 0) {
        reply.read = names;
        reply.seen = seen;
    }
    return reply;
}

// Who a watch read was made by, and how it reads.
interface Reader {
    who: string;
    read: () => boolean;
}

// For `seconds`, reads GetAnyButton on every player (the System player
// included) and on ReInput's controllers each tick, on Frida's own thread.
// The reply counts the ticks and the true reads, and lists each read that
// turned true, up to WATCH_LIST_MAX, with its time and frame. A tick that
// ends past `cap_s` ends the watch with an error, so no reply comes after
// the caller stopped waiting.
async function watch(m: Message) {
    const seconds = m.seconds;
    if (typeof seconds !== "number" || !(seconds > 0)) {
        throw new Error('watch needs a "seconds" above 0');
    }
    const capS = m.cap_s;
    if (typeof capS !== "number" || !(capS > seconds)) {
        throw new Error('watch needs a "cap_s" above its "seconds"');
    }
    const readers = await Il2Cpp.perform(() => {
        const out: Reader[] = allPlayers().map((p) => ({
            who: p.name === null ? `player ${p.id}` : p.name,
            read: () => anyButton(p.player),
        }));
        const ctl = controllers();
        out.push({
            who: "controllers",
            read: () => ctl.method<boolean>("GetAnyButton", 0).invoke(),
        });
        return out;
    });
    const was = new Map<string, boolean>();
    const reads: object[] = [];
    let ticks = 0;
    let trueCount = 0;
    let firstFrame = -1;
    let lastFrame = -1;
    const start = Date.now();
    while (Date.now() - start < seconds * 1000) {
        const frame = await Il2Cpp.perform(() => {
            const f = frameCount();
            for (const r of readers) {
                const value = r.read();
                if (value) {
                    trueCount += 1;
                    if (was.get(r.who) !== true && reads.length < WATCH_LIST_MAX) {
                        reads.push({ ms: Date.now() - start, frame: f, who: r.who });
                    }
                }
                was.set(r.who, value);
            }
            return f;
        });
        ticks += 1;
        if (firstFrame < 0) {
            firstFrame = frame;
        }
        lastFrame = frame;
        if (Date.now() - start >= capS * 1000) {
            throw new Error(`the watch passed its cap of ${capS} s after ${ticks} ticks`);
        }
        await sleep(TICK_MS);
    }
    return { seconds, ticks, first_frame: firstFrame, last_frame: lastFrame,
             readers: readers.map((r) => r.who), true_count: trueCount, reads };
}

// Runs on the main thread. The kit is looked up by the tag setup recorded,
// else by the message's tag, else not at all.
function status(m: Message) {
    const tag = recorded !== null ? recorded.tag : (typeof m.tag === "string" ? m.tag : null);
    const kit = tag === null ? null : findTagged(tag);
    return {
        frame: frameCount(),
        focused: unity("UnityEngine.Application").method<boolean>("get_isFocused").invoke(),
        switch_now: ignoresUnfocused(config()),
        keyboard_now: deviceOn("keyboard"),
        mouse_now: deviceOn("mouse"),
        setup_recorded: recorded !== null,
        kit_tag: tag,
        kit_exists: tag === null ? null : kit !== null,
        kit_id: kit === null ? null : kitId(kit),
        players: allPlayers().map(playerLine),
    };
}

interface Check {
    what: string;
    ok: boolean;
    detail: string;
}

// One teardown step: a thrown error is a failed check, so the steps after it
// still run.
function step(checks: Check[], what: string, fn: () => string | Check): void {
    try {
        const r = fn();
        checks.push(typeof r === "string" ? { what, ok: true, detail: r } : r);
    } catch (e) {
        checks.push({ what, ok: false, detail: `${e}` });
    }
}

// The placeholders a teardown destroys: those setup recorded, and any other
// that a tag in `tags` finds, which is how a teardown with no setup record
// reaches them.
function placeholdersToDestroy(rec: Recorded | null, tags: string[]): Placeholder[] {
    const known = rec !== null ? rec.placeholders : [];
    const found: Placeholder[] = [];
    for (const tag of tags) {
        const controller = known.some((p) => p.tag === tag) ? null : findTagged(tag);
        if (controller !== null) {
            found.push({ tag, controller });
        }
    }
    return [...known, ...found];
}

// Runs on the main thread. Ends a running play and takes out the hook, if
// either is in, through unhook. Clears every element a press wrote, takes the kit
// from its player, destroys it and every placeholder, gives every player back
// the keyboard and mouse flags setup recorded, puts Rewired's keyboard and
// mouse devices and the switch back, and checks each. The check "kit gone"
// covers the kit's tag and every placeholder tag.
function teardown(m: Message) {
    const checks: Check[] = [];
    const rec = recorded;
    const tag = rec !== null ? rec.tag : (typeof m.tag === "string" ? m.tag : null);
    const placeholderTags = rec !== null ? rec.placeholderTags : stringList(m, "placeholder_tags");
    const kit = rec !== null && rec.kit !== null ? rec.kit : (tag === null ? null : findTagged(tag));
    // A play or a hook still in ends first, so no frame writes the kit while
    // it goes. Nothing in: no check line.
    if (play !== null || hook !== null) {
        step(checks, "hook out", () => {
            const p = play;
            if (p !== null) {
                applyStep(p, p.schedule.stop(Date.now()), null);
            }
            unhook(kit);
            return p !== null ? "play stopped, its hook out" : "hook out";
        });
    }
    if (kit !== null) {
        step(checks, "elements cleared", () => {
            // Only each value's kind matters here: a release writes the rest value.
            const elements = [...touched].map(([id, button]) => ({ id, value: button ? true : 0 }));
            write(kit, elements, true);
            touched.clear();
            return `${elements.length} cleared`;
        });
        if (rec !== null && rec.target !== null) {
            const target = rec.target;
            step(checks, "kit removed from player", () => {
                target.helper.method("RemoveController", 1)
                    .overload("Rewired.Controller").invoke(kit);
                return `removed from ${target.name}`;
            });
        }
        step(checks, "kit destroyed", () => destroyController("kit destroyed", kit));
    }
    for (const p of placeholdersToDestroy(rec, placeholderTags)) {
        const what = `${p.tag} destroyed`;
        step(checks, what, () => destroyController(what, p.controller));
    }
    step(checks, "kit gone", () => {
        if (tag === null) {
            return { what: "kit gone", ok: false, detail: "no tag recorded or sent; not looked up" };
        }
        const left: string[] = [];
        for (const t of [tag, ...placeholderTags]) {
            const still = findTagged(t);
            if (still !== null) {
                left.push(`controller ${kitId(still)} still tagged ${t}`);
            }
        }
        return left.length === 0 ? `no controller tagged ${tag} or a placeholder tag`
            : { what: "kit gone", ok: false, detail: left.join("; ") };
    });
    if (rec === null) {
        checks.push({ what: "setup record", ok: false,
                      detail: "setup did not run in this agent, so no flags, devices or switch were put back" });
        return { checks, all_ok: false };
    }
    for (const p of rec.players) {
        step(checks, `player ${p.id} flags`, () => {
            setFlags(p.helper, p.hasKeyboard, p.hasMouse);
            const kb = p.helper.method<boolean>("get_hasKeyboard").invoke();
            const mouse = p.helper.method<boolean>("get_hasMouse").invoke();
            const ok = kb === p.hasKeyboard && mouse === p.hasMouse;
            return { what: `player ${p.id} flags`, ok,
                     detail: `${p.name}: keyboard ${kb} (was ${p.hasKeyboard}), mouse ${mouse} (was ${p.hasMouse})` };
        });
    }
    const devices: [Device, string, boolean][] = [
        ["keyboard", "Keyboard", rec.keyboardWas], ["mouse", "Mouse", rec.mouseWas]];
    for (const [which, name, was] of devices) {
        step(checks, which, () => {
            setDeviceOn(which, was);
            const now = deviceOn(which);
            return { what: which, ok: now === was, detail: `${name}.enabled ${now} (was ${was})` };
        });
    }
    step(checks, "switch", () => {
        const cfg = config();
        setIgnoresUnfocused(cfg, rec.switchWas);
        const now = ignoresUnfocused(cfg);
        return { what: "switch", ok: now === rec.switchWas,
                 detail: `ignoreInputWhenAppNotInFocus ${now} (was ${rec.switchWas})` };
    });
    recorded = null;
    return { checks, all_ok: checks.every((c) => c.ok) };
}

type Outcome<T> = { ok: true; value: T } | { ok: false; error: unknown };

// Runs `fn` through Il2Cpp.perform and settles with its result or its throw.
// The throw is caught inside the scheduled callback itself: the bridge's
// main-thread schedule (Il2Cpp.Thread.schedule) resolves only when its
// callback returns, so a throw there would leave the promise unsettled and
// the agent listening no more.
function settled<T>(fn: () => T, where?: "main"): Promise<T> {
    return Il2Cpp.perform((): Outcome<T> => {
        try {
            return { ok: true, value: fn() };
        } catch (error) {
            return { ok: false, error };
        }
    }, where).then((r) => (r.ok ? r.value : Promise.reject(r.error)));
}

// A pointer as hex digits, with no prefix; kit_tools.py adds it.
function hex(p: NativePointer): string {
    return p.toString(16).replace(/^0x/, "");
}

// The details a native exception (a fault inside the game's own code) leaves
// on the error Frida throws for it: gumjs puts the exception's type, address,
// memory and CPU context on that error (_gum_quick_exception_details_new in
// frida-gum's bindings/gumjs/gumquickvalue.c). Null for any other error.
// kit_tools.py's fault_lines prints it.
function faultOf(e: unknown): object | null {
    if (!(e instanceof Error)) {
        return null;
    }
    const d = e as Error & Partial<ExceptionDetails>;
    if (typeof d.type !== "string" || !(d.address instanceof NativePointer)) {
        return null;
    }
    const module = Process.findModuleByAddress(d.address);
    const registers: { [name: string]: string } = {};
    if (d.context !== undefined) {
        const ctx = d.context as unknown as { [name: string]: unknown };
        for (let o = Object.getPrototypeOf(ctx); o !== null && o !== Object.prototype;
             o = Object.getPrototypeOf(o)) {
            for (const name of Object.getOwnPropertyNames(o)) {
                const v = name in registers ? null : ctx[name];
                if (v instanceof NativePointer) {
                    registers[name] = hex(v);
                }
            }
        }
    }
    return {
        type: d.type,
        address: hex(d.address),
        module: module === null ? null : module.name,
        offset: module === null ? null : hex(d.address.sub(module.base)),
        operation: d.memory === undefined ? null : d.memory.operation,
        memory_address: d.memory === undefined ? null : hex(d.memory.address),
        registers,
        stack: typeof e.stack === "string" ? e.stack.split("\n") : [],
    };
}

// ---- in-game timing --------------------------------------------------------
// The play messages and frame_probe (task IG2 of the in-game timing plan).
// press_schedule.ts decides every frame; this block reads the clock and the
// score, does the writes and sends the events. Every number a play runs by
// comes in its message: press_schedule.ts's header names each one, and
// kit_tools.py's contract block owns them. tests/test_frida_attach.py reads
// this block for numbers of its own.

// The song clock is a double in seconds; the scheduler takes ms.
const MS_PER_S = 1000;
const PLAY_EVENTS = "play_events";

interface Play {
    kit: Il2Cpp.Object;
    schedule: PressSchedule;
    clock: NativePointer;
    stallMs: number;
    lastFrameWall: number;
    timer: ReturnType<typeof setTimeout> | null;
}

let play: Play | null = null;
// The one hook on GameManager.Update, the play's or the frame probe's, and
// every element its owner may write, with whether it is a button, so unhook
// can lift them all. The frame probe alone also hooks the engine's frame
// update and subscribes a delegate to Rewired's InputSourceUpdateEvent
// (docs/clone-hero/07-internals.md, IN-94); unhook takes those out too.
let hook: InvocationListener | null = null;
let engineHook: InvocationListener | null = null;
let inputEvent: Il2Cpp.Object | null = null;
const hookPads = new Map<number, boolean>();

function needAddress(m: Message, key: string): NativePointer {
    const v = m[key];
    if (typeof v !== "number" || !Number.isInteger(v) || !(v > 0)) {
        throw new Error(`${m.type} needs an address "${key}"`);
    }
    return ptr(v);
}

function needAbove0(m: Message, key: string): number {
    const v = m[key];
    if (typeof v !== "number" || !(v > 0)) {
        throw new Error(`${m.type} needs a "${key}" above 0`);
    }
    return v;
}

function addPads(elements: Element[]): void {
    for (const e of elements) {
        hookPads.set(e.id, typeof e.value === "boolean");
        touched.set(e.id, typeof e.value === "boolean");
    }
}

// Places the hook. `each` runs as each GameManager.Update returns, on the
// game's main thread, with the GameManager the call ran on. When `engine` is
// given (the frame probe's), a second hook runs it as the engine's frame
// update is entered, with the engine and the time in seconds it was handed.
// Each hook is kept the moment it is in, so unhook takes out a half-placed pair.
function placeHook(each: (self: NativePointer) => void,
                   engine: ((self: NativePointer, timeS: number) => void) | null = null): void {
    if (hook !== null || engineHook !== null) {
        throw new Error(`a hook on ${GAME_MANAGER}.Update is already in`);
    }
    const update = Il2Cpp.domain.assembly(GAME).image.class(GAME_MANAGER).method("Update", 0);
    if (update.virtualAddress.isNull()) {
        throw new Error(`${GAME_MANAGER}.Update has no code address`);
    }
    const targets: [NativePointer, ScriptInvocationListenerCallbacks,
                    (l: InvocationListener) => void][] = [[update.virtualAddress, {
        onEnter(args) {
            // An instance method gets its object first.
            this.self = args[0];
        },
        onLeave() {
            each(this.self as NativePointer);
        },
    }, (l) => { hook = l; }]];
    if (engine !== null) {
        const frameUpdate = Il2Cpp.domain.assembly(STRIKE).image.class(ENGINE_BASE)
            .method("ʵʺʴʻˀʺˀˁʶʻʷ");
        if (frameUpdate.virtualAddress.isNull()) {
            throw new Error("the engine's frame update has no code address");
        }
        targets.push([frameUpdate.virtualAddress, {
            onEnter(args) {
                // The engine comes first. The time is a double, and the
                // Windows x64 convention passes a double second argument in
                // xmm1, not in the integer register args[1] reads.
                const ctx = this.context as X64CpuContext;
                engine(args[0], new Float64Array(ctx.xmm1)[0]);
            },
        }, (l) => { engineHook = l; }]);
    }
    for (const [at, callbacks, keep] of targets) {
        keep(Interceptor.attach(at, callbacks));
    }
}

// Subscribes `each` to Rewired's InputSourceUpdateEvent through ReInput's
// public add method, the one the game's MIDI kit calls (IN-94). Kept the
// moment it is in, so unhook removes it.
function subscribeInputEvent(each: () => void): void {
    if (inputEvent !== null) {
        throw new Error("a delegate on InputSourceUpdateEvent is already in");
    }
    const action = Il2Cpp.delegate(Il2Cpp.corlib.class("System.Action"), () => { each(); });
    rewired("Rewired.ReInput").method("add_InputSourceUpdateEvent").invoke(action);
    inputEvent = action;
}

// The one way the hooks come out: it detaches each hook and removes the
// input event's delegate, then lifts every element its owner may have
// written. Every play stop, the frame probe's end and teardown come here.
// The delegate stays recorded when its removal fails, so the probe's reply
// says it is still in.
function unhook(kit: Il2Cpp.Object | null): void {
    const h = hook;
    const e = engineHook;
    hook = null;
    engineHook = null;
    try {
        try {
            if (h !== null) {
                h.detach();
            }
        } finally {
            try {
                if (e !== null) {
                    e.detach();
                }
            } finally {
                const d = inputEvent;
                if (d !== null) {
                    rewired("Rewired.ReInput").method("remove_InputSourceUpdateEvent").invoke(d);
                    inputEvent = null;
                }
            }
        }
    } finally {
        const pads = [...hookPads].map(([id, isButton]) => ({ id, value: isButton ? true : 0 }));
        hookPads.clear();
        if (kit !== null && pads.length !== 0) {
            write(kit, pads, true);
        }
    }
}

// Starts the play the message gives and returns at once: the hook plays it
// on the game's frames, and its events reach Python later, with no id.
function startPlay(m: Message) {
    if (play !== null) {
        throw new Error("a play is already running: send stop_play first");
    }
    const rec = recorded;
    if (rec === null || rec.kit === null) {
        throw new Error("no kit: send setup first");
    }
    const kit = rec.kit;
    const clock = needAddress(m, "clock_addr");
    const score = needAddress(m, "score_addr");
    const stallMs = needAbove0(m, "stall_ms");
    const schedule = new PressSchedule(m as unknown as PlayConfig, () => score.readU32());
    const p: Play = { kit, schedule, clock, stallMs, lastFrameWall: Date.now(), timer: null };
    for (const s of schedule.slots.values()) {
        addPads(s.entry.elements);
    }
    placeHook(() => onPlayFrame(p));
    play = p;
    armStallTimer(p, stallMs);
    return { playing: true, entries: schedule.slots.size };
}

// One frame of the play: one clock read, one step.
function onPlayFrame(p: Play): void {
    if (play !== p) {
        return;
    }
    let s: Step;
    let cause: string | null = null;
    try {
        const wall = Date.now();
        p.lastFrameWall = wall;
        s = p.schedule.step(frameCount(), p.clock.readDouble() * MS_PER_S, wall);
    } catch (e) {
        s = p.schedule.stop(Date.now());
        cause = `the frame step failed: ${e}`;
    }
    applyStep(p, s, cause);
}

// Does one Step's releases and writes, then sends its events as one batch. A
// Step that ends the play, and a write that fails, go to endPlay.
function applyStep(p: Play, s: Step, cause: string | null, reason: string | null = null): void {
    let why = cause;
    let events = s.events;
    try {
        for (const r of s.release) {
            write(p.kit, r.elements, true);
        }
        for (const w of s.write) {
            write(p.kit, w.elements, false);
        }
    } catch (e) {
        if (p.schedule.stopReason === null) {
            events = [...events, ...p.schedule.stop(Date.now()).events];
        }
        why = `${why === null ? "" : `${why}; and `}a kit write failed: ${e}`;
    }
    if (p.schedule.stopReason !== null) {
        endPlay(p, events, why, reason);
    } else if (events.length !== 0) {
        send({ type: PLAY_EVENTS, events });
    }
}

// Ends the play: the hook comes out and every pad is lifted, through unhook,
// then the last events go, the stop among them. The stop event also carries
// `cause`, null when the scheduler or stop_play stopped the play, else what
// stopped it from outside the scheduler, and `lift_error`, null unless
// unhook failed. `reason`, when given, replaces the stop event's reason: the
// stall timer's stop is the scheduler's "frozen", not its "stopped".
function endPlay(p: Play, events: object[], cause: string | null, reason: string | null): void {
    play = null;
    if (p.timer !== null) {
        clearTimeout(p.timer);
        p.timer = null;
    }
    let liftError: string | null = null;
    try {
        unhook(p.kit);
    } catch (e) {
        liftError = `${e}`;
    }
    const out = events.map((e) => {
        const ev = e as { [key: string]: unknown };
        return ev.kind === "stop"
            ? { ...ev, ...(reason === null ? {} : { reason }), cause, lift_error: liftError } : ev;
    });
    send({ type: PLAY_EVENTS, events: out });
}

// The scheduler's cap and frozen checks run only inside step(), so a game
// that stops calling Update would never stop the play. This timer, on
// Frida's own thread, stops it once no frame has come for the message's
// stall_ms.
function armStallTimer(p: Play, waitMs: number): void {
    p.timer = setTimeout(() => {
        p.timer = null;
        settled(() => {
            if (play !== p) {
                return;
            }
            const now = Date.now();
            const quiet = now - p.lastFrameWall;
            if (quiet >= p.stallMs) {
                applyStep(p, p.schedule.stop(now), `no frame came for ${quiet} ms`, "frozen");
            } else {
                armStallTimer(p, p.stallMs - quiet);
            }
        }).catch((e) => {
            send({ type: PLAY_EVENTS, events: [], error: `the stall check failed: ${e}` });
        });
    }, waitMs);
}

function retimePlay(m: Message) {
    const p = play;
    if (p === null) {
        throw new Error("retime: no play is running");
    }
    const done = p.schedule.retime(m.read_after as number, m.entries);
    for (const s of p.schedule.slots.values()) {
        addPads(s.entry.elements);
    }
    return done;
}

function stopPlay() {
    const p = play;
    if (p === null) {
        return { stopped: false };
    }
    applyStep(p, p.schedule.stop(Date.now()), null);
    return { stopped: true };
}

// Where the frame probe writes its one press: as GameManager.Update returns,
// or inside Rewired's InputSourceUpdateEvent. kit_tools.py owns the words.
const WRITE_AT_UPDATE = "update";
const WRITE_AT_INPUT_EVENT = "input_event";

// For the message's seconds, logs three marks in every frame, each with a
// running number `seq` shared by all three, so the order they ran in shows:
// every GameManager.Update as it returns (a row of `frames`: its frame, the
// engine clock and GameManager.songTime as read, both in seconds, and the
// wall time), every InputSourceUpdateEvent (a row of `events`: its frame,
// the engine clock and the songTime of the last GameManager seen, as read
// when the event fired), and every entry to the engine's frame update (a
// row of `engine`: its frame, the engine, the time in seconds it was handed,
// and the read list read back at that moment). Each mark also says which
// thread it ran on. The message's required "write_at" says which mark writes
// the press: it puts the message's elements down on the first frame and
// lifts them hold_frames frames later, from that mark alone, and the mark's
// `wrote` says which write it made. After any write it reads the kit's
// player back at every Update mark: GetAnyButton, and each action of an
// optional "read" list by name, as press reads them. The hooks go in and come
// out on the game's main thread, and the reply keeps only the marks of the
// frames between those two, the frames the hooks saw whole. Every hook and
// the delegate come out through unhook.
async function frameProbe(m: Message) {
    const seconds = needAbove0(m, "seconds");
    const capS = m.cap_s;
    if (typeof capS !== "number" || !(capS > seconds)) {
        throw new Error('frame_probe needs a "cap_s" above its "seconds"');
    }
    const writeAt = m.write_at;
    if (writeAt !== WRITE_AT_UPDATE && writeAt !== WRITE_AT_INPUT_EVENT) {
        throw new Error(`frame_probe needs a "write_at" of "${WRITE_AT_UPDATE}" or `
            + `"${WRITE_AT_INPUT_EVENT}", not ${JSON.stringify(writeAt)}`);
    }
    const clock = needAddress(m, "clock_addr");
    const elements = elementsOf(m);
    const hold = needInteger(m, "hold_frames");
    const names = readNames(m);
    const rec = recorded;
    if (rec === null || rec.kit === null || rec.target === null) {
        throw new Error("no kit on a player: send setup first");
    }
    if (play !== null) {
        throw new Error("a play is running: send stop_play first");
    }
    const kit = rec.kit;
    const reader = rec.target.player;
    let frames: { [key: string]: unknown }[] = [];
    let events: { [key: string]: unknown }[] = [];
    let engines: { [key: string]: unknown }[] = [];
    let seq = 0;
    let installFrame: number | null = null;
    let endFrame: number | null = null;
    let downFrame: number | null = null;
    let upFrame: number | null = null;
    let failure: string | null = null;
    let unhookError: string | null = null;
    const managers = new Map<string, Il2Cpp.Object>();
    let lastManager: Il2Cpp.Object | null = null;
    // The frame of a mark, or null when the hooks did not see that frame
    // whole: the frame they went in on, or one after they came out.
    const covered = (): number | null => {
        const f = frameCount();
        return installFrame === null || f <= installFrame || endFrame !== null ? null : f;
    };
    // The press's down or up write, made by whichever mark write_at names.
    const pressStep = (f: number): string | null => {
        if (downFrame === null) {
            write(kit, elements, false);
            downFrame = f;
            return "down";
        }
        if (upFrame === null && f >= downFrame + hold) {
            write(kit, elements, true);
            upFrame = f;
            return "up";
        }
        return null;
    };
    const fail = (where: string, e: unknown) => {
        if (failure === null) {
            failure = `at the ${where} mark, after ${frames.length} frames: ${e}`;
        }
    };
    const onFrame = (self: NativePointer) => {
        if (failure !== null) {
            return;
        }
        try {
            const wall = Date.now();
            const f = covered();
            if (f === null) {
                return;
            }
            seq++;
            let gm = managers.get(self.toString());
            if (gm === undefined) {
                gm = new Il2Cpp.Object(self);
                managers.set(self.toString(), gm);
            }
            lastManager = gm;
            const row: { [key: string]: unknown } = {
                seq, frame: f, engine_clock_s: clock.readDouble(),
                song_time_s: gm.field<number>("songTime").value, wall_ms: wall,
                thread: Process.getCurrentThreadId(), wrote: null,
            };
            if (writeAt === WRITE_AT_UPDATE) {
                row.wrote = pressStep(f);
            }
            row.any_button = anyButton(reader);
            if (names.length !== 0) {
                row.read = names.map((a) => button(reader, a));
            }
            frames.push(row);
        } catch (e) {
            fail(WRITE_AT_UPDATE, e);
        }
    };
    const onEvent = () => {
        if (failure !== null) {
            return;
        }
        try {
            const wall = Date.now();
            const f = covered();
            if (f === null) {
                return;
            }
            seq++;
            const gm = lastManager;
            const row: { [key: string]: unknown } = {
                seq, frame: f, engine_clock_s: clock.readDouble(),
                song_time_s: gm === null ? null : gm.field<number>("songTime").value,
                wall_ms: wall, thread: Process.getCurrentThreadId(), wrote: null,
            };
            if (writeAt === WRITE_AT_INPUT_EVENT) {
                row.wrote = pressStep(f);
            }
            events.push(row);
        } catch (e) {
            fail(WRITE_AT_INPUT_EVENT, e);
        }
    };
    const onEngine = (self: NativePointer, timeS: number) => {
        if (failure !== null) {
            return;
        }
        try {
            const wall = Date.now();
            const f = covered();
            if (f === null) {
                return;
            }
            seq++;
            engines.push({
                seq, frame: f, engine: hex(self), time_s: timeS, wall_ms: wall,
                thread: Process.getCurrentThreadId(),
                read: names.map((a) => button(reader, a)),
            });
        } catch (e) {
            fail("engine", e);
        }
    };
    addPads(elements);
    const start = Date.now();
    try {
        await settled(() => {
            installFrame = frameCount();
            placeHook(onFrame, onEngine);
            subscribeInputEvent(onEvent);
        }, "main");
        await sleep(seconds * MS_PER_S);
    } finally {
        try {
            await settled(() => {
                endFrame = frameCount();
                unhook(kit);
            }, "main");
        } catch (e) {
            unhookError = `${e}`;
        }
    }
    // A mark of the frame the hooks came out on saw only part of it.
    const whole = (row: { [key: string]: unknown }) => row.frame !== endFrame;
    frames = frames.filter(whole);
    events = events.filter(whole);
    engines = engines.filter(whole);
    const hooksOut = { update: hook === null, engine: engineHook === null,
                       input_event: inputEvent === null };
    return {
        seconds, wall_ms: Date.now() - start, write_at: writeAt, hold_frames: hold,
        down_frame: downFrame, up_frame: upFrame, read: names, failure,
        unhook_error: unhookError,
        hook_out: hooksOut.update && hooksOut.engine && hooksOut.input_event,
        hooks_out: hooksOut, install_frame: installFrame, end_frame: endFrame,
        managers: managers.size, frames, events, engine: engines,
    };
}

// ---- end of in-game timing -------------------------------------------------

// The first message LiveAgent.start posts is a read; the kit answers it with
// its status.
function answer(m: Message): Promise<object> {
    switch (m.type) {
        case "read":
        case "status":
            return settled(() => status(m), "main");
        case "setup":
            return settled(() => setup(m), "main");
        case "teardown":
            return settled(() => teardown(m), "main");
        case "press":
            return press(m);
        case "watch":
            return watch(m);
        // play replies once its hook is in, never at the schedule's end.
        case "play":
            return settled(() => startPlay(m));
        case "retime":
            return settled(() => retimePlay(m));
        case "stop_play":
            return settled(() => stopPlay());
        case "frame_probe":
            return frameProbe(m);
        default:
            return Promise.reject(new Error(`unknown message type ${m.type}`));
    }
}

// recv hands over one message per registration, so the handler registers
// itself again once its reply is sent. Messages that arrive meanwhile wait in
// Frida's queue and are answered in order.
function listen() {
    recv((message: Message) => {
        const id = message.id;
        let reply: Promise<object>;
        try {
            reply = answer(message);
        } catch (e) {
            reply = Promise.reject(e);
        }
        reply.then((r) => {
            send({ ...r, id });
        }, (e) => {
            const fault = faultOf(e);
            send(fault === null ? { error: `${message.type} failed: ${e}`, id }
                : { error: `${message.type} failed: ${e}`, fault, id });
        }).finally(listen);
    });
}

listen();
