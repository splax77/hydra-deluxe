// SongLengthJob: works out one song's length off the render thread, through
// the owner (app::chart_song_length_ms, D75), with no audio opened. A result
// saved before the current length rule (kSongLengthStamp) has none, so the
// Paths tab's timeline would stay empty until a re-analysis; AppState runs
// this when such a song is opened and saves what it found
// (RecordStore::fill_song_length). Nothing about the result changes.

#ifndef HYDRA_UI_SONG_LENGTH_JOB_H
#define HYDRA_UI_SONG_LENGTH_JOB_H

#include <string>

#include "app/analysis.h"        // AnalysisSettings
#include "store/record_store.h"  // ChartLibraryEntry, SongLength
#include "ui/job_base.h"

namespace hydra::ui {

class SongLengthJob : public ResultJobBase {
public:
    // `settings` gives the rules and drum options the chart is parsed with;
    // the length itself depends on neither (D75 item 2).
    SongLengthJob(store::ChartLibraryEntry entry, app::AnalysisSettings settings);
    ~SongLengthJob() { shutdown(); }

    void start();

    const store::ChartLibraryEntry& entry() const { return entry_; }
    // Valid once finished() && ok(): always read, with no length when the
    // owner gives none.
    const store::SongLength& song_length() const { return length_; }

private:
    void run();

    store::ChartLibraryEntry entry_;
    app::AnalysisSettings settings_;
    store::SongLength length_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_SONG_LENGTH_JOB_H
