// The state agent: answers each read request with GameManager's state, read by
// field name. It stays loaded between requests, so a caller can ask again and
// again on one attach.
//
// It reads only. It writes no game memory and calls no game method; the
// no-writes check in tests/test_frida_attach.py keeps it that way.
//
// frida_attach.LiveAgent posts each read with an id and waits for the reply
// that carries the same id, so a reply that comes after its caller gave up is
// never taken as the answer to a later read. Every request gets exactly one
// reply: the state, or an error.

import "frida-il2cpp-bridge";
import { lookUpGameManager, GAME_MANAGER } from "./gamemanager.js";

interface ReadRequest {
    type: "read";
    id: number;
}

// No live GameManager is the menu, not a fault, so it comes back as
// `found: false` with lookUpGameManager's reason, never as an error. Both
// replies carry `destroyed`, the destroyed GameManagers the lookup skipped,
// so a read shows when the skip decided the answer.
function readState() {
    // Found on every read, not once: a new song can build a new GameManager,
    // and one kept from an earlier read would go on reporting the old song.
    // This walks the whole heap each time; the caller's read cap is the stop.
    const found = lookUpGameManager();
    if (!("gm" in found)) {
        return { found: false, reason: found.reason, destroyed: found.destroyed };
    }
    const { gm, instances, destroyed } = found;
    return {
        found: true,
        instances,
        destroyed,
        songTime: gm.field<number>("songTime").value,
        isPaused: gm.field<boolean>("isPaused").value,
        isSongOver: gm.field<boolean>("isSongOver").value,
        isSongPlaying: gm.field<boolean>("isSongPlaying").value,
        songLoaded: gm.field<boolean>("songLoaded").value,
    };
}

// recv hands over one message per registration, so the handler registers
// itself again once its reply is sent. Requests that arrive meanwhile wait in
// Frida's queue and are answered in order.
function listen() {
    recv("read", (message: ReadRequest) => {
        const id = message.id;
        Il2Cpp.perform(() => {
            try {
                send({ ...readState(), id });
            } catch (e) {
                send({ error: `reading ${GAME_MANAGER} failed: ${e}`, id });
            }
        }).catch((e) => {
            send({ error: `Il2Cpp.perform failed: ${e}`, id });
        }).finally(listen);
    });
}

listen();
