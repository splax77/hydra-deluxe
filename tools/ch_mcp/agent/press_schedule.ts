// The frame scheduler: given the whole press schedule once, it is called once
// per game frame and says which kit elements to put down, which to lift, and
// what to report. It holds design points 2 and 5 of
// docs/superpowers/plans/2026-11-12-in-game-press-timing.md (task IG1); D339
// in docs/audit/2026-10-03-fix-decisions.md holds the user's answers it follows.
//
// It is plain code with no game in it: no Frida, no il2cpp, no reads of its
// own. kit.ts (task IG2) reads the clock and the score and does the writes;
// tools/ch_mcp/tests/test_press_schedule.py drives it with plain Node. It holds
// no number of its own: the hold, the start lead, the score delay, the cap,
// the stall and the jump-back tolerance all come in the play message, which
// Python builds (task IG3).
//
// ---- The contract (IG2's kit.ts and IG3's Python both use these names) ----
//
// The play message's numbers (PlayConfig), every time in ms:
//   entries         the schedule, a list of Entry
//   hold_frames     frames a press stays down (an integer, 1 or more)
//   start_lead_ms   on the first frame, entries whose fire time is less than
//                   this ahead of the clock are skipped
//   score_delay_ms  how long after a release the score after is read
//   cap_ms          wall time from the first frame after which play stops
//   stall_ms        wall time the clock may hold still before play stops
//   jump_back_ms    how far the clock may fall back before play stops
// The clock's and the score's addresses are kit.ts's, not this class's.
//
// One Entry (one Press of tools/ch_probe/experiments/play_chart.py):
//   seq             the entry's sequence number, unique in the schedule
//   index           its chord's index
//   at_ms           the song clock it fires at (Press.at_ms)
//   last_send_ms    the latest clock it may still be written at
//   elements        the kit elements it writes, each {id, value}, as the
//                   kit's press message carries them
//   after_ms        for a later order group, the gap after its head's real
//                   fire clock; null for a head
//   after_seq       for a later order group, its head's seq; null for a head
//   end_read_after  the chord whose Star Power end read retimes this entry
//                   (Press.end_read_after), or null
//
// What step() and stop() return (Step):
//   write           [{seq, elements}] to put down on this frame
//   release         [{seq, elements}] to lift on this frame
//   events          the reports below, in the order they happened
//
// The event kinds, each with `kind` set to its name:
//   "skipped"  once, on the first frame: seqs (the skipped entries' seqs),
//              count (how many), clock_ms (the first frame's clock)
//   "sent"     one per entry that was written, once its score after is read
//              or play stopped: seq, index, clock_ms_at_press (the clock of
//              its down frame), down_frame, up_frame, down_ms, up_ms,
//              score_before (the score on its down frame), score_after (the
//              score on the first frame at least score_delay_ms after its
//              release; null when play stopped first), frame_gap_ms (wall
//              time since the frame before its down frame; null on the first
//              frame), agent_note
//   "dropped"  one per entry past its last_send_ms when it came due, or whose
//              head was dropped: the same fields as "sent", with
//              clock_ms_at_press the clock it was dropped at, down_frame,
//              up_frame, down_ms and up_ms null, and score_after equal to
//              score_before
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

export interface Entry {
    seq: number;
    index: number;
    at_ms: number;
    last_send_ms: number;
    elements: Element[];
    after_ms: number | null;
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
}

const RETIME_MISSED = "retime_missed";

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
    const afterMs = maybeNumber(o, "after_ms", where);
    const afterSeq = maybeNumber(o, "after_seq", where);
    if ((afterMs === null) !== (afterSeq === null)) {
        fail(`${where} needs both "after_ms" and "after_seq", or neither`);
    }
    if (afterMs !== null && !(afterMs > 0)) {
        fail(`${where} has an "after_ms" that is not above 0`);
    }
    return {
        seq, index,
        at_ms: needNumber(o, "at_ms", where),
        last_send_ms: needNumber(o, "last_send_ms", where),
        elements: checked,
        after_ms: afterMs,
        after_seq: afterSeq,
        end_read_after: maybeNumber(o, "end_read_after", where),
    };
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

function freshSlot(entry: Entry): Slot {
    return {
        entry, state: "pending", outcome: null, clockMs: null, downFrame: null, downMs: null,
        upFrame: null, upMs: null, scoreBefore: null, frameGapMs: null, note: "",
    };
}

