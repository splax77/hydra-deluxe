// The Song class: a parsed chart's tick-ordered sequence of SongTimestamps (a
// Timecode + Chord + solo/SP/fill flags), plus the tempo and meter maps, and
// the small record types it holds. The readers that build one from a file
// (the load_songpath_* functions) live in parse/song.h, which includes this.

#ifndef HYDRA_CORE_SONG_H
#define HYDRA_CORE_SONG_H

#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/timing.h"

namespace hydra {

// Which Clone Hero version's fill rule a run follows. search/graph.h owns what
// it means (the fill deadline) and its mappings (fill_rule_for and the rest);
// it is declared here because Song::check_activations also picks the
// generator of a chart's own fills from it.
enum class FillDeadlineRule { Ch11, Ch10 };

// A timecode paired with a chord and gameplay modifiers.
struct SongTimestamp {
    Timecode timecode;
    Chord chord;
    bool flag_solo = false;
    bool flag_sp = false;
    std::optional<int64_t> activation_length;
    // On the note that ends an SP phrase (flag_sp == true), the tick the phrase
    // began at. The parser otherwise discards the phrase start once the end
    // flag is set; the Preview needs the whole span to shade the phrase, so it
    // is kept here. nullopt on every other note. Not part of the search or the
    // stored record — a display-only addition.
    std::optional<int64_t> sp_phrase_start;

    bool has_activation() const { return activation_length.has_value(); }
};

// The chords either side of a tick, and the one a fill takes there. Each is
// an index into the chord sequence, or nullopt when there is none.
struct ChordsAround {
    std::optional<size_t> before;  // the last chord at or before the tick
    std::optional<size_t> after;   // the first chord after the tick
    std::optional<size_t> pick;    // the nearer of the two, a tie going to `after`
};

// The one owner of "which chord is nearest this tick", a tie going to the
// later chord. It is Clone Hero's own search (docs/clone-hero/code/4045/
// fill_generator.cs, section F4, the tie at line 164). `sequence` must be in
// tick order, as both parsers emit it. A caller with a range drops the chord
// before the tick when it sits earlier than `earliest`, and the chord after
// it when it sits later than `latest`; the pick is made from what is left.
// A caller that searches on from a chord, as Clone Hero 1.0's generator does,
// passes its index as `search_from`: no chord before that index is `after`.
ChordsAround chords_around(const std::vector<SongTimestamp>& sequence, int64_t tick,
                           int64_t earliest = (std::numeric_limits<int64_t>::min)(),
                           int64_t latest = (std::numeric_limits<int64_t>::max)(),
                           size_t search_from = 0);

// A practice section ("Verse 2B", "Chorus 1"), from the chart's own section
// markers. Display only: nothing in the search or the stored record reads these.
struct SongSection {
    int64_t tick = 0;
    std::string name;
};

// One solo section: first and last positions (both inclusive) into
// Song::sequence. What makes a section: find_solo_sections in parse/song.cpp.
struct SoloSection {
    size_t first = 0;
    size_t last = 0;
};

// One Clone Hero drum lane: the coloured strip the game shows down one pad
// column (a single lane) or two (a double lane) during a roll or trill. It
// runs from start_tick to end_tick. `pads` holds the colour columns it
// lights, in red-to-green order; which ones is light_drum_lanes's rule in
// parse/song.cpp. Display only: nothing in the search or the stored record
// reads it.
struct DrumLane {
    int64_t start_tick = 0;
    int64_t end_tick = 0;
    bool double_lane = false;
    std::vector<NoteColor> pads;
};

// How a refused-timing message starts. check_timing_maps and apply_timesig in
// core/song.cpp and the .chart Resolution reader in parse/song.cpp build their
// messages from these; the error's kind, ChartTimingRefused, is what picks its
// sentence.
inline constexpr std::string_view kResolutionRefusalPrefix = "the chart's resolution is ";
inline constexpr std::string_view kTimeSignatureRefusalPrefix = "the time signature at tick ";
inline constexpr std::string_view kTempoRefusalPrefix = "the tempo at tick ";

// Throws ChartFileError, naming the tick, unless every measure in these maps
// lasts a positive, finite time: resolution above 0, a tempo at tick 0, every
// measure at least one tick long, every tempo a finite BPM above 0 (a .mid
// tempo of 0 microseconds per beat is named infinite). Song::build_timing calls it
// first, so both parsers pass through it.
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes);

