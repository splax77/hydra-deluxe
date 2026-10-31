// The kit agent: a virtual drum kit inside the game. It builds one Rewired
// custom controller from the game's own drum definition, hands it to one
// player with the game's default map, presses its pads and buttons on
// request, and takes it away again. Nothing it does touches the keyboard or
// the window, so the game can stay minimized and unfocused.
//
// The plan is docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md
// ("One agent for the kit"). The user allowed method calls and field sets
// where a feature needs them (D272 rulings 2 and 5).
//
// Every game method it calls is a literal name in ALLOWED_CALLS, and every
// name there is listed with its dump line in tools/ch_mcp/kit_methods.json;
// tests/test_frida_attach.py checks both. It reads fields by name.
//
// It holds none of the kit's numbers. The definition id, the tag, the player,
// the element ids and values, the hold in frames, the rewrite flag, the
// thread and the press cap all come in the messages; tools/ch_mcp/kit_tools.py
// owns them and checks the map this agent reads back.
//
// frida_attach.LiveAgent posts each message with an id and takes only the
// reply carrying the same id. Every message gets exactly one reply: its
// answer, or an error. Messages are answered one at a time, in order.

import "frida-il2cpp-bridge";

// The game methods this agent may call, by name. The test above reads this
// list from this file's text, so it stays one literal list.
export const ALLOWED_CALLS = [
    "get_configuration", "get_controllers", "get_players",
    "get_ignoreInputWhenAppNotInFocus", "set_ignoreInputWhenAppNotInFocus",
    "GetFirstCustomControllerWithTag", "CreateCustomController", "DestroyCustomController",
    "get_AllPlayers", "get_id", "get_name",
    "get_hasKeyboard", "set_hasKeyboard", "get_hasMouse", "set_hasMouse",
    "AddController", "RemoveController",
    "LoadDefaultMaps", "GetMaps",
    "get_categoryId", "get_layoutId", "get_enabled", "get_AllMaps",
    "get_actionId", "get_actionDescriptiveName", "get_elementIdentifierId",
    "get_elementIdentifierName", "get_elementType", "get_axisContribution",
    "SetAxisValueById", "SetButtonValueById",
    "get_Count", "get_Item",
    "get_frameCount", "get_isFocused",
    "GetButton", "GetAnyButton",
];

// The images and classes, as the Il2CppDumper dump of build 6142 names them;
// kit_methods.json gives each one's dump line.
const REWIRED = "Rewired_Core";
const UNITY_CORE = "UnityEngine.CoreModule";

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
    switchWas: boolean;
    players: PlayerRecord[];
    kit: Il2Cpp.Object | null;
    target: PlayerRecord | null;
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

function findKit(tag: string): Il2Cpp.Object | null {
    const kit = controllers().method<Il2Cpp.Object>("GetFirstCustomControllerWithTag", 1)
        .invoke(Il2Cpp.string(tag));
    return isNothing(kit) ? null : kit;
}

