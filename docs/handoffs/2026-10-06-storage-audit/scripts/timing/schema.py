import sqlite3, sys
db = sys.argv[1]
c = sqlite3.connect('file:' + db + '?mode=ro', uri=True)
for name, sql in c.execute("select name, sql from sqlite_master where type in ('table','index')"):
    print(name)
    print('   ', (sql or '')[:600])
for t in ['charts', 'results', 'paths', 'path_refs', 'songmeta', 'dynamics']:
    try:
        print(t, c.execute(f'select count(*) from {t}').fetchone()[0])
    except Exception as e:
        print(t, e)
