import json,re,sys,os
S=sys.argv[1]
rd=lambda f: open(os.path.join(S,f),encoding='utf-8').read().split('\n')
rows=json.load(open(os.path.join(S,'tri.json'),encoding='utf-8'))
aud=rd('2026-10-03-derivation-audit.md')
# finding sections
sec={}; cur=None
for i,l in enumerate(aud):
    m=re.match(r'#### (R?\d+(?:\.\d+)?)\.?\s',l)
    if m: cur=m.group(1); sec[cur]=[i+1,i+1]
    elif l.startswith('#') and cur: sec[cur][1]=i; cur=None
    elif cur: sec[cur][1]=i+1
qfiles=['2026-10-04-phase-3-5-questions.md','2026-10-04-phase-6-questions.md','2026-10-04-phase-7-questions.md']
qblocks=[]
for f in qfiles:
    L=rd(f); starts=[i for i,l in enumerate(L) if re.match(r'(\*\*\d+[a-z]?\. |## \d+\. )',l)]
    for k,s in enumerate(starts):
        e=starts[k+1] if k+1<len(starts) else len(L)
        e2=s+1
        while e2<e and not L[e2].startswith('## '): e2+=1
        head=L[s]; ids=set(re.findall(r'\b(\d{1,3})\b',' '.join(re.findall(r'\(([^)]*)\)',head))))
        qblocks.append((f,s+1,e2,ids,'\n'.join(L[s:e2]).strip()))
def packet(r):
    i=r['id']; out=[f"## Finding {i} ({r['kind']}, triage status {r['status_at_head']})",'',"Triage row (docs/audit/2026-10-03-fix-triage.json):",'```json',json.dumps(r,indent=1,ensure_ascii=False),'```']
    if i in sec:
        a,b=sec[i]; out+=[f"Finding text: docs/audit/2026-10-03-derivation-audit.md lines {a}-{b}:",'```','\n'.join(aud[a-1:b]).strip(),'```']
    else: out.append('Finding heading not found in derivation-audit.md (look for it by title).')
    hits=[q for q in qblocks if i in q[3]]
    if hits:
        for f,a,b,_,t in hits: out+=[f"Question citing it in its heading: docs/audit/{f} lines {a}-{b}:",'```',t,'```']
    else: out.append('No question heading in the phase 3-5, 6 or 7 question files cites this id in parentheses. Search the files and the decision log yourself before concluding it was never asked (a search that finds nothing proves nothing alone).')
    return '\n'.join(out)+'\n'
groups={}
for r in rows:
    if not r['decision'] and r['status_at_head']=='fixed': continue
    k=r['kind']; groups.setdefault(k,[]).append(r)
import collections
print({k:len(v) for k,v in groups.items()})
def write(name,rs):
    with open(os.path.join(S,name),'w',encoding='utf-8') as fh:
        fh.write(f"# Packet {name}: {len(rs)} findings\n\nVerbatim extracts made by the main session's script. They save you lookups; they are not evidence. Check every line you quote against the file itself.\n\n")
        for r in rs: fh.write(packet(r)+'\n')
    print(name,len(rs),[r['id'] for r in rs])
def num(r):
    import re
    m=re.findall(r'\d+',r['id']); return (r['id'][0].isalpha(), [int(x) for x in m])
A=sorted(groups['assumption'],key=num); A=[r for r in A if r['decision']]
D=sorted([r for r in groups['drift']+groups.get('double_reading',[]) if r['decision']],key=num)
U=sorted([r for r in groups['duplicate']+groups.get('doc',[]) if r['decision']],key=num)
X=sorted([r for r in rows if r['status_at_head']!='fixed'],key=num)
write('P-A1.md',A[:23]); write('P-A2.md',A[23:])
write('P-D1.md',D[:23]); write('P-D2.md',D[23:45]); write('P-D3.md',D[45:])
xs=[r for r in X if r not in U]
write('P-U.md',U+[r for r in X if r not in U])
