// The frame scheduler: given the whole press schedule once, it is called once
// per game frame and says which kit elements to put down, which to lift, and
// what to report. It holds design points 2 and 5 of
// docs/superpowers/plans/2026-11-12-in-game-press-timing.md (removed; git show b971b235:docs/superpowers/plans/2026-11-12-in-game-press-timing.md); D339
// in docs/audit/2026-10-03-fix-decisions.md holds the user's answers it follows.
//
// It is plain code with no game in it: no Frida, no il2cpp, no reads of its
// own. kit.ts (task IG2) reads the clock and the score and does the writes;
// tools/ch_mcp/tests/test_press_schedule.py drives it with plain Node. It holds
// no number of its own: the hold, the start lead, the score delay, the cap,
// the stall and the jump-back tolerance all come in the play message, which
// Python builds (task IG3). The frame step is measured, and the one count it
// keeps, JUDGE_FRAMES, is the game's (IN-93).
//
// ---- The contract (IG2's kit.ts and IG3's Python both use these names) ----
//
// The play message's numbers (PlayConfig), every time in ms:
//   entries         the schedule, a list of Entry
//   hold_frames     frames a press stays down (an integer, 1 or more)
//   start_lead_ms   on the first frame, heads whose at_ms is less than this
//                   ahead of the clock are skipped, with their later groups
//   score_delay_ms  how long after a release the score after is read
//   cap_ms          wall time from the first frame after which play stops
//   stall_ms        wall time the clock may hold still before play stops
//   jump_back_ms    how far the clock may fall back before play stops
// The clock's and the score's addresses are kit.ts's, not this class's.
//
// ---- Which frame a press goes down on ----
//
// The game judges a press at the song time of the frame that reads it
// (SP-29 on docs/clone-hero/05-star-power.md). The clock this hook reads is
// one frame old, and a press it writes is read one frame later (IN-93 on
// docs/clone-hero/07-internals.md). So a press written on a frame is judged
// JUDGE_FRAMES frame steps after that frame's clock. The step is measured:
// the median of every forward step of the clock since play began, so one
// long frame does not move it. Nothing fires before one step is measured.
//
// On each frame the scheduler predicts the judge time of a press written now
// (the clock plus JUDGE_FRAMES steps) and of one written on the next frame
// (one step more). An entry goes down on the frame whose prediction is
// nearest its at_ms among the frames inside its band and its window: it
// fires now unless the next frame is also inside and nearer. When the frames
// straddle the band (this frame is before it and the next past it), or a
// frame lands past it, no frame can land in it: the entry then goes down on
// the frame inside its window nearest the band, or nearest no_frame_aim_ms
// when the entry gives one, and its event says no_frame_in_band. An entry
// left with no frame inside its window is not sent: it is dropped, and its
// event says no_frame_in_band.
//
// One Entry (one Press of tools/ch_probe/experiments/play_chart.py), every
// time in song ms:
//   seq             the entry's sequence number, unique in the schedule
//   index           its chord's index
//   at_ms           the time it should be judged at (the schedule's chord
//                   time plus its aim_ms)
//   band            where it may be judged, {from_ms, to_ms, from_strict}:
//                   to_ms is inside, from_ms is inside unless from_strict.
//                   Optional; with none, the band is the window
//   window_from_ms  the front and back edge of its chord's hit window, both
//   window_to_ms    inside (TM-27). Optional, both or neither: a probe entry
//                   gives neither, and then fires on the frame nearest at_ms
//                   with no edge at all
//   no_frame_aim_ms when no frame can land in the band, the time the frame
//                   pick aims at instead of the band (a squeeze-in's plain
//                   fallback gives its chord's time); null or left out: the
//                   band itself. Only with a band
//   elements        the kit elements it writes, each {id, value}, as the
//                   kit's press message carries them
//   after_seq       for a later order group, the seq of the group it goes
//                   down the frame after; null for a head. A later group
//                   fires on the first frame after that group's down frame
//                   whose pads are free, and is dropped once its prediction
//                   is past its window's back edge
//   end_read_after  the chord whose Star Power end read retimes this entry
//                   (Press.end_read_after), or null
//
// What step() and stop() return (Step):
//   write           [{seq, elements}] to put down on this frame
//   release         [{seq, elements}] to lift on this frame
//   events          the reports below, in the order they happened
//
// The event kinds, each with `kind` set to its name:
//   "skipped"  once, on the first frame: seqs (the skipped entries' seqs,
//              ascending),
//              count (how many), clock_ms (the first frame's clock)
//   "sent"     one per entry that was written, once its score after and its
//              judge time are read, or play stopped: seq, index,
//              clock_ms_at_press (the clock of its down frame), down_frame,
//              up_frame, down_ms, up_ms, score_before (the score on its down
//              frame), score_after (the score on the first frame at least
//              score_delay_ms after its release; null when play stopped
//              first), frame_gap_ms (wall time since the frame before its
//              down frame; null on the first frame), agent_note, and:
//                step_ms             the measured step it was timed with
//                predicted_judge_ms  the judge time it was timed for
//                judged_ms           the clock read JUDGE_FRAMES frames after
//                                    its down frame, the song time the game
//                                    judged it at (IN-93); null when play
//                                    stopped first
//                judged_in_band      whether judged_ms is inside its band
//                                    (its window when it has no band); null
//                                    with neither, or with no judged_ms
//                no_frame_in_band    true when no frame could land in its
//                                    band and it took the fallback frame
//   "dropped"  one per entry that came due with no frame left inside its
//              window, and one per later group whose head was dropped: the
//              same fields as "sent", with clock_ms_at_press, step_ms and
//              predicted_judge_ms those of the frame it was dropped on,
//              down_frame, up_frame, down_ms, up_ms, judged_ms and
//              judged_in_band null, and score_after equal to score_before
//   "stop"     once, last: reason, frame (null for stop()), clock_ms (the
//              last clock seen), wall_ms
// agent_note is "" or "retime_missed": the entry has an end_read_after whose
// retime had not arrived when it came due.
//
// The stop reasons:
//   "done"         every entry was reported
//   "past_end"     the first frame skipped every entry
//   "stopped"      stop() was called (the stop_play message, or teardown)
//   "cap"          cap_ms of wall time passed since the first frame
//   "frozen"       the clock held still for more than stall_ms of wall time
//   "jumped_back"  the clock fell back by more than jump_back_ms; Python
//                  re-syncs by sending a fresh play
// Every stop lifts every element still down, in the same Step.
//
// retime(read_after, entries) takes the entries Python's retime_from_end gave
// for the end read after chord read_after. They replace every entry not yet
// fired: a pending entry missing from the list is removed, and a listed entry
// that already fired, dropped or was skipped is ignored. It returns
// {removed, ignored}, each a list of seqs. Python posts one after every end
// read, even one that moves nothing, because the arrival is what clears
// retime_missed for that read.

