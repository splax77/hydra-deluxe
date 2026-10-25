// The library pane: the heading, the search box with its hint and errors,
// the status chips, and the sortable, scrolling table of every chart.
// What it shows comes from AppState::library (ui/library_model.h); this file
// draws it and hands clicks back. It replaced the paged table in the 2026-09
// interface redesign (Task 12).

#include "ui/library_parts.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "app/library_query.h"
#include "app/user_messages.h"  // stale_text, stale_chip_text, kNotAnalyzedText, stored_failure_text
#include "core/model.h"         // group_thousands, counted
#include "imgui.h"
#include "imgui_internal.h"  // ImGuiSelectableFlags_SpanAvailWidth
#include "ui/app_state.h"
#include "ui/column_widths.h"
#include "ui/library_model.h"
#include "ui/list_scroll.h"
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace hydra::ui::detail {

namespace {

// The chips' look, from the approved mockup: the selected chip is filled
// teal with an accent border, the others are outlined only. White on the
// selected fill is about 7:1.
const ImVec4 kChipOnColor{0 / 255.0f, 102 / 255.0f, 102 / 255.0f, 1.0f};
const ImVec4 kChipOnHoveredColor{0 / 255.0f, 122 / 255.0f, 122 / 255.0f, 1.0f};
const ImVec4 kChipOffColor{0.0f, 0.0f, 0.0f, 0.0f};
const ImVec4 kChipOffHoveredColor{60 / 255.0f, 60 / 255.0f, 64 / 255.0f, 1.0f};
const ImVec4 kChipOffBorderColor{74 / 255.0f, 74 / 255.0f, 80 / 255.0f, 1.0f};

// Matched text: a dark gold box behind it and light gold letters (the
// mockup's highlight).
const ImU32 kMatchBgColor = IM_COL32(0x4d, 0x42, 0x00, 0xff);
const ImU32 kMatchTextColor = IM_COL32(0xff, 0xe6, 0x80, 0xff);

// The colour of a row's Best path cell, by the row's status.
ImVec4 best_path_color(store::RecordStatus status) {
    switch (status) {
        case store::RecordStatus::Ready: return kBestPathColor;
        case store::RecordStatus::Stale: return kWarningColor;
        case store::RecordStatus::CantAnalyze: return kDimTextColor;
        case store::RecordStatus::NotAnalyzed: break;
    }
    return kNewSongColor;
}

// "1 chart", "97 charts", "12,345 charts".
std::string charts_text(size_t n) {
    return counted(static_cast<int64_t>(n), "chart", "charts");
}

void clear_search(AppState& app) {
    app.library_ui.search_buf[0] = '\0';
    app.library_ui.search_pending = false;
    app.library_ui.search_applied_at = ImGui::GetTime();
    app.set_search("");
}

// "Library   5 of 97 charts".
void render_heading(AppState& app) {
    ImGui::TextUnformatted("Library");
    ImGui::SameLine();
    const size_t shown = app.library_shown_count();
    const size_t total = app.library.rows().size();
    const std::string count =
        shown == total ? charts_text(total)
                       : group_thousands(static_cast<int64_t>(shown)) + " of " + charts_text(total);
    ImGui::TextColored(kNewSongColor, "%s", count.c_str());
}

// The popup's two columns, laid out by the width rule (ui/column_widths.h).
// The key never cuts; the meaning, the one text column, takes all the room the
// key leaves and wraps inside it.
constexpr size_t kTipKeyColumn = 0;
constexpr size_t kTipMeaningColumn = 1;

std::vector<ColumnSpec> search_tips_column_specs() {
    std::vector<ColumnSpec> specs = {
        {"Key", false, 0.0f, measure_in_role(TextRole::Notation)},
        {"What it does", true, 0.0f, measure_in_role(TextRole::Label), 100}};
    const WidthOf label = measure_in_role(TextRole::Label);
    for (ColumnSpec& spec : specs) spec.header_width_of = label;
    return specs;
}

// Column `c`'s text in a tip's row, the text its cell draws.
std::string search_tip_cell(const app::SearchTip& tip, size_t c) {
    return std::string(c == kTipKeyColumn ? tip.example : tip.meaning);
}

// How tall each tip's meaning is once it wraps at the width the rule gave
// that column, for table_outer_height.
std::vector<float> search_tip_meaning_heights(const std::vector<app::SearchTip>& tips,
                                              const ColumnLayout& layout) {
    ScopedFont font(TextRole::Label);
    std::vector<float> heights;
    heights.reserve(tips.size());
    for (const app::SearchTip& tip : tips)
        heights.push_back(ImGui::CalcTextSize(search_tip_cell(tip, kTipMeaningColumn).c_str(),
                                              nullptr, false, layout.widths[kTipMeaningColumn])
                              .y);
    return heights;
}

// The "Search tips" link after the line under the search box, and the popup
// it opens: each of app::search_tips() on its own row, then
// search_tips_note(). The popup starts at `line_left` under the link and is
// `pane_w` wide, the library pane's width, so its meanings wrap inside the
// pane. Escape or a click outside closes it; neither touches the search text.
void render_search_tips(float line_left, float pane_w) {
    static constexpr const char* kLink = "Search tips##searchtips";
    static constexpr const char* kPopup = "##searchtipspopup";
    const ImGuiID popup_id = ImGui::GetID(kPopup);

    // Dear ImGui closes an open popup on Esc in NewFrame, but the key still
    // reads as pressed for the rest of the frame. Claim it, so the song
    // panel's own Esc (details_panel.cpp) doesn't close that too. The same
    // claim as the settings panel's (settings_bar.cpp); whether the popup was
    // open last frame is kept in this window's ImGui storage.
    ImGuiStorage* storage = ImGui::GetStateStorage();
    if (storage->GetBool(popup_id) && !ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        ImGui::SetKeyOwner(ImGuiKey_Escape, popup_id, ImGuiInputFlags_LockThisFrame);

    if (fits_on_line(ImGui::CalcTextSize(kLink, nullptr, true).x, ImGui::GetStyle().ItemSpacing.x))
        ImGui::SameLine();
    if (ImGui::TextLink(kLink)) ImGui::OpenPopup(kPopup);

    ImGui::SetNextWindowPos(ImVec2(line_left, ImGui::GetItemRectMax().y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(pane_w, 0.0f), ImGuiCond_Always);
    if (ImGui::BeginPopup(kPopup)) {
        const std::vector<app::SearchTip>& tips = app::search_tips();
        if (begin_small_table(
                "##searchtipstable", ImGuiTableFlags_None, search_tips_column_specs(), tips.size(),
                [&](size_t r, size_t c) { return search_tip_cell(tips[r], c); },
                [&](const ColumnLayout& layout) {
                    return search_tip_meaning_heights(tips, layout);
                })) {
            {
                ScopedFont font(TextRole::Label);
                ImGui::TableHeadersRow();
            }
            for (const app::SearchTip& tip : tips) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                {
                    ScopedBaselineFont font(TextRole::Notation);
                    ImGui::TextUnformatted(search_tip_cell(tip, kTipKeyColumn).c_str());
                }
                ImGui::TableNextColumn();
                ScopedFont font(TextRole::Label);
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextUnformatted(search_tip_cell(tip, kTipMeaningColumn).c_str());
                ImGui::PopTextWrapPos();
            }
            ImGui::EndTable();
        }
        {
            const std::string_view note = app::search_tips_note();
            ScopedFont font(TextRole::Prose);
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(note.data(), note.data() + note.size());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndPopup();
    }
    storage->SetBool(popup_id, ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None));
}

void render_search_box(AppState& app) {
    LibraryViewState& ui = app.library_ui;
    char (&buf)[256] = ui.search_buf;
    if (!ui.search_synced) {
        std::snprintf(buf, sizeof(buf), "%s", app.search.c_str());
        ui.search_synced = true;
    }
    const ImGuiStyle& style = ImGui::GetStyle();
    const float clear_w = button_slot_width("X##clearsearch");
    ImGui::SetNextItemWidth(-(clear_w + style.ItemSpacing.x));

    // Ctrl+F jumps here from anywhere except behind a dialog: focusing a
    // control behind an open popup would fight its focus.
    const bool any_popup =
        ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    if (!any_popup && !ImGui::GetIO().WantTextInput &&
        ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_F))
        ImGui::SetKeyboardFocusHere();

    // EscapeClearsAll: the first Escape empties the box, the next leaves it.
    bool edited = false;
    {
        ScopedFont font(TextRole::Control);
        edited = ImGui::InputTextWithHint("##search", "Search songs", buf, sizeof(buf),
                                          ImGuiInputTextFlags_EscapeClearsAll);
    }
    hint("Ctrl+F jumps here. Esc clears it.");
    if (edited) ui.search_pending = true;

    // An emptied box applies at once; typing applies at most once per the
    // named constant's interval.
    const double now = ImGui::GetTime();
    if (ui.search_pending &&
        (buf[0] == '\0' || now - ui.search_applied_at >= AppState::kSearchThrottleSeconds)) {
        ui.search_pending = false;
        ui.search_applied_at = now;
        app.set_search(buf);
    }

    ImGui::SameLine();
    if (ImGui::Button("X##clearsearch")) clear_search(app);
    hint("Clear search (Esc)");

    // One short line and the Search tips link, then what the box couldn't
    // read. The keys themselves live in the popup (decision 10 of the
    // 2026-10-24 album-search plan), so this line never grows.
    const float line_left = ImGui::GetCursorScreenPos().x;
    const float pane_w = ImGui::GetContentRegionAvail().x;
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(kNewSongColor, "Type any words, or narrow it down.");
    ImGui::PopTextWrapPos();
    render_search_tips(line_left, pane_w);
    ImGui::PushTextWrapPos(0.0f);
    for (const std::string& error : app.library.query().errors)
        ImGui::TextColored(kWarningColor, "%s", error.c_str());
    ImGui::PopTextWrapPos();
}

bool chip_button(const char* label, bool on, bool disabled) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFrameHeight() * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, on ? kChipOnColor : kChipOffColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, on ? kChipOnHoveredColor : kChipOffHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, on ? kChipOnHoveredColor : kChipOffHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_Border, on ? kAccentColor : kChipOffBorderColor);
    begin_disabled_button(disabled);
    const bool clicked = ImGui::Button(label);
    end_disabled_button(disabled);
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    return clicked;
}

