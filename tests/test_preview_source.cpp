// Tests for app/preview_source: resolving a chart's notes and locating its
// audio across the three source kinds. No .sng/.srb ship in the corpus, so the
// container cases fabricate one byte-for-byte the way the loaders read it (a
// real corpus chart stands in for the notes), then confirm the audio entries
// the note loaders skip come back intact.

#define _CRT_SECURE_NO_WARNINGS

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_source.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "multidiff_chart.h"
#include "parse/song.h"
#include "sng_util.h"
#include "srb_util.h"
#include "temp_util.h"

using namespace hydra;
using namespace hydra::app;

namespace {

using testtemp::write_bytes;

// This process's scratch folder for the fixtures (testtemp::temp_dir).
std::string fixture_dir() { return testtemp::temp_dir("prevsrc"); }

std::string make_subdir(const std::string& name) {
    std::string dir = fixture_dir() + "\\" + name;
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);
    return dir;
}

std::vector<uint8_t> bytes_of(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

// .sng fixtures come from testsng::make_sng (tests/sng_util.h).

// ---- .srb fixture --------------------------------------------------------

// An .srb (tests/srb_util.h) whose notes entry is `notes_filename`, with one
// trailing stream per `extra`, like a real bundle's audio and art streams.
std::vector<uint8_t> make_srb(const std::vector<uint8_t>& notes,
                              const std::vector<std::vector<uint8_t>>& extra,
                              const std::string& notes_filename = "notes.mid") {
    return testsrb::make_srb(testsrb::make_metadata(notes_filename), notes, extra);
}

// multidiff's chart with an Offset line in its [Song] section.
std::vector<uint8_t> chart_with_offset(const std::string& seconds) {
    std::string s = multidiff::chart_text();
    const std::string anchor = "  Resolution = 192\n";
    const size_t at = s.find(anchor);
    REQUIRE(at != std::string::npos);
    s.insert(at + anchor.size(), "  Offset = " + seconds + "\n");
    return bytes_of(s);
}

}  // namespace

TEST_CASE("is_audio_filename / looks_like_audio recognize the formats") {
    CHECK(is_audio_filename("song.ogg"));
    CHECK(is_audio_filename("DRUMS.OPUS"));
    CHECK(is_audio_filename("x.mp3"));
    CHECK(is_audio_filename("y.wav"));
    CHECK(is_audio_filename("z.flac"));
    CHECK_FALSE(is_audio_filename("album.png"));
    CHECK_FALSE(is_audio_filename("notes.chart"));

    // The decoder's rule (audio::sniff_format): an Ogg stream counts only when
    // its first page names a codec the decoder has, and a RIFF only when it
    // holds a WAVE.
    CHECK_FALSE(looks_like_audio(bytes_of("OggS\x00\x02")));
    CHECK(looks_like_audio(bytes_of("OggS OpusHead")));
    CHECK(looks_like_audio(bytes_of("OggS vorbis")));
    CHECK(looks_like_audio(bytes_of("RIFF....WAVE")));
    CHECK_FALSE(looks_like_audio(bytes_of("RIFF....AVI ")));
    CHECK(looks_like_audio(bytes_of("fLaC")));
    CHECK(looks_like_audio(bytes_of("ID3\x03")));
    CHECK(looks_like_audio({0xFF, 0xFB, 0x90, 0x00}));  // MP3 frame sync
    CHECK_FALSE(looks_like_audio(bytes_of("\x89PNG\r\n")));
    CHECK_FALSE(looks_like_audio({}));
}

TEST_CASE("is_song_stem: an audio file that is not a standalone preview clip") {
    CHECK(is_song_stem("song.ogg"));
    CHECK_FALSE(is_song_stem("Preview.OGG"));
    CHECK_FALSE(is_song_stem("preview.opus"));
    CHECK(is_song_stem("preview2.ogg"));
    CHECK_FALSE(is_song_stem("album.jpg"));
    CHECK_FALSE(is_song_stem("notes.chart"));
}