// Pending entries in the order they are looked at: by planned fire time, so a
// head always comes before its later groups, then by seq.
function byPlan(a: Slot, b: Slot): number {
    return a.entry.at_ms - b.entry.at_ms || a.entry.seq - b.entry.seq;
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
        this.slots = new Map(entries.map((e) => [e.seq, freshSlot(e)]));
        this.checkHeads(this.slots);
        this.retimedReads = new Set();
        this.held = new Map();
        this.startWall = null;
        this.lastWall = null;
        this.lastClock = null;
        this.lastMove = null;
        this.stopReason = null;
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

        // Releases come first; a pad lifted on this frame takes no new press
        // until the next one.
        const busy = new Set<number>();
        for (const s of this.slots.values()) {
            if (s.state === "down" && frame >= (s.downFrame as number) + this.holdFrames) {
                this.lift(s, frame, wallMs, out);
                for (const e of s.entry.elements) {
                    busy.add(e.id);
                }
            }
        }

        const pending = [...this.slots.values()].filter((s) => s.state === "pending").sort(byPlan);
        for (const s of pending) {
            const e = s.entry;
            let dueMs = e.at_ms;
            let headLost = false;
            if (e.after_seq !== null) {
                const head = this.slots.get(e.after_seq) as Slot;
                if (head.outcome === "sent") {
                    dueMs = (head.clockMs as number) + (e.after_ms as number);
                } else if (head.outcome === null) {
                    continue;
                } else {
                    headLost = true;
                }
            }
            if (!headLost && clockMs < dueMs) {
                continue;
            }
            const note = e.end_read_after !== null && !this.retimedReads.has(e.end_read_after)
                ? RETIME_MISSED : "";
            if (headLost || clockMs > e.last_send_ms) {
                s.state = "done";
                s.outcome = "dropped";
                s.clockMs = clockMs;
                s.scoreBefore = scoreNow();
                s.frameGapMs = gap;
                s.note = note;
                out.events.push(this.event(s, s.scoreBefore));
                continue;
            }
            if (e.elements.some((el) => this.held.has(el.id) || busy.has(el.id))) {
                continue;
            }
            s.state = "down";
            s.outcome = "sent";
            s.clockMs = clockMs;
            s.downFrame = frame;
            s.downMs = wallMs;
            s.scoreBefore = scoreNow();
            s.frameGapMs = gap;
            s.note = note;
            for (const el of e.elements) {
                this.held.set(el.id, e.seq);
                busy.add(el.id);
            }
            out.write.push({ seq: e.seq, elements: e.elements });
        }

        for (const s of this.slots.values()) {
            if (s.state === "up" && wallMs >= (s.upMs as number) + this.scoreDelayMs) {
                s.state = "done";
                out.events.push(this.event(s, scoreNow()));
            }
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
                next.set(e.seq, freshSlot(e));
            }
        }
        // Checked before anything changes, so a bad retime leaves the
        // schedule as it was.
        this.checkHeads(next);
        this.slots = next;
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

    // On the first frame: every head whose fire time is less than the start
    // lead ahead of the clock, and every later group of a skipped head.
    skipAtLoad(clockMs: number, out: Step): void {
        const seqs: number[] = [];
        const pending = [...this.slots.values()].sort(byPlan);
        for (const s of pending) {
            const e = s.entry;
            const skip = e.after_seq === null
                ? e.at_ms < clockMs + this.startLeadMs
                : (this.slots.get(e.after_seq) as Slot).outcome === "skipped";
            if (skip) {
                s.state = "done";
                s.outcome = "skipped";
                seqs.push(e.seq);
            }
        }
        out.events.push({ kind: "skipped", seqs, count: seqs.length, clock_ms: clockMs });
    }

    lift(s: Slot, frame: number | null, wallMs: number, out: Step): void {
        s.state = "up";
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
        for (const s of this.slots.values()) {
            if (s.state !== "done") {
                return false;
            }
        }
        return true;
    }

    // Lifts every element still down, reports every written entry not yet
    // reported (its score after unread), then the stop.
    finish(reason: string, frame: number | null, clockMs: number | null, wallMs: number,
           out: Step): void {
        for (const s of this.slots.values()) {
            if (s.state === "down") {
                this.lift(s, frame, wallMs, out);
            }
        }
        for (const s of this.slots.values()) {
            if (s.state === "up") {
                s.state = "done";
                out.events.push(this.event(s, null));
            }
        }
        this.stopReason = reason;
        out.events.push({ kind: "stop", reason, frame, clock_ms: clockMs, wall_ms: wallMs });
    }

    event(s: Slot, scoreAfter: number | null): object {
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
        };
    }
}