export interface Element {
    id: number;
    value: number | boolean;
}

export interface Band {
    from_ms: number;
    to_ms: number;
    from_strict: boolean;
}

export interface Window {
    from_ms: number;
    to_ms: number;
}

export interface Entry {
    seq: number;
    index: number;
    at_ms: number;
    band: Band | null;
    window: Window | null;
    no_frame_aim_ms: number | null;
    elements: Element[];
    after_seq: number | null;
    end_read_after: number | null;
}

export interface PlayConfig {
    entries: Entry[];
    hold_frames: number;
    start_lead_ms: number;
    score_delay_ms: number;
    cap_ms: number;
    stall_ms: number;
    jump_back_ms: number;
}

export interface Write {
    seq: number;
    elements: Element[];
}

export interface Step {
    write: Write[];
    release: Write[];
    events: object[];
}

export interface Retimed {
    removed: number[];
    ignored: number[];
}

type State = "pending" | "down" | "up" | "done";
type Outcome = "sent" | "dropped" | "skipped" | null;

interface Slot {
    entry: Entry;
    state: State;
    outcome: Outcome;
    clockMs: number | null;
    downFrame: number | null;
    downMs: number | null;
    upFrame: number | null;
    upMs: number | null;
    scoreBefore: number | null;
    frameGapMs: number | null;
    note: string;
    // The step and the predicted judge time of the frame it went down or was
    // dropped on.
    stepMs: number | null;
    predictedMs: number | null;
    // No frame could land in its band, so it takes the fallback frame.
    noFrameInBand: boolean;
    // Step calls since its down frame, until the judge time is read.
    stepsSinceDown: number;
    judgedMs: number | null;
    // Its place in `slots`' order, which the releases and reports of one
    // frame keep.
    order: number;
    // For a head, see wakeMs().
    wakeMs: number;
}

const RETIME_MISSED = "retime_missed";

// A press written on a frame is judged this many frame steps after the clock
// read on that frame: the clock is a frame old, and the write is read a frame
// later (IN-93 on docs/clone-hero/07-internals.md).
export const JUDGE_FRAMES = 2;

// The median of every number added, kept in two heaps: the low half, largest
// first, and the high half, smallest first.
class RunningMedian {
    low: number[] = [];
    high: number[] = [];