TEST_CASE("find_loose_audio: stems beside the notes, preview and art excluded") {
    std::string dir = make_subdir("loose");
    write_bytes(dir + "\\notes.chart", bytes_of("[Song]"));
    write_bytes(dir + "\\song.ogg", bytes_of("OggS song"));
    write_bytes(dir + "\\drums.opus", bytes_of("OggS drums"));
    write_bytes(dir + "\\preview.ogg", bytes_of("OggS preview"));
    write_bytes(dir + "\\album.png", bytes_of("\x89PNG"));

    std::vector<PreviewAudioStem> stems = find_loose_audio(dir);
    REQUIRE(stems.size() == 2);  // preview and png excluded
    CHECK(stems[0].label == "drums");  // sorted by label
    CHECK(stems[1].label == "song");
    for (const PreviewAudioStem& s : stems) {
        CHECK(s.from_file());
        CHECK(s.bytes.empty());
    }
}

TEST_CASE("extract_sng_audio: audio entries come back XOR-demasked") {
    std::vector<uint8_t> song = bytes_of("OggS the song bytes");
    std::vector<uint8_t> drums = bytes_of("OggS the drums stem bytes!!");
    std::vector<uint8_t> notes = hydra::read_file_bytes(
        corpus::first_chart_with_suffix(".chart"));

    std::vector<uint8_t> sng =
        testsng::make_sng({}, {{"notes.chart", notes},
                               {"song.ogg", song},
                               {"drums.opus", drums},
                               {"preview.ogg", bytes_of("OggS vorbis preview clip")},
                               {"album.jpg", bytes_of("\xFF\xD8" "art")}});
    std::string path = fixture_dir() + "\\bundle.sng";
    write_bytes(path, sng);

    std::vector<PreviewAudioStem> stems = extract_sng_audio(path);
    // The notes and the .jpg are not audio; the preview clip is not part of
    // the song.
    REQUIRE(stems.size() == 2);
    CHECK(stems[0].label == "song");
    CHECK(stems[0].bytes == song);
    CHECK(stems[1].label == "drums");
    CHECK(stems[1].bytes == drums);

    // resolve_preview_source parses the embedded chart and returns the audio.
    PreviewSource src = resolve_preview_source(path, true, true);
    CHECK_FALSE(src.song.is_empty());
    CHECK(src.stems.size() == 2);
}

TEST_CASE("extract_srb_audio: trailing audio streams inflate; art is skipped") {
    std::vector<uint8_t> notes = hydra::read_file_bytes(
        corpus::first_chart_with_suffix(".mid"));
    std::vector<uint8_t> ogg = bytes_of("OggS OpusHead payload for the preview");
    std::vector<uint8_t> art(512, 0);
    std::memcpy(art.data(), "\x89PNG\r\n\x1a\n", 8);

    std::string path = fixture_dir() + "\\bundle.srb";
    write_bytes(path, make_srb(notes, {ogg, art}));

    std::vector<PreviewAudioStem> stems = extract_srb_audio(path);
    REQUIRE(stems.size() == 1);  // the PNG stream is not audio
    CHECK(stems[0].bytes == ogg);

    // A bundle with no trailing streams yields no audio, and never throws.
    std::string bare = fixture_dir() + "\\bare.srb";
    write_bytes(bare, make_srb(notes, {}));
    CHECK(extract_srb_audio(bare).empty());

    PreviewSource src = resolve_preview_source(path, true, true);
    CHECK_FALSE(src.song.is_empty());
    CHECK(src.stems.size() == 1);
}

TEST_CASE("extract_srb_audio: a stream with an audio magic but no codec is not a stem") {
    // The decoder could not open the bare Ogg stream, so it is not kept, and
    // the walk goes on to the tagged stream behind it.
    std::vector<uint8_t> notes = hydra::read_file_bytes(
        corpus::first_chart_with_suffix(".mid"));
    std::vector<uint8_t> bare_ogg = bytes_of("OggS");
    std::vector<uint8_t> tagged_ogg = bytes_of("OggS vorbis the real song");

    std::string path = fixture_dir() + "\\untagged.srb";
    write_bytes(path, make_srb(notes, {bare_ogg, tagged_ogg}));

    std::vector<PreviewAudioStem> stems = extract_srb_audio(path);
    REQUIRE(stems.size() == 1);
    CHECK(stems[0].bytes == tagged_ogg);
}

