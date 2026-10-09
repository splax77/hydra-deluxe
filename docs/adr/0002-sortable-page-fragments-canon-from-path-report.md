# Sortable-page fragments are canon from the pinned path report

_Superseded by ADR 0016 (2026-09): `hydra_report.py` is gone, nothing pins the page bytes, and the three report pages now share one stylesheet and one script._

The two HTML reports (path index, dmleaderboards comparison) assemble from
shared fragments in `app/html_page.cpp`. The path report's assembled bytes are
pinned to `hydra_report.py`'s PAGE string (byte-exact parity), so the
fragments are taken verbatim from that page — comments, `\uXXXX` escape style
and all — and the dm page adopts them, which leaves a few rules there that
match nothing (`.toggle`, `td.path`, `tr.best`). The alternative — normalizing
both pages to a cleaner shared template — was rejected because it breaks the
parity pin. Do not strip the "unused" rules from the dm page's output or
re-inline the fragments per page; per-page differences belong in each page's
own column-width, chip-color, body, and script pieces only.

_Note, 2026-10-08: the path report and the dmleaderboards comparison are no
longer HTML pages. They are Hydra windows, and `hydra_report` is gone (D103,
ADR 0027). Only the fill-spawn comparison is still a page._
