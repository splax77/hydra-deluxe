Lines for the next release's notes, gathered as work merges. Fold them into the release text when it ships.

**Reports open inside Hydra.** The path report and the dmleaderboards comparison now open in their own Hydra windows instead of a web page. `hydra_report` is gone.

**Closing the path report frees its memory.** Closing the window lets the report go, and opening it again builds a fresh one. A build still running when you close it stops.
