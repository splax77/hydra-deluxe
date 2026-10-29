#include "pathbuilder/pb_menu.h"

#include "imgui.h"

namespace hydra::pathbuilder {

MenuChoice draw_menu(bool can_undo, bool can_redo) {
    MenuChoice choice = MenuChoice::None;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...", "Ctrl+O")) choice = MenuChoice::Open;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, can_undo)) choice = MenuChoice::Undo;
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, can_redo)) choice = MenuChoice::Redo;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (choice == MenuChoice::None && !ImGui::GetIO().WantTextInput) {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
            choice = MenuChoice::Open;
        else if (can_undo && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
            choice = MenuChoice::Undo;
        else if (can_redo && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_RouteGlobal))
            choice = MenuChoice::Redo;
    }
    return choice;
}

}  // namespace hydra::pathbuilder
