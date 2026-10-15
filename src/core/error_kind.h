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
    DatabaseUpgrade,  // converting a 2.2.0 to 2.7.0 library file failed and left it whole
    OldLibraryRefused,         // the text is the path of a library file too old to convert
    OldLibrarySetAsideFailed,  // the text is the system's reason the rename failed
    DatabaseWrite,
    DatabaseRead,
    SongFileMissing,
    HashFailed,
    ChartUnreadable,
    ChartTimingRefused,  // the text names the chart line, and the sentence quotes it
    NoteShuffleFreezes,  // the game hangs shuffling this chart (apply_note_shuffle, D104 item 5)
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
};

// An error that carries its kind. what() is the raw text, unchanged.
class KindedError : public std::runtime_error {
public:
    // `http_status` is the status code a server sent, given only with
    // NetHttpStatus.
    KindedError(ErrorKind kind, const std::string& what,
                std::optional<int> http_status = std::nullopt)
        : std::runtime_error(what), kind_(kind), http_status_(http_status) {}

    ErrorKind kind() const noexcept { return kind_; }
    std::optional<int> http_status() const noexcept { return http_status_; }

private:
    ErrorKind kind_;
    std::optional<int> http_status_;
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
