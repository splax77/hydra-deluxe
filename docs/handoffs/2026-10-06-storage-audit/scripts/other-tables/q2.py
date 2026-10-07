import sqlite3, os, collections, statistics
db = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\db\hydra.db"
c = sqlite3.connect("file:" + db + "?mode=ro", uri=True)
q = lambda s, *a: c.execute(s, a).fetchall()
print("--- charts")
print("bytes md5,name,artist,charter,path,folder,sig:", q("SELECT SUM(length(md5)),SUM(length(name)),SUM(length(artist)),SUM(length(charter)),SUM(length(path)),SUM(length(folder)),SUM(length(sig)), COUNT(stated_length_ms), COUNT(delay_ms), SUM(delay_ms<>0) FROM charts"))
rows = q("SELECT md5, path, folder, sig FROM charts")
# is folder derivable from path? test: path startswith folder; folder == some ancestor; how many distinct folders
derivable = collections.Counter()
roots = collections.Counter()
for md5, path, folder, sig in rows:
    roots[folder] += 1
    if path.lower().startswith(folder.lower().rstrip("\\/") + "\\") or path.lower().startswith(folder.lower().rstrip("\\/") + "/"):
        derivable["path under folder"] += 1
    elif path.lower() == folder.lower():
        derivable["equal"] += 1
    else:
        derivable["NOT under folder"] += 1
print("folder vs path:", derivable)
print("distinct folders:", len(roots), "top:", roots.most_common(5))
print("sample rows:", rows[:3])
print("sig samples:", [r[3] for r in rows[:5]])
print("md5 case:", q("SELECT SUM(md5 = lower(md5)), SUM(md5 <> lower(md5)) FROM charts"), q("SELECT SUM(hyhash = lower(hyhash)) FROM songmeta"))
print("path kinds:", q("SELECT lower(substr(path, -4)), COUNT(*) FROM charts GROUP BY 1 ORDER BY 2 DESC"))
print("--- duplicate md5s")
print("md5 multiplicity:", q("SELECT n, COUNT(*) FROM (SELECT md5, COUNT(*) n FROM charts GROUP BY md5) GROUP BY n"))
dups = q("SELECT md5, COUNT(*), COUNT(DISTINCT folder), COUNT(DISTINCT name), COUNT(DISTINCT lower(path)), COUNT(DISTINCT sig), GROUP_CONCAT(path, ' || ') FROM charts GROUP BY md5 HAVING COUNT(*)>1")
print("dup groups:", len(dups))
print("dup groups spanning >1 root folder:", sum(1 for d in dups if d[2] > 1))
print("dup groups with differing names:", sum(1 for d in dups if d[3] > 1))
print("dup groups with differing sig:", sum(1 for d in dups if d[5] > 1))
for d in dups[:6]:
    print("  ", d[0][:8], d[1], d[6][:300])
print("--- songmeta")
print("ref_name differs from charts name (naming copy):", q("SELECT COUNT(*) FROM songmeta s JOIN (SELECT md5, name, artist, charter, MIN(rowid) FROM charts GROUP BY md5) c ON c.md5=s.hyhash WHERE s.ref_name<>c.name OR s.ref_artist<>c.artist OR s.ref_charter<>c.charter"))
tm = [r[0] for r in q("SELECT length(tempomap) FROM songmeta")]
tm.sort()
print("tempomap bytes: n", len(tm), "sum", sum(tm), "median", tm[len(tm)//2], "p99", tm[int(len(tm)*.99)], "max", tm[-1], "min", tm[0])
# decode header: i64 res, u32 ntpm, ..., u32 nbpm
import struct
def counts(b):
    res = struct.unpack_from("<q", b, 0)[0]; ntpm = struct.unpack_from("<I", b, 8)[0]
    off = 12 + 16*ntpm; nbpm = struct.unpack_from("<I", b, off)[0]
    return res, ntpm, nbpm
cs = [counts(r[0]) for r in q("SELECT tempomap FROM songmeta")]
print("tick res values:", collections.Counter(x[0] for x in cs).most_common(5))
print("tpm changes: median", statistics.median(x[1] for x in cs), "max", max(x[1] for x in cs))
print("bpm changes: median", statistics.median(x[2] for x in cs), "max", max(x[2] for x in cs), "sum", sum(x[2] for x in cs))
print("length stamp vs results version:", q("SELECT r.hyversion, s.length_version, COUNT(*) FROM songmeta s JOIN results r ON r.hyhash=s.hyhash GROUP BY 1,2"))
print("--- dynamics")
print("dynamics bytes:", q("SELECT SUM(length(md5)), SUM(length(difficulty)), SUM(length(blob)), MIN(length(blob)), MAX(length(blob)), AVG(length(blob)) FROM dynamics"))
print("dynamics keys:", q("SELECT difficulty, pro, count_version, COUNT(*) FROM dynamics GROUP BY 1,2,3"))
print("dynamics md5 per chart:", q("SELECT n, COUNT(*) FROM (SELECT md5, COUNT(*) n FROM dynamics GROUP BY md5) GROUP BY n"))
print("charts (distinct md5) with no dynamics row:", q("SELECT COUNT(DISTINCT md5) FROM charts ch WHERE NOT EXISTS (SELECT 1 FROM dynamics d WHERE d.md5=ch.md5)"))
print("charts (distinct md5) with no results row:", q("SELECT COUNT(DISTINCT md5) FROM charts ch WHERE NOT EXISTS (SELECT 1 FROM results r WHERE r.hyhash=ch.md5)"))
print("charts (distinct md5) with no songmeta row:", q("SELECT COUNT(DISTINCT md5) FROM charts ch WHERE NOT EXISTS (SELECT 1 FROM songmeta s WHERE s.hyhash=ch.md5)"))
print("--- old records table?", q("SELECT name FROM sqlite_master WHERE name LIKE '%record%' OR name LIKE '%songlength%' OR name LIKE '%before_upgrade%'"))
print("--- index/key overhead estimate (bytes of key text)")
print("paths PK key text (hyhash+chartmode+phash):", q("SELECT SUM(length(hyhash)+length(chartmode)+length(phash)) FROM paths"))
print("path_refs row text:", q("SELECT SUM(length(hyhash)+length(chartmode)+length(phash)) FROM path_refs"))
print("results UNIQUE key text per row (hyhash+chartmode+rules_fp):", q("SELECT SUM(length(hyhash)+length(chartmode)+length(rules_fp)) FROM results"))
