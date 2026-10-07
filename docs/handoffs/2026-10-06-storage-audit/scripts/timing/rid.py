import sqlite3, sys
for db in sys.argv[1:]:
    c = sqlite3.connect('file:' + db + '?mode=ro', uri=True)
    print(db[-40:], c.execute("select hyhash, result_id from results where hyhash in "
                              "('0fb8ca6ab7301cd20f2b3df527fe2050','89ba4281b17f29a0bbc73a1611518a46') order by 1").fetchall(),
          'max', c.execute('select max(result_id) from results').fetchone()[0])
