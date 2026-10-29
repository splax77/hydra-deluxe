// The two base classes every background job derives from: the thread and
// cancel lifecycle (JobBase) and the finished flag, ok/error result surface
// and outcome on top of it (ResultJobBase). Read by the job headers in src/ui/
// (library_jobs.h, preview_load_job.h, dm_jobs.h, update_job.h); the views
// only ever touch the concrete jobs, never these directly. It lives in app/
// because it needs no ImGui, so a library without the GUI can hold a job.

#ifndef HYDRA_APP_JOB_BASE_H
#define HYDRA_APP_JOB_BASE_H

#include <atomic>
#include <exception>
#include <functional>
#include <string>
#include <thread>

#include "app/user_messages.h"
#include "core/error_kind.h"

namespace hydra::ui {

// Thrown by JobBase::throw_if_cancelled() between a job's steps. run_guarded
// turns it into a failed run whose error() reads Cancelled's text. Its own
// type exists so library_jobs.cpp can catch a job's stop by type.
struct JobCancelled : Cancelled {
    JobCancelled() = default;
};

// Common lifecycle for every job: an owning worker thread and a cancel flag
// the worker polls. Derived destructors call shutdown() so the join happens
// while the derived members the worker touches are still alive. It has no
// finished flag: the scan and the batch publish theirs in their snapshots,
// so only ResultJobBase, whose run sets it, offers finished().
class JobBase {
public:
    void cancel() { cancel_.store(true); }
    bool is_cancelled() const { return cancel_.load(); }

    JobBase(const JobBase&) = delete;
    JobBase& operator=(const JobBase&) = delete;

protected:
    JobBase() = default;
    ~JobBase() = default;  // jobs are held and destroyed by concrete type

    void spawn(std::function<void()> fn) { thread_ = std::thread(std::move(fn)); }
    void shutdown() {
        cancel_.store(true);
        if (thread_.joinable()) thread_.join();
    }
    // Stops the job here when cancel() was called. The UI thread joins a
    // cancelled job, so a long job gives up between its steps instead of
    // running to the end while the window waits.
    void throw_if_cancelled() const {
        if (cancel_.load()) throw JobCancelled{};
    }

    std::thread thread_;
    std::atomic<bool> cancel_{false};
};

// How a finished result job ended. ResultJobBase::outcome() is the one place
// that weighs its two inputs, the run's return and the cancel flag.
enum class JobOutcome { Done, Cancelled, Failed };

// Adds the finished flag, the ok/error result surface and the guarded-run
// tail shared by the jobs that produce one result instead of a mutex-guarded
// snapshot. The finished flag's release-store publishes everything the
// worker wrote before it (the render thread's finished() load acquires).
class ResultJobBase : public JobBase {
public:
    bool finished() const { return finished_.load(); }
    // How the run ended, read once finished() is true. A run that returned
    // ok is Done even when a Cancel came after its last step: its result is
    // whole, so the reports keep it (cancel_dm_report says the same). The
    // click's analysis is the one exception, written beside its reader in
    // AppState::update_view_job.
    JobOutcome outcome() const {
        if (ok_) return JobOutcome::Done;
        if (is_cancelled()) return JobOutcome::Cancelled;
        return JobOutcome::Failed;
    }
    bool ok() const { return ok_; }
    // The details line under message(): app::plain_error_detail of the
    // exception, so empty when the message already quotes it. A cancelled
    // job still reads "cancelled" here.
    const std::string& error() const { return error_; }
    // What the user reads: app::plain_error of the exception. Empty when the
    // job succeeded or was cancelled, because a cancel is the user's own
    // click and needs no message.
    const std::string& message() const { return message_; }

protected:
    // Runs the job body; f returns whether the job succeeded. Any escaping
    // exception becomes the job's error text and plain message. Always
    // publishes finished.
    template <class F>
    void run_guarded(F&& f) {
        try {
            ok_ = f();
        } catch (const std::exception& e) {
            fail(e);
            return;
        }
        finished_.store(true);
    }

    // Records a failed run: the details line, the plain message unless the
    // job was cancelled, not ok, and then publishes finished. run_guarded's
    // catch and a job whose thread cannot start both end here.
    void fail(const std::exception& e) {
        error_ = app::plain_error_detail(e);
        if (!is_cancelled()) message_ = app::plain_error(e);
        ok_ = false;
        finished_.store(true);
    }

    std::atomic<bool> finished_{false};
    bool ok_ = false;
    std::string error_;
    std::string message_;
};

}  // namespace hydra::ui

#endif  // HYDRA_APP_JOB_BASE_H
