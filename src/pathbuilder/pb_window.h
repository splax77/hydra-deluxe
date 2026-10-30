// The Path Builder's main window, drawn each frame: the menu bar, the path
// list on the left, the highway in the middle and the decision list on the
// right. main.cpp and the headless runner (tests/pb_ui) both draw through
// draw_window, so a test sees what the player sees.

#ifndef HYDRA_PATHBUILDER_PB_WINDOW_H
#define HYDRA_PATHBUILDER_PB_WINDOW_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pathbuilder/pb_paths.h"
#include "pathbuilder/pb_state.h"

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
    const std::vector<PathRow>& rows() const { return rows_; }
    PathListUi& list_ui() { return list_ui_; }
    // The saved branches File > Open path... added on this chart, the last
    // rows of the left list.
    const std::vector<Path>& branches() const { return branches_; }
    // Why the last paste, path file open or save changed nothing: the plain
    // sentence and the details line (pb_files.h's Refusal). Empty when none
    // is showing.
    const std::string& refusal() const { return refusal_; }
    const std::string& refusal_detail() const { return refusal_detail_; }

    // Draws one frame of the window.
    void draw();

private:
    std::unique_ptr<ui::PreviewController> preview_;
    // The chart rows_ was built for; rows_ is rebuilt when another opens.
    const OpenChart* rows_chart_ = nullptr;
    // rows_ points into the record and into branches_, so it is rebuilt after
    // every change to branches_ (rebuild_rows).
    std::vector<Path> branches_;
    std::vector<PathRow> rows_;
    PathListUi list_ui_;
    // The refusal on screen, and the player's history generation it was
    // made at: any later change to the player's path takes it away.
    std::string refusal_;
    std::string refusal_detail_;
    uint64_t refusal_generation_ = 0;
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
    void rebuild_rows();
    void act_on_menu();
    // Shows `sentence` and `detail` as the refusal (empty clears it).
    void set_refusal(std::string sentence, std::string detail);
    void draw_body(const OpenChart& chart);
    void draw_highway(const OpenChart& chart, const Path* shown);
    // The path the highway shows: the picked row's, else the player's own.
    const Path* shown_path() const;
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_WINDOW_H
