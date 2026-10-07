// A small thread pool shared by the library scan (hashing chart files) and
// the batch runner (analyzing charts).
//
// Picture a kitchen pass. Cooks (worker threads) each take the next ticket,
// cook it, and set the plate on the pass. One runner (the calling thread)
// carries plates out in the order they land. The pass only holds a few
// plates: when it is full, a cook waits with the next plate in hand until
// the runner clears a space. When the kitchen closes early, the last cook to
// leave rings the bell, so the runner never stands at an empty pass waiting
// for a plate nobody is cooking.

#ifndef HYDRA_APP_WORK_POOL_H
#define HYDRA_APP_WORK_POOL_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace hydra::app {

constexpr size_t kWorkPoolWaitingPerWorker = 2;

// Runs work(i) for every i in [0, count) on up to `worker_count` threads, and
// hands each result to consume(Result&&) on the calling thread in the order
// the results finish. consume never runs on a worker, so it may write the
// store or touch UI state.
//
// At most kWorkPoolWaitingPerWorker results per worker wait for consume. A
// worker that finishes while that line is full holds its result until there
// is room, so a slow consume slows the workers down instead of letting
// results pile up: a whole-library batch once queued about 6 GB of analyzed
// charts behind its store writes and ran the machine out of memory.
//
// Once *cancel is set, workers stop taking new items. Items already running
// finish (work decides how fast) and still reach consume, so consume sees
// every item that was started and nothing else. The call returns once every
// worker has left. The last one out wakes the consumer while holding the
// lock, so the wake-up cannot slip in between the consumer's check and its
// wait.
//
// If consume throws, workers stop taking items, the pool is joined, and the
// exception propagates. work must not throw: catch inside it. Result must be
// default-constructible and movable.
//
// worker_count must be at least 1; batch_worker_count (app/analysis.h) owns
// that floor. A smaller count throws std::invalid_argument naming it, since
// with no cooks nothing would ever reach the pass.
//
// idle() runs on the calling thread whenever the runner finds the pass empty,
// just before it waits, and once more after the last plate. Most callers leave
// it out; run_batch passes its flush_group (app/analysis.cpp).
struct NoIdle {
    void operator()() const {}
};

template <typename Result, typename Work, typename Consume, typename Idle = NoIdle>
void run_work_pool(size_t count, int worker_count, const std::atomic<bool>* cancel,
                   Work&& work, Consume&& consume, Idle&& idle = Idle{}) {
    if (worker_count < 1)
        throw std::invalid_argument("run_work_pool: worker count " +
                                    std::to_string(worker_count) + " is below 1");
    if (count == 0) return;

    std::mutex mu;
    std::condition_variable cv;    // the consumer waits here for a result
    std::condition_variable room;  // workers wait here for space in the line
    std::deque<Result> finished;   // guarded by mu
    std::atomic<size_t> next{0};
    std::atomic<bool> stop{false};

    const size_t nworkers = std::min(count, static_cast<size_t>(worker_count));
    const size_t max_waiting = kWorkPoolWaitingPerWorker * nworkers;
    size_t live = nworkers;  // workers still running; guarded by mu

    std::vector<std::thread> pool;
    pool.reserve(nworkers);
    for (size_t w = 0; w < nworkers; ++w) {
        pool.emplace_back([&] {
            for (;;) {
                if (stop.load() || (cancel && cancel->load())) break;
                const size_t i = next.fetch_add(1);
                if (i >= count) break;
                Result r = work(i);
                {
                    std::unique_lock<std::mutex> lock(mu);
                    // stop is set under mu, so its wake-up can't slip past here.
                    room.wait(lock, [&] { return finished.size() < max_waiting || stop.load(); });
                    if (stop.load()) break;  // consume threw: nobody takes r
                    finished.push_back(std::move(r));
                }
                cv.notify_one();
            }
            std::lock_guard<std::mutex> lock(mu);
            if (--live == 0) cv.notify_one();
        });
    }

    try {
        for (;;) {
            bool empty;
            {
                std::lock_guard<std::mutex> lock(mu);
                empty = finished.empty();
            }
            if (empty) idle();
            Result r;
            {
                std::unique_lock<std::mutex> lock(mu);
                cv.wait(lock, [&] { return !finished.empty() || live == 0; });
                if (finished.empty()) break;  // every worker has left
                r = std::move(finished.front());
                finished.pop_front();
            }
            room.notify_one();
            consume(std::move(r));
        }
        idle();
    } catch (...) {
        {
            std::lock_guard<std::mutex> lock(mu);
            stop.store(true);
        }
        room.notify_all();
        for (std::thread& t : pool) t.join();
        throw;
    }
    for (std::thread& t : pool) t.join();
}

}  // namespace hydra::app

#endif  // HYDRA_APP_WORK_POOL_H
