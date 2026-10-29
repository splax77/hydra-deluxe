// The Path Builder's menu bar: File > Open, Edit > Undo and Redo. The menu
// only reports what was chosen; pb_window.cpp acts on it.

#ifndef HYDRA_PATHBUILDER_PB_MENU_H
#define HYDRA_PATHBUILDER_PB_MENU_H

namespace hydra::pathbuilder {

// What the player chose from the menu this frame (at most one item).
enum class MenuChoice { None, Open, Undo, Redo };

// Draws the menu bar of the current window (opened with
// ImGuiWindowFlags_MenuBar). Undo and Redo are greyed when there is nothing
// to undo or redo. Ctrl+O, Ctrl+Z and Ctrl+Y choose the same items.
MenuChoice draw_menu(bool can_undo, bool can_redo);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_MENU_H
