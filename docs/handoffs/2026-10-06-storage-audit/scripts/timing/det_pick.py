import sqlite3, sys, os, random, collections
db = sys.argv[1]
c = sqlite3.connect('file:' + db + '?mode=ro', uri=True)
print(c.execute("select sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills, count(*) from results where hyversion='2.1.0+allzero' group by 1,2,3,4,5,6").fetchall())
big = ['0fb8ca6ab7301cd20f2b3df527fe2050', 'd6d8cc18cd596e14e71918a2f6b0dea3',
       '4c1b646d56fc48a3aacd217f6a9c96b5', '89ba4281b17f29a0bbc73a1611518a46',
       'b106d1452b83c6ff46ab3c5af3f60c67', 'c0572b15ef0fa88234e5d8a3c5a392a7',
       '2865ec30dbe400b0f697eec02a54eea7', '8eb37af7c06c92d02a8b2df48ce1fbca',
       'c244fffa70073a918a2986297fd5ecf8', '972bcc35b49ab456244c4f5145f7ab2f']
rows = c.execute("select c.md5, c.path from charts c join results r on r.hyhash=c.md5 where r.hyversion='2.1.0+allzero' and c.path not like '%Misc Downloads%'").fetchall()
random.seed(7)
rand = random.sample(sorted(set(rows)), 12)
folders = []
for md5, p in [(m, c.execute('select path from charts where md5=?', (m,)).fetchone()[0]) for m in big] + rand:
    f = os.path.dirname(p)
    if f not in folders:
        folders.append(f)
print(len(folders), 'folders')
n = collections.Counter()
allpaths = [r[0] for r in c.execute('select path from charts')]
for f in folders:
    n[f] = sum(1 for p in allpaths if p.startswith(f + os.sep))
    print(f'  {n[f]:4d} charts under  {f}')
open(sys.argv[2], 'w', encoding='utf-8').write('\n'.join(folders) + '\n')
