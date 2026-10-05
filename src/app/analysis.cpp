#include "app/analysis.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_set>

#include "app/dynamics_breakdown.h"
#include "app/preview_view.h"  // has_song_length
#include "app/user_messages.h"  // plain_error
#include "app/work_pool.h"
#include "core/error_kind.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "parse/srb.h"
#include "parse/chart_files.h"
#include "parse/sng.h"
#include "search/pather.h"

namespace hydra::app {

namespace {

// Folder listings, directory checks and folder-and-name joins come from
// core/winstr (list_dir, is_directory_utf8, join_folder), which handle paths
// of any length.

// os.path.relpath(target, base), for the folders this walk already knows are
// nested under `base` (or equal to it). Falls back to the raw target for any
// path that isn't, which discover_charts's own walk never produces.
std::string relpath(const std::string& target, const std::string& base) {
    std::string t = target, b = base;
    while (!t.empty() && (t.back() == '\\' || t.back() == '/')) t.pop_back();
    while (!b.empty() && (b.back() == '\\' || b.back() == '/')) b.pop_back();
    if (t == b) return ".";
    if (t.size() > b.size() && t.compare(0, b.size(), b) == 0 &&
        (t[b.size()] == '\\' || t[b.size()] == '/'))
        return t.substr(b.size() + 1);
    return t;
}

// ---- MD5 (Windows CNG), mirroring hashlib.file_digest(f, "md5") ----------
//
// The digest of the full raw chart file is record identity (songmeta.hyhash),
// so it must stay exactly MD5-of-all-bytes; only *how* the bytes reach the
// hash changed: streamed in chunks (like Python's file_digest) instead of a
// whole-file buffer, with the algorithm provider opened once per scan worker
// instead of once per file.

class Md5Provider {
public:
    Md5Provider() {
        if (!BCRYPT_SUCCESS(
                BCryptOpenAlgorithmProvider(&alg_, BCRYPT_MD5_ALGORITHM, nullptr, 0)))
            throw KindedError(ErrorKind::HashFailed, "BCryptOpenAlgorithmProvider(MD5) failed");
    }
    ~Md5Provider() {
        if (alg_) BCryptCloseAlgorithmProvider(alg_, 0);
    }
    Md5Provider(const Md5Provider&) = delete;
    Md5Provider& operator=(const Md5Provider&) = delete;

    BCRYPT_ALG_HANDLE handle() const { return alg_; }

private:
    BCRYPT_ALG_HANDLE alg_ = nullptr;
};

struct HashedFile {
    std::string md5;
    std::vector<uint8_t> head;  // first `head_capture` bytes, for .sng metadata
};

HashedFile stream_md5(BCRYPT_ALG_HANDLE alg, const std::string& path,
                      size_t head_capture) {
    FILE* f = fopen_utf8(path, L"rb");
    if (f == nullptr) throw KindedError(ErrorKind::SongFileMissing, "cannot open file: " + path);

    BCRYPT_HASH_HANDLE hash = nullptr;
    if (!BCRYPT_SUCCESS(BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0))) {
        std::fclose(f);
        throw KindedError(ErrorKind::HashFailed, "MD5 hashing failed");
    }

    HashedFile out;
    std::vector<uint8_t> buf(1 << 20);
    size_t got;
    while ((got = std::fread(buf.data(), 1, buf.size(), f)) > 0) {
        BCryptHashData(hash, buf.data(), static_cast<ULONG>(got), 0);
        if (out.head.size() < head_capture) {
            size_t want = std::min(head_capture - out.head.size(), got);
            out.head.insert(out.head.end(), buf.data(), buf.data() + want);
        }
    }
    std::fclose(f);

    UCHAR digest[16];
    bool ok = BCRYPT_SUCCESS(BCryptFinishHash(hash, digest, sizeof(digest), 0));
    BCryptDestroyHash(hash);
    if (!ok) throw KindedError(ErrorKind::HashFailed, "MD5 hashing failed");