    add(v: number): void {
        if (this.low.length === 0 || v <= this.low[0]) {
            heapPush(this.low, v, (a, b) => a > b);
        } else {
            heapPush(this.high, v, (a, b) => a < b);
        }
        if (this.low.length > this.high.length + 1) {
            heapPush(this.high, heapPop(this.low, (a, b) => a > b), (a, b) => a < b);
        } else if (this.high.length > this.low.length) {
            heapPush(this.low, heapPop(this.high, (a, b) => a < b), (a, b) => a > b);
        }
    }

    // null until a number is added.
    median(): number | null {
        if (this.low.length === 0) {
            return null;
        }
        return this.low.length > this.high.length ? this.low[0] : (this.low[0] + this.high[0]) / 2;
    }
}

// `first(a, b)` is true when a belongs nearer the top of the heap than b.
function heapPush(h: number[], v: number, first: (a: number, b: number) => boolean): void {
    h.push(v);
    let i = h.length - 1;
    while (i > 0) {
        const up = (i - 1) >> 1;
        if (!first(h[i], h[up])) {
            break;
        }
        [h[i], h[up]] = [h[up], h[i]];
        i = up;
    }
}

function heapPop(h: number[], first: (a: number, b: number) => boolean): number {
    const top = h[0];
    const last = h.pop() as number;
    if (h.length !== 0) {
        h[0] = last;
        let i = 0;
        for (;;) {
            const l = 2 * i + 1;
            const r = l + 1;
            let best = i;
            if (l < h.length && first(h[l], h[best])) {
                best = l;
            }
            if (r < h.length && first(h[r], h[best])) {
                best = r;
            }
            if (best === i) {
                break;
            }
            [h[i], h[best]] = [h[best], h[i]];
            i = best;
        }
    }
    return top;
}

function inWindow(w: Window | null, ms: number): boolean {
    return w === null || (ms >= w.from_ms && ms <= w.to_ms);
}

function beforeBand(b: Band, ms: number): boolean {
    return b.from_strict ? ms <= b.from_ms : ms < b.from_ms;
}

function inBand(b: Band, ms: number): boolean {
    return !beforeBand(b, ms) && ms <= b.to_ms;
}

// The band an entry is picked inside: its own, else its window, else none.
function bandOf(e: Entry): Band | null {
    if (e.band !== null) {
        return e.band;
    }
    return e.window === null ? null : { from_ms: e.window.from_ms, to_ms: e.window.to_ms, from_strict: false };
}

// How far a judge time `ms` is from where the entry aims, in its normal pick
// or, once no frame can land in its band, its fallback pick.
function distance(e: Entry, fallback: boolean, ms: number): number {
    if (!fallback) {
        return Math.abs(ms - e.at_ms);
    }
    if (e.no_frame_aim_ms !== null) {
        return Math.abs(ms - e.no_frame_aim_ms);
    }
    const b = bandOf(e) as Band;
    return ms < b.from_ms ? b.from_ms - ms : ms > b.to_ms ? ms - b.to_ms : 0;
}

// Whether the entry may go down on a frame whose judge time is `ms`.
function allowed(e: Entry, fallback: boolean, ms: number): boolean {
    if (!inWindow(e.window, ms)) {
        return false;
    }
    const b = bandOf(e);
    return fallback || b === null || inBand(b, ms);
}

function fail(what: string): never {
    throw new Error(`press schedule: ${what}`);
}

function isNumber(v: unknown): v is number {
    return typeof v === "number" && Number.isFinite(v);
}

function needNumber(o: { [key: string]: unknown }, key: string, where: string): number {
    const v = o[key];
    if (!isNumber(v)) {
        fail(`${where} needs a number "${key}": ${JSON.stringify(v)}`);
    }
    return v;
}

function maybeNumber(o: { [key: string]: unknown }, key: string, where: string): number | null {
    const v = o[key];
    if (v === undefined || v === null) {
        return null;
    }
    if (!isNumber(v)) {
        fail(`${where} has a "${key}" that is not a number: ${JSON.stringify(v)}`);
    }
    return v;
}

