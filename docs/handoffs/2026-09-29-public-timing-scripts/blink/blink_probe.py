# Time the public engine on one chart with the runfolder's settings
# (Expert, Pro, 2x, scores 4) and break down where the time goes.
#   python blink_probe.py <notes.mid> <out.json> [--nolimit] [--fix] [--timeout S]
# --fix adds Hydra Deluxe's filtered-path band (see engine.cpp reduce_group)
# in front of the unchanged public reduction. A watchdog stops the run if free
# RAM drops under 5 GB or the timeout passes, and still writes the stats.
import bisect, ctypes, ctypes.wintypes as wt, json, os, sys, threading, time
from itertools import combinations

import hydra.hysong as hysong
import hydra.hypath as hypath
import hydra.hydata as hydata

chart, out = sys.argv[1], sys.argv[2]
NOLIMIT = '--nolimit' in sys.argv
FIX = '--fix' in sys.argv
TIMEOUT = float(sys.argv[sys.argv.index('--timeout') + 1]) if '--timeout' in sys.argv else 1800
# --search-timeout S: stop S seconds after the search starts, so a slow file
# read doesn't eat the budget. --trace: sample progress and cost every 2 s of
# search, so a stopped run can be extrapolated.
SEARCH_TIMEOUT = float(sys.argv[sys.argv.index('--search-timeout') + 1]) if '--search-timeout' in sys.argv else None
TRACE = '--trace' in sys.argv
# --depth N: the score range ("top N scores"); the runfolder and app default is 4.
DEPTH = int(sys.argv[sys.argv.index('--depth') + 1]) if '--depth' in sys.argv else 4
MIN_FREE_GB = float(sys.argv[sys.argv.index('--min-free-gb') + 1]) if '--min-free-gb' in sys.argv else 5.0
MS = None if NOLIMIT else 10

class PM(ctypes.Structure):
    _fields_ = [('cb', wt.DWORD), ('pf', wt.DWORD), ('peak', ctypes.c_size_t), ('ws', ctypes.c_size_t)] + \
               [(f'x{i}', ctypes.c_size_t) for i in range(6)]
class SM(ctypes.Structure):
    _fields_ = [('len', wt.DWORD), ('load', wt.DWORD), ('tot', ctypes.c_ulonglong), ('avail', ctypes.c_ulonglong)] + \
               [(f'x{i}', ctypes.c_ulonglong) for i in range(5)]
k32 = ctypes.WinDLL('kernel32')
k32.GetCurrentProcess.restype = wt.HANDLE
k32.K32GetProcessMemoryInfo.argtypes = [wt.HANDLE, ctypes.POINTER(PM), wt.DWORD]
def mem_mb():
    p = PM(); p.cb = ctypes.sizeof(p)
    k32.K32GetProcessMemoryInfo(k32.GetCurrentProcess(), ctypes.byref(p), p.cb)
    return p.ws / 2**20, p.peak / 2**20
def free_gb():
    s = SM(); s.len = ctypes.sizeof(s); k32.GlobalMemoryStatusEx(ctypes.byref(s)); return s.avail / 2**30

st = {'chart': chart, 'ms_limit': MS, 'depth': DEPTH, 'fix': FIX, 'finished': False, 'stopped_because': None,
      'reduce_s': 0.0, 'filtercheck_s': 0.0, 'filtercheck_calls': 0, 'copy_s': 0.0, 'copies': 0,
      'variant_copies': 0, 'steps': 0, 'max_frontier': 0, 'max_group': 0, 'pair_comparisons': 0,
      'fix_dropped': 0, 'progress': 0.0}
T0 = time.perf_counter()
deadline = [T0 + TIMEOUT]
t_search = [None]
if TRACE:
    st['trace'] = []   # [search_s, progress, ws_mb, copies, variant_copies, pair_comparisons, frontier]

def dump():
    st['elapsed_s'] = round(time.perf_counter() - T0, 2)
    st['ws_mb'], st['peak_mb'] = [round(x) for x in mem_mb()]
    st['min_free_gb'] = round(min(st.get('min_free_gb', 99), free_gb()), 2)
    with open(out, 'w') as f:
        json.dump(st, f, indent=1)

def watchdog():
    while True:
        time.sleep(1)
        g = free_gb()
        st['min_free_gb'] = min(st.get('min_free_gb', 99), g)
        why = f'free RAM < {MIN_FREE_GB:g} GB' if g < MIN_FREE_GB else ('timeout' if time.perf_counter() > deadline[0] else None)
        if why:
            st['stopped_because'] = why
            dump()
            os._exit(3)
threading.Thread(target=watchdog, daemon=True).start()

# --- instrumentation (wrappers only; the engine code is unchanged) ---
_orig_filter = hydata.Path.passes_ms_filter
def timed_filter(self, ms):
    t = time.perf_counter(); r = _orig_filter(self, ms)
    st['filtercheck_s'] += time.perf_counter() - t; st['filtercheck_calls'] += 1
    return r