    static const char* kHexDigits = "0123456789abcdef";
    out.md5.resize(32);
    for (int i = 0; i < 16; ++i) {
        out.md5[static_cast<size_t>(2 * i)] = kHexDigits[digest[i] >> 4];
        out.md5[static_cast<size_t>(2 * i + 1)] = kHexDigits[digest[i] & 0xF];
    }
    return out;
}

// ---- song.ini metadata, mirroring ScanItem.get_metadata_ini --------------
//
// The name, artist and charter keys, read through read_song_ini_keys (the
// one song.ini reader, shared with the Preview's delay). Its declaration in
// analysis.h says which lines count.

std::tuple<std::string, std::string, std::string> read_metadata_ini(const std::string& path) {
    const std::map<std::string, std::string> ini = read_song_ini_keys(path);

    // Empty = missing or blank; discover_charts applies the one fallback for
    // each (title_or_unknown, artist_or_unknown, charter_or_unknown).
    std::string title;
    std::string artist;
    std::string charter;

    if (auto it = ini.find("name"); it != ini.end()) title = it->second;
    if (auto it = ini.find("artist"); it != ini.end()) artist = it->second;
    // Only `charter` — Python's get_metadata_ini never reads the `frets`
    // alias, and some inis carry both with different values.
    if (auto it = ini.find("charter"); it != ini.end()) charter = it->second;
    return {title, artist, charter};
}

// ---- .sng metadata, mirroring ScanItem.get_metadata_sng -------------------
//
// Parses the metadata block from the head bytes captured while the file was
// being hashed — the old version read the entire archive (chart + audio, can
// be hundreds of MB) a second time to get three strings from its first few
// KB. A truncated buffer degrades exactly like a truncated file did: the
// bounds checks stop early and missing keys stay empty.

// How much of a .sng/.srb to keep for metadata. A .sng block starts at
// kSngMetadataOffset and a .srb's deflated block at kSrbHeaderSize, both a few
// dozen bytes in; real metadata is a few KB, so 1 MB is far beyond any
// legitimate block.
constexpr size_t kSngHeadCapture = 1 << 20;

std::tuple<std::string, std::string, std::string> parse_sng_metadata(
    const std::vector<uint8_t>& buf) {
    // Empty = missing or blank; discover_charts applies the one fallback for
    // each (title_or_unknown, artist_or_unknown, charter_or_unknown).
    std::string title;
    std::string artist;
    std::string charter;

    for (const auto& [raw_key, value] : sng_read_metadata(buf)) {
        const std::string key = to_lower_ascii(raw_key);
        if (key == "name") title = value;
        else if (key == "artist") artist = value;
        else if (key == "charter") charter = value;
    }

    return {title, artist, charter};
}

// ---- .srb metadata --------------------------------------------------------
//
// Clone Hero's bundled songs (see parse/srb.h for the reverse-engineered
// container layout). srb_read_metadata reads the metadata block, which the
// head bytes captured while hashing always contain (kSngHeadCapture). Any
// parse failure leaves the fields empty, matching the .sng path.

std::tuple<std::string, std::string, std::string> parse_srb_metadata(
    const std::vector<uint8_t>& buf) {
    // Empty = missing or blank; discover_charts applies the one fallback for
    // each (title_or_unknown, artist_or_unknown, charter_or_unknown).
    std::string title;
    std::string artist;
    std::string charter;

    try {
        const ByteSource head = memory_byte_source(buf);
        const SrbMetadataRead read = srb_read_metadata(head);
        if (read.parsed) {
            const SrbMetadata& md = read.fields;
            if (!md.name.empty()) title = md.name;
            artist = md.artist;
            charter = md.charter;
        }
    } catch (const std::exception&) {
        // Corrupt/truncated container: keep the defaults.
    }

    return {title, artist, charter};
}

// ---- discovery ----------------------------------------------------------
//
// Two stages. Enumerate: a serial single-pass walk (one directory listing
// per folder, no file contents touched) collecting every chart-bearing
// folder's pending work. Read: a batch_worker_count() thread pool hashes the
// chart files and reads their metadata, short-circuiting through the rescan
// cache when a chart's fingerprint is unchanged (sig_unchanged). Results keep the
// walk's order, so output ordering matches the old serial scanner.

// One chart the walk found, before any of its bytes have been read. Folder
// charts are a notes.mid/.chart plus a song.ini; .sng and .srb are standalone
// archives with embedded metadata.
enum class ChartKind { Folder, Sng, Srb };

struct PendingChart {
    ChartKind kind = ChartKind::Folder;
    std::string notes_path;  // the hashed file: notes.mid/.chart, .sng, or .srb
    std::string ini_path;    // empty for archives
    std::string rootfolder;
    std::string sig;
};

// The rescan cache's key: sizes and mtimes, so an unchanged file is not read
// again. It only says the file is unchanged. Whether the rows read from it
// still hold what this build's hash and name readers would read is the
// stored kChartMetaStamp's answer (store/stored_versions.h), not this one's.
std::string sig_of(const DirEntry& notes, const DirEntry* ini) {
    std::string sig = std::to_string(notes.size) + ":" + std::to_string(notes.mtime);
    if (ini) sig += ":" + std::to_string(ini->size) + ":" + std::to_string(ini->mtime);
    return sig;
}

// The rescan cache's one "unchanged" test: a fingerprint was stored and the
// files on disk still give the same one. The scan's cache lookup and
// chart_files_unchanged both ask it.
bool sig_unchanged(const std::string& stored, const std::string& now) {
    return !stored.empty() && stored == now;
}

// The kind of chart a file is, by its name. Anything that is not an archive
// can only be a folder chart's notes file.
ChartKind chart_kind_of(const std::string& name) {
    switch (chart_format_of(name)) {
        case ChartFormat::Sng: return ChartKind::Sng;
        case ChartFormat::Srb: return ChartKind::Srb;
        default: return ChartKind::Folder;
    }
}

// One chart file in its folder's listing, as the scan records it: its kind,
// its path, the song.ini that goes with a folder chart, and the fingerprint
// of the files that make it up. Nothing when a folder chart has no song.ini.
// The walk and chart_files_unchanged both ask this, so which files go into a
// fingerprint is decided here once. The rootfolder is the caller's to fill.
std::optional<PendingChart> pending_chart_of(const std::string& dir, const DirEntry& chart,
                                             const std::vector<DirEntry>& listing) {
    PendingChart pc;
    pc.kind = chart_kind_of(chart.name);
    pc.notes_path = join_folder(dir, chart.name);
    const DirEntry* ini = nullptr;
    if (pc.kind == ChartKind::Folder) {
        ini = find_song_ini(listing);
        if (!ini) return std::nullopt;
        pc.ini_path = join_folder(dir, ini->name);
    }
    pc.sig = sig_of(chart, ini);
    return pc;
}

}  // namespace

