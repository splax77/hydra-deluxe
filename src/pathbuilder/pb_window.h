// The Path Builder's main window, drawn each frame in this order: the head
// row (title and mode, the three scores, Undo, Start over, Finish the rest for
// me, "Show Hydra's best path"), the Preview's timeline over a window of the
// song (ui::PreviewTimeline, the third mockup's), then
// Hydra's own Preview panel (ui::draw_preview_panel) with a side column beside
// it holding "Where to activate next" above "Your activations". The top
// follows the fifth mockup
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-5/top-mockup.html),
// and what lies under the timeline the second
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-2/
// mockup_template.html, answers.md answer 4). Everything drawn about the chart
// is the Preview's; every sentence is pb_text's. main.cpp and the headless
// runner (tests/pb_ui) both draw through draw(), so a test sees what the
// player sees. Plan: docs/superpowers/plans/2026-10-31-path-builder-redesign-2.md,
// task SC-3b.

#ifndef HYDRA_PATHBUILDER_PB_WINDOW_H
#define HYDRA_PATHBUILDER_PB_WINDOW_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pathbuilder/pb_state.h"
#include "ui/preview_timeline.h"  // kTimelineActivationsLabel
#include "ui/timeline_view.h"

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
// Each score's hint, shown as the tooltip on its figure (plan Q9).
inline constexpr const char* kYourScoreHint = "what your activations so far score";
inline constexpr const char* kBestPossibleHint = "Hydra's best path for this chart";
inline constexpr const char* kBestReachHint =
    "keeping your activations and playing the rest perfectly";
inline constexpr const char* kUndoLabel = "Undo";
inline constexpr const char* kFinishTooltip =
    "Keeps your activations and lets Hydra choose the best rest of the path";
// The keys the "Your activations" label's hover shows, the fifth mockup's
// words (top-mockup.html line 553): one per app::CostTier, then one per look
// of a fill that has no cost.
inline constexpr const char* kLegendBestText = "Best place to activate";
inline constexpr const char* kLegendUnderOnePercentText = "Costs under 1% of the best score";
inline constexpr const char* kLegendCostsMoreText = "Costs more";
inline constexpr const char* kLegendMineText = "Your activation";
inline constexpr const char* kLegendCantText = "Can't activate here (hover to see why)";
inline constexpr const char* kLegendLockedText = "Before your last activation";
inline constexpr const char* kShowBestPathLabel = "Show Hydra's best path";
// The steps panel's heading is the timeline's name for the same activations.
inline constexpr const char* kStepsHeading = ui::kTimelineActivationsLabel;
inline constexpr const char* kRemoveLastLabel = "Remove";
inline constexpr const char* kRemoveEarlierLabel = "Remove this and later ones";
inline constexpr const char* kEndItAtLabel = "End it at:";

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
    // The selected step and the history generation it was picked at.
    std::optional<size_t> selected_;
    uint64_t selected_generation_ = 0;
    // How tall the opened step's block in "Your activations" was when the
    // panel last scrolled to it; select_step clears it to scroll again.
    float steps_block_h_ = -1.0f;
    // The step a click or an end choice selects once its edit lands, as the
    // mockup selects the step it just made.
    std::optional<size_t> pending_select_;
    // "Show Hydra's best path".
    bool show_best_ = false;
    // While the pointer is on a fill or its list row: the fill, whose row the
    // list draws lit.
    std::optional<size_t> hovered_fill_;
    // The fill whose list row the pointer was on last frame; the list is
    // drawn after the timeline, so the timeline reads it a frame later.
    std::optional<size_t> row_hovered_fill_;
    // The stretch of the song the timeline shows (ui/timeline_view.h). None
    // until the chart's Preview has loaded; then it opens with open_view.
    std::optional<ui::TimelineView> view_;
    // Whether the window turns its pages with the playhead while playing
    // (the third mockup's followOff, run backwards): a press of Play turns it
    // on, and a scroll by hand while playing turns it off.
    bool follow_ = true;
    // The Preview's clock, and whether it was playing, as the timeline's drag
    // left them last frame. The next frame compares against them, so it can
    // tell a Play press or a jump from the drag's own seeks.
    double last_position_ms_ = 0.0;
    bool was_playing_ = false;
    // A drag on the timeline (ui::TimelineEvents::dragged_ms) is running.
    bool dragging_ = false;
    // The Preview's volume for this run: the settings file's at the first
    // open (PbState::preview_volume), then the player's slider; never
    // written back (plan Q2).
    int volume_pct_;
    bool volume_from_file_ = false;
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
    void start_over();
    void select_step(size_t index);
    // Opens the Preview on the shown path and takes its load in. False while
    // it has no scene to draw from.
    bool open_preview(const OpenChart& chart, const Path* shown);
    void draw_body(const OpenChart& chart);
    void draw_head(const OpenChart& chart);
    void draw_timeline(const OpenChart& chart);
    // Says whether a drag on the timeline is holding the song, to dragging_
    // and to the controller's scrubbing rule.
    void hold_for_drag(bool held);
    // Moves the window for what the Preview's clock did since last frame:
    // it opens the window at the first frame, follows the playhead while
    // playing, and shows a jump the paused clock made (ui::follow_playing,
    // ui::show). `length_ms` is the Preview's playback end. A drag moves
    // nothing.
    void move_view_with_clock(double length_ms);
    // Centres the window on scene fill `index` when the timeline does not
    // show it (a list row's click).
    void scroll_to_fill(const app::PreviewScene& scene, size_t index, double length_ms);
    // The Preview panel, or the controller's error in its place.
    void draw_preview(const OpenChart& chart);
    void draw_choices(const OpenChart& chart);
    void draw_steps(const OpenChart& chart);
    // What a click on scene fill `index` does (the timeline, the highway or a
    // list row): an offered fill fill_reason accepts starts a window, a taken
    // one selects its step, any other does nothing. Every click also parks
    // the Preview's clock on the fill's dot (ui::timeline_fill_ms).
    void click_fill(const OpenChart& chart, size_t index);
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_WINDOW_H