// All, then one chip per record status, each with its count over what the
// search matches. A status chip shows status_label's word; All keeps its
// own. A group with nothing in it can't be picked.
void render_chips(AppState& app) {
    static constexpr StatusChip kChips[] = {StatusChip::All, StatusChip::NotAnalyzed,
                                            StatusChip::Stale, StatusChip::Analyzed,
                                            StatusChip::CantAnalyze};
    const ChipCounts& counts = app.library.counts();
    // A chip that doesn't fit after the last one starts a new line. A
    // button's width is known before it is drawn (button_slot_width).
    const ImGuiStyle& style = ImGui::GetStyle();
    for (size_t i = 0; i < std::size(kChips); ++i) {
        const StatusChip chip = kChips[i];
        const size_t n = counts.of(chip);
        const std::string label = chip_label(chip, n);
        if (i > 0 && fits_on_line(button_slot_width(label.c_str()), style.ItemSpacing.x))
            ImGui::SameLine();
        const bool on = app.library.chip() == chip;
        if (chip_button(label.c_str(), on, !on && n == 0)) app.library.set_chip(chip);
        if (chip == StatusChip::Stale) hint(app::stale_chip_text().c_str());
    }
}

// Draws the matched parts of text already drawn at `pos` again, in the
// highlight colours, clipped at max_x.
void overlay_matches(ImVec2 pos, const std::string& text, const std::vector<app::MatchSpan>& spans,
                     float max_x) {
    if (spans.empty()) return;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float h = ImGui::GetTextLineHeight();
    draw->PushClipRect(pos, ImVec2(max_x, pos.y + h), true);
    for (const app::MatchSpan& s : spans) {
        const float x0 = pos.x + ImGui::CalcTextSize(text.data(), text.data() + s.begin).x;
        const float x1 = pos.x + ImGui::CalcTextSize(text.data(), text.data() + s.end).x;
        if (x0 >= max_x) break;
        draw->AddRectFilled(ImVec2(x0, pos.y), ImVec2(x1, pos.y + h), kMatchBgColor, 2.0f);
        draw->AddText(ImVec2(x0, pos.y), kMatchTextColor, text.data() + s.begin,
                      text.data() + s.end);
    }
    draw->PopClipRect();
}

