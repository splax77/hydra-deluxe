#include "app/export_name.h"

#include <cstdio>

namespace hydra::app {

namespace {

// Whether Windows refuses `c` in a file name. The list is Microsoft's, from
// "Naming Files, Paths, and Namespaces" (learn.microsoft.com).
bool refused_in_file_name(unsigned char c) {
    if (c >= 1 && c <= 31) return true;
    switch (c) {
        case '<': case '>': case ':': case '"': case '/': case '\\': case '|': case '?': case '*':
            return true;
        default:
            return false;
    }
}

// `date` as "2026-11-23".
std::string date_text(const ExportDate& date) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", date.year, date.month, date.day);
    return buf;
}

}  // namespace

ExportDate local_date(std::time_t t) {
    std::tm local{};
    localtime_s(&local, &t);
    return {local.tm_year + 1900, local.tm_mon + 1, local.tm_mday};
}

std::string file_name_safe(std::string_view name) {
    std::string out(name);
    for (char& c : out)
        if (refused_in_file_name(static_cast<unsigned char>(c))) c = '_';
    return out;
}

std::string path_report_export_name(const ExportDate& date) {
    return "Path report " + date_text(date) + ".xlsx";
}

std::string dm_report_export_name(std::string_view player, const ExportDate& date) {
    return "dmleaderboards " + file_name_safe(player) + " " + date_text(date) + ".xlsx";
}

}  // namespace hydra::app
