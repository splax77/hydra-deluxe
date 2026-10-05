// SongLengthJob: reads one song's audio length off the render thread
// (audio::song_length_ms, D69). A result saved before Hydra read audio
// lengths has none, so the Paths tab's timeline would stay empty until a
// re-analysis; AppState runs this when such a song is opened and saves what it
// found (RecordStore::fill_song_length). The chart is parsed too, for its
// Offset only: the length is in chart time. Nothing about the result changes.

#ifndef HYDRA_UI_SONG_LENGTH_JOB_H
#define HYDRA_UI_SONG_LENGTH_JOB_H

#include <string>

#include "app/analysis.h"        // AnalysisSettings
#include "store/record_store.h"  // ChartLibraryEntry, SongLength
#include "ui/job_base.h"

namespace hydra::ui {

class SongLengthJob : public ResultJobBase {
public:
    // Reads the chart the way an analysis under `settings` would, so its
    // Offset matches what that analysis saw.
    SongLengthJob(store::ChartLibraryEntry entry, app::AnalysisSettings settings);
    ~SongLengthJob() { shutdown(); }

    void start();

    const store::ChartLibraryEntry& entry() const { return entry_; }
    // Valid once finished() && ok(): always read, with no length for a song
    // whose audio has no usable length (app::song_length_found).
    const store::SongLength& song_length() const { return length_; }

private:
    void run();

    store::ChartLibraryEntry entry_;
    app::AnalysisSettings settings_;
    store::SongLength length_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_SONG_LENGTH_JOB_H