// One entry as the message gave it, checked and with its optional fields
// filled in as null.
function readEntry(raw: unknown): Entry {
    if (typeof raw !== "object" || raw === null) {
        fail(`an entry is an object: ${JSON.stringify(raw)}`);
    }
    const o = raw as { [key: string]: unknown };
    const where = `entry ${JSON.stringify(o.seq)}`;
    const seq = needNumber(o, "seq", where);
    const index = needNumber(o, "index", where);
    if (!Number.isInteger(seq) || !Number.isInteger(index)) {
        fail(`${where} needs a whole "seq" and "index"`);
    }
    const elements = o.elements;
    if (!Array.isArray(elements) || elements.length === 0) {
        fail(`${where} needs a list of elements`);
    }
    const checked = elements.map((e: unknown): Element => {
        const el = e as { [key: string]: unknown } | null;
        const ok = typeof el === "object" && el !== null && Number.isInteger(el.id)
            && (isNumber(el.value) || el.value === true);
        if (!ok) {
            fail(`${where} has an element that is not {id, value} with a number or true: `
                + JSON.stringify(e));
        }
        const good = e as { [key: string]: unknown };
        return { id: good.id as number, value: good.value as number | boolean };
    });
    // The keys the frame pick replaced: a sender that still gives one was
    // built for the old contract, and is refused rather than half read.
    for (const gone of ["last_send_ms", "after_ms"]) {
        if (o[gone] !== undefined) {
            fail(`${where} gives "${gone}", which the frame pick replaced; see the contract above`);
        }
    }
    const band = readBand(o.band, where);
    const window = readWindow(o, where);
    const noFrameAim = maybeNumber(o, "no_frame_aim_ms", where);
    if (noFrameAim !== null && band === null) {
        fail(`${where} gives "no_frame_aim_ms" with no "band"`);
    }
    return {
        seq, index,
        at_ms: needNumber(o, "at_ms", where),
        band,
        window,
        no_frame_aim_ms: noFrameAim,
        elements: checked,
        after_seq: maybeNumber(o, "after_seq", where),
        end_read_after: maybeNumber(o, "end_read_after", where),
    };
}

function readBand(raw: unknown, where: string): Band | null {
    if (raw === undefined || raw === null) {
        return null;
    }
    if (typeof raw !== "object") {
        fail(`${where} has a "band" that is not an object: ${JSON.stringify(raw)}`);
    }
    const b = raw as { [key: string]: unknown };
    const fromMs = needNumber(b, "from_ms", `${where}'s band`);
    const toMs = needNumber(b, "to_ms", `${where}'s band`);
    if (typeof b.from_strict !== "boolean") {
        fail(`${where}'s band needs a true or false "from_strict": ${JSON.stringify(b.from_strict)}`);
    }
    if (fromMs > toMs) {
        fail(`${where}'s band starts after it ends: ${fromMs} > ${toMs}`);
    }
    return { from_ms: fromMs, to_ms: toMs, from_strict: b.from_strict };
}

function readWindow(o: { [key: string]: unknown }, where: string): Window | null {
    const fromMs = maybeNumber(o, "window_from_ms", where);
    const toMs = maybeNumber(o, "window_to_ms", where);
    if ((fromMs === null) !== (toMs === null)) {
        fail(`${where} needs both "window_from_ms" and "window_to_ms", or neither`);
    }
    if (fromMs === null || toMs === null) {
        return null;
    }
    if (fromMs > toMs) {
        fail(`${where}'s window starts after it ends: ${fromMs} > ${toMs}`);
    }
    return { from_ms: fromMs, to_ms: toMs };
}

function readEntries(raw: unknown): Entry[] {
    if (!Array.isArray(raw)) {
        fail(`"entries" is a list: ${JSON.stringify(raw)}`);
    }
    const entries = raw.map(readEntry);
    const seen = new Set<number>();
    for (const e of entries) {
        if (seen.has(e.seq)) {
            fail(`seq ${e.seq} is listed twice`);
        }
        seen.add(e.seq);
    }
    return entries;
}

// A head that is not yet in its fallback pick does nothing on a frame whose
// next-frame judge time is below this: it cannot fire before its window's
// front edge (or, with no window, its band's front edge, or with neither its
// at_ms), and cannot turn to its fallback or be dropped before its band's or
// window's back edge. step() looks at no head before this, which is what keeps
// a frame's work to the presses near the clock.
function wakeMs(e: Entry): number {
    const b = bandOf(e);
    if (e.window !== null) {
        return Math.min(e.window.from_ms, (b as Band).to_ms);
    }
    return b === null ? e.at_ms : b.from_ms;
}

function freshSlot(entry: Entry, order: number): Slot {
    return {
        entry, state: "pending", outcome: null, clockMs: null, downFrame: null, downMs: null,
        upFrame: null, upMs: null, scoreBefore: null, frameGapMs: null, note: "",
        stepMs: null, predictedMs: null, noFrameInBand: false, stepsSinceDown: 0, judgedMs: null,
        order, wakeMs: wakeMs(entry),
    };
}

// Pending entries in the order they are looked at: by planned judge time, so a
// head always comes before its later groups, then by seq.
function byPlan(a: Slot, b: Slot): number {
    return a.entry.at_ms - b.entry.at_ms || a.entry.seq - b.entry.seq;
}

function byOrder(a: Slot, b: Slot): number {
    return a.order - b.order;
}

