// Files the fuzzer once broke a parser with, kept as permanent tests.
//
// A crash file from a fuzz run goes into testdata/fuzz/<target>/ (README.md
// there says how). The case below feeds every file it finds through the same
// harness body the fuzz exes run (tests/fuzz/harness.h), so a file that
// crashed a parser keeps failing here until the parser is fixed, and a new
// file needs no code. Every target's folder must be there; an empty one
// passes.
//
// The seed cases write the fuzz seeds for tools/fuzz_seeds.ps1, because the
// only builders of the .sng and .srb containers are the test fixtures. They
// run in every suite run too, writing to a temp folder, so a seed that fails
// to load or a target with no seeds fails there and not only in the fuzz
// workflow.

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

const fs::path kFuzzRoot = fs::u8path(corpus::testdata_root() + "/fuzz");

// A saved input, or something else in the folder (.gitkeep, notes) whose name
// starts with a dot or is the README.
bool is_saved_input(const fs::directory_entry& e) {
    if (!e.is_regular_file()) return false;
    const std::string name = e.path().filename().u8string();
    return !name.empty() && name[0] != '.' && name != "README.md";
}

// One container seed: the fuzz target it seeds, its file name, its bytes.
struct ContainerSeed {
    std::string target;
    std::string name;
    std::vector<uint8_t> bytes;
};

// The .sng and .srb wrappers of the first corpus chart of each format, so the
// fuzzer starts from container files the parser accepts. A corpus with no
// chart of a format throws (first_chart_with_suffix), never skips.
std::vector<ContainerSeed> container_seeds() {
    struct Notes {
        const char* ext;
        const char* filename;
    };
    std::vector<ContainerSeed> out;
    for (Notes n : {Notes{".mid", "notes.mid"}, Notes{".chart", "notes.chart"}}) {
        const std::vector<uint8_t> notes =
            hydra::read_file_bytes(corpus::first_chart_with_suffix(n.ext));
        ContainerSeed sng;
        sng.target = "sng";
        sng.name = std::string("seed") + n.ext + ".sng";
        sng.bytes = testsng::make_sng({{"name", "Fuzz seed"}}, {{n.filename, notes}});
        out.push_back(sng);
        ContainerSeed srb;
        srb.target = "srb";
        srb.name = std::string("seed") + n.ext + ".srb";
        srb.bytes = testsrb::make_srb(testsrb::make_metadata(n.filename), notes);
        out.push_back(srb);
    }
    return out;
}

}  // namespace

TEST_CASE("fuzz regressions: every file under testdata/fuzz/<target> is rejected without crashing") {
    int files = 0;
    for (const hydra::fuzz::Target& t : kTargets) {
        CAPTURE(t.name);
        const fs::path dir = kFuzzRoot / t.name;
        // A missing folder would replay nothing for its target and still pass.
        CHECK_MESSAGE(fs::is_directory(dir), "testdata/fuzz/" << t.name << " is missing");
        if (!fs::is_directory(dir)) continue;
        for (const fs::directory_entry& e : fs::directory_iterator(dir)) {
            if (!is_saved_input(e)) continue;
            const std::string path = e.path().u8string();
            INFO(path);
            const std::vector<uint8_t> bytes = hydra::read_file_bytes(path);
            // run_one turns away a std::exception, the proper rejection; any
            // other exception escapes it and fails here. A crash ends the run.
            CHECK_NOTHROW(hydra::fuzz::run_one(t.parse, bytes.data(), bytes.size()));
            ++files;
        }
    }
    MESSAGE("replayed " << files << " saved fuzz inputs");
    // An emptied testdata/fuzz would otherwise pass with nothing replayed.
    CHECK(files > 0);

    // A folder that names no target would be skipped silently above.
    REQUIRE(fs::is_directory(kFuzzRoot));
    for (const fs::directory_entry& e : fs::directory_iterator(kFuzzRoot)) {
        if (!e.is_directory()) continue;
        const std::string name = e.path().filename().u8string();
        bool known = false;
        for (const hydra::fuzz::Target& t : kTargets) known = known || name == t.name;
        CHECK_MESSAGE(known, "testdata/fuzz/" << name << " is not a fuzz target");
    }
}

TEST_CASE("fuzz seeds: the container seeds of the first corpus chart of each format load") {
    const std::vector<ContainerSeed> seeds = container_seeds();
    REQUIRE_FALSE(seeds.empty());
    for (const ContainerSeed& s : seeds) {
        CAPTURE(s.name);
        if (s.target == "sng") {
            CHECK_NOTHROW(hydra::fuzz::parse_sng(s.bytes));
        } else {
            CHECK(s.target == "srb");
            CHECK_NOTHROW(hydra::fuzz::parse_srb(s.bytes));
        }
    }
}

// Writes one seed folder per target: to HYDRA_FUZZ_SEED_DIR when
// tools/fuzz_seeds.ps1 sets it, and otherwise to a temp folder that is
// removed after. The folders are the list of targets the workflow fuzzes
// (.github/workflows/fuzz.yml reads them), so a target in kTargets that gets
// no seeds fails here in every run.
TEST_CASE("fuzz seeds: every target gets a seed folder") {
    const std::optional<std::string> env_dir = read_env("HYDRA_FUZZ_SEED_DIR");
    const bool to_temp = !env_dir || env_dir->empty();
    std::string out_dir;
    if (to_temp) {
        testtemp::remove_dir(testtemp::temp_dir("fuzz_seeds"));
        out_dir = testtemp::temp_dir("fuzz_seeds");
    } else {
        out_dir = *env_dir;
    }
    const fs::path root = fs::u8path(out_dir);
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
    for (const fs::directory_entry& e : fs::directory_iterator(fs::u8path(corpus::testdata_root() + "/audio")))
        if (e.is_regular_file())
            put("audio", e.path().filename().u8string(), hydra::read_file_bytes(e.path().u8string()));
    for (const fs::directory_entry& e : fs::directory_iterator(fs::u8path(corpus::testdata_root() + "/preview")))
        if (e.is_regular_file() && e.path().extension() == ".png")
            put("image", e.path().filename().u8string(), hydra::read_file_bytes(e.path().u8string()));

    // The containers, which the case above checks load.
    for (const ContainerSeed& s : container_seeds()) put(s.target, s.name, s.bytes);

    for (const hydra::fuzz::Target& t : kTargets) {
        int files = 0;
        if (fs::is_directory(root / t.name))
            for (const fs::directory_entry& e : fs::directory_iterator(root / t.name))
                files += e.is_regular_file() ? 1 : 0;
        CHECK_MESSAGE(files > 0, "no seeds written for the fuzz target " << t.name);
    }
    if (to_temp) testtemp::remove_dir(out_dir);
}
