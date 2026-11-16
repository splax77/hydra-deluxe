// The switches of the mutation build. Each planted bug in a rewritten copy of
// a source file sits behind its own numbered switch, and the HYDRA_MUTANT
// environment variable names the one switch that is on. With it unset every
// switch is off, so one build holds every planted bug and each runs alone.
// The plan has been removed; read its section "Where the switches live" with
// `git show 5a0848fc:docs/superpowers/plans/2026-11-15-mutation-tool.md`.
//
// Each switch also notes that its line ran. The doctest listener in
// tools/mutate/listener.cpp reads those notes, so the mapping run can say
// which test reaches which switch.
//
// With every switch off a copy must behave exactly as its original, and some
// tests count heap allocations while they run. So once static start-up is
// over, nothing a switch does allocates or takes a lock: the variable is read
// before main, and each note is a few atomic operations on storage that
// exists from the start.
//
// Only the mutation build includes this header; the normal build never does.
// The rewriter (rewrite.py, include_at) writes the include line into each copy,
// after the copy's own leading #define, #undef and #pragma lines, which the
// C runtime's headers read when they are first included.

#ifndef HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H
#define HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
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

// Read once. A function-local static is set up the first time any switch
// runs, so a switch met during static start-up in another file still sees
// the variable. Reading it allocates (read_env returns a std::string), so the
// listener (listener.cpp, read_at_start) makes that first time come before
// main, and no test is ever the first to read it.
inline long active_switch() {
    static const long active = read_active_switch();
    return active;
}

// The reach notes. The listener starts a new period at each test's start and
// end, and a switch notes itself once per period. Period 0 is never current,
// so every switch notes itself the first time it runs.
//
// Each switch has one Reach of its own, a template variable that needs no
// constructor at run time, so it exists before any code runs. The first note
// links it onto reach_list with a compare-and-swap, which needs neither a
// lock nor the heap; the listener walks that list.
struct Reach {
    std::atomic<unsigned> period{0};            // the period it last noted itself in
    std::atomic<unsigned long long> order{0};   // note_clock when it did, for the listener's order
    std::atomic<bool> linked{false};
    long number = 0;                            // set before it is linked, never after
    Reach* next = nullptr;                      // likewise
};

inline std::atomic<unsigned> current_period{1};
inline std::atomic<unsigned long long> note_clock{0};
inline std::atomic<Reach*> reach_list{nullptr};

template <long N>
inline Reach reach_of{};

template <long N>
inline void note_reached() {
    Reach& r = reach_of<N>;
    for (;;) {
        const unsigned now = current_period.load(std::memory_order_acquire);
        unsigned was = r.period.load(std::memory_order_relaxed);
        if (was == now) return;
        if (!r.period.compare_exchange_strong(was, now, std::memory_order_acq_rel)) continue;
        r.order.store(note_clock.fetch_add(1, std::memory_order_relaxed), std::memory_order_relaxed);
        if (!r.linked.exchange(true, std::memory_order_acq_rel)) {
            r.number = N;
            Reach* head = reach_list.load(std::memory_order_relaxed);
            do {
                r.next = head;
            } while (!reach_list.compare_exchange_weak(head, &r, std::memory_order_release,
                                                       std::memory_order_relaxed));
        }
        // The listener may have started a new period while this note was
        // made; if so the note belongs to the new one, so it is made again.
        if (current_period.load(std::memory_order_acquire) == now) return;
    }
}

// Hands back the numbers noted in the period that is ending, in the order
// they were first noted, and starts a new period. The listener calls it at
// each test's start and end, never inside a test, so it may allocate.
inline std::vector<long> take_reached() {
    const unsigned ended = current_period.fetch_add(1, std::memory_order_acq_rel);
    std::vector<std::pair<unsigned long long, long>> noted;
    for (Reach* r = reach_list.load(std::memory_order_acquire); r != nullptr; r = r->next) {
        if (r->period.load(std::memory_order_acquire) == ended)
            noted.emplace_back(r->order.load(std::memory_order_relaxed), r->number);
    }
    std::sort(noted.begin(), noted.end());
    std::vector<long> out;
    out.reserve(noted.size());
    for (const auto& n : noted) out.push_back(n.second);
    return out;
}

// Notes switch N, then says whether it is the one switch that is on.
template <long N>
inline bool is_on() {
    note_reached<N>();
    return active_switch() == N;
}