TEST_CASE("resolve_preview_source: a .srb with no extractable audio falls "
          "back to a loose file beside it") {
    // Mirrors a real Clone Hero bundle: extract_srb_audio finds nothing
    // (its audio is encrypted, outside the DEFLATE chain this reads), so
    // resolve_preview_source should still find a stem placed next to it.
    std::string dir = make_subdir("srb_fallback");
    std::vector<uint8_t> notes = hydra::read_file_bytes(
        corpus::first_chart_with_suffix(".mid"));
    write_bytes(dir + "\\bundle.srb", make_srb(notes, {}));
    write_bytes(dir + "\\song.ogg", bytes_of("OggS fallback audio"));

    CHECK(extract_srb_audio(dir + "\\bundle.srb").empty());

    PreviewSource src = resolve_preview_source(dir + "\\bundle.srb", true, true);
    CHECK_FALSE(src.song.is_empty());
    REQUIRE(src.stems.size() == 1);
    CHECK(src.stems[0].label == "song");
    CHECK(src.stems[0].from_file());
}

TEST_CASE("resolve_preview_source: a loose chart parses and finds its audio") {
    std::string dir = make_subdir("resolve_loose");
    std::vector<uint8_t> notes = hydra::read_file_bytes(
        corpus::first_chart_with_suffix(".chart"));
    write_bytes(dir + "\\notes.chart", notes);
    write_bytes(dir + "\\song.ogg", bytes_of("OggS audio"));

    PreviewSource src = resolve_preview_source(dir + "\\notes.chart", true, true);
    CHECK_FALSE(src.song.is_empty());
    REQUIRE(src.stems.size() == 1);
    CHECK(src.stems[0].label == "song");
    CHECK(src.stems[0].from_file());
}

TEST_CASE("containers pass the difficulty through to the chart inside") {
    // .sng and .srb are pure containers: they hand the extracted notes to the
    // .mid/.chart loader, so the difficulty has to survive the trip. The
    // fixture chart has four deliberately different difficulty sections.
    const std::vector<uint8_t> notes = multidiff::chart_bytes();

    std::string sng = fixture_dir() + "\\multidiff.sng";
    write_bytes(sng, testsng::make_sng({}, {{"notes.chart", notes},
                                            {"song.ogg", bytes_of("OggS audio")}}));

    std::string srb = fixture_dir() + "\\multidiff.srb";
    write_bytes(srb, make_srb(notes, {}, "notes.chart"));

    for (const std::string& path : {sng, srb}) {
        CHECK(resolve_preview_source(path, true, true).song.sequence.size() ==
              multidiff::kExpertChords);
        CHECK(resolve_preview_source(path, true, true, Difficulty::Hard)
                  .song.sequence.size() == multidiff::kHardChords);
        CHECK(resolve_preview_source(path, true, true, Difficulty::Easy)
                  .song.sequence.size() == multidiff::kEasyChords);
        // No [MediumDrums] section: an empty song, not a fallback.
        CHECK(resolve_preview_source(path, true, true, Difficulty::Medium)
                  .song.is_empty());
    }

    // The same, straight through the loaders (no preview resolution involved).
    CHECK(load_songpath_sng(sng, true, true, Difficulty::Hard).sequence.size() ==
          multidiff::kHardChords);
    CHECK(load_songpath_srb(srb, true, true, Difficulty::Hard).sequence.size() ==
          multidiff::kHardChords);
}

TEST_CASE("read_ini_delay_ms reads song.ini delay in milliseconds") {
    const std::string dir = make_subdir("ini_delay");
    const std::string ini = dir + "\\song.ini";

    write_bytes(ini, bytes_of("[song]\nname = X\ndelay = 1016\n"));
    REQUIRE(read_ini_delay_ms(ini).has_value());
    CHECK(*read_ini_delay_ms(ini) == doctest::Approx(1016.0));

    write_bytes(ini, bytes_of("[Song]\r\nDelay = -250\r\n"));  // any case, CRLF
    REQUIRE(read_ini_delay_ms(ini).has_value());
    CHECK(*read_ini_delay_ms(ini) == doctest::Approx(-250.0));

    write_bytes(ini, bytes_of("[song]\nname = X\n"));
    CHECK_FALSE(read_ini_delay_ms(ini).has_value());

    write_bytes(ini, bytes_of("[song]\ndelay = soon\n"));
    CHECK_FALSE(read_ini_delay_ms(ini).has_value());

    CHECK_FALSE(read_ini_delay_ms(dir + "\\missing.ini").has_value());
}