function byWake(a: Slot, b: Slot): number {
    return a.wakeMs - b.wakeMs || byPlan(a, b);
}

export class PressSchedule {
    holdFrames: number;
    startLeadMs: number;
    scoreDelayMs: number;
    capMs: number;
    stallMs: number;
    jumpBackMs: number;
    readScore: () => number;
    slots: Map<number, Slot>;
    // The reads whose retime has arrived, by read_after chord.
    retimedReads: Set<number>;
    // Element id -> the seq holding it down.
    held: Map<number, number>;
    startWall: number | null;
    lastWall: number | null;
    lastClock: number | null;
    lastMove: number | null;
    stopReason: string | null;
    // The clock of the frame before, every frame, and the steps measured.
    frameClock: number | null;
    steps: RunningMedian;
    // The queues a frame reads instead of every slot, so its work follows the
    // presses near the clock, not the schedule's length. queue() builds them
    // at load and after each retime; step() keeps them.
    //   heads       pending heads by wakeMs, from headsFrom on; slots done
    //               since are dropped as a frame's walk reaches them
    //   planHeads   pending heads by plan, for the first frame's skip
    //   fallback    pending heads in their fallback pick
    //   waiting     head seq -> its pending later groups, while it is pending
    //   ready       pending later groups whose head is settled
    //   down, up    slots held down, and lifted but not yet reported
    //   judging     slots written whose judge time is not yet read
    //   open        how many slots are not yet reported
    heads: Slot[];
    headsFrom: number;
    planHeads: Slot[];
    fallback: Set<Slot>;
    waiting: Map<number, Slot[]>;
    ready: Set<Slot>;
    down: Set<Slot>;
    up: Set<Slot>;
    judging: Set<Slot>;
    open: number;
    nextOrder: number;
    // How many slots the last step() looked at, counting each look; the test
    // of a frame's cost reads it.
    examined: number;

    // `config` is the play message; `readScore` reads the game's score, and
    // is called at most once per step.
    constructor(config: PlayConfig, readScore: () => number) {
        const c = config as unknown as { [key: string]: unknown };
        const hold = needNumber(c, "hold_frames", "play");
        if (!Number.isInteger(hold) || hold < 1) {
            fail(`"hold_frames" is a whole number of 1 or more: ${hold}`);
        }
        this.holdFrames = hold;
        this.startLeadMs = needNumber(c, "start_lead_ms", "play");
        this.scoreDelayMs = needNumber(c, "score_delay_ms", "play");
        this.capMs = needNumber(c, "cap_ms", "play");
        this.stallMs = needNumber(c, "stall_ms", "play");
        this.jumpBackMs = needNumber(c, "jump_back_ms", "play");
        this.readScore = readScore;
        const entries = readEntries(c.entries);
        this.slots = new Map(entries.map((e, i): [number, Slot] => [e.seq, freshSlot(e, i)]));
        this.nextOrder = entries.length;
        this.checkHeads(this.slots);
        this.retimedReads = new Set();
        this.held = new Map();
        this.startWall = null;
        this.lastWall = null;
        this.lastClock = null;
        this.lastMove = null;
        this.stopReason = null;
        this.frameClock = null;
        this.steps = new RunningMedian();
        this.heads = [];
        this.headsFrom = 0;
        this.planHeads = [];
        this.fallback = new Set();
        this.waiting = new Map();
        this.ready = new Set();
        this.down = new Set();
        this.up = new Set();
        this.judging = new Set();
        this.open = 0;
        this.examined = 0;
        this.queue();
    }

    // Builds the pending queues from `slots`. The held, lifted and judging
    // sets hold no pending slot, so a retime leaves them as they are.
    queue(): void {
        const heads: Slot[] = [];
        this.fallback = new Set();
        this.waiting = new Map();
        this.ready = new Set();
        this.open = 0;
        for (const s of this.slots.values()) {
            if (s.state !== "done") {
                this.open += 1;
            }
            if (s.state !== "pending") {
                continue;
            }
            const head = s.entry.after_seq;
            if (head === null) {
                heads.push(s);
                if (s.noFrameInBand) {
                    this.fallback.add(s);
                }
            } else if ((this.slots.get(head) as Slot).outcome === null) {
                const list = this.waiting.get(head);
                if (list === undefined) {
                    this.waiting.set(head, [s]);
                } else {
                    list.push(s);
                }
            } else {
                this.ready.add(s);
            }
        }
        this.planHeads = this.startWall === null ? [...heads].sort(byPlan) : [];
        this.heads = heads.sort(byWake);
        this.headsFrom = 0;
    }

    // A slot leaves the pending queues for good.
    markDone(s: Slot): void {
        s.state = "done";
        this.open -= 1;
        this.ready.delete(s);
        this.fallback.delete(s);
        this.up.delete(s);
    }

