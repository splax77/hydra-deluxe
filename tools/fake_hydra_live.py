"""A stand-in for Hydra's live channel, so the MCP helper can be tested with no build.

It speaks the wire protocol in the plan's "The live channel" section
(docs/superpowers/plans/2026-10-14-hydra-mcp-server.md): it makes the named
pipe for its own process id, writes the hello line on connect, then answers
one JSON line per script line. The answers are canned; they only have to look
like the real ones closely enough for the helper's parsing.

Extra verbs let the tests drive the bad cases:

    fake-hang           never answer (the helper must give up and kill us)
    fake-exit <code>    write a log line and exit with that code; "dump" after
                        the code also leaves a crash.dmp in the scratch folder
    fake-big            answer with 50,000 characters of text
    fake-close          close the pipe but keep running (the helper must kill us)

It reads the same flags the real exes get and ignores what it does not need.
"--as <exe name>" says which exe it stands in for, which sets the mode.
"""

import ctypes
import json
import os
import sys
import time
from ctypes import wintypes
from pathlib import Path

PIPE_ACCESS_DUPLEX = 0x00000003
PIPE_TYPE_BYTE_READMODE_BYTE_WAIT = 0x00000000
ERROR_PIPE_CONNECTED = 535
BUFFER_BYTES = 65536

# How long fake-hang and fake-close sleep. Only a cap so that a stand-in
# nobody kills still goes away; the helper's tests kill it long before.
HANG_CAP_S = 600

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
k32.CreateNamedPipeW.restype = wintypes.HANDLE
k32.CreateNamedPipeW.argtypes = [
    wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD,
    wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, wintypes.LPVOID,
]
k32.ConnectNamedPipe.argtypes = [wintypes.HANDLE, wintypes.LPVOID]
k32.ReadFile.argtypes = [
    wintypes.HANDLE, wintypes.LPVOID, wintypes.DWORD, ctypes.POINTER(wintypes.DWORD),
    wintypes.LPVOID,
]
k32.WriteFile.argtypes = [
    wintypes.HANDLE, wintypes.LPCVOID, wintypes.DWORD, ctypes.POINTER(wintypes.DWORD),
    wintypes.LPVOID,
]
k32.CloseHandle.argtypes = [wintypes.HANDLE]

INVALID_HANDLE = wintypes.HANDLE(-1).value

# What fake-big answers with: 1,000 lines of 49 characters plus a newline.
BIG_LINE = "x" * 45 + " row"
BIG_TEXT = "\n".join(BIG_LINE for _ in range(1000)) + "\n"


def log(msg):
    print(msg, flush=True)


class Pipe:
    def __init__(self, name):
        self.h = k32.CreateNamedPipeW(
            name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE_READMODE_BYTE_WAIT, 1,
            BUFFER_BYTES, BUFFER_BYTES, 0, None,
        )
        if self.h in (None, INVALID_HANDLE):
            raise OSError(ctypes.get_last_error(), "CreateNamedPipeW failed")

    def accept(self):
        if not k32.ConnectNamedPipe(self.h, None):
            err = ctypes.get_last_error()
            if err != ERROR_PIPE_CONNECTED:
                raise OSError(err, "ConnectNamedPipe failed")

    def read(self):
        buf = ctypes.create_string_buffer(BUFFER_BYTES)
        got = wintypes.DWORD(0)
        if not k32.ReadFile(self.h, buf, BUFFER_BYTES, ctypes.byref(got), None):
            return b""
        return buf.raw[: got.value]

    def write(self, data):
        done = 0
        while done < len(data):
            chunk = data[done:]
            wrote = wintypes.DWORD(0)
            if not k32.WriteFile(self.h, chunk, len(chunk), ctypes.byref(wrote), None):
                raise OSError(ctypes.get_last_error(), "WriteFile failed")
            done += wrote.value


def reply(pipe, obj):
    pipe.write((json.dumps(obj) + "\n").encode("utf-8"))


def ok(pipe, text=""):
    reply(pipe, {"ok": True, "text": text})


def fail(pipe, error, log_text):
    reply(pipe, {"ok": False, "error": error, "log": log_text})


