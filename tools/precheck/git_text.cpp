#include "git_text.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <thread>

#include "core/winstr.h"

namespace precheck {

namespace {

// One argument quoted so the C runtime's command-line reader gives it back
// unchanged: backslashes count only before a quote.
std::wstring quote_arg(const std::wstring& a) {
    if (!a.empty() && a.find_first_of(L" \t\n\v\"") == std::wstring::npos) return a;
    std::wstring out = L"\"";
    size_t slashes = 0;
    for (const wchar_t c : a) {
        if (c == L'\\') {
            ++slashes;
            continue;
        }
        if (c == L'"') out.append(slashes * 2 + 1, L'\\');
        else out.append(slashes, L'\\');
        slashes = 0;
        out += c;
    }
    out.append(slashes * 2, L'\\');
    out += L'"';
    return out;
}

std::string join(const std::vector<std::string>& args) {
    std::string s;
    for (const std::string& a : args) {
        if (!s.empty()) s += ' ';
        s += a;
    }
    return s;
}

std::string trimmed(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    const size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

std::vector<std::string> split_lines(const std::string& out) {
    std::vector<std::string> lines;
    size_t at = 0;
    while (at < out.size()) {
        size_t nl = out.find('\n', at);
        if (nl == std::string::npos) nl = out.size();
        std::string line = out.substr(at, nl - at);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(std::move(line));
        at = nl + 1;
    }
    return lines;
}

}  // namespace

struct ChildProcess::Impl {
    HANDLE process = nullptr;
    HANDLE in_write = nullptr;
    HANDLE out_read = nullptr;
    HANDLE err_read = nullptr;
    std::thread err_thread;
    std::string err;
    std::string buf;  // stdout read but not handed out yet
    bool out_ended = false;

