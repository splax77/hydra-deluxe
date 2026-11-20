// How long a job has left, from the rate of its work (D271 item 4).
// The batch strip and the Preview loader both read their time left from here;
// each brings its own unit (charts or bytes) and its own rule for the rate.

#ifndef HYDRA_APP_TIME_LEFT_H
#define HYDRA_APP_TIME_LEFT_H

#include <algorithm>
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
    // The rate is the work done since the clock's last answer, over at least
    // kTimeLeftStepSeconds: the Preview loader's bytes.
    static TimeLeftClock over_last_second() { return TimeLeftClock(Rule::LastSecond, 0.0); }
    // The rate is all the work done over all the job's time so far, on a
    // clock that reads 0 when the job starts: the batch strip's charts
    // (D352). Nothing shows until `shows_after` units have finished.
    static TimeLeftClock since_start(double shows_after) {
        return TimeLeftClock(Rule::SinceStart, (std::max)(1.0, shows_after));
    }

    // `done` units of work had finished by `now_s`. Only the last-second rule
    // keeps it, as its window's start; the since-start rule reads nothing but
    // read's own arguments.
    void record(double now_s, double done) {
        if (rule_ == Rule::LastSecond && !window_start_s_) {
            window_start_s_ = now_s;
            window_start_done_ = done;
        }
    }

    // Seconds left of `total`, `done` of it finished (the caller's own count),
    // at the rule's rate, or empty while the rate is not known. A reading
    // less than kTimeLeftStepSeconds after the last answer returns that answer.
    std::optional<double> read(double now_s, double done, double total) {
        if (answered_at_s_ && now_s - *answered_at_s_ < kTimeLeftStepSeconds) return answer_;
        double rate = 0.0;  // units a second
        if (rule_ == Rule::LastSecond) {
            if (!window_start_s_) return answer_;
            const double span = now_s - *window_start_s_;
            if (span < kTimeLeftStepSeconds) return answer_;
            rate = (std::max)(0.0, done - window_start_done_) / span;
            window_start_s_ = now_s;
            window_start_done_ = done;
        } else {
            if (done < shows_after_ || !(now_s > 0.0)) return answer_;
            rate = done / now_s;
        }
        answer_ = rate > 0.0 ? std::optional<double>((std::max)(0.0, total - done) / rate)
                             : std::nullopt;
        answered_at_s_ = now_s;
        return answer_;
    }

    // record, then read.
    std::optional<double> update(double now_s, double done, double total) {
        record(now_s, done);
        return read(now_s, done, total);
    }

private:
    enum class Rule { LastSecond, SinceStart };
    TimeLeftClock(Rule rule, double shows_after) : rule_(rule), shows_after_(shows_after) {}

    Rule rule_;
    double shows_after_;  // the since-start rule's gate, in units finished
    std::optional<double> window_start_s_;  // the last-second window's start
    double window_start_done_ = 0.0;
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
