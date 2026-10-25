// How hydra_precheck reads a repository: every git call it makes, and the
// file texts at a revision. Nothing here writes to the repository; every
// call goes through `git -C <root>`, so a fixture repo in a temp folder reads
// the same way as the real one.
#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace precheck {

// A git call or the repository itself failed. what() is the sentence the
// exe prints before it exits non-zero.
struct GitError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// A child process started with its stdin, stdout and stderr on pipes. The
// one process launcher in tools/precheck/.
class ChildProcess {
public:
    // Starts `args[0]` (found on PATH) with the rest as its arguments, each
    // quoted the way the C runtime reads a command line back. Throws
    // GitError naming the program when it cannot start.
    explicit ChildProcess(const std::vector<std::string>& args);
    ~ChildProcess();
    ChildProcess(const ChildProcess&) = delete;
    ChildProcess& operator=(const ChildProcess&) = delete;

    // Writes all of `bytes` to the child's stdin.
    void write(const std::string& bytes);
    // Closes the child's stdin, so a child that reads to the end stops.
    void close_stdin();
    // Reads one line from stdout, without its "\n". False at the end.
    bool read_line(std::string& line);
    // Reads exactly n bytes from stdout. False when stdout ends first.
    bool read_exact(size_t n, std::string& out);
    // Reads stdout to its end.
    std::string read_all();
    // Waits for the child to exit and returns its exit code, with everything
    // it wrote to stderr in `err`.
    int wait(std::string& err);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// One long-lived `git cat-file --batch` process: every file text at a
// revision comes through it, so reading a thousand files starts one process.
class CatFileBatch {
public:
    explicit CatFileBatch(const std::string& root);
    ~CatFileBatch();
    CatFileBatch(const CatFileBatch&) = delete;
    CatFileBatch& operator=(const CatFileBatch&) = delete;

    // The bytes of `path` at `rev`, exactly as stored, or nothing when the
    // revision has no such file.
    std::optional<std::string> raw(const std::string& rev, const std::string& path);
    // The text of `path` at `rev` in the form every check reads (tip_text_form).
    // Throws GitError when the revision has no such file.
    std::string text(const std::string& rev, const std::string& path);

private:
    std::unique_ptr<ChildProcess> proc_;
};

// A file's text as the script's Get-TipText gave it: line breaks are "\n",
// a leading UTF-8 byte order mark and one final line break are dropped.
std::string tip_text_form(const std::string& raw);

// A git repository, opened at any folder inside it.
class Repo {
public:
    // Throws GitError "not a git repository: <path>" when `path` is not in one.
    explicit Repo(const std::string& path);
    ~Repo();

    // The work tree's top folder, as git prints it.
    const std::string& root() const { return root_; }
    // Runs `git -C <root> <args>` and returns its stdout as lines (a "\r"
    // before each "\n" dropped). Throws GitError "git <args> failed: <what
    // git printed>" on a non-zero exit.
    std::vector<std::string> git(const std::vector<std::string>& args) const;
    // The commit hash a name means (a branch, a tag, a hash, HEAD~2...). The
    // one place a name becomes a commit hash.
    std::string resolve_commit(const std::string& name) const;
    // The one cat-file process, started on first use.
    CatFileBatch& files();

private:
    std::string root_;
    std::unique_ptr<CatFileBatch> files_;
};

// Runs `git -C <dir> <args>`, as Repo::git does, before a Repo exists.
std::vector<std::string> run_git(const std::string& dir, const std::vector<std::string>& args);

}  // namespace precheck
