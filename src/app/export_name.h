// The file name the "Export to Excel..." save dialog suggests for a report
// window, and the one rule that makes a player's name safe to put in it.

#ifndef HYDRA_APP_EXPORT_NAME_H
#define HYDRA_APP_EXPORT_NAME_H

#include <ctime>
#include <string>
#include <string_view>

namespace hydra::app {

// A calendar day: year, month 1 to 12, day 1 to 31.
struct ExportDate {
    int year = 0;
    int month = 0;
    int day = 0;
};

// The local calendar day `t` falls on.
ExportDate local_date(std::time_t t);

// `name` with each character Windows refuses in a file name replaced by "_",
// one for one, so the length never changes. Bytes of a UTF-8 character past
// ASCII are kept as they are.
std::string file_name_safe(std::string_view name);

// The path report's suggested file: "Path report 2026-11-23.xlsx".
std::string path_report_export_name(const ExportDate& date);

// The comparison's suggested file, `player` made safe by file_name_safe:
// "dmleaderboards Jane 2026-11-23.xlsx".
std::string dm_report_export_name(std::string_view player, const ExportDate& date);

}  // namespace hydra::app

#endif  // HYDRA_APP_EXPORT_NAME_H