// The meter a chart has before its first time signature: 4/4. Song's
// constructor writes it through apply_timesig, and the Preview's
// PreviewTimeSig reads it for a scene with no song.
inline constexpr int kDefaultTimeSigNumerator = 4;
inline constexpr int kDefaultTimeSigDenominator = 4;

// The song speed, in percent, at which a chart plays as its file is written:
// Clone Hero's default speed (TM-34 on docs/clone-hero/03-timing.md). A Song
// is timed at it until Song::set_speed_percent says otherwise.
inline constexpr int kNormalSpeedPercent = 100;

// The feature name Song::check_activations adds to Song::features when it
// places fills for a chart that has none of its own.
inline constexpr const char* kGeneratedFillsFeature = "Auto-Generated Fills";

// A parsed chart: the timestamp sequence plus the tempo/meter maps it was built
// from. Timing is snapshotted once the maps are complete (build_timing).
class Song {
public:
    // Starts at the default meter (kDefaultTimeSig*), written by apply_timesig
    // like every other meter.
    explicit Song(int64_t resolution);

    int64_t tick_resolution() const { return tick_resolution_; }

    // Tick-keyed meter (ticks per measure) and tempo (BPM) maps, filled during
    // parsing. std::map keeps them sorted, which the timing indexes rely on.
    std::map<int64_t, int64_t> tpm_changes;
    std::map<int64_t, double> bpm_changes;
    // The chart's own time signatures, tick -> (numerator, denominator),
    // recorded by the same parser call that writes tpm_changes. Display only
    // (the Preview's time box): the engine reads tpm_changes, and a length
    // alone cannot tell 6/8 from 3/4.
    std::map<int64_t, std::pair<int, int>> timesig_changes;

    std::vector<SongTimestamp> sequence;
    std::vector<std::string> features;
    bool dynamics_enabled = false;
    // .mid only (finding 64, D24). When the drum track's dynamics tag came
    // after at least one ghost- or accent-velocity note, the tag's tick and
    // how many such notes came before it (Clone Hero prices them as plain).
    // nullopt and 0 when the tag came before every marked note, or there is
    // no tag (dynamics_enabled says which). The .mid parser stores them and
    // count_dynamics reads them.
    std::optional<int64_t> dynamics_late_tag_tick;
    int dynamics_marks_before_tag = 0;
    // .chart [Song] Offset, in seconds, when the file sets one. The Preview
    // and app::song_length_ms read it to line the audio up with the chart
    // (preview_audio_offset_ms); scoring works in chart time.
    std::optional<double> chart_offset_s;

    // Practice sections in tick order. A section marker can sit past the last
    // note, so these ticks are not bounded by the sequence.
    std::vector<SongSection> practice_sections;

    // The tick of the chart's end event, the first one in file order, or
    // nullopt when it has none. Which text counts is is_end_event in
    // parse/song.cpp. Its one reader is app::song_length_ms.
    std::optional<int64_t> end_event_tick;

    // The tick where the file's last note ends, over every instrument chart
    // at every difficulty, not only the drums parsed into `sequence`; nullopt
    // when the file has no such note. Clone Hero measures a song's last note
    // this way. In a .mid, MidiFile::last_note_end_tick says which tracks
    // count; in a .chart, ChartParser::load_sections reads every section's
    // note lines. Its readers are app::song_length_ms and ch10_last_line_tick.
    std::optional<int64_t> last_note_end_tick;

    // Solo sections in song order. Both parsers fill it once the sequence is
    // finished; see find_solo_sections in parse/song.cpp. Display and replay
    // only; nothing stores it.
    std::vector<SoloSection> solo_sections;

    // Drum lanes shown at the loaded difficulty, in lane-start order. The
    // loaders light them once, after Note Shuffle, so with Note Shuffle on
    // the pads are the shuffled notes' pads: the game's lane holds its notes,
    // which the shuffle recolours. With Note Shuffle on the pad rule also
    // leaves the game's on purpose (D254). See light_drum_lanes in
    // parse/song.cpp. Display only; nothing in the search or the stored record
    // reads it.
    std::vector<DrumLane> drum_lanes;

    // Which game version's generator placed this song's fills, set by
    // check_activations; nullopt when the chart wrote its own fills (or none
    // has run). The one answer to "was this fill generated". Display only:
    // the Preview draws a generated fill's span from it.
    std::optional<FillDeadlineRule> generated_fills;