// Chart libraries are UTF-8 in practice; a leading BOM is stripped and
// anything else is read byte-for-byte rather than replicating Python's
// utf-8/utf-8-sig/ansi fallback chain.
std::map<std::string, std::string> read_song_ini_keys(const std::string& path) {
    std::vector<uint8_t> raw = read_file_bytes(path);
    size_t start = 0;
    if (raw.size() >= 3 && raw[0] == 0xEF && raw[1] == 0xBB && raw[2] == 0xBF) start = 3;
    std::string text(reinterpret_cast<const char*>(raw.data() + start), raw.size() - start);

    std::map<std::string, std::string> keys;
    bool in_song_section = false;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t eol = text.find('\n', pos);
        std::string line = text.substr(pos, eol == std::string::npos ? std::string::npos
                                                                      : eol - pos);
        pos = (eol == std::string::npos) ? text.size() + 1 : eol + 1;

        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' ||
                                 line.back() == '\t'))
            line.pop_back();
        size_t a = line.find_first_not_of(" \t");
        if (a == std::string::npos) continue;
        line = line.substr(a);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            std::string section = to_lower_ascii(line.substr(1, line.size() - 2));
            in_song_section = (section == "song");
            continue;
        }
        if (!in_song_section) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = to_lower_ascii(line.substr(0, eq));
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
        std::string value = line.substr(eq + 1);
        size_t vb = value.find_first_not_of(" \t");
        value = (vb == std::string::npos) ? std::string() : value.substr(vb);
        keys[key] = value;
    }
    return keys;
}

