// The stall harness's fake game. stall_harness.py starts it as the fake Frida
// script of a game-timed pass.
//
// It runs the real PressSchedule of tools/ch_mcp/agent/press_schedule.ts (Node
// erases its type annotations) on a wall-clock frame ticker, at the frame rate
// the harness passes. The song clock is its own: the wall time since the
// `play` message arrived, begun at the clock the harness passes. It answers
// the messages kit.ts answers for a play, and sends each frame's events as one
// batch, as kit.ts does. It writes no kit: a write only counts a fire of its
// entry, and the score rises by one per write.
//
// Run by stall_harness.py only:
//     node stall_game.mjs <press_schedule.ts URL> <frames a second> <clock start ms>
//
// Lines in on stdin are posted messages, each JSON with its id. Lines out on
// stdout are payloads, each JSON: a reply carries the message's id, an event
// batch carries none. stall_harness.py's game script owns the one message
// kit.ts lacks, `fake_stats`, which reports how many times each seq was
// written.

const [schedulerUrl, fpsText, clockStartText] = process.argv.slice(2);
const { PressSchedule } = await import(schedulerUrl);

const framePeriodMs = 1000 / Number(fpsText);
const clockStartMs = Number(clockStartText);
if (!(framePeriodMs > 0) || !Number.isFinite(clockStartMs)) {
    throw new Error(`stall_game needs a frame rate and a clock start: ${process.argv}`);
}

// Epoch ms with a fraction, as kit.ts stamps wall times in epoch ms.
const wallMs = () => performance.timeOrigin + performance.now();

let frame = 0;
let nextFrameAt = performance.now();
let play = null;
let score = 0;
const fires = new Map();
let open = true;

function send(payload) {
    process.stdout.write(JSON.stringify(payload) + "\n");
}

function applyStep(step, cause) {
    for (const w of step.write) {
        fires.set(w.seq, (fires.get(w.seq) ?? 0) + 1);
        score += 1;
    }
    let events = step.events;
    if (play.schedule.stopReason !== null) {
        events = events.map((e) => (e.kind === "stop" ? { ...e, cause, lift_error: null } : e));
        play = null;
        send({ type: "play_events", events });
    } else if (events.length !== 0) {
        send({ type: "play_events", events });
    }
}

function onFrame() {
    frame += 1;
    if (play === null) {
        return;
    }
    const wall = wallMs();
    const clock = play.clockAt0 + (performance.now() - play.wall0);
    let step;
    let cause = null;
    try {
        step = play.schedule.step(frame, clock, wall);
    } catch (e) {
        step = play.schedule.stop(wallMs());
        cause = `the frame step failed: ${e}`;
    }
    applyStep(step, cause);
}

function answer(m) {
    switch (m.type) {
        case "read":
        case "status":
            return { found: true };
        case "play": {
            if (play !== null) {
                throw new Error("a play is already running: send stop_play first");
            }
            const schedule = new PressSchedule(m, () => score);
            play = { schedule, clockAt0: clockStartMs, wall0: performance.now() };
            return { playing: true, entries: schedule.slots.size };
        }
        case "retime":
            if (play === null) {
                throw new Error("retime: no play is running");
            }
            return play.schedule.retime(m.read_after, m.entries);
        case "stop_play":
            if (play === null) {
                return { stopped: false };
            }
            applyStep(play.schedule.stop(wallMs()), null);
            return { stopped: true };
        case "fake_stats":
            return { fires: Object.fromEntries(fires), frames: frame };
        default:
            throw new Error(`unknown message type ${m.type}`);
    }
}

let pending = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => {
    pending += chunk;
    let cut;
    while ((cut = pending.indexOf("\n")) >= 0) {
        const line = pending.slice(0, cut);
        pending = pending.slice(cut + 1);
        if (line.trim() === "") {
            continue;
        }
        const m = JSON.parse(line);
        try {
            send({ ...answer(m), id: m.id });
        } catch (e) {
            send({ error: `${m.type} failed: ${e}`, id: m.id });
        }
    }
});
process.stdin.on("end", () => {
    open = false;
});

// The ticker: one frame each time a frame's period has passed. setImmediate
// lets stdin's messages in between frames. A tick that comes late runs one
// frame and starts the count again, as a game that hitched runs one long frame.
function tick() {
    if (!open) {
        return;
    }
    const now = performance.now();
    if (now >= nextFrameAt) {
        onFrame();
        nextFrameAt = now - nextFrameAt > framePeriodMs ? now + framePeriodMs
            : nextFrameAt + framePeriodMs;
    }
    setImmediate(tick);
}
tick();
