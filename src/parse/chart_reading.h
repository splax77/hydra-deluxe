// Which notes a chart is read as: the difficulty and the other choices the
// parser's loaders (parse/song.h) take. Kept in this small header so code that
// only names a reading need not pull in the whole parser or the app layer.

#ifndef HYDRA_PARSE_CHART_READING_H
#define HYDRA_PARSE_CHART_READING_H

#include "core/model.h"  // kNormalSpeedPercent

namespace hydra {

// Which charted difficulty to read. One chart carries all four: a .mid keeps
// them in the single drum track at four pitch bases, a .chart keeps
// them in four named sections. Expert is ChartReading's default, so a default
// reading reads exactly what Hydra always read.
enum class Difficulty { Expert, Hard, Medium, Easy };

// The song speed setting's default: another name for kNormalSpeedPercent
// (core/model.h), which owns the speed's default and limits.
inline constexpr int kDefaultSpeedPercent = kNormalSpeedPercent;

// The four choices that pick which notes a chart is read as (the difficulty,
// Pro Drums, 2x Bass and Note Shuffle), and the song speed the chart is timed
// at. Settings::reading() (app/config.h) is where they are read off the
// settings, and Settings::with_reading where they are written back. What
// carries them takes this one value, so another choice is added here and in
// those two (concept audit M19).
// The defaults read a chart as written, at Expert and at 100%.
struct ChartReading {
    Difficulty difficulty = Difficulty::Expert;
    bool prodrums = true;
    bool bass2x = true;
    bool noteshuffle = false;
    // The song speed in whole percent. The parsers hand it to
    // Song::set_speed_percent before the song's timing is built, so every
    // chart time is read at this speed. It is not part of the chart mode
    // (app::chartmode_key); a stored result keys it in its lens instead.
    int speed_percent = kNormalSpeedPercent;

    bool operator==(const ChartReading& o) const {
        return difficulty == o.difficulty && prodrums == o.prodrums && bass2x == o.bass2x &&
               noteshuffle == o.noteshuffle && speed_percent == o.speed_percent;
    }
    bool operator!=(const ChartReading& o) const { return !(*this == o); }
};

}  // namespace hydra

#endif  // HYDRA_PARSE_CHART_READING_H
