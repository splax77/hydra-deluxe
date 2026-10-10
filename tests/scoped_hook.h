// A process-wide test seam (net::set_fetcher, set_report_analyzer_for_test
// and the like) installed for one scope. The seams hold one callable for the
// whole process, so a hook cleared by hand on a test's last line is skipped
// when a REQUIRE fails partway: the next test inherits a hook whose captured
// locals are gone. This one is cleared on every exit path.
//
// Declare it after the locals the hook captures, so it is destroyed (and the
// seam cleared) before they are. A seam the code calls through the global at
// call time (the fetcher, open-in-browser) must also outlive every thread
// that can call it, so declare the guard before the object that joins those
// threads (the AppState or the job).
//
// The seams have no getter, so the destructor puts back the empty hook, which
// is what each seam starts with and what every test leaves.

#ifndef HYDRA_TESTS_SCOPED_HOOK_H
#define HYDRA_TESTS_SCOPED_HOOK_H

#include <functional>
#include <utility>

class ScopedHook {
public:
    // Calls set(hook, extra...) now and set({}, extra...) when the scope ends.
    // `extra` is a seam's other arguments, such as the batch's worker count.
    template <class Set, class Hook, class... Extra>
    ScopedHook(Set set, Hook&& hook, Extra... extra)
        : clear_([set, extra...] { set({}, extra...); }) {
        set(std::forward<Hook>(hook), extra...);
    }
    ~ScopedHook() { clear_(); }
    ScopedHook(const ScopedHook&) = delete;
    ScopedHook& operator=(const ScopedHook&) = delete;

private:
    std::function<void()> clear_;
};

#endif  // HYDRA_TESTS_SCOPED_HOOK_H