TEST_CASE("preview_audio_offset_ms combines delay and Offset") {
    CHECK(preview_audio_offset_ms(std::nullopt, std::nullopt) == doctest::Approx(0.0));
    CHECK(preview_audio_offset_ms(1016.0, std::nullopt) == doctest::Approx(1016.0));
    CHECK(preview_audio_offset_ms(std::nullopt, 0.25) == doctest::Approx(250.0));
    // Lunaris: delay = 0 in song.ini, Offset = 0.25 in the chart. A delay of 0
    // counts as unset, so the Offset applies (measured +248 ms at the game).
    CHECK(preview_audio_offset_ms(0.0, 0.25) == doctest::Approx(250.0));
    // The delay-500 copy of Lunaris from Task 17 step 1: both set, and the
    // nonzero delay replaces the Offset (measured +496 ms at the game).
    CHECK(preview_audio_offset_ms(500.0, 0.25) == doctest::Approx(500.0));
}

TEST_CASE("resolve_preview_source reads delay from a song.ini in any case") {
    const std::string dir = make_subdir("ini_delay_case");
    write_bytes(dir + "\\notes.chart", multidiff::chart_bytes());
    write_bytes(dir + "\\Song.INI", bytes_of("[song]\ndelay = 500\n"));
    const PreviewSource src =
        resolve_preview_source(dir + "\\notes.chart", true, true, Difficulty::Expert);
    CHECK(src.audio_offset_ms == doctest::Approx(500.0));
}

TEST_CASE("sng_delay_ms reads the delay key in any case") {
    const std::vector<uint8_t> notes = multidiff::chart_bytes();
    const std::vector<uint8_t> upper =
        testsng::make_sng({{"DELAY", "-120"}}, {{"notes.chart", notes}});
    REQUIRE(sng_delay_ms(upper).has_value());
    CHECK(*sng_delay_ms(upper) == doctest::Approx(-120.0));

    CHECK_FALSE(
        sng_delay_ms(testsng::make_sng({{"delay", "soon"}}, {{"notes.chart", notes}})).has_value());
    CHECK_FALSE(
        sng_delay_ms(testsng::make_sng({{"name", "X"}}, {{"notes.chart", notes}})).has_value());
    CHECK_FALSE(sng_delay_ms(testsng::make_sng({}, {{"notes.chart", notes}})).has_value());
}

TEST_CASE("resolve_preview_source: a .sng's metadata delay replaces the chart Offset") {
    const std::vector<uint8_t> notes = chart_with_offset("0.25");

    const std::string with_delay = fixture_dir() + "\\delay500.sng";
    write_bytes(with_delay,
                testsng::make_sng({{"name", "X"}, {"delay", "500"}}, {{"notes.chart", notes}}));
    CHECK(resolve_preview_source(with_delay, true, true).audio_offset_ms ==
          doctest::Approx(500.0));

    // A delay of 0 counts as unset (the Lunaris rule), whatever the key's case.
    const std::string zero_delay = fixture_dir() + "\\delay0.sng";
    write_bytes(zero_delay, testsng::make_sng({{"Delay", "0"}}, {{"notes.chart", notes}}));
    CHECK(resolve_preview_source(zero_delay, true, true).audio_offset_ms ==
          doctest::Approx(250.0));

    // No delay key at all: the chart's Offset applies.
    const std::string no_delay = fixture_dir() + "\\nodelay.sng";
    write_bytes(no_delay, testsng::make_sng({{"name", "X"}}, {{"notes.chart", notes}}));
    CHECK(resolve_preview_source(no_delay, true, true).audio_offset_ms ==
          doctest::Approx(250.0));
}

