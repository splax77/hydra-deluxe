// The Path Builder's menu bar: File > Open, Edit > Undo, Redo, Copy and Paste.
// The menu only reports what was chosen; pb_window.cpp acts on it.

#ifndef HYDRA_PATHBUILDER_PB_MENU_H
#define HYDRA_PATHBUILDER_PB_MENU_H

namespace hydra::pathbuilder {

// What the player chose from the menu this frame (at most one item).
enum class MenuChoice { None, Open, Undo, Redo, Copy, Paste };

// Draws the menu bar of the current window (opened with
// ImGuiWindowFlags_MenuBar). Each Edit item is greyed when its flag is false:
// nothing to undo or redo, no path on screen to copy, no chart to paste onto.
// Ctrl+O, Ctrl+Z, Ctrl+Y, Ctrl+C and Ctrl+V choose the same items.
MenuChoice draw_menu(bool can_undo, bool can_redo, bool can_copy = false, bool can_paste = false);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_MENU_H
