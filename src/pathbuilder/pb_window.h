// The Path Builder's main window, drawn each frame in the approved mockup's
// order (docs/superpowers/specs/2026-10-30-path-builder-mockup/
// mockup_template.html): the title and mode line, the intro, the three
// scores and the buttons, the guide line, the legend, the Preview's
// whole-song timeline (ui::PreviewTimeline), then the Preview's highway with
// the steps panel beside it. Everything drawn about the chart is the
// Preview's; every sentence is pb_text's. main.cpp and the headless runner
// (tests/pb_ui) both draw through draw(), so a test sees what the player
// sees. Plan: docs/superpowers/plans/2026-10-30-path-builder-redesign.md,
// task PB-5.

#ifndef HYDRA_PATHBUILDER_PB_WINDOW_H
#define HYDRA_PATHBUILDER_PB_WINDOW_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pathbuilder/pb_state.h"
#include "pathbuilder/pb_text.h"  // GuideMessage

struct ID3D11Device;
struct ID3D11DeviceContext;
struct HWND__;
typedef HWND__* HWND;

namespace hydra::ui {
class PreviewController;
}

namespace hydra::pathbuilder {

// The ImGui window draw_window fills; the headless runner's home_ref.
inline constexpr const char* kMainWindowName = "Path Builder";

// The labels the screen shows, each the mockup's words.
inline constexpr const char* kYourScoreLabel = "Your score";
inline constexpr const char* kBestPossibleLabel = "Best possible";
inline constexpr const char* kBestReachLabel = "Best you can still reach";
inline constexpr const char* kYourScoreHint = "what your activations so far score";
inline constexpr const char* kBestPossibleHint = "Hydra's best path for this chart";
inline constexpr const char* kBestReachHint =
    "keeping your activations and playing the rest perfectly";
inline constexpr const char* kUndoLabel = "Undo";
inline constexpr const char* kFinishTooltip =
    "Keeps your activations and lets Hydra choose the best rest of the path";
inline constexpr const char* kLegendCanText = "You can activate here";
inline constexpr const char* kLegendMineText = "Your activation";
inline constexpr const char* kLegendCantText = "Can't activate here (hover to see why)";
inline constexpr const char* kLegendSeekText =
    "Click anywhere on the timeline to move the highway there.";
inline constexpr const char* kShowBestPathLabel = "Show Hydra's best path";
inline constexpr const char* kHighwayHeading = "Highway";
inline constexpr const char* kPlayLabel = "Play";
inline constexpr const char* kPauseLabel = "Pause";
inline constexpr const char* kStepsHeading = "Your activations";
inline constexpr const char* kRemoveLastLabel = "Remove";
inline constexpr const char* kRemoveEarlierLabel = "Remove this and later ones";
inline constexpr const char* kEndItAtLabel = "End it at:";
inline constexpr const char* kNoTimingNeededText = "no timing needed";
// The cost under the best next fill on the timeline.
inline constexpr const char* kBestFillLabel = "best";

// The builder's whole state between frames.
class PbApp {
public:
    // `device` and `context` are the D3D11 pair the highway renders on. Null
    // (a test with no device) leaves the highway out.
    PbApp(ID3D11Device* device, ID3D11DeviceContext* context);
    ~PbApp();
    PbApp(const PbApp&) = delete;
    PbApp& operator=(const PbApp&) = delete;

    PbState state;
    // The Win32 window the file dialog sits over; null in the headless runner.
    HWND owner = nullptr;

    // Opens `notespath`, as File > Open, a dropped file and the command line do.
    void open(const std::string& notespath);
    // An open or the highway's load is still running.
    bool busy() const;
    // Stops the highway's audio and frees its scene (PreviewController::close).
    // Call outside a frame, before the device goes.
    void close_preview();

    ui::PreviewController* preview() { return preview_.get(); }
    // Why the last paste, path file open or save changed nothing: the plain
    // sentence and the details line (pb_files.h's Refusal). Empty when none
    // is showing.
    const std::string& refusal() const { return refusal_; }
    const std::string& refusal_detail() const { return refusal_detail_; }
    // The guide line as it reads this frame (guide_text).
    const GuideMessage& guide() const { return guide_; }
    // The step the player selected (a band, a taken fill or a step row), as
    // an index into the path's activations; none after Escape or a change of
    // the player's path.
    std::optional<size_t> selected_step() const;

    // The path the screen shows: the player's own, or the chart's empty path
    // while the player has no window (OpenChart::empty_path). Null with no
    // chart open.
    const Path* shown_path() const;

    // Draws one frame of the window.
    void draw();

private:
    std::unique_ptr<ui::PreviewController> preview_;
    // The chart the screen state below was set up for.
    const OpenChart* chart_seen_ = nullptr;
    // The refusal on screen, and the player's history generation it was
    // made at: any later change to the player's path takes it away.
    std::string refusal_;
    std::string refusal_detail_;
    uint64_t refusal_generation_ = 0;
    // The one-shot guide message (pb_text's GuideMessage) and the history
    // generation it belongs to; it goes when the player's path changes. A
    // removal's message waits in pending_guide_ until its edit lands.
    std::optional<GuideMessage> guide_message_;
    uint64_t guide_message_generation_ = 0;
    std::optional<GuideMessage> pending_guide_;
    GuideMessage guide_;
    // The selected step and the history generation it was picked at.
    std::optional<size_t> selected_;
    uint64_t selected_generation_ = 0;
    // The step a click or an end choice selects once its edit lands, as the
    // mockup selects the step it just made.
    std::optional<size_t> pending_select_;
    // "Show Hydra's best path".
    bool show_best_ = false;
    // While the pointer is on a fill: the fill, and the time and play state
    // the Preview goes back to when the pointer leaves.
    std::optional<size_t> hovered_fill_;
    double hover_return_ms_ = 0.0;
    bool hover_return_playing_ = false;
    // The overlay key of the path the highway shows (app::path_overlay_key),
    // and what it was built for, so it is rebuilt only when that changes.
    std::string overlay_key_;
    const Path* overlay_for_ = nullptr;
    uint64_t overlay_generation_ = 0;  // PathHistory::generation it was built at
    bool overlay_built_ = false;
    // What the window says under the menu when a file dialog never appeared;
    // null when the last dialog did.
    const char* dialog_failed_ = nullptr;

    void take_in_chart();
    void act_on_menu();
    // Shows `sentence` and `detail` as the refusal (empty clears it).
    void set_refusal(std::string sentence, std::string detail);
    // Makes `m` the guide's one-shot message for the path as it is now.
    void set_guide_message(GuideMessage m);
    void start_over();
    void select_step(size_t index);
    // Opens the Preview on the shown path and takes its load in. False while
    // it has no scene to draw from.
    bool open_preview(const OpenChart& chart, const Path* shown);
    void draw_body(const OpenChart& chart);
    void draw_scores(const OpenChart& chart);
    void draw_guide(const OpenChart& chart);
    void draw_legend(const OpenChart& chart);
    void draw_timeline(const OpenChart& chart);
    void draw_highway(const OpenChart& chart);
    void draw_steps(const OpenChart& chart);
    // What a click on scene fill `index` does (the timeline or the highway):
    // an offered fill fill_reason accepts starts a window, a taken one
    // selects its step, any other puts its reason in the guide line.
    void click_fill(const OpenChart& chart, size_t index);
    // The guide line's facts for the shown path's scene.
    GuideFacts guide_facts(const OpenChart& chart) const;
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_WINDOW_H
