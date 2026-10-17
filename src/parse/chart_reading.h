// Which notes a chart is read as: the difficulty and the other choices the
// parser's loaders (parse/song.h) take. Kept in this small header so code that
// only names a reading need not pull in the whole parser or the app layer.

#ifndef HYDRA_PARSE_CHART_READING_H
#define HYDRA_PARSE_CHART_READING_H

namespace hydra {

// Which charted difficulty to read. One chart carries all four: a .mid keeps
// them in the single drum track at four pitch bases, a .chart keeps
// them in four named sections. Expert is ChartReading's default, so a default
// reading reads exactly what Hydra always read.
enum class Difficulty { Expert, Hard, Medium, Easy };

// The four choices that pick which notes a chart is read as: the difficulty,
// Pro Drums, 2x Bass and Note Shuffle. Settings::reading() (app/config.h) is
// where they are read off the settings, and Settings::with_reading where they
// are written back. What carries them takes this one value, so a fifth choice
// is added here and in those two (concept audit M19).
// The defaults read a chart as written, at Expert.
struct ChartReading {
    Difficulty difficulty = Difficulty::Expert;
    bool prodrums = true;
    bool bass2x = true;
    bool noteshuffle = false;

    bool operator==(const ChartReading& o) const {
        return difficulty == o.difficulty && prodrums == o.prodrums && bass2x == o.bass2x &&
               noteshuffle == o.noteshuffle;
    }
    bool operator!=(const ChartReading& o) const { return !(*this == o); }
};

}  // namespace hydra

#endif  // HYDRA_PARSE_CHART_READING_H