    // Reads more stdout into buf. False at the end.
    bool fill() {
        if (out_ended) return false;
        char chunk[65536];
        DWORD got = 0;
        if (!ReadFile(out_read, chunk, sizeof(chunk), &got, nullptr) || got == 0) {
            out_ended = true;
            return false;
        }
        buf.append(chunk, got);
        return true;
    }
};

ChildProcess::ChildProcess(const std::vector<std::string>& args) : impl_(std::make_unique<Impl>()) {
    std::wstring cmd;
    for (const std::string& a : args) {
        if (!cmd.empty()) cmd += L' ';
        cmd += quote_arg(hydra::utf8_to_wide(a));
    }
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE in_read = nullptr, out_write = nullptr, err_write = nullptr;
    if (!CreatePipe(&in_read, &impl_->in_write, &sa, 0) ||
        !CreatePipe(&impl_->out_read, &out_write, &sa, 0) ||
        !CreatePipe(&impl_->err_read, &err_write, &sa, 0))
        throw GitError("could not make pipes to start " + args.at(0));
    // Only the child's ends are inherited.
    SetHandleInformation(impl_->in_write, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(impl_->out_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(impl_->err_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = in_read;
    si.hStdOutput = out_write;
    si.hStdError = err_write;
    PROCESS_INFORMATION pi{};
    const BOOL started = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(in_read);
    CloseHandle(out_write);
    CloseHandle(err_write);
    if (!started) {
        CloseHandle(impl_->in_write);
        CloseHandle(impl_->out_read);
        CloseHandle(impl_->err_read);
        throw GitError("could not start " + args.at(0) + " (is it on PATH?)");
    }
    CloseHandle(pi.hThread);
    impl_->process = pi.hProcess;
    // stderr drains on its own thread, so a child that fills it never
    // blocks while the caller reads stdout.
    Impl* impl = impl_.get();
    impl_->err_thread = std::thread([impl] {
        char chunk[4096];
        DWORD got = 0;
        while (ReadFile(impl->err_read, chunk, sizeof(chunk), &got, nullptr) && got > 0)
            impl->err.append(chunk, got);
    });
}

ChildProcess::~ChildProcess() {
    if (!impl_->process) return;
    std::string err;
    wait(err);
}

void ChildProcess::write(const std::string& bytes) {
    size_t at = 0;
    while (at < bytes.size()) {
        DWORD put = 0;
        const DWORD want = static_cast<DWORD>(std::min<size_t>(bytes.size() - at, 1 << 20));
        if (!WriteFile(impl_->in_write, bytes.data() + at, want, &put, nullptr) || put == 0)
            throw GitError("a child process stopped reading its input");
        at += put;
    }
}

void ChildProcess::close_stdin() {
    if (impl_->in_write) CloseHandle(impl_->in_write);
    impl_->in_write = nullptr;
}

bool ChildProcess::read_line(std::string& line) {
    size_t nl;
    while ((nl = impl_->buf.find('\n')) == std::string::npos)
        if (!impl_->fill()) break;
    if (nl == std::string::npos) {
        if (impl_->buf.empty()) return false;
        line = std::move(impl_->buf);
        impl_->buf.clear();
        return true;
    }
    line = impl_->buf.substr(0, nl);
    impl_->buf.erase(0, nl + 1);
    return true;
}

bool ChildProcess::read_exact(size_t n, std::string& out) {
    while (impl_->buf.size() < n)
        if (!impl_->fill()) return false;
    out = impl_->buf.substr(0, n);
    impl_->buf.erase(0, n);
    return true;
}

std::string ChildProcess::read_all() {
    while (impl_->fill()) {
    }
    std::string all = std::move(impl_->buf);
    impl_->buf.clear();
    return all;
}

int ChildProcess::wait(std::string& err) {
    if (!impl_->process) return -1;
    close_stdin();
    WaitForSingleObject(impl_->process, INFINITE);
    if (impl_->err_thread.joinable()) impl_->err_thread.join();
    DWORD code = 0;
    GetExitCodeProcess(impl_->process, &code);
    CloseHandle(impl_->process);
    CloseHandle(impl_->out_read);
    CloseHandle(impl_->err_read);
    impl_->process = nullptr;
    err = std::move(impl_->err);
    return static_cast<int>(code);
}

std::vector<std::string> run_git(const std::string& dir, const std::vector<std::string>& args) {
    std::vector<std::string> full = {"git", "-C", dir};
    full.insert(full.end(), args.begin(), args.end());
    ChildProcess p(full);
    p.close_stdin();
    const std::string out = p.read_all();
    std::string err;
    if (p.wait(err) != 0)
        throw GitError("git " + join(args) + " failed: " + trimmed(out + err));
    return split_lines(out);
}

CatFileBatch::CatFileBatch(const std::string& root)
    : proc_(std::make_unique<ChildProcess>(
          std::vector<std::string>{"git", "-C", root, "cat-file", "--batch"})) {}

CatFileBatch::~CatFileBatch() = default;

std::optional<std::string> CatFileBatch::raw(const std::string& rev, const std::string& path) {
    const std::string name = rev + ":" + path;
    proc_->write(name + "\n");
    std::string header;
    if (!proc_->read_line(header)) throw GitError("git cat-file --batch stopped before " + name);
    // "<hash> <type> <size>", or "<name> missing" (or "ambiguous").
    const size_t sp2 = header.rfind(' ');
    const size_t sp1 = sp2 == std::string::npos ? std::string::npos : header.rfind(' ', sp2 - 1);
    if (sp1 == std::string::npos || header.compare(sp1 + 1, sp2 - sp1 - 1, "blob") != 0) {
        if (header.size() > name.size() && header.compare(0, name.size(), name) == 0) return std::nullopt;
        throw GitError("git cat-file --batch: " + name + " is not a file (" + header + ")");
    }
    const size_t size = std::stoull(header.substr(sp2 + 1));
    std::string body, nl;
    if (!proc_->read_exact(size, body) || !proc_->read_exact(1, nl))
        throw GitError("git cat-file --batch stopped inside " + name);
    return body;
}

std::string CatFileBatch::text(const std::string& rev, const std::string& path) {
    std::optional<std::string> r = raw(rev, path);
    if (!r) throw GitError("git cat-file: " + rev + " has no file " + path);
    return tip_text_form(*r);
}

std::string tip_text_form(const std::string& raw) {
    // The script read each file as PowerShell reads a program's output: one
    // line per "\r\n", "\n" or lone "\r", a byte order mark dropped, and the
    // lines joined with "\n".
    size_t from = raw.compare(0, 3, "\xEF\xBB\xBF") == 0 ? 3 : 0;
    std::string out;
    out.reserve(raw.size());
    for (size_t i = from; i < raw.size(); ++i) {
        const char c = raw[i];
        if (c == '\r') {
            out += '\n';
            if (i + 1 < raw.size() && raw[i + 1] == '\n') ++i;
        } else {
            out += c;
        }
    }
    if (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

Repo::Repo(const std::string& path) {
    std::vector<std::string> top;
    try {
        top = run_git(path, {"rev-parse", "--show-toplevel"});
    } catch (const GitError&) {
        throw GitError("not a git repository: " + path);
    }
    if (top.empty() || trimmed(top[0]).empty()) throw GitError("not a git repository: " + path);
    root_ = trimmed(top[0]);
}

Repo::~Repo() = default;

std::vector<std::string> Repo::git(const std::vector<std::string>& args) const {
    return run_git(root_, args);
}

std::string Repo::resolve_commit(const std::string& name) const {
    const std::vector<std::string> out = git({"rev-parse", "--verify", name + "^{commit}"});
    if (out.empty()) throw GitError("git rev-parse --verify " + name + "^{commit} printed nothing");
    return out[0];
}

CatFileBatch& Repo::files() {
    if (!files_) files_ = std::make_unique<CatFileBatch>(root_);
    return *files_;
}

}  // namespace precheck