// Hashes the whole chart file with MD5, the same way the library scan does.
// So the result here always matches the hyhash already stored in the
// songmeta/charts rows for that chart.
std::string hash_chart_file(const std::string& path) {
    try {
        Md5Provider md5;
        HashedFile hf = stream_md5(md5.handle(), path, 0);
        return hf.md5;
    } catch (const std::exception&) {
        return {};
    }
}

bool chart_files_unchanged(const std::string& notespath, const std::string& sig) {
    // The same listing the scan's walk reads, so the fingerprint comes from
    // the same find data the stored one was made from.
    const std::string dir = parent_folder(notespath);
    const std::vector<DirEntry> entries = list_dir(dir);
    for (const DirEntry& e : entries)
        if (!e.is_dir && join_folder(dir, e.name) == notespath) {
            const std::optional<PendingChart> now = pending_chart_of(dir, e, entries);
            return now && sig_unchanged(sig, now->sig);
        }
    return false;
}

std::string normalize_chart_hash(std::string_view hash) { return to_lower_ascii(hash); }

store::ChartLibraryEntry to_library_entry(const ScanItem& item) {
    store::ChartLibraryEntry e;
    e.md5 = item.md5;
    e.title = item.title;
    e.artist = item.artist;
    e.charter = item.charter;
    e.notespath = item.notespath;
    e.rootfolder = item.rootfolder;
    e.sig = item.sig;
    return e;
}

