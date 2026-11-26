// The state agent: answers each read request with GameManager's state, read by
// field name, and the menu block menu.ts reads. It stays loaded between
// requests, so a caller can ask again and again on one attach.
//
// It reads fields by name; the only game methods it calls are the ones
// menu.ts names. The checks in tests/test_frida_attach.py keep it that way.
// As it loads, it places menu.ts's one hook, which records the player screens
// the menu read reports (watchPlayerScreens says why).
//
// frida_attach.LiveAgent posts each read with an id and waits for the reply
// that carries the same id, so a reply that comes after its caller gave up is
// never taken as the answer to a later read. Every request gets exactly one
// reply: the state, or an error.

import "frida-il2cpp-bridge";
import { lookUpGameManager, GAME_MANAGER } from "./gamemanager.js";
import { playerScreenWatchFailed, readMenu, watchPlayerScreens } from "./menu.js";

interface ReadRequest {
    type: "read";
    id: number;
}

// The song_speed setting, as the Il2CppDumper dump of build 6142 names it.
// The settings class (dump.cs line 12599, in CloneHero.dll) holds it in its
// static field at 0x10 (line 12604); settings_load_374D50.asm lines 2016 to
// 2030 build it there with the text 'song_speed'. Its value is the first field
// its base class declares (dump.cs line 533657). The dump prints 0x0 for every
// field of that generic class, so the agent sends the offset the game itself
// gives the field, which ch_state shows beside the value; the launch agent's
// setter writes the value at launch_patch.json's song_speed value_offset.
const SETTINGS_ASSEMBLY = "CloneHero";
const SETTINGS_CLASS = "ʹʺʽˁʽˁˀʼʶʷʼ";
const SONG_SPEED_STATIC = "ʳʷʴʺʳʹʷʾʲʷʾ";
const SONG_SPEED_VALUE = "ʾʶʲʼʺʳʶʿʲʻʲ";

// The song_speed setting's value and the offset of the field it was read
// from, or the error that stopped the read. A failed read never fails the
// GameManager read beside it.
function songSpeedBlock() {
    try {
        const assembly = Il2Cpp.domain.tryAssembly(SETTINGS_ASSEMBLY);
        if (assembly === null) {
            return { songSpeed: `not read: ${SETTINGS_ASSEMBLY} assembly not found` };
        }
        const klass = assembly.image.tryClass(SETTINGS_CLASS);
        if (klass === null) {
            return { songSpeed: `not read: ${SETTINGS_CLASS} not found` };
        }
        const setting = klass.field<Il2Cpp.Object>(SONG_SPEED_STATIC).value;
        if (setting.isNull()) {
            return { songSpeed: `not read: ${SETTINGS_CLASS}.${SONG_SPEED_STATIC} is null` };
        }
        const value = setting.field<number>(SONG_SPEED_VALUE);
        return {
            songSpeed: value.value,
            songSpeedField: `${SONG_SPEED_VALUE} at +0x${value.offset.toString(16)}`,
        };
    } catch (e) {
        return { songSpeed: `not read: ${e}` };
    }
}

// The menu block, or the error that stopped it. A failed menu read never
// fails the GameManager read beside it.
function menuBlock() {
    try {
        return readMenu();
    } catch (e) {
        return { error: `reading the menus failed: ${e}` };
    }
}

// No live GameManager is the menu, not a fault, so it comes back as
// `found: false` with lookUpGameManager's reason, never as an error. Both
// replies carry `destroyed`, the destroyed GameManagers the lookup skipped,
// so a read shows when the skip decided the answer, and `menu`, menu.ts's
// block.
function readState() {
    // Found on every read, not once: a new song can build a new GameManager,
    // and one kept from an earlier read would go on reporting the old song.
    // This walks the whole heap each time; the caller's read cap is the stop.
    const found = lookUpGameManager();
    const menu = menuBlock();
    const speed = songSpeedBlock();
    if (!("gm" in found)) {
        return { found: false, reason: found.reason, destroyed: found.destroyed, menu, ...speed };
    }
    const { gm, instances, destroyed } = found;
    return {
        found: true,
        instances,
        destroyed,
        menu,
        songTime: gm.field<number>("songTime").value,
        isPaused: gm.field<boolean>("isPaused").value,
        isSongOver: gm.field<boolean>("isSongOver").value,
        isSongPlaying: gm.field<boolean>("isSongPlaying").value,
        songLoaded: gm.field<boolean>("songLoaded").value,
        // GameManager.Awake sets it to the speed ratio (TM-57 on docs/clone-hero/03-timing.md).
        practiceSpeed: gm.field<number>("practiceSpeed").value,
        ...speed,
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

// The hook is asked for as the agent loads, ahead of every read.
// A failure shows in every menu read after it, as the player screen's reason.
Il2Cpp.perform(watchPlayerScreens).catch(playerScreenWatchFailed);
listen();
