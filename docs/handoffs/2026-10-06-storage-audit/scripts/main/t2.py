import subprocess, time, json, shutil, os
REL = 'C:/Users/Patrick/Downloads/Hydra/hydra-test/build-ship/Release/'
B = REL + 'hydra_batch.exe'; R = REL + 'hydra_replay.exe'
SC = 'C:/Clone Hero/songs/synchotic/Sync Charts/'
charts = [('blink-182 Discography', SC + 'Misc/HopH2O Discography Charts/blink-182 - Discography'),
          ('Rise Against Discography', SC + 'Misc/HopH2O Discography Charts/Rise Against - Discography (2024)'),
          ('Hail The Sun Discography', SC + 'Misc/HopH2O Discography Charts/Hail The Sun - Discography (2025)'),
          ('Wings of a Butterfly (median)', SC + 'Rock Band/Rock Band 2 DLC/HIM - Wings of a Butterfly')]

def t(cmd, n=3, any_rc=False):
    best = 1e9
    for _ in range(n):
        s = time.perf_counter()
        try:
            p = subprocess.run(cmd, capture_output=True, text=True, timeout=20)
        except subprocess.TimeoutExpired:
            print('  TIMEOUT >20 s:', os.path.basename(cmd[0]), cmd[1]); return None
        d = time.perf_counter() - s
        if p.returncode and not any_rc:
            print('  FAIL', os.path.basename(cmd[0]), cmd[1], (p.stdout + p.stderr)[-400:]); return None
        best = min(best, d)
    return best * 1000

shutil.copy('tiny.db', 't.db')
start = t([R, '--help'], 5, any_rc=True)
print(f'program start only (hydra_replay --help): {start:.0f} ms')
for name, folder in reversed(charts):
    print('==', name, flush=True)
    chart = folder + '/notes.mid'
    ana = t([B, '--db', 't.db', '--redo', folder]); print(f'  full analysis + save (batch --redo):     {ana:6.0f} ms', flush=True)
    dump = t([R, 'dump', '--no-analyze', '--db', 't.db', '--chart', chart, '--cap', '4', '--ms', '10',
              '--depth-mode', 'scores', '--depth', '4', '--out', 'd.json'])
    if dump is None: continue
    d = json.load(open('d.json'))
    print(f'  load stored record (dump --no-analyze):  {dump:6.0f} ms   source={d.get("source")} json={os.path.getsize("d.json")//1024} KB', flush=True)
    p0 = d['paths'][0]
    acts = p0.get('activations') or p0.get('acts') or []
    k = next((k for k in ('tick', 'act_tick', 'timecode', 'ticks') if acts and k in acts[0]), None)
    if not k: print('  path keys', list(p0.keys()), 'act keys', list(acts[0].keys()) if acts else None); continue
    ticks = [a[k] for a in acts]
    tg = t([R, 'target', '--chart', chart, '--ticks', ','.join(map(str, ticks)), '--cap', '4', '--out', 'g.json'], 1)
    if tg is None: continue
    g = json.load(open('g.json'))
    print(f'  engine re-prices best path (target):     {tg:6.0f} ms   {len(ticks)} activations, realized={g.get("realized")} json={os.path.getsize("g.json")//1024} KB', flush=True)
