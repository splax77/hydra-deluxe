#include "ui/song_length_job.h"

#include "app/song_length.h"
#include "parse/song.h"

namespace hydra::ui {

SongLengthJob::SongLengthJob(store::ChartLibraryEntry entry, app::AnalysisSettings settings)
    : entry_(std::move(entry)), settings_(std::move(settings)) {}

void SongLengthJob::start() { spawn([this] { run(); }); }

void SongLengthJob::run() {
    run_guarded([this] {
        // The metadata read and the parse each run as one call and cannot
        // stop midway, so the job looks at its cancel flag around them.
        throw_if_cancelled();
        const store::ChartTimingMeta meta =
            app::chart_timing_meta(entry_.timing, entry_.notespath);
        throw_if_cancelled();
        // The chart as an analysis under these settings parses it. The owner
        // takes its Offset from it and parses the Expert chart itself when its
        // backup needs one (chart_song_length_ms).
        const Song song = load_songpath(entry_.notespath, settings_.prodrums, settings_.bass2x,
                                        settings_.difficulty, settings_.rules);
        throw_if_cancelled();
        length_ = store::SongLength::found(app::chart_song_length_ms(
            meta, entry_.notespath, song, settings_.difficulty, settings_.bass2x,
            settings_.rules));
        return true;
    });
}

}  // namespace hydra::ui