// A cell's text, ellipsized, with the query's matches highlighted.
void cell_text(const std::string& text, const std::vector<app::MatchSpan>& spans) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float max_x = pos.x + ImGui::GetContentRegionAvail().x;
    text_ellipsized(text.c_str());
    overlay_matches(pos, text, spans, max_x);
}

// Draws a row's cut title at `pos`. It goes straight to the draw list: the
// row's Selectable already gave the text log the full title.
void draw_cut_title(ImVec2 pos, const std::string& shown) {
    ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), shown.c_str());
}

// Which hidden columns this frame's second lines named, for the footer.
struct SecondLineUse {
    bool folder = false;
    bool charter = false;
};

// The second line under a title while a search is on: where the row matched
// when that place isn't a visible column. The folder by default.
struct SecondLine {
    std::string text;
    std::vector<app::MatchSpan> spans;
    bool is_charter = false;
};

SecondLine second_line(const app::LibraryQuery& q, const LibraryRow& row, bool folder_shown,
                       bool charter_shown) {
    SecondLine line;
    line.text = library_cell_text(row, kColumnFolder);
    if (!folder_shown) line.spans = app::match_spans(q, app::QueryField::Folder, line.text);
    if (line.spans.empty() && !charter_shown) {
        const std::string charter = library_cell_text(row, kColumnCharter);
        std::vector<app::MatchSpan> spans = app::match_spans(q, app::QueryField::Charter, charter);
        if (!spans.empty()) {
            const std::string prefix = "charted by ";
            line.text = prefix + charter;
            for (app::MatchSpan& s : spans) {
                s.begin += prefix.size();
                s.end += prefix.size();
            }
            line.spans = std::move(spans);
            line.is_charter = true;
        }
    }
    return line;
}