function kitId(kit: Il2Cpp.Object): number {
    return kit.field<number>("id").value;
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

function needInteger(m: Message, key: string): number {
    const v = m[key];
    if (typeof v !== "number" || !Number.isInteger(v) || v < 1) {
        throw new Error(`${m.type} needs a whole number "${key}" of 1 or more`);
    }
    return v;
}

// Runs on the main thread. Records the switch and every player's keyboard and
// mouse flags before it changes anything, then turns the switch off, takes
// the keyboard and mouse from every player, finds or builds the kit, gives it
// to the named player and loads the default maps.
function setup(m: Message) {
    const tag = needString(m, "tag");
    const definitionId = needInteger(m, "definition_id");
    const playerName = needString(m, "player");
    const cfg = config();
    const players = allPlayers();
    // A second setup keeps the first one's record: by then the flags read are
    // the ones the first setup set, not the user's.
    const first = recorded === null;
    if (recorded === null) {
        recorded = { tag, switchWas: ignoresUnfocused(cfg), players, kit: null, target: null };
    }
    const rec = recorded;
    setIgnoresUnfocused(cfg, false);
    for (const p of players) {
        setFlags(p.helper, false, false);
    }
    let kit = findKit(tag);
    const created = kit === null;
    if (kit === null) {
        kit = controllers().method<Il2Cpp.Object>("CreateCustomController", 2)
            .overload("System.Int32", "System.String").invoke(definitionId, Il2Cpp.string(tag));
        if (isNothing(kit)) {
            throw new Error(`CreateCustomController(${definitionId}, "${tag}") returned nothing`);
        }
    }
    rec.kit = kit;
    const target = players.find((p) => p.name === playerName);
    if (target === undefined) {
        throw new Error(`no player named ${playerName}; players: ${players.map((p) => p.name).join(", ")}`);
    }
    rec.target = target;
    target.helper.method("AddController", 2)
        .overload("Rewired.Controller", "System.Boolean").invoke(kit, true);
    target.helper.field<Il2Cpp.Object>("maps").value
        .method("LoadDefaultMaps", 1).invoke(customType());
    return {
        first_setup: first,
        switch_was: rec.switchWas,
        switch_now: ignoresUnfocused(cfg),
        players_was: rec.players.map(playerLine),
        players_now: allPlayers().map(playerLine),
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

// Sets every element, waits until the frame count has moved on by the hold,
// writing the values again each tick when `rewrite` is true, then clears
// every element. The release runs even when the wait fails. Each step runs on
// the main thread when `on_main` is true, else on Frida's own thread. With a
// "read" list, each step also reads those actions on the kit's player, and
// the reply's `seen` holds each change.
async function press(m: Message) {
    const rec = recorded;
    if (rec === null || rec.kit === null) {
        throw new Error("no kit: send setup first");
    }
    const kit = rec.kit;
    const elements = elementsOf(m);
    const hold = needInteger(m, "hold_frames");
    const capS = m.cap_s;
    if (typeof capS !== "number" || !(capS > 0)) {
        throw new Error('press needs a "cap_s" above 0');
    }
    const rewrite = m.rewrite === true;
    const onMain = m.on_main === true;
    const where = onMain ? "main" : undefined;
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
    for (const e of elements) {
        touched.set(e.id, typeof e.value === "boolean");
    }
    const start = Date.now();
    let downFrame = -1;
    let downMs = start;
    let upFrame = -1;
    let upMs = start;
    let failure: Error | null = null;
    // The down write sits inside the try too: a write that fails half-way has
    // already put some elements down, and the release below clears them all.
    try {
        downFrame = await Il2Cpp.perform(() => {
            write(kit, elements, false);
            const f = frameCount();
            look(f);
            return f;
        }, where);
        downMs = Date.now();
        let frame = downFrame;
        while (frame < downFrame + hold) {
            if (Date.now() - start >= capS * 1000) {
                failure = new Error(`the hold of ${hold} frames did not pass within the cap of `
                    + `${capS} s: down at frame ${downFrame}, last frame seen ${frame}`);
                break;
            }
            await sleep(TICK_MS);
            frame = await Il2Cpp.perform(() => {
                if (rewrite) {
                    write(kit, elements, false);
                }
                const f = frameCount();
                look(f);
                return f;
            }, where);
        }
    } catch (e) {
        failure = e instanceof Error ? e : new Error(`${e}`);
    }
    // The release runs on every path: after the hold, after a failed wait and
    // at the cap. Its read is the last one the reply carries.
    try {
        upFrame = await Il2Cpp.perform(() => {
            write(kit, elements, true);
            const f = frameCount();
            look(f);
            return f;
        }, where);
        upMs = Date.now();
    } catch (e) {
        throw new Error(`${failure === null ? "" : `${failure.message}; and `}the release failed, `
            + `so a pad may still be down (send teardown): ${e}`);
    }
    if (failure !== null) {
        throw new Error(`${failure.message}; released at frame ${upFrame}`);
    }
    const reply: { [key: string]: unknown } = {
        down_frame: downFrame, up_frame: upFrame, down_ms: downMs, up_ms: upMs,
        hold_frames: hold, rewrite, on_main: onMain,
    };
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
            read: () => p.player.method<boolean>("GetAnyButton", 0).invoke(),
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
    const kit = tag === null ? null : findKit(tag);
    return {
        frame: frameCount(),
        focused: unity("UnityEngine.Application").method<boolean>("get_isFocused").invoke(),
        switch_now: ignoresUnfocused(config()),
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

// Runs on the main thread. Clears every element a press wrote, takes the kit
// from its player, destroys it, gives every player back the keyboard and
// mouse flags setup recorded, puts the switch back, and checks each.
function teardown(m: Message) {
    const checks: Check[] = [];
    const rec = recorded;
    const tag = rec !== null ? rec.tag : (typeof m.tag === "string" ? m.tag : null);
    const kit = rec !== null && rec.kit !== null ? rec.kit : (tag === null ? null : findKit(tag));
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
        step(checks, "kit destroyed", () => {
            const done = controllers().method<boolean>("DestroyCustomController", 1).invoke(kit);
            return { what: "kit destroyed", ok: done, detail: `DestroyCustomController returned ${done}` };
        });
    }
    step(checks, "kit gone", () => {
        if (tag === null) {
            return { what: "kit gone", ok: false, detail: "no tag recorded or sent; not looked up" };
        }
        const left = findKit(tag);
        return left === null ? `no controller tagged ${tag}`
            : { what: "kit gone", ok: false, detail: `controller ${kitId(left)} still tagged ${tag}` };
    });
    if (rec === null) {
        checks.push({ what: "setup record", ok: false,
                      detail: "setup did not run in this agent, so no flags or switch were put back" });
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

// The first message LiveAgent.start posts is a read; the kit answers it with
// its status.
function answer(m: Message): Promise<object> {
    switch (m.type) {
        case "read":
        case "status":
            return Il2Cpp.perform(() => status(m), "main");
        case "setup":
            return Il2Cpp.perform(() => setup(m), "main");
        case "teardown":
            return Il2Cpp.perform(() => teardown(m), "main");
        case "press":
            return press(m);
        case "watch":
            return watch(m);
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
            send({ error: `${message.type} failed: ${e}`, id });
        }).finally(listen);
    });
}

listen();