// The compare helpers. The rewriter (rewrite.py, changed_span) writes a
// flipped comparison as a call to one of these on its two operands, with the
// switch number as N, because a type may define == and no !=, or < and no >=,
// and the flipped operator itself would then not build. Each helper uses the
// real operator when the operands have it, and otherwise builds the same
// meaning from the operator they do have. With neither, the static_assert
// stops the build; the error names the helper and N, the switch.
//
// The operands come by const reference: a bit-field binds to one through a
// temporary, and no operand is copied or changed.

#define HYDRA_MUTANT_HAS_OP(name, op)                                                        \
    template <class A, class B, class = void>                                                \
    struct name : std::false_type {};                                                        \
    template <class A, class B>                                                              \
    struct name<A, B, std::void_t<decltype(std::declval<const A&>() op std::declval<const B&>())>> \
        : std::true_type {};
HYDRA_MUTANT_HAS_OP(has_eq, ==)
HYDRA_MUTANT_HAS_OP(has_ne, !=)
HYDRA_MUTANT_HAS_OP(has_lt, <)
HYDRA_MUTANT_HAS_OP(has_le, <=)
HYDRA_MUTANT_HAS_OP(has_gt, >)
HYDRA_MUTANT_HAS_OP(has_ge, >=)
#undef HYDRA_MUTANT_HAS_OP

// False for every N, but only once the helper is instantiated, so the
// static_assert fires for the one switch whose operands have no operator.
template <long N, class... T>
inline constexpr bool no_operator_for_switch = false;

// The original comparison compiled with no warning, but inside a helper a
// literal operand is no longer a constant, so MSVC can warn about a signed
// and unsigned mix (C4018, C4389) or a bool and int mix (C4805) that the
// original, with the same meaning, did not. /WX would turn those into errors.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4018 4389 4805)
#endif

template <long N, class A, class B>
inline bool eq(const A& a, const B& b) {
    if constexpr (has_eq<A, B>::value) {
        return a == b;
    } else if constexpr (has_ne<A, B>::value) {
        return !(a != b);
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::eq<N>: the operands of switch N have neither == nor !=");
        (void)a, (void)b;
        return false;
    }
}

template <long N, class A, class B>
inline bool ne(const A& a, const B& b) {
    if constexpr (has_ne<A, B>::value) {
        return a != b;
    } else if constexpr (has_eq<A, B>::value) {
        return !(a == b);
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::ne<N>: the operands of switch N have neither != nor ==");
        (void)a, (void)b;
        return false;
    }
}

template <long N, class A, class B>
inline bool lt(const A& a, const B& b) {
    if constexpr (has_lt<A, B>::value) {
        return a < b;
    } else if constexpr (has_gt<B, A>::value) {
        return b > a;
    } else if constexpr (has_ge<A, B>::value) {
        return !(a >= b);
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::lt<N>: the operands of switch N have none of a < b, b > a, a >= b");
        (void)a, (void)b;
        return false;
    }
}

template <long N, class A, class B>
inline bool le(const A& a, const B& b) {
    if constexpr (has_le<A, B>::value) {
        return a <= b;
    } else if constexpr (has_lt<B, A>::value) {
        return !(b < a);
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::le<N>: the operands of switch N have neither a <= b nor b < a");
        (void)a, (void)b;
        return false;
    }
}

template <long N, class A, class B>
inline bool gt(const A& a, const B& b) {
    if constexpr (has_gt<A, B>::value) {
        return a > b;
    } else if constexpr (has_lt<B, A>::value) {
        return b < a;
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::gt<N>: the operands of switch N have neither a > b nor b < a");
        (void)a, (void)b;
        return false;
    }
}

template <long N, class A, class B>
inline bool ge(const A& a, const B& b) {
    if constexpr (has_ge<A, B>::value) {
        return a >= b;
    } else if constexpr (has_lt<A, B>::value) {
        return !(a < b);
    } else {
        static_assert(no_operator_for_switch<N, A, B>, "hydra_mutant::ge<N>: the operands of switch N have neither a >= b nor a < b");
        (void)a, (void)b;
        return false;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace hydra_mutant

// HYDRA_MUT(n, original, changed): the changed expression when switch n is on,
// otherwise the original. Only the chosen side is evaluated. n must be an
// integer literal. The preprocessor splits macro arguments at every comma that
// is not inside round brackets, angle brackets included, so a side holding
// such a comma must itself be passed in round brackets.
#define HYDRA_MUT(n, original, changed) \
    (::hydra_mutant::is_on<(n)>() ? (changed) : (original))

#endif  // HYDRA_TOOLS_MUTATE_HYDRA_MUTANT_H
