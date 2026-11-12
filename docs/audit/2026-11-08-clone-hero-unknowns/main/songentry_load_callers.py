"""Print the instructions before each direct call to SongEntry's chart loader 0x35C110.

Run from the repo root: py docs/audit/2026-11-08-clone-hero-unknowns/main/songentry_load_callers.py
Each key is the start of the method that holds the call (looked up in dump.cs);
the loader is an instance method, so its first bool argument is in edx.
Writes to stdout only.
"""
import subprocess, sys
sites = {"d8d80": ["d92ba"], "235ad0": ["235bcd"], "284f00": ["285045"], "289460": ["2894cf"],
         "3968d0": ["396a4f"], "27e730": ["27e86d"], "28a880": ["28a8ef"], "2847c0": ["2848ff"],
         "285830": ["2858a2"], "3978e0": ["397a79"], "3970d0": ["397261"], "2880e0": ["28821f"]}
for start, calls in sites.items():
    out = subprocess.run([sys.executable, "-m", "tools.ch_mcp.chdis", start, "1400"], capture_output=True,
                         text=True, encoding="utf-8", errors="replace").stdout.splitlines()
    for c in calls:
        # call site address is the return address of the call; find the call instruction ending there
        for i, l in enumerate(out):
            if l.startswith("%08x" % int(c, 16)):
                print("==", start, c)
                for x in out[max(0, i - 9):i + 1]:
                    print(ascii(x))
                break
        else:
            print("==", start, c, "NOT FOUND")
