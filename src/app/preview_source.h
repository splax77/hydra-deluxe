// Preview source resolution — find a chart's notes and its audio so the Preview
// can draw the highway and play the song.
//
// A stored record keeps only timing and paths, not the note stream (see
// docs/adr and record_store), so the Preview re-parses the chart file here.
// Audio is never stored either: this module locates it. Three sources, matching
// the chart kinds Hydra scans:
//   * a loose folder — audio sits beside the notes file (song.ogg, drums.opus,
//     stems, ...); a standalone preview clip (preview.ogg) is not part of the
//     song, so it is left out here and in a .sng alike (is_song_stem);
//   * a .sng container — audio lives in the same XOR-masked file table the
//     notes come from;
//   * a .srb container — art streams follow the notes stream in the DEFLATE
//     chain; the song audio lives past the chain in a section encrypted with
//     AES-128-CFB (a hardcoded key and a per-blob IV derived from the blob
//     header). extract_srb_audio decrypts these blobs in-memory.
//
// The audio bytes/paths produced here are decoded and mixed by the audio engine
// (Phase 3); this module does no decoding.

#ifndef HYDRA_APP_PREVIEW_SOURCE_H
#define HYDRA_APP_PREVIEW_SOURCE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/rules.h"
#include "parse/song.h"

namespace hydra::app {

// One audio track for the Preview. A loose track names a file on disk (`path`
// set, `bytes` empty); a container track carries its extracted, decompressed
// bytes (`bytes` set, `path` empty). `label` is a short stem name for display
// and mixing ("song", "drums", or a container stream tag).
struct PreviewAudioStem {
    std::string label;
    std::string path;
    std::vector<uint8_t> bytes;

