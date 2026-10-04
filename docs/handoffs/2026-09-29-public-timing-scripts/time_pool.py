# Memory-safe timing of the public Hydra engine over a saved chart list.
# Written 2026-09-29 for the next session; NOT YET RUN. Smoke-test it first
# (see the handoff, step 3).
#
# What it fixes compared with the 2026-09-29 run:
#   - Keeps no results. Each chart's record is dropped as soon as it's timed,
#     so memory doesn't grow over the run (runfolder keeps every record for
#     its final JSON dump).
#   - A few workers pull charts from one shared queue, instead of 20 fixed
#     sections. A giant chart holds up one worker, not a whole section.
#   - Workers restart every --recycle charts, which hands their memory back.
#   - A watchdog samples free RAM every 2 s. Below --warn-gb the run is marked
#     as possibly paged; below --stop-gb it stops the pool before Windows
#     starts swapping.
#   - Writes one line per chart as it goes, so a stopped run keeps its data.
#
# usage (from anywhere):
#   python time_pool.py <charts.json> <out_dir> --hydra-root <public clone>
#          [--workers 4] [--recycle 50] [--only-largest N | --skip-largest N]
import argparse
import csv
import ctypes
import ctypes.wintypes as wt
import json
import os
import sys
import threading
import time
from multiprocessing import Pool

# Children re-import this module on Windows, so the path setup happens here,
# driven by an environment variable the parent sets before starting the pool.
if os.environ.get('HYDRA_ROOT'):
    sys.path.insert(0, os.environ['HYDRA_ROOT'])


class _ProcMem(ctypes.Structure):
    _fields_ = [('cb', wt.DWORD), ('PageFaultCount', wt.DWORD),
                ('PeakWorkingSetSize', ctypes.c_size_t), ('WorkingSetSize', ctypes.c_size_t),
                ('QuotaPeakPagedPoolUsage', ctypes.c_size_t), ('QuotaPagedPoolUsage', ctypes.c_size_t),
                ('QuotaPeakNonPagedPoolUsage', ctypes.c_size_t),
                ('QuotaNonPagedPoolUsage', ctypes.c_size_t),
                ('PagefileUsage', ctypes.c_size_t), ('PeakPagefileUsage', ctypes.c_size_t)]


class _SysMem(ctypes.Structure):
    _fields_ = [('dwLength', wt.DWORD), ('dwMemoryLoad', wt.DWORD),
                ('ullTotalPhys', ctypes.c_ulonglong), ('ullAvailPhys', ctypes.c_ulonglong),
                ('ullTotalPageFile', ctypes.c_ulonglong), ('ullAvailPageFile', ctypes.c_ulonglong),
                ('ullTotalVirtual', ctypes.c_ulonglong), ('ullAvailVirtual', ctypes.c_ulonglong),
                ('ullAvailExtendedVirtual', ctypes.c_ulonglong)]


_k32 = ctypes.WinDLL('kernel32')
_k32.GetCurrentProcess.restype = wt.HANDLE
_k32.K32GetProcessMemoryInfo.argtypes = [wt.HANDLE, ctypes.POINTER(_ProcMem), wt.DWORD]


def process_mem_mb():
    """(working set now, peak working set since this process started), in MB."""
    pm = _ProcMem()
    pm.cb = ctypes.sizeof(pm)
    _k32.K32GetProcessMemoryInfo(_k32.GetCurrentProcess(), ctypes.byref(pm), pm.cb)
    return pm.WorkingSetSize / 2**20, pm.PeakWorkingSetSize / 2**20


def free_ram_gb():
    sm = _SysMem()
    sm.dwLength = ctypes.sizeof(sm)
    _k32.GlobalMemoryStatusEx(ctypes.byref(sm))
    return sm.ullAvailPhys / 2**30