hydata.Path.passes_ms_filter = timed_filter

_orig_copy = hydata.Path.copy
_orig_prep = hydata.Path.prepare_variants

def shared_copy(self):
    """Path.copy, but the new path shares the tied variants instead of
    deep-copying them. Its variants list is still its own, so later appends
    don't leak between branches."""
    saved = self.variants
    self.variants = []
    try:
        c = _orig_copy(self)
    finally:
        self.variants = saved
    c.variants = list(saved)
    return c

def shared_prep(self):
    # Variants are only written here, at the very end; copy each one right
    # before it is filled in, so shared variants never see another path's data.
    self.variants = [shared_copy(v) for v in self.variants]
    _orig_prep(self)

SHARE = '--share' in sys.argv
st['share'] = SHARE
base_copy = shared_copy if SHARE else _orig_copy
if SHARE:
    hydata.Path.prepare_variants = shared_prep

_depth = [0]
def timed_copy(self):
    if _depth[0]:                      # a variant copied inside a copy
        st['variant_copies'] += 1
        return _orig_copy(self)
    _depth[0] += 1
    t = time.perf_counter()
    try:
        return base_copy(self)
    finally:
        st['copy_s'] += time.perf_counter() - t; st['copies'] += 1; _depth[0] -= 1
hydata.Path.copy = timed_copy

def group_key(p):
    if p.is_complete():
        return (p.is_active_sp(), 0)
    return (p.is_active_sp(), p.sp_end_time if p.is_active_sp() else p.sp)

_orig_reduce = hypath.GraphPather._reduce_iteration_paths
def reduce(self, depth_mode, depth_value, ms_filter):
    t = time.perf_counter()
    if FIX and ms_filter is not None:
        # Deluxe's band: drop a too-hard path once more than depth_value
        # distinct scores in its group beat it.
        groups = {}
        for p in self._iteration_paths:
            if p.buffered_sqinout_sp == 0:
                groups.setdefault(group_key(p), []).append(p)
        drop = set()
        for members in groups.values():
            scores = sorted({p.data.totalscore() for p in members})
            for p in members:
                if not p.data.passes_ms_filter(ms_filter):
                    s = p.data.totalscore()
                    higher = len(scores) - bisect.bisect_right(scores, s)
                    if (depth_mode == 'scores' and higher > depth_value) or \
                       (depth_mode == 'points' and s + depth_value < scores[-1]):
                        drop.add(p)
        if drop:
            st['fix_dropped'] += len(drop)
            self._iteration_paths = [p for p in self._iteration_paths if p not in drop]
    n = len(self._iteration_paths)
    st['steps'] += 1
    st['max_frontier'] = max(st['max_frontier'], n)
    sizes = {}
    for p in self._iteration_paths:
        if p.buffered_sqinout_sp == 0:
            k = group_key(p); sizes[k] = sizes.get(k, 0) + 1
    for g in sizes.values():
        st['pair_comparisons'] += g * (g - 1) // 2
        st['max_group'] = max(st['max_group'], g)
    _orig_reduce(self, depth_mode, depth_value, ms_filter)
    st['reduce_s'] += time.perf_counter() - t
hypath.GraphPather._reduce_iteration_paths = reduce

_last_trace = [0.0]
def progress(tc, frac):
    st['progress'] = round(frac, 4)
    if TRACE:
        now = time.perf_counter()
        if now - _last_trace[0] >= 2:
            _last_trace[0] = now
            st['trace'].append([round(now - t_search[0], 2), round(frac, 5), round(mem_mb()[0]),
                                st['copies'], st['variant_copies'], st['pair_comparisons'],
                                st['max_frontier']])
    if st['steps'] % 200 == 0:
        dump()

# --- the run, split the way hyutil._analyze does it ---
t = time.perf_counter()
loader = {'.mid': hysong.load_songpath_mid, '.chart': hysong.load_songpath_chart,
          '.sng': hysong.load_songpath_sng}[os.path.splitext(chart)[1].lower()]
song = loader(chart, 'Expert', True, True)
st['parse_s'] = round(time.perf_counter() - t, 3)
t = time.perf_counter()
graph = hypath.ScoreGraph(song)
st['graph_s'] = round(time.perf_counter() - t, 3)
t = time.perf_counter()
t_search[0] = _last_trace[0] = t
if SEARCH_TIMEOUT is not None:
    deadline[0] = t + SEARCH_TIMEOUT
pather = hypath.GraphPather()
pather.read(graph, 'scores', DEPTH, MS, progress)
st['search_s'] = round(time.perf_counter() - t, 3)

paths = list(pather.record.all_paths())
st['finished'] = True
st['paths'] = len(paths)
st['best_score'] = max(p.totalscore() for p in paths)
st['result'] = sorted([p.totalscore(), p.pathstring()] for p in paths)
for key in ('reduce_s', 'filtercheck_s', 'copy_s'):
    st[key] = round(st[key], 2)
dump()
print(json.dumps({k: v for k, v in st.items() if k != 'result'}, indent=1))