    // Whether the chart wrote a fill that the coda skipped, set by
    // place_authored_fills in parse/song.cpp, which owns the skip. Such a
    // fill places no chord, yet it still stops check_activations from
    // generating: Clone Hero 1.1's note builder runs its generator only when
    // the track's fill list is empty (0x20D3375), and a skipped fill stays in
    // that list (FL-27 and FL-08 on docs/clone-hero/04-fills.md). A written
    // fill that lands on no chord does not set it (FL-28).
    bool coda_skipped_fill = false;

    // The song speed the chart is timed at, a whole percent (kNormalSpeedPercent
    // until set). Every tick-to-time answer follows it, through
    // SongTiming::speed_ratio (TM-56 on docs/clone-hero/03-timing.md). Set it
    // before build_timing: the timecodes a song holds are made from its
    // timing, so a later change would leave them at the old speed, and this
    // throws std::logic_error then. A percent of 0 or below throws
    // std::invalid_argument. The one place the percent becomes the ratio.
    void set_speed_percent(int percent) {
        if (timing_) throw std::logic_error("song speed set after build_timing");
        if (percent <= 0) throw std::invalid_argument("song speed percent must be above 0");
        speed_percent_ = percent;
        speed_ratio_ = static_cast<double>(percent) / static_cast<double>(kNormalSpeedPercent);
    }
    int speed_percent() const { return speed_percent_; }

    // Snapshot the timing indexes from the current maps, at the song speed.
    // Called once the tempo track has been fully mapped and before any
    // timecode is made. Timing that can't measure time is refused here
    // (check_timing_maps).
    void build_timing() {
        check_timing_maps(tick_resolution_, tpm_changes, bpm_changes);
        timing_.emplace(tick_resolution_, tpm_changes, bpm_changes, timesig_changes,
                        speed_ratio_);
    }
    const SongTiming& timing() const { return *timing_; }
    Timecode timecode(int64_t ticks) const { return timing_->timecode(ticks); }
    Timecode start_time() const { return timing_->timecode(0); }

    bool is_empty() const { return sequence.empty(); }

    // How many SP phrases the chart offers. No run can bank more bars than
    // this, so it's the natural clamp on how tall a search graph is worth
    // building.
    int sp_phrase_count() const;

    // How many notes the chart has, at the difficulty and drum options it was
    // loaded with: every chord's Chord::count(), added up. The one answer to
    // the chart's note total. The engine stores it on every path (the Notes
    // column), and the Dynamics tab's Totals are pinned against it by test.
    int note_count() const;

    // If the chart has no drum fills, synthesize them as the game version
    // `fills` names would. No default: a caller names the game version.
    void check_activations(const core::Rules& rules, FillDeadlineRule fills);

private:
    // The two generators check_activations chooses between. 1.0's reads no
    // rule: its numbers are the game's own literals.
    void place_fills_ch10();
    void place_fills_ch11(const core::Rules& rules);

    int64_t tick_resolution_;
    int speed_percent_ = kNormalSpeedPercent;
    double speed_ratio_ = 1.0;
    std::optional<SongTiming> timing_;
};

// A time signature at `tick`: ticks per measure = resolution * 4 * num / den,
// and the signature itself for display, written together. The one owner of
// the meter rule: both parsers, Song's default and test fixtures call it. A
// numerator of 0 is ignored; a bottom number of 0 or less refuses the chart.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator);

// The last tick of Clone Hero 1.0's measure-line walk, which the walk
// includes: F10 of docs/clone-hero/code/4045/fill_generator.md, at 100% song
// speed. The chart's last note end is Song::last_note_end_tick, the one answer
// to it, read as 0 when the chart has no note, as the game's F10 does.
int64_t ch10_last_line_tick(const Song& song);

// Clone Hero 1.0's measure lines, in tick order: the list its fill generator
// walks (FL-40 on docs/clone-hero/04-fills.md). They are the measure lines of
// MeasureIndex::walk from tick 0 to `last_tick`, that tick included, less the
// ones 1.0 stores as beats (fill_generator.cs lines 223 to 233). MeasureIndex
// is not changed for it, so the 1.1 generator's lines stay as they are.
std::vector<int64_t> ch10_measure_lines(const MeasureIndex& grid, int64_t last_tick);

// Whether a cymbal marker on `pad` counts, with Pro Drums on (`pro`) or off.
// The one home of that rule: the chart readers and the Preview's gem builder
// ask it instead of keeping their own copy. Which pads can carry a cymbal at
// all is allows_cymbals' question (core/model.h), and this reads it.
bool cymbal_counts(NoteColor pad, bool pro);

}  // namespace hydra

#endif  // HYDRA_CORE_SONG_H
