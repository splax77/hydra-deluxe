// Files the fuzzer once broke a parser with, kept as permanent tests.
//
// A crash file from a fuzz run goes into testdata/fuzz/<target>/ (README.md
// there says how). The case below feeds every file it finds through the same
// harness body the fuzz exes run (tests/fuzz/harness.h), so a file that
// crashed a parser keeps failing here until the parser is fixed, and a new
// file needs no code. An empty or missing folder passes.
//
// The second case writes the fuzz seeds for tools/fuzz_seeds.ps1, because the
// only builders of the .sng and .srb containers are the test fixtures.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "corpus_util.h"
#include "env_util.h"
#include "fuzz/harness.h"
#include "parse/chart_files.h"
#include "sng_util.h"
#include "srb_util.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using hydra::fuzz::kTargets;

namespace {

const fs::path kFuzzRoot = fs::u8path(HYDRA_TESTDATA_DIR "/fuzz");

// A saved input, or something else in the folder (.gitkeep, notes) whose name
// starts with a dot or is the README.
bool is_saved_input(const fs::directory_entry& e) {
    if (!e.is_regular_file()) return false;
    const std::string name = e.path().filename().u8string();
    return !name.empty() && name[0] != '.' && name != "README.md";
}

}  // namespace

TEST_CASE("fuzz regressions: every file under testdata/fuzz/<target> is rejected without crashing") {
    int files = 0;
    for (const hydra::fuzz::Target& t : kTargets) {
        CAPTURE(t.name);
        const fs::path dir = kFuzzRoot / t.name;
        if (!fs::is_directory(dir)) continue;
        for (const fs::directory_entry& e : fs::directory_iterator(dir)) {
            if (!is_saved_input(e)) continue;
            const std::string path = e.path().u8string();
            INFO(path);
            const std::vector<uint8_t> bytes = hydra::read_file_bytes(path);
            CHECK(hydra::fuzz::run_one(t.parse, bytes.data(), bytes.size()) == 0);
            ++files;
        }
    }
    MESSAGE("replayed " << files << " saved fuzz inputs");

    // A folder that names no target would be skipped silently above.
    if (fs::is_directory(kFuzzRoot)) {
        for (const fs::directory_entry& e : fs::directory_iterator(kFuzzRoot)) {
            if (!e.is_directory()) continue;
            const std::string name = e.path().filename().u8string();
            bool known = false;
            for (const hydra::fuzz::Target& t : kTargets) known = known || name == t.name;
            CHECK_MESSAGE(known, "testdata/fuzz/" << name << " is not a fuzz target");
        }
    }
}

// Runs only when tools/fuzz_seeds.ps1 sets HYDRA_FUZZ_SEED_DIR; otherwise it
// does nothing. It writes one seed folder per target, so the folders are the
// list of targets the workflow fuzzes (.github/workflows/fuzz.yml reads them),
// and a target in kTargets that gets no seeds fails here.
TEST_CASE("fuzz seeds: write every target's seed folder when HYDRA_FUZZ_SEED_DIR is set") {
    const std::optional<std::string> out = read_env("HYDRA_FUZZ_SEED_DIR");
    if (!out || out->empty()) return;
    const fs::path root = fs::u8path(*out);
    const auto put = [&](const std::string& target, const std::string& name,
                         const std::vector<uint8_t>& bytes) {
        fs::create_directories(root / target);
        testtemp::write_bytes((root / target / name).u8string(), bytes);
    };

    // The notes seeds are the corpus charts themselves, split by format the
    // way the app splits them (chart_paths, chart_format_of).
    struct NotesTarget {
        const char* target;  // also the seed files' extension
        hydra::ChartFormat format;
    };
    for (const NotesTarget& n : {NotesTarget{"chart", hydra::ChartFormat::Chart},
                                 NotesTarget{"mid", hydra::ChartFormat::Mid}}) {
        int i = 0;
        for (const std::string& path : corpus::chart_paths()) {
            if (hydra::chart_format_of(path) != n.format) continue;
            put(n.target, std::to_string(i++) + "." + n.target, hydra::read_file_bytes(path));
        }
    }

    // Every file in the audio fixture folder, and the PNG the preview tests
    // compare against.
    for (const fs::directory_entry& e : fs::directory_iterator(fs::u8path(HYDRA_TESTDATA_DIR "/audio")))
        if (e.is_regular_file())
            put("audio", e.path().filename().u8string(), hydra::read_file_bytes(e.path().u8string()));
    for (const fs::directory_entry& e : fs::directory_iterator(fs::u8path(HYDRA_TESTDATA_DIR "/preview")))
        if (e.is_regular_file() && e.path().extension() == ".png")
            put("image", e.path().filename().u8string(), hydra::read_file_bytes(e.path().u8string()));

    // The containers wrap the first corpus chart of each format, and each seed
    // must load, so the fuzzer starts from files the parser accepts.
    struct Notes {
        const char* ext;
        const char* filename;
    };
    for (Notes n : {Notes{".mid", "notes.mid"}, Notes{".chart", "notes.chart"}}) {
        CAPTURE(n.ext);
        const std::vector<uint8_t> notes =
            hydra::read_file_bytes(corpus::first_chart_with_suffix(n.ext));

        const std::vector<uint8_t> sng =
            testsng::make_sng({{"name", "Fuzz seed"}}, {{n.filename, notes}});
        CHECK_NOTHROW(hydra::fuzz::parse_sng(sng));
        put("sng", std::string("seed") + n.ext + ".sng", sng);

        const std::vector<uint8_t> srb =
            testsrb::make_srb(testsrb::make_metadata(n.filename), notes);
        CHECK_NOTHROW(hydra::fuzz::parse_srb(srb));
        put("srb", std::string("seed") + n.ext + ".srb", srb);
    }

    for (const hydra::fuzz::Target& t : kTargets) {
        int files = 0;
        if (fs::is_directory(root / t.name))
            for (const fs::directory_entry& e : fs::directory_iterator(root / t.name))
                files += e.is_regular_file() ? 1 : 0;
        CHECK_MESSAGE(files > 0, "no seeds written for the fuzz target " << t.name);
    }
}
