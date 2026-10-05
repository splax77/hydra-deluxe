// See song_audio.h.

#include "audio/song_audio.h"

#include <exception>
#include <utility>

#include "audio/frames.h"
#include "audio/mapped_file.h"
#include "core/winstr.h"  // read_file_bytes

namespace hydra::audio {

std::vector<std::optional<StemBytes>> map_song_stems(std::vector<app::PreviewAudioStem> stems) {
    std::vector<std::optional<StemBytes>> out;
    out.reserve(stems.size());
    for (app::PreviewAudioStem& s : stems) {
        std::optional<StemBytes> bytes;
        if (s.from_file()) {
            try {
                bytes.emplace();
                bytes->mapped = MappedFile::open(s.path);
            } catch (const std::exception&) {
                bytes.reset();
            }
        } else {
            bytes.emplace();
            bytes->owned = std::move(s.bytes);
        }
        out.push_back(std::move(bytes));
    }
    return out;
}

std::vector<std::unique_ptr<StemReader>> open_song_stems(
    std::vector<std::optional<StemBytes>> stems, const OpenProgress& progress) {
    uint64_t total = 0;
    for (const std::optional<StemBytes>& bytes : stems)
        if (bytes) total += bytes->size();

    std::vector<std::unique_ptr<StemReader>> readers;
    uint64_t before = 0;  // bytes of the stems already opened
    for (std::optional<StemBytes>& bytes : stems) {
        if (!bytes) continue;  // it would not map: no bytes, no reader
        const uint64_t size = bytes->size();
        try {
            // The Opus index reports every 4 MB; returning false stops it there.
            OpenProgress stem_progress;
            if (progress)
                stem_progress = [&progress, before, total](uint64_t done, uint64_t) {
                    return progress(before + done, total);
                };
            readers.push_back(open_stem_reader(std::move(*bytes), stem_progress));
        } catch (const OpenCancelled&) {
            throw;
        } catch (const std::exception&) {
            // A stem that won't open is skipped (see the header).
        }
        before += size;
        if (progress && !progress(before, total)) throw OpenCancelled{};
    }
    return readers;
}

SongMix mix_song_stems(std::vector<std::unique_ptr<StemReader>> readers, double offset_ms) {
    SongMix out;
    out.audio_offset_ms = offset_ms;
    int64_t front_pad = 0;
    if (offset_ms < 0.0) {
        // Silence in front, rounded to whole frames (see SongMix).
        front_pad = frames_of_ms(-offset_ms, kOutRate);
        out.audio_offset_ms = 0.0;
    }
    out.mix = std::make_unique<StreamMix>(std::move(readers), kOutRate, kOutChannels, front_pad);
    if (out.mix->length_frames() > front_pad)
        out.end_chart_ms = audio_end_chart_ms(*out.mix, out.audio_offset_ms);
    return out;
}

std::optional<double> song_length_ms(const std::string& notespath, const Song& song) {
    const app::SharedBytes container = app::read_preview_container(read_file_bytes, notespath);
    const double offset_ms = app::chart_audio_offset_ms(notespath, container, song.chart_offset_s);
    std::vector<std::unique_ptr<StemReader>> readers =
        open_song_stems(map_song_stems(app::resolve_preview_stems(notespath, container)));
    return mix_song_stems(std::move(readers), offset_ms).end_chart_ms;
}

}  // namespace hydra::audio