TEST_CASE("resolve_preview_source: a .srb uses its chart's Offset") {
    const std::string path = fixture_dir() + "\\offset.srb";
    write_bytes(path, make_srb(chart_with_offset("0.25"), {}, "notes.chart"));
    CHECK(resolve_preview_source(path, true, true).audio_offset_ms ==
          doctest::Approx(250.0));
}

namespace {

// Every Song field written out as text, so two parses compare in one check
// and a difference names the field.
std::string song_text(const Song& s) {
    std::string out;
    char num[64];
    auto add = [&](const char* fmt, auto... v) {
        std::snprintf(num, sizeof num, fmt, v...);
        out += num;
    };
    add("res %lld dyn %d\n", static_cast<long long>(s.tick_resolution()),
        s.dynamics_enabled ? 1 : 0);
    if (s.chart_offset_s) add("offset %.17g\n", *s.chart_offset_s);
    for (const auto& [t, v] : s.tpm_changes) add("tpm %lld %lld\n", static_cast<long long>(t), static_cast<long long>(v));
    for (const auto& [t, v] : s.bpm_changes) add("bpm %lld %.17g\n", static_cast<long long>(t), v);
    for (const auto& [t, v] : s.timesig_changes)
        add("ts %lld %d/%d\n", static_cast<long long>(t), v.first, v.second);
    for (const std::string& f : s.features) out += "feature " + f + "\n";
    for (const SongSection& sec : s.practice_sections) {
        add("section %lld ", static_cast<long long>(sec.tick));
        out += sec.name + "\n";
    }
    for (const SongTimestamp& ts : s.sequence) {
        add("%lld %.17g ", static_cast<long long>(ts.timecode.ticks()), ts.timecode.ms());
        out += ts.chord.code();
        add(" solo %d sp %d act %lld spstart %lld\n", ts.flag_solo ? 1 : 0, ts.flag_sp ? 1 : 0,
            static_cast<long long>(ts.activation_length.value_or(-1)),
            static_cast<long long>(ts.sp_phrase_start.value_or(-1)));
    }
    return out;
}

// read_file_bytes, counting how many times each path was read.
struct CountingReader {
    std::map<std::string, int> reads;
    FileBytesReader reader() {
        return [this](const std::string& path) {
            ++reads[path];
            return hydra::read_file_bytes(path);
        };
    }
};

}  // namespace

TEST_CASE("resolve_preview_source reads a .sng or .srb from disk once") {
    const std::vector<uint8_t> notes = chart_with_offset("0.25");
    const std::vector<uint8_t> ogg = bytes_of("OggS vorbis the one stem");

    const std::string sng = fixture_dir() + "\\readonce.sng";
    write_bytes(sng,
                testsng::make_sng({{"delay", "500"}}, {{"notes.chart", notes}, {"song.ogg", ogg}}));
    const std::string srb = fixture_dir() + "\\readonce.srb";
    write_bytes(srb, make_srb(notes, {ogg}, "notes.chart"));

    for (const std::string& path : {sng, srb}) {
        CAPTURE(path);
        CountingReader count;
        const PreviewSource src =
            resolve_preview_source_reading(count.reader(), path, true, true);
        CHECK(count.reads.size() == 1);
        CHECK(count.reads[path] == 1);
        CHECK_FALSE(src.song.is_empty());
        REQUIRE(src.stems.size() == 1);
        CHECK(src.stems[0].bytes == ogg);
    }

    // A loose chart's files are read by the loaders, not through the reader.
    const std::string dir = make_subdir("readonce_loose");
    write_bytes(dir + "\\notes.chart", notes);
    CountingReader loose;
    CHECK_FALSE(resolve_preview_source_reading(loose.reader(), dir + "\\notes.chart", true, true)
                    .song.is_empty());
    CHECK(loose.reads.empty());
}

