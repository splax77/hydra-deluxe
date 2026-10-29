// The one place the agents find the game's song clock: the live GameManager.
// clock.ts and trace.ts both call it.
//
// It reads only: it looks the class up by name and asks the garbage collector
// for its live instances. It writes no game memory and calls no game method.

import "frida-il2cpp-bridge";

// The image and class that hold the song clock, as the Il2CppDumper dump names
// them.
const ASSEMBLY_NAME = "CloneHero";
const CLASS_NAME = "GameManager";

export const GAME_MANAGER = CLASS_NAME;

export interface LiveGameManager {
    gm: Il2Cpp.Object;
    instances: number;
}

// The first live GameManager and how many there are, or the error text an
// agent sends when there is none. Call it inside Il2Cpp.perform. The choose
// call walks the whole heap, so call it once per run, not per tick.
export function findGameManager(): LiveGameManager | string {
    const assembly = Il2Cpp.domain.tryAssembly(ASSEMBLY_NAME);
    if (assembly === null) {
        return `${ASSEMBLY_NAME} assembly not found`;
    }
    const klass = assembly.image.tryClass(CLASS_NAME);
    if (klass === null) {
        return `${CLASS_NAME} not found`;
    }
    const live = Il2Cpp.gc.choose(klass);
    if (live.length === 0) {
        return `no live ${CLASS_NAME}`;
    }
    return { gm: live[0], instances: live.length };
}