std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders, const ScanCallbacks& callbacks,
    const store::ChartLibraryCache* cache) {
    std::vector<std::string> errors;
    const std::atomic<bool>* cancel = callbacks.cancel;

    // ---- stage 1: enumerate ------------------------------------------------
    std::vector<PendingChart> pending;
    std::vector<std::pair<std::string, std::string>> unexplored;
    std::set<std::string> visited;
    for (const std::string& root : rootfolders) {
        if (is_directory_utf8(root)) unexplored.push_back({root, root});
        visited.insert(root);
    }

    while (!unexplored.empty()) {
        if (cancel && cancel->load()) break;
        auto [dir, origin] = unexplored.back();
        unexplored.pop_back();

        try {
            std::vector<DirEntry> entries = list_dir(dir);

            // The folder's files, by name and entry in the same order, so the
            // notes-file pick's index leads back to the entry (pending_chart_of
            // fingerprints it).
            std::vector<std::string> file_names;
            std::vector<const DirEntry*> files;
            std::vector<const DirEntry*> found_archives;
            std::vector<const DirEntry*> subdirs;
            for (const DirEntry& e : entries) {
                if (e.is_dir) {
                    subdirs.push_back(&e);
                    continue;
                }
                file_names.push_back(e.name);
                files.push_back(&e);
                if (chart_kind_of(e.name) != ChartKind::Folder) found_archives.push_back(&e);
            }

            std::string rootfolder = relpath(parent_folder(dir), origin);
            const auto add_chart = [&](const DirEntry& chart) {
                std::optional<PendingChart> pc = pending_chart_of(dir, chart, entries);
                if (!pc) return;
                pc->rootfolder = rootfolder;
                pending.push_back(std::move(*pc));
            };
            const std::optional<NotesFilePick> pick = pick_notes_file(file_names);
            if (pick) add_chart(*files[pick->index]);
            for (const DirEntry* archive : found_archives) add_chart(*archive);

            for (const DirEntry* sub : subdirs) {
                std::string subpath = join_folder(dir, sub->name);
                if (visited.insert(subpath).second) {
                    if (callbacks.on_folders)
                        callbacks.on_folders(static_cast<int>(visited.size()));
                    unexplored.push_back({subpath, origin});
                }
            }
        } catch (const std::exception& e) {
            errors.push_back(e.what());
        }
    }

    // ---- stage 2: read (hash + metadata), parallel -------------------------
    int total = static_cast<int>(pending.size());
    if (callbacks.on_charts) callbacks.on_charts(0, total, 0);

    std::vector<std::optional<ScanItem>> results(pending.size());
    if (total > 0 && !(cancel && cancel->load())) {
        struct ReadNote {
            bool cached = false;
            std::string error;
        };
        int done = 0, cached_count = 0;
        run_work_pool<ReadNote>(
            pending.size(), batch_worker_count(), cancel,
            [&](size_t i) {
                // One CNG provider per worker thread, reused across every file
                // it hashes and closed when the worker exits. Created lazily
                // so an all-cache-hits rescan never touches CNG at all.
                thread_local std::optional<Md5Provider> md5;

                const PendingChart& pc = pending[i];
                ReadNote note;
                try {
                    if (cache) {
                        auto it = cache->find(pc.notes_path);
                        if (it != cache->end() && sig_unchanged(it->second.sig, pc.sig)) {
                            // Field by field, like to_library_entry: seven
                            // strings in a positional list could swap unseen.
                            ScanItem item;
                            item.md5 = it->second.md5;
                            item.title = it->second.title;
                            item.artist = it->second.artist;
                            item.charter = it->second.charter;
                            item.notespath = pc.notes_path;
                            item.rootfolder = pc.rootfolder;
                            item.sig = pc.sig;
                            results[i] = std::move(item);
                            note.cached = true;
                        }
                    }
                    if (!results[i]) {
                        if (!md5) md5.emplace();
                        ScanItem item;
                        if (pc.kind != ChartKind::Folder) {
                            HashedFile hf =
                                stream_md5(md5->handle(), pc.notes_path, kSngHeadCapture);
                            item.md5 = std::move(hf.md5);
                            std::tie(item.title, item.artist, item.charter) =
                                pc.kind == ChartKind::Sng
                                    ? parse_sng_metadata(hf.head)
                                    : parse_srb_metadata(hf.head);
                        } else {
                            HashedFile hf = stream_md5(md5->handle(), pc.notes_path, 0);
                            item.md5 = std::move(hf.md5);
                            std::tie(item.title, item.artist, item.charter) =
                                read_metadata_ini(pc.ini_path);
                        }
                        item.notespath = pc.notes_path;
                        item.rootfolder = pc.rootfolder;
                        item.sig = pc.sig;
                        results[i] = std::move(item);
                    }
                } catch (const std::exception& e) {
                    note.error = e.what();
                }
                return note;
            },
            // Progress and error callbacks fire here, on the calling thread
            // only, as they do for run_batch.
            [&](ReadNote&& note) {
                ++done;
                if (note.cached) ++cached_count;
                if (!note.error.empty()) errors.push_back(std::move(note.error));
                if (callbacks.on_charts) callbacks.on_charts(done, total, cached_count);
            });
    }

    std::vector<ScanItem> scanitems;
    scanitems.reserve(results.size());
    for (std::optional<ScanItem>& r : results) {
        if (!r) continue;
        // The one stored fallback for each field, whichever source produced
        // it: a fresh song.ini, .sng or .srb read, or the rescan cache holding
        // an older scan's blank or "<unknown title>". What a screen shows is
        // decided later by display_title, display_artist and display_charter;
        // a stored "<unknown artist>" shows as "(unknown)" (D56 item 2).
        r->title = title_or_unknown(std::move(r->title));
        r->artist = artist_or_unknown(std::move(r->artist));
        r->charter = charter_or_unknown(std::move(r->charter));
        scanitems.push_back(std::move(*r));
    }
    return {scanitems, errors};
}

