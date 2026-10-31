// The Path Builder's menu bar: File > Open..., Open path... and Save path...,
// Edit > Undo, Redo, Start over, Copy and Paste. The menu only reports what
// was chosen; pb_window.cpp acts on it.

#ifndef HYDRA_PATHBUILDER_PB_MENU_H
#define HYDRA_PATHBUILDER_PB_MENU_H

namespace hydra::pathbuilder {

// What the "Start over" button and menu item say (the mockup's `#clear`).
inline constexpr const char* kStartOverLabel = "Start over";

// What the player chose from the menu this frame (at most one item).
enum class MenuChoice { None, Open, OpenPath, SavePath, Undo, Redo, StartOver, Copy, Paste };

// Which items can be chosen this frame. A false flag greys its item and
// turns its shortcut off.
struct MenuFlags {
    bool can_undo = false;        // something to undo
    bool can_redo = false;        // something to redo
    bool can_start_over = false;  // the player has a window to clear
    bool can_open_path = false;  // a chart to open a path file onto
    bool can_save_path = false;  // a path on screen to save
    bool can_copy = false;       // a path on screen to copy
    bool can_paste = false;      // a chart to paste onto
};

// Draws the menu bar of the current window (opened with
// ImGuiWindowFlags_MenuBar). Ctrl+O, Ctrl+Z, Ctrl+Y, Ctrl+C and Ctrl+V choose
// the same items as Open..., Undo, Redo, Copy and Paste.
MenuChoice draw_menu(const MenuFlags& flags);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_MENU_H