// The table's columns as the width rule (ui/column_widths.h) sees them. Each
// one is text that ends in "…" when its cell is too short, so each may cut,
// and each takes its share of the room (the user's shares, 2026-10-14).
std::vector<ColumnSpec> library_column_specs() {
    std::vector<ColumnSpec> specs(kLibraryColumnCount);
    specs[kColumnTitle] = {"Title", true, 0.0f, {}, 17};
    specs[kColumnArtist] = {"Artist", true, 0.0f, {}, 10};
    specs[kColumnCharter] = {"Charter", true, 0.0f, {}, 9};
    specs[kColumnFolder] = {"Folder", true, 0.0f, {}, 39};
    specs[kColumnBestPath] = {"Best path", true, 0.0f, {}, 25};
    // The header row draws in Label, each cell in its column's role.
    const WidthOf label = measure_in_role(TextRole::Label);
    for (int c = 0; c < kLibraryColumnCount; ++c) {
        ColumnSpec& spec = specs[static_cast<size_t>(c)];
        spec.header_width_of = label;
        spec.width_of =
            measure_in_role(c == kColumnBestPath ? TextRole::Notation : TextRole::Title);
    }
    return specs;
}

// Brings the table's cached measure up to date when the UI scale changed.
// Every column is text, and measure_widths never reads a text column's cells,
// so the measure takes no rows: new rows or new summaries change nothing.
void update_column_widths(AppState& app) {
    LibraryViewState::ColumnWidths& cache = app.library_ui.column_widths;
    if (cache.measured.stale()) {
        cache.specs = library_column_specs();
        cache.measured = measure_widths(cache.specs, 0, CellText());
    }
}