    // `s` has its outcome: its later groups stop waiting on it, and are
    // returned.
    settle(s: Slot): Slot[] {
        const list = this.waiting.get(s.entry.seq);
        if (list === undefined) {
            return [];
        }
        this.waiting.delete(s.entry.seq);
        const out: Slot[] = [];
        for (const f of list) {
            this.examined += 1;
            if (f.state === "pending") {
                this.ready.add(f);
                out.push(f);
            }
        }
        return out;
    }

    // The pending slots that may act on this frame, by plan: the heads whose
    // wakeMs the next frame's judge time has reached, the heads in their
    // fallback pick, and the later groups whose head is settled.
    candidates(nextMs: number | null): Slot[] {
        const out: Slot[] = [];
        if (nextMs !== null) {
            const live: Slot[] = [];
            let j = this.headsFrom;
            while (j < this.heads.length && this.heads[j].wakeMs <= nextMs) {
                this.examined += 1;
                if (this.heads[j].state === "pending") {
                    live.push(this.heads[j]);
                }
                j += 1;
            }
            // The live ones move up against the first head not yet reached,
            // in their order, so the done ones fall out of the queue.
            const from = j - live.length;
            for (let k = 0; k < live.length; k++) {
                this.heads[from + k] = live[k];
            }
            this.headsFrom = from;
            for (const s of live) {
                if (!s.noFrameInBand) {
                    out.push(s);
                }
            }
        }
        for (const s of this.fallback) {
            out.push(s);
        }
        for (const s of this.ready) {
            out.push(s);
        }
        return out.sort(byPlan);
    }

    // Every later group names a head the schedule holds.
    checkHeads(slots: Map<number, Slot>): void {
        for (const s of slots.values()) {
            const head = s.entry.after_seq;
            if (head !== null && (head === s.entry.seq || !slots.has(head))) {
                fail(`entry ${s.entry.seq} follows seq ${head}, which the schedule does not hold`);
            }
        }
    }

