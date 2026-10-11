import re
src=open("double_lane_timer_sim.py",encoding="utf-8").read()
start=src.index("def simulate"); end=src.index("rows = []")
ns={}; exec(src[start:end],ns)
sim=ns["simulate"]
def run(name,notes): print(name, sim(notes,None,True))
# (t, colour, flagged, last)
run("A@0,B@0.5,A@1.0,B@1.5 (alternating 0.5s)", [(0,2,1,0),(0.5,3,1,0),(1.0,2,1,0),(1.5,3,1,1)])
run("A@0,B@0.8 (slow second pad)",               [(0,2,1,0),(0.8,3,1,1)])
run("A,A,A,A each 0.3s then B@1.3",              [(0,2,1,0),(0.3,2,1,0),(0.6,2,1,0),(0.9,2,1,0),(1.3,3,1,1)])
run("fast trill 12 notes @0.1s",                 [(i*0.1,2+(i%2),1,i==11) for i in range(12)])