SecondLineUse render_table(AppState& app, ImVec2 size) {
    SecondLineUse used;
    update_column_widths(app);
    LibraryViewState::ColumnWidths& widths = app.library_ui.column_widths;
    const std::vector<ColumnSpec>& columns = widths.specs;
    // A dragged column holds until the rows reload or the search changes
    // (decision 5). Both counters only ever go up, so their sum moves
    // whenever either does.
    const std::uint64_t reset_key = app.library.rows_version() + app.library.query_version();
    widths.room = table_room("##librarytable", size.x, columns.size(), reset_key);
    const ColumnLayout layout = place_columns(widths.measured, columns, widths.room);
    if (!ImGui::BeginTable("##librarytable", kLibraryColumnCount, table_flags(), size,
                           layout.inner_width))
        return used;

    ImGui::TableSetupScrollFreeze(0, 1);  // the header row stays on screen
    setup_column(columns[kColumnTitle], layout, kColumnTitle,
                 ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide,
                 static_cast<ImGuiID>(LibrarySort::Title));
    setup_column(columns[kColumnArtist], layout, kColumnArtist, 0,
                 static_cast<ImGuiID>(LibrarySort::Artist));
    setup_column(columns[kColumnCharter], layout, kColumnCharter, 0,
                 static_cast<ImGuiID>(LibrarySort::Charter));
    setup_column(columns[kColumnFolder], layout, kColumnFolder, 0,
                 static_cast<ImGuiID>(LibrarySort::Folder));
    // Highest score first on the first click.
    setup_column(columns[kColumnBestPath], layout, kColumnBestPath,
                 ImGuiTableColumnFlags_PreferSortDescending,
                 static_cast<ImGuiID>(LibrarySort::BestPath));

    // The columns always stay in the order set up above (imgui#9519).
    keep_table_column_order();

    // Charter and Folder make way for the song panel: hidden when it opens,
    // shown when it closes. In between, the header's right-click menu shows
    // or hides any column but Title.
    // On the table's first frame ImGui loads hydra_ui.ini's saved column state
    // after this point, so a column saved hidden would win over the panel.
    // The panel's choice goes in once that frame is over, as the report
    // tables wait to push their sort (report_frame::table).
    const bool panel_open = app.details_open();
    if (table_settled() && app.library_ui.columns_for_panel != panel_open) {
        ImGui::TableSetColumnEnabled(kColumnCharter, !panel_open);
        ImGui::TableSetColumnEnabled(kColumnFolder, !panel_open);
        app.library_ui.columns_for_panel = panel_open;
    }
    apply_column_widths(layout);
    ImGui::TableHeadersRow();

    if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs()) {
        if ((specs->SpecsDirty || !app.library_ui.sort_synced) && specs->SpecsCount > 0) {
            const ImGuiTableColumnSortSpecs& s = specs->Specs[0];
            app.library.set_sort(static_cast<LibrarySort>(s.ColumnUserID),
                                 s.SortDirection != ImGuiSortDirection_Descending);
        }
        specs->SpecsDirty = false;
        app.library_ui.sort_synced = true;
    }

    const app::LibraryQuery& q = app.library.query();
    const bool folder_shown =
        (ImGui::TableGetColumnFlags(kColumnFolder) & ImGuiTableColumnFlags_IsEnabled) != 0;
    const bool charter_shown =
        (ImGui::TableGetColumnFlags(kColumnCharter) & ImGuiTableColumnFlags_IsEnabled) != 0;
    const bool searching_words = !q.terms.empty();
    // Every row gets the same height (the clipper needs that): two lines
    // while a word search is on and a column is hidden, one otherwise.
    const bool two_lines = searching_words && (!folder_shown || !charter_shown);
    const float line_h = ImGui::GetTextLineHeight();
    const float line_gap = ImGui::GetStyle().ItemSpacing.y;
    const float row_h = two_lines ? line_h * 2.0f + line_gap : line_h;

    const std::vector<size_t>& order = app.library_view_order();
    const std::vector<LibraryRow>& rows = app.library.rows();
    const std::string selected_path = app.selected ? app.selected->notespath : std::string();

    // A new selection asks the scroll owner (ui/list_scroll.h), once, how to
    // move the list to its row. Only a change of the selected song counts: a
    // reload that keeps the open song (follow_selected_row) moves nothing.
    std::optional<size_t> scroll_to;
    if (selected_path != app.library_ui.scrolled_to) {
        app.library_ui.scrolled_to = selected_path;
        for (size_t k = 0; k < order.size(); ++k)
            if (app.is_selected_row(rows[order[k]].entry)) {
                scroll_to = k;
                break;
            }
    }

    // Only the rows on screen are drawn.
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(order.size()));
    if (scroll_to) clipper.IncludeItemByIndex(static_cast<int>(*scroll_to));
    while (clipper.Step()) {
        for (int k = clipper.DisplayStart; k < clipper.DisplayEnd; ++k) {
            const size_t index = order[static_cast<size_t>(k)];
            const LibraryRow& row = rows[index];
            ImGui::TableNextRow(ImGuiTableRowFlags_None, row_h);
            ImGui::PushID(static_cast<int>(index));

            // The row's Selectable goes first, spanning every column, so the
            // whole row is one click target. Its label is the title, which is
            // also how GUI tests click a row ("**/<title>").
            ImGui::TableSetColumnIndex(kColumnTitle);
            const ImVec2 title_pos = ImGui::GetCursorScreenPos();
            const float title_w = ImGui::GetContentRegionAvail().x;
            const bool selected = app.is_selected_row(row.entry);
            // A title too long for its cell ends in "..." like the other
            // columns: the Selectable keeps its label (its ID, and the text
            // log) but draws it invisibly and lays out only the cell's width,
            // and the cut title is drawn over it. The cut is the one cutting
            // rule (render::ellipsize, measured with text_width), which hands
            // a title that fits back unchanged: that is the fit test. Its
            // kept width is where the search highlight stops, before the "…".
            float title_kept_w = 0.0f;
            const std::string title = library_cell_text(row, kColumnTitle);
            const std::string title_shown =
                render::ellipsize(title, title_w, text_width, title_kept_w);
            const bool title_cut = title_shown != title;
            if (title_cut) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGuiSelectableFlags row_flags = ImGuiSelectableFlags_SpanAllColumns;
            if (title_cut) row_flags |= ImGuiSelectableFlags_SpanAvailWidth;
            const bool clicked = ImGui::Selectable(title.c_str(), selected, row_flags,
                                                   ImVec2(title_cut ? title_w : 0.0f, row_h));
            if (title_cut) ImGui::PopStyleColor();
            if (clicked) app.select(row.entry, SelectionSource::Click);
            if (ImGui::TableGetHoveredColumn() == kColumnTitle && title_cut)
                overflow_tooltip(title.c_str());
            if (title_cut) draw_cut_title(title_pos, title_shown);
            const float title_text_w = title_cut ? title_kept_w : title_w;
            if (scroll_to && *scroll_to == static_cast<size_t>(k))
                scroll_last_row(selection_scroll(app.last_selection_source));
            if (searching_words)
                overlay_matches(title_pos, title,
                                app::match_spans(q, app::QueryField::Title, title),
                                title_pos.x + title_text_w);
            if (two_lines) {
                const SecondLine line = second_line(q, row, folder_shown, charter_shown);
                ImGui::SetCursorScreenPos(ImVec2(title_pos.x, title_pos.y + line_h + line_gap));
                ImGui::PushStyleColor(ImGuiCol_Text, kNewSongColor);
                const ImVec2 line_pos = ImGui::GetCursorScreenPos();
                text_ellipsized(line.text.c_str());
                ImGui::PopStyleColor();
                overlay_matches(line_pos, line.text, line.spans, title_pos.x + title_w);
                if (!line.spans.empty()) (line.is_charter ? used.charter : used.folder) = true;
            }

            // Every column but Title can be hidden from the header menu, and a
            // hidden column's TableSetColumnIndex returns false. Each cell
            // draws library_cell_text, the text its column is measured from.
            const auto searched_cell = [&](int column, app::QueryField field) {
                if (!ImGui::TableSetColumnIndex(column)) return;
                const std::string text = library_cell_text(row, column);
                cell_text(text, searching_words ? app::match_spans(q, field, text)
                                                : std::vector<app::MatchSpan>{});
            };
            searched_cell(kColumnArtist, app::QueryField::Artist);
            searched_cell(kColumnCharter, app::QueryField::Charter);
            searched_cell(kColumnFolder, app::QueryField::Folder);

            if (!ImGui::TableSetColumnIndex(kColumnBestPath)) {
                ImGui::PopID();
                continue;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, best_path_color(row.status));
            {
                ScopedBaselineFont font(TextRole::Notation);
                text_ellipsized(library_cell_text(row, kColumnBestPath).c_str());
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                switch (row.status) {
                    case store::RecordStatus::NotAnalyzed:
                        ImGui::SetTooltip("%s", app::kNotAnalyzedText);
                        break;
                    case store::RecordStatus::Stale:
                        // This one row's real cause, from the store.
                        ImGui::SetTooltip(
                            "%s", app::stale_text(row.stale_build, row.stale_rules).c_str());
                        break;
                    case store::RecordStatus::CantAnalyze:
                        // This one chart's stored reason, worded as a live
                        // failure of that kind would be.
                        if (row.failure_kind)
                            ImGui::SetTooltip("%s", app::stored_failure_text(*row.failure_kind,
                                                                             row.failure_text)
                                                        .c_str());
                        break;
                    case store::RecordStatus::Ready: break;
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
    return used;
}

// Under the table while a search is on: why the rows on screen matched when
// the reason is in a hidden column, and a way out.
void render_footer(AppState& app, const SecondLineUse& used) {
    const char* why = used.folder && used.charter ? "Matched on folder and charter."
                      : used.folder               ? "Matched on folder."
                      : used.charter              ? "Matched on charter."
                                                  : nullptr;
    if (why) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(kNewSongColor, "%s", why);
        ImGui::SameLine();
    }
    if (ImGui::SmallButton("Clear search")) clear_search(app);
}

}  // namespace

void render_library(AppState& app) {
    // Job-driven refreshes first, every frame, whatever is drawn below.
    app.tick_library(ImGui::GetTime());
    // An empty library shows the main window's "no songs" message instead.
    if (app.library.rows().empty()) return;

    render_heading(app);
    render_search_box(app);
    render_chips(app);
    ImGui::Spacing();

    if (app.library_shown_count() == 0) {
        // The chips fall back to All when their group empties, so nothing
        // shown means the search matched nothing.
        ImGui::TextUnformatted("No charts match your search.");
        if (ImGui::Button("Clear search")) clear_search(app);
        return;
    }

    const bool searching = app.library.searching();
    const float footer_h = searching ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
    const float table_h = std::max(ImGui::GetContentRegionAvail().y - footer_h,
                                   ImGui::GetTextLineHeightWithSpacing() * 4.0f);
    const SecondLineUse used = render_table(app, ImVec2(0.0f, table_h));
    if (searching) render_footer(app, used);
}

}  // namespace hydra::ui::detail
