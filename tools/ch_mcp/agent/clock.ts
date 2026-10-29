// The clock agent: reads GameManager's song clock by field name and sends one
// sample per tick until the cap the Python side passes in.
//
// It reads only. It writes no game memory and calls no game method; the
// no-writes check in tests/test_frida_attach.py keeps it that way.
//
// frida_attach.run_agent posts the start message with the cap, writes every
// message this agent sends, and ends the run on the signal ends_run reads.

import "frida-il2cpp-bridge";

// The sampling interval the user approved (plan 2026-10-28, question 5).
const SAMPLE_MS = 250;

// The image and class this agent reads, as the Il2CppDumper dump names them.
const ASSEMBLY_NAME = "CloneHero";
const CLASS_NAME = "GameManager";

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
    const assembly = Il2Cpp.domain.tryAssembly(ASSEMBLY_NAME);
    if (assembly === null) {
        send({ error: `${ASSEMBLY_NAME} assembly not found` });
        return;
    }
    const klass = assembly.image.tryClass(CLASS_NAME);
    if (klass === null) {
        send({ error: `${CLASS_NAME} not found` });
        return;
    }
    const live = Il2Cpp.gc.choose(klass);
    if (live.length === 0) {
        send({ error: `no live ${CLASS_NAME}` });
        return;
    }
    const gm = live[0];
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
            send(readSample(gm, live.length));
            samples += 1;
        } catch (e) {
            clearInterval(timer);
            send({ error: `reading ${CLASS_NAME} failed: ${e}` });
        }
    }, SAMPLE_MS);
}

recv("start", (message: { cap_s: number }) => {
    Il2Cpp.perform(() => run(message.cap_s)).catch((e) => {
        send({ error: `Il2Cpp.perform failed: ${e}` });
    });
});
