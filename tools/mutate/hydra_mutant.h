// The switches of the mutation build. Each planted bug in a rewritten copy of
// a source file sits behind its own numbered switch, and the HYDRA_MUTANT
// environment variable names the one switch that is on. With it unset every
// switch is off, so one build holds every planted bug and each runs alone.
// The plan is docs/superpowers/plans/2026-11-15-mutation-tool.md, "Where the
// switches live".
//
// Each switch also notes that its line ran. The doctest listener in
// tools/mutate/listener.cpp reads those notes, so the mapping run can say
// which test reaches which switch.
//
// Only the mutation build includes this header; the normal build never does.

#ifndef HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H
#define HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "../../tests/env_util.h"

namespace hydra_mutant {

// The switch HYDRA_MUTANT names, or -1 when it is unset. A value that is not a
// whole number of zero or more stops the program: a typo would otherwise run
// every switch off and report each planted bug as uncaught.
inline long read_active_switch() {
    const std::optional<std::string> text = read_env("HYDRA_MUTANT");
    if (!text) return -1;
    char* end = nullptr;
    errno = 0;
    const long n = std::strtol(text->c_str(), &end, 10);
    if (text->empty() || end != text->c_str() + text->size() || errno != 0 || n < 0) {
        std::fprintf(stderr, "HYDRA_MUTANT=\"%s\" is not a switch number\n", text->c_str());
        std::fflush(stderr);
        std::abort();
    }
    return n;
}

// Read once, on first use. A function-local static is initialised the first
// time any switch runs, so a switch met during static start-up in another file
// still sees the variable.
inline long active_switch() {
    static const long active = read_active_switch();
    return active;
}

// The reach notes. The listener starts a new period at each test, and a switch
// notes its number once per period. A period of 0 is never current, so every
// switch notes itself the first time it runs.
struct ReachLog {
    std::mutex mutex;
    std::atomic<unsigned> period{1};
    std::vector<long> reached;  // switch numbers noted in the current period
};

inline ReachLog& reach_log() {
    static ReachLog log;
    return log;
}

// The period in which switch N last noted itself. One variable per switch,
// so the common case (already noted this period) costs one atomic load and
// takes no lock.
template <long N>
inline std::atomic<unsigned> noted_in_period{0};

template <long N>
inline void note_reached() {
    ReachLog& log = reach_log();
    const unsigned now = log.period.load(std::memory_order_acquire);
    if (noted_in_period<N>.load(std::memory_order_relaxed) == now) return;
    std::lock_guard<std::mutex> lock(log.mutex);
    // The period may have moved, or another thread noted N, while this one
    // waited for the lock; both are read again under it.
    const unsigned current = log.period.load(std::memory_order_relaxed);
    if (noted_in_period<N>.load(std::memory_order_relaxed) == current) return;
    noted_in_period<N>.store(current, std::memory_order_relaxed);
    log.reached.push_back(N);
}

// Hands back the numbers noted since the last call and starts a new period.
// The listener calls it at each test's start and end.
inline std::vector<long> take_reached() {
    ReachLog& log = reach_log();
    std::lock_guard<std::mutex> lock(log.mutex);
    std::vector<long> out;
    out.swap(log.reached);
    log.period.fetch_add(1, std::memory_order_acq_rel);
    return out;
}

// Notes switch N, then says whether it is the one switch that is on.
template <long N>
inline bool is_on() {
    note_reached<N>();
    return active_switch() == N;
}

}  // namespace hydra_mutant

// HYDRA_MUT(n, original, changed): the changed expression when switch n is on,
// otherwise the original. Only the chosen side is evaluated. n must be an
// integer literal. The preprocessor splits macro arguments at every comma that
// is not inside round brackets, angle brackets included, so a side holding
// such a comma must itself be passed in round brackets.
#define HYDRA_MUT(n, original, changed) \
    (::hydra_mutant::is_on<(n)>() ? (changed) : (original))

#endif  // HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H
