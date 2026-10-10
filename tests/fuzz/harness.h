// The one body every fuzz target runs, and the six parsers it feeds.
//
// A fuzzer hands Hydra random bytes. A parser that turns them away with an
// exception is doing its job; anything else (a crash, an AddressSanitizer
// report, a hang, an abort) is a bug. Each tests/fuzz/fuzz_<name>.cpp passes
// its bytes to run_one with one entry from kTargets. A doctest can include
// this header too and replay saved crash files through the same body, so what
// counts as "rejected properly" is written once, here.
//
// The parsers run at the batch's default settings (app::AnalysisSettings), so
// the fuzzer reads files the way an untouched install does.

#ifndef HYDRA_TESTS_FUZZ_HARNESS_H
#define HYDRA_TESTS_FUZZ_HARNESS_H

#include <cstddef>
#include <cstdint>
#include <exception>
#include <vector>

#include "app/analysis.h"
#include "audio/decode.h"
#include "image/decode.h"
#include "parse/song.h"

namespace hydra::fuzz {

using Parse = void (*)(const std::vector<uint8_t>& bytes);

inline const app::AnalysisSettings& default_settings() {
    static const app::AnalysisSettings settings;
    return settings;
}

// The four byte loaders in parse/song.h share one signature, so the settings
// are handed over in one place.
using SongLoader = Song (*)(const std::vector<uint8_t>&, bool, bool, Difficulty,
                            const core::Rules&, bool);
inline void parse_song(SongLoader load, const std::vector<uint8_t>& bytes) {
    const app::AnalysisSettings& s = default_settings();
    (void)load(bytes, s.prodrums, s.bass2x, s.difficulty, s.rules, s.noteshuffle);
}
inline void parse_chart(const std::vector<uint8_t>& bytes) { parse_song(load_songbytes_chart, bytes); }
inline void parse_mid(const std::vector<uint8_t>& bytes) { parse_song(load_songbytes_mid, bytes); }
inline void parse_sng(const std::vector<uint8_t>& bytes) { parse_song(load_songbytes_sng, bytes); }
inline void parse_srb(const std::vector<uint8_t>& bytes) { parse_song(load_songbytes_srb, bytes); }
// decode_audio sniffs the container itself (sniff_format) before it picks a
// decoder, so this one target covers the sniffer too.
inline void parse_audio(const std::vector<uint8_t>& bytes) {
    (void)audio::decode_audio(bytes);
}
// decode_image reports failure with an empty image, not an exception.
inline void parse_image(const std::vector<uint8_t>& bytes) {
    (void)image::decode_image(bytes);
}

// Each target's name, which is also its exe's suffix (fuzz_<name>) and its
// folder name for saved files.
struct Target {
    const char* name;
    Parse parse;
};
inline constexpr Target kTargets[] = {
    {"chart", parse_chart}, {"mid", parse_mid},     {"sng", parse_sng},
    {"srb", parse_srb},     {"audio", parse_audio}, {"image", parse_image},
};

// The harness body. The bytes are copied into a vector of their own size, so
// a read past the end lands outside the allocation where AddressSanitizer
// sees it. Returns 0, as libFuzzer requires.
inline int run_one(Parse parse, const uint8_t* data, std::size_t size) {
    std::vector<uint8_t> bytes(data, data + size);
    try {
        parse(bytes);
    } catch (const std::exception&) {
        // A bad file reported properly: the expected outcome.
    }
    return 0;
}

}  // namespace hydra::fuzz

#endif  // HYDRA_TESTS_FUZZ_HARNESS_H
