"""Put the data file inside the page, so the mockup is one file that opens anywhere."""
import sys
tpl, data, out = sys.argv[1:4]
t = open(tpl, encoding="utf-8").read()
d = open(data, encoding="utf-8").read()
tag = '<script src="pb-data.js"></script>'
assert tag in t
open(out, "w", encoding="utf-8").write(t.replace(tag, "<script>" + d + "</script>"))
