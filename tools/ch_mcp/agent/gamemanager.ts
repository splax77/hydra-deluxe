// The one place the agents find the game's song clock: the live GameManager.
// state.ts calls lookUpGameManager; clock.ts and trace.ts call findGameManager,
// which is the same lookup with its failure as one line of text.
//
// It reads by name: it looks the class up, asks the garbage collector for the
// instances it holds, and reads one field of each. It also owns isDestroyed,
// the one test of whether a Unity object is destroyed, which menu.ts calls for
// the menu classes.

import "frida-il2cpp-bridge";

// The image and class that hold the song clock, as the Il2CppDumper dump names
// them.
const ASSEMBLY_NAME = "CloneHero";
const CLASS_NAME = "GameManager";

// The field of UnityEngine.Object, the base class of every MonoBehaviour,
// that holds the native object behind the managed one (the dump's class
// Object in namespace UnityEngine). Unity clears it when it destroys the
// object, while the managed object can stay on the heap until the collector
// runs, so the collector's list can hold GameManagers the game has destroyed.
const NATIVE_OBJECT_FIELD = "m_CachedPtr";

export const GAME_MANAGER = CLASS_NAME;

export interface LiveGameManager {
    gm: Il2Cpp.Object;
    // How many of the collector's GameManagers are alive, and how many it
    // still holds that the game has destroyed.
    instances: number;
    destroyed: number;
}

export interface NoLiveGameManager {
    reason: string;
    destroyed: number;
}

export function isDestroyed(obj: Il2Cpp.Object): boolean {
    return obj.field<NativePointer>(NATIVE_OBJECT_FIELD).value.isNull();
}

// The first live GameManager, how many are alive and how many destroyed ones
// were skipped; or, when none is alive, why, with the same destroyed count.
// Call it inside Il2Cpp.perform. Each call walks the whole heap (the choose
// call), so it is not cheap. clock.ts and trace.ts call it once per run;
// state.ts calls it on every read, and live step L1 of the plan
// 2026-10-28-ch-mcp-server.md timed that read.
export function lookUpGameManager(): LiveGameManager | NoLiveGameManager {
    const assembly = Il2Cpp.domain.tryAssembly(ASSEMBLY_NAME);
    if (assembly === null) {
        return { reason: `${ASSEMBLY_NAME} assembly not found`, destroyed: 0 };
    }
    const klass = assembly.image.tryClass(CLASS_NAME);
    if (klass === null) {
        return { reason: `${CLASS_NAME} not found`, destroyed: 0 };
    }
    const held = Il2Cpp.gc.choose(klass);
    const live = held.filter((obj) => !isDestroyed(obj));
    const destroyed = held.length - live.length;
    if (live.length === 0) {
        return { reason: `no live ${CLASS_NAME}`, destroyed };
    }
    return { gm: live[0], instances: live.length, destroyed };
}

// lookUpGameManager, with its failure as the error text an agent sends.
export function findGameManager(): LiveGameManager | string {
    const found = lookUpGameManager();
    return "gm" in found ? found : found.reason;
}
