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

    // Draws one frame of the window.
    void draw();

private:
    std::unique_ptr<ui::PreviewController> preview_;
    // The chart rows_ was built for; rows_ is rebuilt when another opens.
    const OpenChart* rows_chart_ = nullptr;
    std::vector<PathRow> rows_;
    PathListUi list_ui_;
    // The overlay key of the path the highway shows (app::path_overlay_key),
    // and what it was built for, so it is rebuilt only when that changes.
    std::string overlay_key_;
    const Path* overlay_for_ = nullptr;
    uint64_t overlay_generation_ = 0;  // PathHistory::generation it was built at
    bool overlay_built_ = false;
    // The file dialog never appeared: said once under the menu.
    bool dialog_failed_ = false;

    void take_in_chart();
    void act_on_menu();
    void draw_body(const OpenChart& chart);
    void draw_highway(const OpenChart& chart, const Path* shown);
    // The path the highway shows: the picked row's, else the player's own.
    const Path* shown_path() const;
};

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_WINDOW_H