def time_one(chart):
    """Analyze one chart exactly as the timed runfolder did, keep nothing."""
    import hydra.hyutil as hyutil
    t0, c0 = time.perf_counter(), time.process_time()
    ok, err = 1, ''
    try:
        hyutil.analyze_chart_file(chart['path'], 'Expert', True, True, 'scores', 4, 10,
                                  export_tempomap=True)
    except Exception as e:
        ok, err = 0, repr(e)[:200]
    wall, cpu = time.perf_counter() - t0, time.process_time() - c0
    ws, peak = process_mem_mb()
    return chart['raw'], wall, cpu, ok, round(ws), round(peak), os.getpid(), err


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('charts_json')
    ap.add_argument('out_dir')
    ap.add_argument('--hydra-root', required=True)
    ap.add_argument('--workers', type=int, default=4)
    ap.add_argument('--recycle', type=int, default=50)
    ap.add_argument('--only-largest', type=int, default=0)
    ap.add_argument('--skip-largest', type=int, default=0)
    ap.add_argument('--limit', type=int, default=0, help='first N charts only (smoke test)')
    ap.add_argument('--warn-gb', type=float, default=6.0)
    ap.add_argument('--stop-gb', type=float, default=3.0)
    a = ap.parse_args()

    os.environ['HYDRA_ROOT'] = os.path.abspath(a.hydra_root)
    os.makedirs(a.out_dir, exist_ok=True)
    with open(a.charts_json, encoding='utf-8') as f:
        charts = json.load(f)['charts']

    # Biggest chart files first, so the slow ones start early instead of
    # straggling at the end. File size is a rough proxy: a .sng also holds audio.
    order = sorted(charts, key=lambda c: os.path.getsize(c['path']), reverse=True)
    if a.only_largest:
        todo = order[:a.only_largest]
    elif a.skip_largest:
        todo = order[a.skip_largest:]
    else:
        todo = order
    if a.limit:
        todo = todo[:a.limit]

    low_ram = threading.Event()
    stop = threading.Event()
    done = threading.Event()
    min_free = [free_ram_gb()]

    def watchdog():
        with open(os.path.join(a.out_dir, 'ram.csv'), 'w', newline='') as fr:
            w = csv.writer(fr)
            w.writerow(['t', 'free_gb'])
            t0 = time.perf_counter()
            while not done.is_set():
                g = free_ram_gb()
                min_free[0] = min(min_free[0], g)
                w.writerow([round(time.perf_counter() - t0, 1), round(g, 2)])
                fr.flush()
                if g < a.warn_gb and not low_ram.is_set():
                    low_ram.set()
                    print(f'!! free RAM {g:.1f} GB: times from here may include paging', flush=True)
                if g < a.stop_gb:
                    print(f'!! free RAM {g:.1f} GB: stopping before Windows swaps', flush=True)
                    stop.set()
                    return
                time.sleep(2)

    threading.Thread(target=watchdog, daemon=True).start()

    by_raw = {c['raw']: c for c in charts}
    sums = {'n': 0, 'ok': 0, 'wall': 0.0, 'cpu': 0.0}
    t_start = time.perf_counter()
    with open(os.path.join(a.out_dir, 'times.csv'), 'w', newline='', encoding='utf-8') as ft:
        w = csv.writer(ft)
        w.writerow(['raw', 'wall_s', 'cpu_s', 'ok', 'ws_mb', 'peak_mb', 'pid', 'low_ram',
                    'song', 'error'])
        with Pool(a.workers, maxtasksperchild=a.recycle) as pool:
            for raw, wall, cpu, ok, ws, peak, pid, err in pool.imap_unordered(
                    time_one, todo, chunksize=1):
                c = by_raw[raw]
                w.writerow([raw, round(wall, 3), round(cpu, 3), ok, ws, peak, pid,
                            int(low_ram.is_set()), f"{c['artist']} - {c['title']}", err])
                ft.flush()
                sums['n'] += 1
                sums['ok'] += ok
                sums['wall'] += wall
                sums['cpu'] += cpu
                if sums['n'] % 250 == 0:
                    print(f"{sums['n']}/{len(todo)} charts, {time.perf_counter() - t_start:.0f}s,"
                          f" free RAM {free_ram_gb():.1f} GB", flush=True)
                if stop.is_set():
                    pool.terminate()
                    break
    done.set()

    # The one clean public number from 2026-09-29: charts with raw index below
    # 2250 took 394.5 s in a single process with nothing else running. If this
    # run covered them all, the ratio tells how much the workers slowed each other.
    baseline = [r for r in by_raw if r < 2250]
    summary = {'charts_done': sums['n'], 'of': len(todo), 'ok': sums['ok'],
               'sum_wall_s': round(sums['wall'], 1), 'sum_cpu_s': round(sums['cpu'], 1),
               'pool_wall_s': round(time.perf_counter() - t_start, 1),
               'workers': a.workers, 'min_free_ram_gb': round(min_free[0], 2),
               'ram_went_low': low_ram.is_set(), 'stopped_for_ram': stop.is_set()}
    with open(os.path.join(a.out_dir, 'times.csv'), encoding='utf-8') as ft:
        rows = list(csv.DictReader(ft))
    base_rows = [float(r['wall_s']) for r in rows if int(r['raw']) < 2250]
    if len(base_rows) == len(baseline):
        summary['baseline_ratio'] = round(sum(base_rows) / 394.5, 3)
    with open(os.path.join(a.out_dir, 'summary.json'), 'w') as fs:
        json.dump(summary, fs, indent=1)
    print(json.dumps(summary, indent=1))


if __name__ == '__main__':
    main()