    // One game frame: `frame` is its frame number, `clockMs` the song clock
    // and `wallMs` the wall time, both in ms.
    step(frame: number, clockMs: number, wallMs: number): Step {
        const out: Step = { write: [], release: [], events: [] };
        this.examined = 0;
        if (this.stopReason !== null) {
            return out;
        }
        const gap = this.lastWall === null ? null : wallMs - this.lastWall;
        this.lastWall = wallMs;
        let score: number | null = null;
        const scoreNow = (): number => {
            if (score === null) {
                score = this.readScore();
            }
            return score;
        };

        if (this.startWall === null) {
            this.startWall = wallMs;
            this.lastClock = clockMs;
            this.lastMove = wallMs;
            this.skipAtLoad(clockMs, out);
            if (this.allReported()) {
                this.finish("past_end", frame, clockMs, wallMs, out);
                return out;
            }
        } else {
            const reason = this.stopFor(clockMs, wallMs);
            if (reason !== null) {
                this.finish(reason, frame, clockMs, wallMs, out);
                return out;
            }
        }

        // A step is a forward move of the clock from the frame before; a
        // frame whose clock held still or fell back measures nothing.
        if (this.frameClock !== null && clockMs > this.frameClock) {
            this.steps.add(clockMs - this.frameClock);
        }
        this.frameClock = clockMs;
        const stepMs = this.steps.median();
        // The judge time of a press written on this frame and on the next.
        const nowMs = stepMs === null ? null : clockMs + JUDGE_FRAMES * stepMs;
        const nextMs = stepMs === null ? null : (nowMs as number) + stepMs;

        // The clock read JUDGE_FRAMES frames after a down frame is the song
        // time its press was judged at (IN-93).
        for (const s of this.judging) {
            this.examined += 1;
            s.stepsSinceDown += 1;
            if (s.stepsSinceDown === JUDGE_FRAMES) {
                s.judgedMs = clockMs;
                this.judging.delete(s);
            }
        }

        // Releases come first; a pad lifted on this frame takes no new press
        // until the next one.
        const busy = new Set<number>();
        const lifting: Slot[] = [];
        for (const s of this.down) {
            this.examined += 1;
            if (frame >= (s.downFrame as number) + this.holdFrames) {
                lifting.push(s);
            }
        }
        for (const s of lifting.sort(byOrder)) {
            this.lift(s, frame, wallMs, out);
            for (const e of s.entry.elements) {
                busy.add(e.id);
            }
        }

        const pending = this.candidates(nextMs);
        // A slot settled on this frame frees its later groups; one that comes
        // after it by plan is looked at on this frame too, in its place.
        const settled = (s: Slot, at: number): void => {
            for (const f of this.settle(s)) {
                if (byPlan(f, s) > 0) {
                    let k = at + 1;
                    while (k < pending.length && byPlan(pending[k], f) < 0) {
                        k += 1;
                    }
                    pending.splice(k, 0, f);
                }
            }
        };
        for (let i = 0; i < pending.length; i++) {
            const s = pending[i];
            this.examined += 1;
            const e = s.entry;
            let headLost = false;
            if (e.after_seq !== null) {
                const head = this.slots.get(e.after_seq) as Slot;
                if (head.outcome === null || (head.outcome === "sent" && frame <= (head.downFrame as number))) {
                    continue;
                }
                headLost = head.outcome !== "sent";
            }
            if (!headLost && (nowMs === null || nextMs === null)) {
                continue;  // no step measured yet
            }
            const note = e.end_read_after !== null && !this.retimedReads.has(e.end_read_after)
                ? RETIME_MISSED : "";
            const drop = (): void => {
                this.markDone(s);
                s.outcome = "dropped";
                s.clockMs = clockMs;
                s.scoreBefore = scoreNow();
                s.frameGapMs = gap;
                s.note = note;
                s.stepMs = stepMs;
                s.predictedMs = nowMs;
                out.events.push(this.event(s, s.scoreBefore));
                settled(s, i);
            };
            if (headLost) {
                drop();
                continue;
            }
            const now = nowMs as number;
            const next = nextMs as number;
            let fire: boolean;
            if (e.after_seq !== null) {
                // A later group goes down the frame after the group it
                // follows, so the game scores the groups in order (SC-38 on
                // docs/clone-hero/06-scoring.md), unless its window is gone.
                if (e.window !== null && now > e.window.to_ms) {
                    drop();
                    continue;
                }
                fire = true;
            } else {
                const band = bandOf(e);
                if (!s.noFrameInBand && band !== null && !inBand(band, now)
                    && (now > band.to_ms || next > band.to_ms)) {
                    // This frame is past the band, or before it with the next
                    // one past it: no frame lands inside.
                    s.noFrameInBand = true;
                    this.fallback.add(s);
                }
                if (e.window !== null && now > e.window.to_ms) {
                    drop();
                    continue;
                }
                const fb = s.noFrameInBand;
                fire = allowed(e, fb, now)
                    && !(allowed(e, fb, next) && distance(e, fb, next) < distance(e, fb, now));
            }
            if (!fire || e.elements.some((el) => this.held.has(el.id) || busy.has(el.id))) {
                continue;
            }
            s.state = "down";
            this.ready.delete(s);
            this.fallback.delete(s);
            this.down.add(s);
            this.judging.add(s);
            s.outcome = "sent";
            s.clockMs = clockMs;
            s.downFrame = frame;
            s.downMs = wallMs;
            s.scoreBefore = scoreNow();
            s.frameGapMs = gap;
            s.note = note;
            s.stepMs = stepMs;
            s.predictedMs = now;
            for (const el of e.elements) {
                this.held.set(el.id, e.seq);
                busy.add(el.id);
            }
            out.write.push({ seq: e.seq, elements: e.elements });
            settled(s, i);
        }

        const reporting: Slot[] = [];
        for (const s of this.up) {
            this.examined += 1;
            if (s.judgedMs !== null && wallMs >= (s.upMs as number) + this.scoreDelayMs) {
                reporting.push(s);
            }
        }
        for (const s of reporting.sort(byOrder)) {
            this.markDone(s);
            out.events.push(this.event(s, scoreNow()));
        }

        if (this.allReported()) {
            this.finish("done", frame, clockMs, wallMs, out);
        }
        return out;
    }

    // Ends play at once, outside any frame: the stop_play message and
    // teardown.
    stop(wallMs: number): Step {
        const out: Step = { write: [], release: [], events: [] };
        if (this.stopReason === null) {
            this.finish("stopped", null, this.lastClock, wallMs, out);
        }
        return out;
    }

