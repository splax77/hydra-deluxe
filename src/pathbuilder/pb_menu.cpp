#include "pathbuilder/pb_menu.h"

#include "imgui.h"

namespace hydra::pathbuilder {

MenuChoice draw_menu(bool can_undo, bool can_redo, bool can_copy, bool can_paste) {
    MenuChoice choice = MenuChoice::None;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...", "Ctrl+O")) choice = MenuChoice::Open;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, can_undo)) choice = MenuChoice::Undo;
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, can_redo)) choice = MenuChoice::Redo;
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, can_copy)) choice = MenuChoice::Copy;
            if (ImGui::MenuItem("Paste", "Ctrl+V", false, can_paste)) choice = MenuChoice::Paste;
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
        else if (can_copy && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_RouteGlobal))
            choice = MenuChoice::Copy;
        else if (can_paste && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V, ImGuiInputFlags_RouteGlobal))
            choice = MenuChoice::Paste;
    }
    return choice;
}

}  // namespace hydra::pathbuilder