std::pair<std::vector<ScanItem>, std::vector<std::string>> discover_charts(
    const std::vector<std::string>& rootfolders,
    const std::function<void(int)>& cb_progress) {
    ScanCallbacks callbacks;
    callbacks.on_folders = cb_progress;
    return discover_charts(rootfolders, callbacks, nullptr);
}

// ---- analysis -------------------------------------------------------------

AnalysisResult analyze_chart_file(const std::string& filepath,
                                  const AnalysisSettings& settings,
                                  const std::function<void(float)>& on_progress) {
    // A chart with no charting at the asked difficulty (no Hard charting is
    // the common case) throws NoNotesError, which names that difficulty.
    Song song = load_songpath_with_notes(filepath, settings.prodrums, settings.bass2x,
                                         settings.difficulty, settings.rules);
    HydraRecord record = analyze_chart(song, settings, on_progress);
    return AnalysisResult{std::move(record), std::move(song)};
}

// ---- batch runner -----------------------------------------------------

// 8 = a memory/throughput choice: enough threads to keep a modern CPU busy
// while capping peak memory (a discography chart can reach hundreds of MB per
// worker). Kept fixed; batch_worker_count in analysis.h leaves one core free.
constexpr int kBatchMaxWorkers = 8;

int batch_worker_count() {
    unsigned int hw = std::thread::hardware_concurrency();
    int guess = hw > 0 ? static_cast<int>(hw) - 1 : 1;
    return std::max(1, std::min(guess, kBatchMaxWorkers));
}

namespace {

struct WorkResult {
    ScanItem item;
    std::optional<store::PreparedRow> row;
    std::optional<AnalysisResult> analysis;
    // Counted on the worker, so the consumer only writes.
    std::optional<store::DynamicsEntry> dynamics;
    // The song's audio length, read on the worker; not read without a reader.
    store::SongLength length;
    // A failed chart, from its analysis or its save: record_failure fills
    // these in from the exception while its type is still known (the
    // sentence is plain_error's, the error is the raw text).
    bool failed = false;
    std::string sentence;
    std::string error;
    // The search stopped at a cancel: neither a result nor a failure.
    bool cancelled = false;
};

// The one way a chart becomes a failure, from its analysis or its save.
void record_failure(WorkResult& wr, const std::exception& e) {
    wr.failed = true;
    wr.sentence = plain_error(e);
    wr.error = e.what();
}

}  // namespace

std::unordered_set<std::string> charts_with_result(store::RecordStore& store,
                                                   const BatchRun& run, bool redo) {
    if (redo) return {};
    // RecordStore::analyzed_hashes decides what "already has a result" means.
    return store.analyzed_hashes(run.chartmode, run.cap_query(), run.lens);
}

BatchPlan plan_batch(const std::vector<ScanItem>& items,
                     const std::unordered_set<std::string>& already) {
    BatchPlan plan;
    // Each md5 is looked at once: its first copy runs or is skipped, and any
    // later copy is passed over. Whichever copy runs, the store names the
    // chart from the copy the scan listed first (D63).
    std::unordered_set<std::string> seen;
    for (const ScanItem& item : items) {
        if (!seen.insert(item.md5).second) continue;
        if (already.count(item.md5))
            ++plan.skipped;
        else
            plan.todo.push_back(item);
    }
    return plan;
}

