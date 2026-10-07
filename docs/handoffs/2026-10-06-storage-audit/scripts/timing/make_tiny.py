import sqlite3, sys, shutil
src, dst = sys.argv[1], sys.argv[2]
keep = sys.argv[3].split(',')
shutil.copyfile(src, dst)
c = sqlite3.connect(dst)
q = ','.join('?' * len(keep))
c.execute(f'delete from results where hyhash not in ({q})', keep)
c.execute('delete from path_refs where result_id not in (select result_id from results)')
c.execute(f'delete from paths where hyhash not in ({q})', keep)
c.execute(f'delete from songmeta where hyhash not in ({q})', keep)
c.execute(f'delete from dynamics where md5 not in ({q})', keep)
c.execute(f'delete from charts where md5 not in ({q})', keep)
c.commit()
c.execute('vacuum')
for t in ['results', 'paths', 'path_refs', 'songmeta', 'charts']:
    print(t, c.execute(f'select count(*) from {t}').fetchone()[0])
print(c.execute(f'select hyhash, (select path from charts where md5=hyhash) from results').fetchall())
