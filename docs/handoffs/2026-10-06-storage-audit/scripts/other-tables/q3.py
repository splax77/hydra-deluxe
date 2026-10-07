import sqlite3
db = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\db\hydra.db"
c = sqlite3.connect("file:" + db + "?mode=ro", uri=True)
q = lambda s, *a: c.execute(s, a).fetchall()
print("orphan songmeta tempomap bytes:", q("SELECT SUM(length(tempomap)), SUM(length(ref_name)+length(ref_artist)+length(ref_charter)) FROM songmeta s WHERE NOT EXISTS (SELECT 1 FROM charts ch WHERE ch.md5=s.hyhash)"))
print("orphan results structure bytes + their nodes:", q("SELECT SUM(length(r.structure)), (SELECT SUM(length(p.payload)) FROM paths p JOIN path_refs pr ON pr.hyhash=p.hyhash AND pr.chartmode=p.chartmode AND pr.phash=p.phash JOIN results r2 ON r2.result_id=pr.result_id WHERE NOT EXISTS (SELECT 1 FROM charts ch WHERE ch.md5=r2.hyhash)) FROM results r WHERE NOT EXISTS (SELECT 1 FROM charts ch WHERE ch.md5=r.hyhash)"))
print("old-version rows for charts still in library:", q("SELECT hyhash, chartmode, hyversion FROM results r WHERE hyversion <> '2.1.0+allzero' AND EXISTS (SELECT 1 FROM charts ch WHERE ch.md5=r.hyhash)"))
# hex text hash bytes stored across the file (table rows only, indexes double some)
print("hex-hash text bytes in table rows (results+songmeta+paths+path_refs+dynamics+charts):",
      q("SELECT (SELECT SUM(length(hyhash)) FROM results)+(SELECT SUM(length(hyhash)) FROM songmeta)+(SELECT SUM(length(hyhash)+length(phash)) FROM paths)+(SELECT SUM(length(hyhash)+length(phash)) FROM path_refs)+(SELECT SUM(length(md5)) FROM dynamics)+(SELECT SUM(length(md5)) FROM charts)"))
print("chartmode text bytes (results+paths+path_refs):", q("SELECT (SELECT SUM(length(chartmode)) FROM results)+(SELECT SUM(length(chartmode)) FROM paths)+(SELECT SUM(length(chartmode)) FROM path_refs)"))
# tempomap bpm-change distribution: how many songs carry > 1000 changes and their bytes
print("songs by bpm-change count buckets:", q("""SELECT CASE WHEN n<=32 THEN 'a<=32' WHEN n<=256 THEN 'b<=256' WHEN n<=2048 THEN 'c<=2048' ELSE 'd>2048' END AS b, COUNT(*), SUM(bytes) FROM (SELECT length(tempomap) AS bytes, (length(tempomap)-12-16*1)/16 AS n FROM songmeta) GROUP BY b"""))
# how big would nodes-inline structure be: per result, structure + sum(payload)
print("inline structure+payload per result: avg, max, total:", q("SELECT AVG(t), MAX(t), SUM(t) FROM (SELECT r.result_id, length(r.structure) + COALESCE((SELECT SUM(length(p.payload)) FROM path_refs pr JOIN paths p ON p.hyhash=pr.hyhash AND p.chartmode=pr.chartmode AND p.phash=pr.phash WHERE pr.result_id=r.result_id),0) AS t FROM results r)"))
