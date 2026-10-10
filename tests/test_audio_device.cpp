// Tests for audio/device: the one place Hydra opens a real output device.
// miniaudio is built with only its WASAPI backend (CMakeLists.txt), so this
// proves that backend still opens, starts and plays the default device. It
// needs a machine with an audio output, which every Hydra dev machine has.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>

#include "audio/device.h"
#include "wait_util.h"

using namespace hydra::audio;

TEST_CASE("PreviewAudioDevice opens and starts the default output device") {
    // The real device, whatever an earlier case set; the guard puts the old
    // value back when the case ends.
    struct HeadlessRestore {
        bool old;
        ~HeadlessRestore() { set_headless(old); }
    } restore{set_headless(false)};

    std::atomic<int64_t> pulls{0};
    std::unique_ptr<PreviewAudioDevice> device;
    REQUIRE_NOTHROW(device = std::make_unique<PreviewAudioDevice>(
                        2, 48000, [&pulls](float* out, int64_t frames) {
                            std::fill(out, out + frames * 2, 0.0f);
                            pulls.fetch_add(1);
                            return int64_t{0};
                        }));
    REQUIRE_NOTHROW(device->start());  // throws if the device won't start
    REQUIRE(device->started());

    // A capped wait for the device thread's first pull. The 2 s cap is the
    // user's number (test fidelity plan, Task 7): a hang detector, not a
    // timing test.
    testwait::wait_until([&pulls] { return pulls.load() != 0; },
                         "the device thread to pull audio (it never did)",
                         std::chrono::seconds(2));

    device.reset();  // stops the device and waits for the callback first
}