class Fake:
    def __init__(self, mode, scratch, shots):
        self.mode = mode
        self.scratch = scratch
        self.shots = shots
        self.charts = 0
        self.search = ""

    def state(self):
        return f"state:\n  store=ready\n  charts={self.charts} shown={self.charts} search=\"{self.search}\"\n"

    def run(self, pipe, line):
        verb, _, rest = line.partition(" ")
        rest = rest.strip()
        if verb == "quit":
            ok(pipe)
            return False
        if verb == "fake-hang":
            time.sleep(HANG_CAP_S)
            return False
        if verb == "fake-close":
            # Drop the pipe but keep running: the helper must kill us.
            k32.CloseHandle(pipe.h)
            time.sleep(HANG_CAP_S)
            return False
        if verb == "fake-exit":
            parts = rest.split()
            code = int(parts[0]) if parts else 1
            if "dump" in parts[1:]:
                (self.scratch / "crash.dmp").write_bytes(b"MDMP")
            log(f"fake: exiting with code {code}")
            os._exit(code)
        if verb == "fake-big":
            ok(pipe, BIG_TEXT)
            return True
        if verb in ("click", "check", "uncheck"):
            if "missing" in rest:
                fail(pipe, f"{verb} failed", f"[error] Unable to locate item: '{rest}'\n")
            else:
                if verb == "click" and rest.lower().startswith("scan"):
                    self.charts = 97
                ok(pipe)
            return True
        if verb == "type":
            ref, bar, text = rest.partition("|")
            if not bar:
                fail(pipe, "type needs \"<ref> | <text>\"", "")
            else:
                if "Search" in ref:
                    self.search = text.strip()
                ok(pipe)
            return True
        if verb in ("window", "key", "timeout", "wait", "wait-idle"):
            ok(pipe)
            return True
        if verb == "wait-text":
            if "never" in rest:
                fail(pipe, f"\"{rest}\" did not appear within 30s", "")
            else:
                ok(pipe)
            return True
        if verb == "text":
            ok(pipe, "Library\nScan library\n2x Bass\nSearch\nAnalyze shown (0)...\n")
            return True
        if verb == "dump":
            ok(pipe,
               'window "Hydra" id=0x1 pos=(0,0) size=(1280,800)\n'
               '  "Scan library" id=0x2 rect=(10,40 120x22) \n'
               '  "2x Bass" id=0x3 rect=(140,40 80x22) checkable\n'
               '  "Search" id=0x4 rect=(230,40 300x22) \n'
               'window "Settings" id=0x5 pos=(100,100) size=(400,300)\n'
               '  "2x Bass##s" id=0x6 rect=(110,140 80x22) checkable\n')
            return True
        if verb == "rect":
            ok(pipe, f'  "{rest}" id=0x2 rect=(10,40 120x22) \n')
            return True
        if verb == "state":
            ok(pipe, self.state())
            return True
        if verb == "reset":
            self.charts = 0
            self.search = ""
            ok(pipe)
            return True
        if verb == "screenshot":
            path = self.shots / f"{rest}.png"
            ok(pipe, f"{path}\n")
            return True
        fail(pipe, f"unknown verb \"{verb}\"", "")
        return True


def main(argv):
    exe_name = "hydra_uitest.exe"
    shots = None
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--as" and i + 1 < len(argv):
            exe_name = argv[i + 1]
            i += 1
        elif a == "--shots" and i + 1 < len(argv):
            shots = Path(argv[i + 1])
            i += 1
        elif a in ("--db", "--ini", "--ui-ini") and i + 1 < len(argv):
            i += 1
        i += 1
    mode = "window" if exe_name.lower() == "hydra.exe" else "headless"
    base = shots.parent if shots else Path(os.environ.get("TEMP", "."))
    scratch = base / f"fake_engine_{os.getpid()}"
    scratch.mkdir(parents=True, exist_ok=True)
    fake = Fake(mode, scratch, shots or scratch)

    pipe = Pipe(rf"\\.\pipe\hydra-live-{os.getpid()}")
    log(f"fake: pipe made for pid {os.getpid()}")
    pipe.accept()
    reply(pipe, {"ok": True, "hello": {"mode": mode, "scratch": str(scratch), "pid": os.getpid()}})
    pending = b""
    while True:
        data = pipe.read()
        if not data:
            log("fake: client went away")
            return 0
        pending += data
        while b"\n" in pending:
            raw, pending = pending.split(b"\n", 1)
            line = raw.decode("utf-8").strip()
            if not line:
                continue
            log(f"> {line}")
            if not fake.run(pipe, line):
                return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
