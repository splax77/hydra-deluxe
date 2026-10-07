import subprocess,time,json
R='C:/Users/Patrick/Downloads/Hydra/hydra-test/build-ship/Release/hydra_replay.exe'
chart='C:/Clone Hero/songs/synchotic/Sync Charts/Misc/HopH2O Discography Charts/blink-182 - Discography/notes.mid'
d=json.load(open('d.json')); acts=d['paths'][0]['activations']
k=next(k for k in ('tick','act_tick','timecode') if k in acts[0]); ticks=[a[k] for a in acts]
for n in (5,10,15,20,25,30,40,60):
  s=time.perf_counter()
  try:
    p=subprocess.run([R,'target','--chart',chart,'--ticks',','.join(map(str,ticks[:n])),'--cap','4','--out','g.json'],capture_output=True,text=True,timeout=20)
    g=json.load(open('g.json')); print(f'first {n:3d} acts: {1000*(time.perf_counter()-s):7.0f} ms  paths returned {len(g.get("paths",[]))} realized={g.get("realized")}',flush=True)
  except subprocess.TimeoutExpired: print(f'first {n:3d} acts: TIMEOUT >20 s',flush=True); break