    retime(readAfter: number, raw: unknown): Retimed {
        if (!Number.isInteger(readAfter)) {
            fail(`a retime needs a whole "read_after": ${JSON.stringify(readAfter)}`);
        }
        const entries = readEntries(raw);
        const listed = new Set(entries.map((e) => e.seq));
        const next = new Map(this.slots);
        const removed: number[] = [];
        const ignored: number[] = [];
        for (const [seq, s] of this.slots) {
            if (s.state === "pending" && !listed.has(seq)) {
                next.delete(seq);
                removed.push(seq);
            }
        }
        for (const e of entries) {
            const s = next.get(e.seq);
            if (s !== undefined && s.state !== "pending") {
                ignored.push(e.seq);
            } else {
                // A replaced slot keeps its place in the Map, so its order;
                // a new one gets its order below.
                next.set(e.seq, freshSlot(e, s === undefined ? -1 : s.order));
            }
        }
        // Checked before anything changes, so a bad retime leaves the
        // schedule as it was.
        this.checkHeads(next);
        // New slots go after every slot there was, in the order listed.
        let order = this.nextOrder;
        for (const s of next.values()) {
            if (!this.slots.has(s.entry.seq)) {
                s.order = order;
                order += 1;
            }
        }
        this.nextOrder = order;
        this.slots = next;
        this.queue();
        this.retimedReads.add(readAfter);
        return { removed, ignored };
    }

    // Why play stops on this frame, or null. The cap comes first, as
    // play_chords asks it before each clock read; the jump back and the
    // stall follow in live.StoppedCheck's order.
    stopFor(clockMs: number, wallMs: number): string | null {
        if (wallMs - (this.startWall as number) >= this.capMs) {
            return "cap";
        }
        if (clockMs < (this.lastClock as number) - this.jumpBackMs) {
            return "jumped_back";
        }
        if (clockMs !== this.lastClock) {
            this.lastClock = clockMs;
            this.lastMove = wallMs;
            return null;
        }
        if (wallMs - (this.lastMove as number) > this.stallMs) {
            return "frozen";
        }
        return null;
    }

    // On the first frame: every head whose at_ms is less than the start
    // lead ahead of the clock, and every later group of a skipped head.
    // The heads skipped are the front of planHeads. Heads are judged first, so
    // every later group of a skipped head is skipped; a later group that
    // follows a later group is skipped when that one is skipped and comes
    // before it by plan. One that is not skipped is left ready, and the frame
    // drops it.
    skipAtLoad(clockMs: number, out: Step): void {
        const seqs: number[] = [];
        const skipped: Slot[] = [];
        const skip = (s: Slot): void => {
            this.markDone(s);
            s.outcome = "skipped";
            seqs.push(s.entry.seq);
            skipped.push(s);
        };
        for (const s of this.planHeads) {
            this.examined += 1;
            if (!(s.entry.at_ms < clockMs + this.startLeadMs)) {
                break;
            }
            skip(s);
        }
        this.planHeads = [];
        for (let k = 0; k < skipped.length; k++) {
            const x = skipped[k];
            for (const f of this.settle(x)) {
                if (x.entry.after_seq === null || byPlan(x, f) < 0) {
                    skip(f);
                }
            }
        }
        seqs.sort((a, b) => a - b);
        out.events.push({ kind: "skipped", seqs, count: seqs.length, clock_ms: clockMs });
    }

    lift(s: Slot, frame: number | null, wallMs: number, out: Step): void {
        s.state = "up";
        this.down.delete(s);
        this.up.add(s);
        s.upFrame = frame;
        s.upMs = wallMs;
        for (const el of s.entry.elements) {
            if (this.held.get(el.id) === s.entry.seq) {
                this.held.delete(el.id);
            }
        }
        out.release.push({ seq: s.entry.seq, elements: s.entry.elements });
    }

    allReported(): boolean {
        return this.open === 0;
    }

    // Lifts every element still down, reports every written entry not yet
    // reported (its score after unread), then the stop.
    finish(reason: string, frame: number | null, clockMs: number | null, wallMs: number,
           out: Step): void {
        for (const s of [...this.down].sort(byOrder)) {
            this.examined += 1;
            this.lift(s, frame, wallMs, out);
        }
        for (const s of [...this.up].sort(byOrder)) {
            this.examined += 1;
            this.markDone(s);
            out.events.push(this.event(s, null));
        }
        this.stopReason = reason;
        out.events.push({ kind: "stop", reason, frame, clock_ms: clockMs, wall_ms: wallMs });
    }

    event(s: Slot, scoreAfter: number | null): object {
        const band = bandOf(s.entry);
        return {
            kind: s.outcome,
            seq: s.entry.seq,
            index: s.entry.index,
            clock_ms_at_press: s.clockMs,
            down_frame: s.downFrame,
            up_frame: s.upFrame,
            down_ms: s.downMs,
            up_ms: s.upMs,
            score_before: s.scoreBefore,
            score_after: scoreAfter,
            frame_gap_ms: s.frameGapMs,
            agent_note: s.note,
            step_ms: s.stepMs,
            predicted_judge_ms: s.predictedMs,
            judged_ms: s.judgedMs,
            judged_in_band: band === null || s.judgedMs === null ? null : inBand(band, s.judgedMs),
            no_frame_in_band: s.noFrameInBand,
        };
    }
}
