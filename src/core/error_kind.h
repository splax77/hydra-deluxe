// What kind of failure an error is. The thrower knows; it says so here, and
// app::plain_error turns the kind into the sentence the user reads (its
// switch on ErrorKind is the one owner of those sentences). The error's text
// stays the raw detail for the small details line and the CLI.
//
// It lives in core because the throwers live in every folder below app/, and
// none of them includes app/.

#ifndef HYDRA_CORE_ERROR_KIND_H
#define HYDRA_CORE_ERROR_KIND_H

#include <optional>
#include <stdexcept>
#include <string>

namespace hydra {

// One kind per plain sentence. Running out of memory is not here: the
// standard library throws std::bad_alloc, and plain_error checks that type.
enum class ErrorKind {
    Cancelled,
    DatabaseOpen,
    DatabaseUpgrade,  // converting a Convertible library file failed and left it whole
    OldLibraryRefused,         // the text is the path of a library file too old to convert
    OldLibrarySetAsideFailed,  // the text is the system's reason the rename failed
    DatabaseWrite,
    DatabaseRead,
    SongFileMissing,
    HashFailed,
    ChartUnreadable,
    ChartTimingRefused,  // the text names the chart line, and the sentence quotes it
    NoteShuffleFreezes,  // the game hangs shuffling this chart (apply_note_shuffle, D104 item 5)
    NoNotes,             // no notes at the asked difficulty; the text is NoNotesError's sentence
    AlreadyPlain,        // the text is already written for the user
    SearchBroken,
    NetUnreachable,
    NetTimeout,
    NetHttpStatus,  // carries the HTTP status code
    NetBadReply,
    NoScores,
    NoRecords,
    ReportWrite,
    RulesFile,
    AudioDecode,
    PreviewAssets,
    UpdateUnverified,  // an update refused before install; the text names which check failed
};

// ---- remembered failures ---------------------------------------------------
//
// A failure the chart's own content causes fails the same way every time the
// same file is read, so the store keeps it as the chart's result under its
// profile (the user's answer 1, 2026-10-15, in
// docs/superpowers/plans/2026-10-15-report-covers-whole-library.md). Any
// other failure can pass on a retry and is never stored.

// One remembered kind and the name the records table stores it under.
struct RememberedFailure {
    ErrorKind kind;
    const char* name;
};

// The one list of remembered kinds. remembered_failure, failure_kind_name and
// failure_kind_from_name all read it, so a kind added here is remembered,
// stored and read back by the one edit. A stored name never changes: a file
// keeps it.
inline constexpr RememberedFailure kRememberedFailures[] = {
    {ErrorKind::NoNotes, "NoNotes"},
    {ErrorKind::ChartTimingRefused, "ChartTimingRefused"},
    {ErrorKind::NoteShuffleFreezes, "NoteShuffleFreezes"},
    {ErrorKind::ChartUnreadable, "ChartUnreadable"},
};

// Whether a failure of this kind is stored as the chart's result.
inline bool remembered_failure(ErrorKind kind) {
    for (const RememberedFailure& r : kRememberedFailures)
        if (r.kind == kind) return true;
    return false;
}

// The name a remembered kind is stored under; nullptr for any other kind.
inline const char* failure_kind_name(ErrorKind kind) {
    for (const RememberedFailure& r : kRememberedFailures)
        if (r.kind == kind) return r.name;
    return nullptr;
}

// The remembered kind stored under `name`; empty for a name this build does
// not know (one a later build wrote).
inline std::optional<ErrorKind> failure_kind_from_name(const std::string& name) {
    for (const RememberedFailure& r : kRememberedFailures)
        if (name == r.name) return r.kind;
    return std::nullopt;
}

// The servers Hydra talks to, so a network failure's sentence names the one
// that failed (app::plain_error). The transport's callers name their server
// by the host name in kNetServices.
enum class NetService {
    Leaderboard,  // dmleaderboards
    GitHub,       // the updater's release check and download
};

// Each server and the host name its caller hands net::http_get, which the
// raw detail text also shows.
struct NetServiceName {
    NetService service;
    const char* host_name;
};
inline constexpr NetServiceName kNetServices[] = {
    {NetService::Leaderboard, "leaderboard"},
    {NetService::GitHub, "GitHub"},
};

// The server a host name stands for; empty for a name not in kNetServices.
inline std::optional<NetService> net_service_from_host_name(const std::string& host_name) {
    for (const NetServiceName& s : kNetServices)
        if (host_name == s.host_name) return s.service;
    return std::nullopt;
}

// An error that carries its kind. what() is the raw text, unchanged.
class KindedError : public std::runtime_error {
public:
    // `http_status` is the status code a server sent, given only with
    // NetHttpStatus. `service` is the server a Net* failure came from, set
    // by the transport (net/http.cpp); net_sentences in app/user_messages.cpp
    // says which sentences an error without one gets.
    KindedError(ErrorKind kind, const std::string& what,
                std::optional<int> http_status = std::nullopt,
                std::optional<NetService> service = std::nullopt)
        : std::runtime_error(what), kind_(kind), http_status_(http_status), service_(service) {}

    ErrorKind kind() const noexcept { return kind_; }
    std::optional<int> http_status() const noexcept { return http_status_; }
    std::optional<NetService> service() const noexcept { return service_; }

private:
    ErrorKind kind_;
    std::optional<int> http_status_;
    std::optional<NetService> service_;
};

// The one error a step throws when it was asked to stop. Every layer that
// stops on request throws this or a type derived from it, so the kind and the
// raw text are built here once. Layers that catch their own stop by type
// derive a named type from it (ui::JobCancelled, audio::OpenCancelled). A
// thrower with a more telling raw text passes it; the plain sentence comes
// from the kind either way.
class Cancelled : public KindedError {
public:
    explicit Cancelled(const std::string& what = "cancelled")
        : KindedError(ErrorKind::Cancelled, what) {}
};

}  // namespace hydra

#endif  // HYDRA_CORE_ERROR_KIND_H
