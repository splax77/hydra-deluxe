// Plumbing for the fill comparison's HTML page (app/fill_report.cpp): its
// stylesheet and script, template substitution, and the escaping helpers it
// embeds its row data with.

#ifndef HYDRA_APP_HTML_PAGE_H
#define HYDRA_APP_HTML_PAGE_H

#include <string>
#include <string_view>

namespace hydra::app::html {

// html.escape(s, quote=True): & first, then the rest.
std::string html_escape(const std::string& s);

// json.dumps string escaping with the default ensure_ascii=True: every
// non-ASCII code point becomes \uXXXX (a surrogate pair beyond the BMP),
// control characters get their short escapes, and everything else passes
// through. Input is UTF-8. Appends the quoted string to `out`.
void json_escape_into(std::string& out, const std::string& s);

// Fill a page template: guard the embedded JSON against `</` closing the
// script tag, then substitute __SUBTITLE__, __FOOTER__, and __DATA__.
std::string render_page(const char* page_template, std::string data_json,
                        const std::string& subtitle, const std::string& footer);

// ---- the shared report page -----------------------------------------------
// The fill page is one stylesheet and one script wrapped around its own
// title, body markup and PAGE settings (docs/adr/0016). Both stay ASCII: this
// file compiles into hydra_core, so a glyph goes in as an HTML entity or a
// \uXXXX JavaScript escape.
extern const char* const kReportCss;     // every rule the fill page uses
extern const char* const kReportJsHead;  // the data tag, DATA, DASH, fmt
extern const char* const kReportJs;      // sorting, filtering, drawing, first render

// One page's template: a whole standards-mode document (doctype, <html
// lang="en">, head, body) with the shared stylesheet and script around the
// page's <title> text, its body markup and its `const PAGE = {...};` script.
// Between the data tag and PAGE it writes `const FOLD = {...};`, the
// search_fold_table the search box folds a typed query with. Every row in
// PAGE.rows carries `search`, its search_field text; PAGE.filter() takes no
// query and keeps rows by the page's own controls only.
// The result still carries __SUBTITLE__, __FOOTER__ and __DATA__ for
// render_page to fill.
std::string page_template(const char* title, const char* body, const char* page_js);

}  // namespace hydra::app::html

#endif  // HYDRA_APP_HTML_PAGE_H
