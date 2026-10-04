# Discover once, exactly as hydra_runfolder.py does, and save the list so the
# section workers analyze the same charts in the same order.
import json
import sys
import time

import hydra.hyutil as hyutil

t0 = time.perf_counter()
scanitems, errors = hyutil.discover_charts([sys.argv[1]])
elapsed = time.perf_counter() - t0

# runfolder analyzes each md5 once (first occurrence); keep the raw index so
# per-chart times line up with the single run's [timing] checkpoints.
seen = set()
charts = []
for raw_idx, s in enumerate(scanitems, 1):
    if s.md5 in seen:
        continue
    seen.add(s.md5)
    charts.append({'raw': raw_idx, 'md5': s.md5, 'title': s.title, 'artist': s.artist,
                   'charter': s.charter, 'path': s.notespath})

with open(sys.argv[2], 'w', encoding='utf-8') as f:
    json.dump({'discovery_s': elapsed, 'found': len(scanitems), 'charts': charts}, f)
print(f"found {len(scanitems)}, unique {len(charts)}, discovery {elapsed:.1f}s")