// The Preview load runs the two halves side by side on one container read.
TEST_CASE("the song and stem halves share one container read and match the whole") {
    const std::vector<uint8_t> notes = chart_with_offset("0.25");
    const std::vector<uint8_t> ogg = bytes_of("OggS vorbis the one stem");
    const std::string sng = fixture_dir() + "\\halves.sng";
    write_bytes(sng,
                testsng::make_sng({{"delay", "500"}}, {{"notes.chart", notes}, {"song.ogg", ogg}}));
    const std::string srb = fixture_dir() + "\\halves.srb";
    write_bytes(srb, make_srb(notes, {ogg}, "notes.chart"));

    for (const std::string& path : {sng, srb}) {
        CAPTURE(path);
        CountingReader count;
        const SharedBytes container = read_preview_container(count.reader(), path);
        REQUIRE(container != nullptr);
        const PreviewSong ps = resolve_preview_song(path, container, true, true);
        const std::vector<PreviewAudioStem> stems = resolve_preview_stems(path, container);
        CHECK(count.reads[path] == 1);
        const PreviewSource whole = resolve_preview_source(path, true, true);
        CHECK(ps.song.chart_offset_s == whole.song.chart_offset_s);
        CHECK(ps.audio_offset_ms == whole.audio_offset_ms);
        REQUIRE(stems.size() == 1);
        CHECK(stems[0].bytes == ogg);
        // A stop request before the first entry gives no stems.
        if (path == sng) CHECK(resolve_preview_stems(path, container, [] { return false; }).empty());
    }
    // A loose chart has no container to read.
    CountingReader none;
    CHECK(read_preview_container(none.reader(), "C:\\x\\notes.chart") == nullptr);
    CHECK(none.reads.empty());
}

TEST_CASE("container charts give the same Song, stems and offset as the chart inside") {
    // What the loaders did before the read-once change: decode the notes entry
    // and parse it with the .mid/.chart byte loader. Each container must still
    // give exactly that Song, the audio bytes it holds, and the same offset.
    const std::vector<uint8_t> chart = chart_with_offset("0.25");
    const std::vector<uint8_t> mid =
        hydra::read_file_bytes(corpus::first_chart_with_suffix(".mid"));
    const std::vector<uint8_t> song_ogg = bytes_of("OggS vorbis song stem");
    const std::vector<uint8_t> drums_ogg = bytes_of("OggS vorbis drums stem, a bit longer");

    struct Case {
        const char* file;
        std::vector<uint8_t> container;
        bool is_mid;
        std::vector<std::vector<uint8_t>> stems;
        double offset_ms;
    };
    const std::vector<Case> cases = {
        {"eq_chart.sng",
         testsng::make_sng({{"delay", "120"}}, {{"notes.chart", chart},
                                                {"song.ogg", song_ogg},
                                                {"drums.ogg", drums_ogg}}),
         false, {song_ogg, drums_ogg}, 120.0},
        {"eq_chart_nodelay.sng",
         testsng::make_sng({}, {{"notes.chart", chart}, {"song.ogg", song_ogg}}), false,
         {song_ogg}, 250.0},
        {"eq_mid.sng", testsng::make_sng({}, {{"song.ogg", song_ogg}, {"notes.mid", mid}}), true,
         {song_ogg}, 0.0},
        {"eq_chart.srb", make_srb(chart, {song_ogg, drums_ogg}, "notes.chart"), false,
         {song_ogg, drums_ogg}, 250.0},
        {"eq_mid.srb", make_srb(mid, {song_ogg}), true, {song_ogg}, 0.0},
    };

    for (const Case& c : cases) {
        CAPTURE(c.file);
        const std::string path = fixture_dir() + "\\" + c.file;
        write_bytes(path, c.container);
        for (Difficulty d : {Difficulty::Expert, Difficulty::Hard}) {
            const Song expected = c.is_mid ? load_songbytes_mid(mid, true, true, d)
                                           : load_songbytes_chart(chart, true, true, d);
            const std::string want = song_text(expected);
            CHECK(song_text(load_songpath(path, true, true, d)) == want);
            CHECK(song_text(load_songpath_from_bytes(path, c.container, true, true, d)) == want);

            const PreviewSource src = resolve_preview_source(path, true, true, d);
            CHECK(song_text(src.song) == want);
            REQUIRE(src.stems.size() == c.stems.size());
            for (size_t i = 0; i < c.stems.size(); ++i) {
                CHECK(src.stems[i].bytes == c.stems[i]);
                CHECK_FALSE(src.stems[i].from_file());
            }
            CHECK(src.audio_offset_ms == doctest::Approx(c.offset_ms));
        }
    }
}