    bool from_file() const { return !path.empty(); }
};

// A chart resolved for preview: its re-parsed notes and every audio stem found.
// `stems` may be empty — a chart with no locatable audio previews silently.
struct PreviewSource {
    Song song;
    std::vector<PreviewAudioStem> stems;
    // Where chart time 0 sits in the audio (audio_ms_of_chart_ms in
    // audio/frames.h turns it into a position), from chart_audio_offset_ms.
    double audio_offset_ms = 0.0;
};

// Parse `notespath` (any supported chart kind) and gather its audio.
// pro/bass2x/difficulty mirror the analysis toggles so the previewed notes
// match the analyzed ones. rules places the fills the same way analysis does.
// A .sng or .srb is read from disk once and the same bytes feed the notes and
// the audio.
PreviewSource resolve_preview_source(const std::string& notespath, bool pro,
                                     bool bass2x,
                                     Difficulty difficulty = Difficulty::Expert,
                                     const core::Rules& rules = core::default_rules());

// Reads a whole file's bytes, like read_file_bytes.
using FileBytesReader = std::function<std::vector<uint8_t>(const std::string&)>;

// resolve_preview_source with the container read going through `read_bytes`
// (resolve_preview_source passes read_file_bytes). Tests pass a counting
// wrapper to prove a .sng/.srb is read once. Loose charts don't use it.
PreviewSource resolve_preview_source_reading(const FileBytesReader& read_bytes,
                                             const std::string& notespath, bool pro,
                                             bool bass2x,
                                             Difficulty difficulty = Difficulty::Expert,
                                             const core::Rules& rules = core::default_rules());

// ---- the two halves, for a load that runs them side by side -------------
//
// resolve_preview_source is these three calls in a row. The Preview load job
// calls them itself so the chart parse and the audio open can run at once:
// read the container (if any) first, then hand the same bytes to both halves.

// A .sng or .srb chart's whole file, read once through `read_bytes` and shared
// read-only by both halves. Null for a loose chart (its halves read their own
// files).
using SharedBytes = std::shared_ptr<const std::vector<uint8_t>>;
SharedBytes read_preview_container(const FileBytesReader& read_bytes,
                                   const std::string& notespath);

// The notes half: the parsed song and where chart time 0 sits in the audio.
struct PreviewSong {
    Song song;
    double audio_offset_ms = 0.0;  // as PreviewSource::audio_offset_ms
};
// `container` is read_preview_container's result for the same notespath.
PreviewSong resolve_preview_song(const std::string& notespath, const SharedBytes& container,
                                 bool pro, bool bass2x,
                                 Difficulty difficulty = Difficulty::Expert,
                                 const core::Rules& rules = core::default_rules());

// Where chart time 0 sits in the audio of a chart already parsed
// (PreviewSource::audio_offset_ms): the chart's own delay, from its song.ini
// or .sng metadata, against `chart_offset_s` (the parsed Song's
// chart_offset_s), through preview_audio_offset_ms. `container` is
// read_preview_container's result for the same notespath. resolve_preview_song
// and the song's length (audio::song_length_ms) both ask here.
double chart_audio_offset_ms(const std::string& notespath, const SharedBytes& container,
                             std::optional<double> chart_offset_s);

// Asked between a container's audio entries; return false to stop early.
using KeepGoing = std::function<bool()>;
// The audio half: every stem, located but not decoded. Loose stems carry a
// path; container stems carry their extracted bytes. If `keep_going` returns
// false, extraction stops and the stems found so far come back.
std::vector<PreviewAudioStem> resolve_preview_stems(const std::string& notespath,
                                                    const SharedBytes& container,
                                                    const KeepGoing& keep_going = nullptr);

// ---- pieces, exposed for testing and reuse -------------------------------

// song.ini's `delay` in milliseconds, or nullopt when the file or key is
// missing or the value is not a number.
std::optional<double> read_ini_delay_ms(const std::string& ini_path);

// A .sng container's `delay` metadata in milliseconds, the key matched in any
// case (the last one wins, like the library scan's metadata read), or nullopt
// when it is missing or not a number.
std::optional<double> sng_delay_ms(const std::vector<uint8_t>& sng_bytes);

// The Preview's audio offset in ms from the two values Clone Hero reads.
double preview_audio_offset_ms(std::optional<double> ini_delay_ms,
                               std::optional<double> chart_offset_s);

// True if `filename` ends in an audio extension Hydra can decode
// (.ogg/.opus/.mp3/.wav/.flac), case-insensitive.
bool is_audio_filename(const std::string& filename);

// True if `filename` is a song stem: an audio file (is_audio_filename) whose
// base name is not "preview" in any case. A standalone preview clip is a short
// clip, not part of the song, so every named source (a loose folder, a .sng)
// leaves it out of the mix through this one test.
bool is_song_stem(const std::string& filename);

// True if the decoder can open `bytes`: it asks the decoder's own rule
// (audio::sniff_format, core/audio_sniff.h), so what the extractor keeps is
// exactly what the decoder can play. Used to pick audio streams out of a .srb's
// unnamed trailing streams and skip album art.
bool looks_like_audio(const std::vector<uint8_t>& bytes);

// Audio files sitting beside a loose notes file: every song stem in `folder`
// (is_song_stem), so a standalone "preview" clip is left out, since it is a
// short clip and not part of the song. Labels are the base filename.
std::vector<PreviewAudioStem> find_loose_audio(const std::string& folder);

// Audio blobs embedded in a .sng container: its file table's song stems
// (is_song_stem), XOR-demasked to their original bytes. A standalone "preview"
// clip is left out, as in a loose folder, since it is a short clip and not part
// of the song. Labels are the entries' base filenames.
std::vector<PreviewAudioStem> extract_sng_audio(const std::string& path);

// Audio from a .srb container's encrypted section.  Walks past the DEFLATE
// chain (metadata, notes, art), then parses and AES-128-CFB-decrypts the
// audio blob chain that follows.  Each decrypted Ogg blob becomes a stem.
std::vector<PreviewAudioStem> extract_srb_audio(const std::string& path);

}  // namespace hydra::app

#endif  // HYDRA_APP_PREVIEW_SOURCE_H
