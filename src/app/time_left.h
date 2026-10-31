// How long a job has left, from the rate over its recent work (D271 item 4).
// The batch strip and the Preview loader both read their time left from here;
// each brings its own unit (charts or bytes) and its own window.

#ifndef HYDRA_APP_TIME_LEFT_H
#define HYDRA_APP_TIME_LEFT_H

#include <algorithm>
#include <deque>
#include <optional>
#include <string>

#include "app/display_format.h"  // time_left_text

namespace hydra::app {

// A time-left figure changes at most once in this many seconds (D276 item 8),
// and the Preview's byte window spans at least this long. D273 item 1 keeps
// the value ("the 1 s time-left pacing").
inline constexpr double kTimeLeftStepSeconds = 1.0;

// Turns readings of "work done so far" into seconds left. Times are seconds
// on the job's own clock: the clock's caller decides what counts as time (the
// batch leaves paused stretches out of it).
class TimeLeftClock {
public:
    // The window is the work done since the clock's last answer, at least
    // kTimeLeftStepSeconds of it: the Preview loader's bytes.
    static TimeLeftClock over_last_second() { return TimeLeftClock(0); }
    // The window is the last `units` units finished: the batch strip's
    // charts (D283). Nothing shows until that many have finished. The rate
    // is `units` over the time from the `units`-th last finish to now.
    static TimeLeftClock over_last_units(int units) { return TimeLeftClock((std::max)(1, units)); }

    // `done` units of work had finished by `now_s`. Units that finish in one
    // reading share its time.
    void record(double now_s, double done) {
        if (units_ == 0) {
            if (!window_start_s_) {
                window_start_s_ = now_s;
                window_start_done_ = done;
            }
        } else {
            const double before = done_;
            if (done > before) {
                const double fresh = (std::min)(done - before, static_cast<double>(units_));
                for (int i = 0; i < static_cast<int>(fresh); ++i) finishes_.push_back(now_s);
                while (static_cast<int>(finishes_.size()) > units_) finishes_.pop_front();
            }
        }
        done_ = (std::max)(done_, done);
    }

    // Seconds left of `total` at the window's rate, or empty while the rate
    // is not known. A reading less than kTimeLeftStepSeconds after the last
    // answer returns that answer.
    std::optional<double> read(double now_s, double total) {
        if (answered_at_s_ && now_s - *answered_at_s_ < kTimeLeftStepSeconds) return answer_;
        double rate = 0.0;  // units a second over the window
        if (units_ == 0) {
            if (!window_start_s_) return answer_;
            const double span = now_s - *window_start_s_;
            if (span < kTimeLeftStepSeconds) return answer_;
            rate = (std::max)(0.0, done_ - window_start_done_) / span;
            window_start_s_ = now_s;
            window_start_done_ = done_;
        } else {
            if (static_cast<int>(finishes_.size()) < units_) return answer_;
            const double span = now_s - finishes_.front();
            if (!(span > 0.0)) return answer_;
            rate = units_ / span;
        }
        answer_ = rate > 0.0 ? std::optional<double>((std::max)(0.0, total - done_) / rate)
                             : std::nullopt;
        answered_at_s_ = now_s;
        return answer_;
    }

    // record, then read.
    std::optional<double> update(double now_s, double done, double total) {
        record(now_s, done);
        return read(now_s, total);
    }

private:
    explicit TimeLeftClock(int units) : units_(units) {}

    int units_;           // 0: the last-second window; else the last `units_` finished
    double done_ = 0.0;   // the most work any reading has reported
    std::optional<double> window_start_s_;  // the last-second window's start
    double window_start_done_ = 0.0;
    std::deque<double> finishes_;  // the last units_ finish times, oldest first
    std::optional<double> answer_;
    std::optional<double> answered_at_s_;
};

// The time-left line of a job `age_s` old whose gate is `gate_s`:
// time_left_text's words for `time_left_s`, or "" before the gate or while the
// rate is not known (`time_left_s` < 0).
inline std::string time_left_line(double age_s, double gate_s, double time_left_s) {
    if (age_s < gate_s || !(time_left_s >= 0.0)) return "";
    return time_left_text(time_left_s);
}

}  // namespace hydra::app

#endif  // HYDRA_APP_TIME_LEFT_H