store::SongLength song_length_found(std::optional<double> audio_length_ms) {
    if (audio_length_ms && !has_song_length(*audio_length_ms)) audio_length_ms.reset();
    return store::SongLength::found(audio_length_ms);
}

store::SongLength read_song_length_or_keep(const SongLengthReader& reader,
                                           const std::string& notespath, const Song& song) {
    if (!reader) return {};
    try {
        return song_length_found(reader(notespath, song));
    } catch (const std::exception&) {
        return {};
    }
}

void run_batch(const std::vector<ScanItem>& items, const BatchRun& run,
               store::RecordStore& store, bool redo, int worker_count,
               const BatchCallbacks& callbacks) {
    const AnalysisSettings& settings = run.settings;
    const std::atomic<bool>* cancel = callbacks.cancel;
    const store::CapQuery cap = run.cap_query();

    const BatchPlan plan = plan_batch(items, charts_with_result(store, run, redo));
    const std::vector<ScanItem>& todo = plan.todo;

    BatchProgress progress;
    progress.total = static_cast<int>(todo.size());
    progress.skipped = plan.skipped;
    if (callbacks.on_progress) callbacks.on_progress(progress);
    if (todo.empty()) return;

    // A running search checks for cancel in its progress callback. Throwing
    // AnalysisCancelled there unwinds it at the next tick (the engine reports
    // every half percent of the chart), the way the single-chart Analyze
    // button stops. With no cancel flag there is nothing to check, so the
    // search gets no callback at all, exactly as before.
    std::function<void(float)> check_cancel;
    if (cancel)
        check_cancel = [cancel](float) {
            if (cancel->load(std::memory_order_relaxed)) throw AnalysisCancelled{};
        };
    const ChartAnalyzer analyze =
        callbacks.analyze ? callbacks.analyze : ChartAnalyzer(analyze_chart_file);

    run_work_pool<WorkResult>(
        todo.size(), worker_count, cancel,
        [&](size_t i) {
            const ScanItem& item = todo[i];
            WorkResult wr;
            wr.item = item;
            try {
                AnalysisResult ar = analyze(item.notespath, settings, check_cancel);
                wr.row = store::prepare_row(
                    store::RecordKey{item.md5, run.chartmode, cap, run.lens}, ar.record);
                wr.dynamics = dynamics_entry_from_analysis(
                    item.md5, ar.song, settings.bass2x, settings.difficulty,
                    settings.prodrums);
                wr.length =
                    read_song_length_or_keep(callbacks.read_song_length, item.notespath, ar.song);
                wr.analysis = std::move(ar);
            } catch (const AnalysisCancelled&) {
                wr.cancelled = true;
            } catch (const std::exception& e) {
                record_failure(wr, e);
            }
            return wr;
        },
        [&](WorkResult&& wr) {
            // Once cancel is seen nothing more is written or reported, as
            // before. A search stopped part-way is not a failure, and a result
            // that finished alongside the cancel is dropped.
            if (wr.cancelled || (cancel && cancel->load())) return;

            if (!wr.failed) {
                // A save that fails makes this chart a failure, and the batch
                // goes on with the next one (D71, ER2 open question 5).
                try {
                    store.save_analysis(wr.item.md5, wr.item.title, wr.item.artist,
                                        wr.item.charter, wr.analysis->song, *wr.row,
                                        wr.dynamics, wr.length);
                } catch (const std::exception& e) {
                    record_failure(wr, e);
                }
            }
            if (wr.failed) {
                ++progress.failed;
                if (callbacks.on_error) callbacks.on_error(wr.item.title, wr.sentence, wr.error);
            } else {
                ++progress.analyzed;
                if (callbacks.on_result) callbacks.on_result(wr.item, *wr.row);
            }

            progress.completed = progress.analyzed + progress.failed;
            progress.current_title = wr.item.title;
            if (callbacks.on_progress) callbacks.on_progress(progress);
        });
}

}  // namespace hydra::app
