// The clock agent: reads GameManager's song clock by field name and sends one
// sample per tick until the cap the Python side passes in.
//
// It reads only. It writes no game memory and calls no game method; the
// no-writes check in tests/test_frida_attach.py keeps it that way.
//
// frida_attach.run_agent posts the start message with the cap, writes every
// message this agent sends, and ends the run on the signal ends_run reads.

import "frida-il2cpp-bridge";
import { findGameManager, GAME_MANAGER } from "./gamemanager.js";

// The sampling interval the user approved (plan 2026-10-28, question 5).
const SAMPLE_MS = 250;

function readSample(gm: Il2Cpp.Object, instances: number) {
    return {
        t: Date.now(),
        songTime: gm.field<number>("songTime").value,
        isSongOver: gm.field<boolean>("isSongOver").value,
        isSongPlaying: gm.field<boolean>("isSongPlaying").value,
        isPaused: gm.field<boolean>("isPaused").value,
        instances,
    };
}

function run(capS: number) {
    // Found once, not per tick: the lookup walks the whole heap. If the game
    // builds a new GameManager for a restarted song, this one keeps reporting
    // its last values and `instances` stays at the count seen here.
    // frida_attach.summarize_clock counts a restart that reuses this
    // GameManager as songTime going backwards.
    const found = findGameManager();
    if (typeof found === "string") {
        send({ error: found });
        return;
    }
    const { gm, instances } = found;
    const startMs = Date.now();
    const capMs = capS * 1000;
    let samples = 0;
    const timer = setInterval(() => {
        try {
            if (Date.now() - startMs >= capMs) {
                clearInterval(timer);
                send({ done: "cap", samples });
                return;
            }
            send(readSample(gm, instances));
            samples += 1;
        } catch (e) {
            clearInterval(timer);
            send({ error: `reading ${GAME_MANAGER} failed: ${e}` });
        }
    }, SAMPLE_MS);
}

recv("start", (message: { cap_s: number }) => {
    Il2Cpp.perform(() => run(message.cap_s)).catch((e) => {
        send({ error: `Il2Cpp.perform failed: ${e}` });
    });
});
