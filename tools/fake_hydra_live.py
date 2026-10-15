"""A stand-in for Hydra's live channel, for the MCP helper's transport tests.

It knows no command. It makes the pipe --pipe names, writes a hello line on
connect, and answers each line by echoing it back in a reply line (the wire
shape is in "The protocol after T8" in
docs/superpowers/plans/2026-10-14-hydra-mcp-server.md). Every test of what a
command does runs against the real hydra_uitest --live instead.

Lines that drive the transport failures the real exe cannot be made to show:

    fake-hang           never answer (the helper must give up and kill us)
    fake-exit <code>    write a log line and exit with that code; "dump" after
                        the code also writes the crash dump the hello line named
    fake-big            answer with 50,000 characters of text
    fake-close          close the pipe but keep running (the helper must kill us)

"quit" answers and exits 0, so hydra_stop works against it.

"--as <exe name>" says which exe it stands in for, which sets the mode. In
window mode its log goes where --uitest-log says, as Hydra.exe's does.
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

# The timeout the hello line and every reply carry. Deliberately not the real
# exe's starting value, so a test can tell the helper read it from the line.
SESSION_TIMEOUT_S = 25

# What fake-big answers with: 1,000 lines of 49 characters plus a newline.
BIG_LINE = "x" * 45 + " row"
BIG_TEXT = "\n".join(BIG_LINE for _ in range(1000)) + "\n"


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


def answer(pipe, text):
    reply(pipe, {"ok": True, "text": text, "timeout": SESSION_TIMEOUT_S})


def main(argv):
    exe_name = "hydra_uitest.exe"
    shots = pipe_name = log_path = None
    i = 0
    while i < len(argv):
        a = argv[i]
        value = argv[i + 1] if i + 1 < len(argv) else None
        if a == "--as" and value:
            exe_name = value
        elif a == "--shots" and value:
            shots = Path(value)
        elif a == "--pipe" and value:
            pipe_name = value
        elif a == "--uitest-log" and value:
            log_path = Path(value)
        elif a in ("--db", "--ini", "--ui-ini") and value:
            pass
        else:
            i += 1
            continue
        i += 2
    if not pipe_name:
        print("fake: --live needs --pipe <name>", flush=True)
        return 2
    mode = "window" if exe_name.lower() == "hydra.exe" else "headless"
    base = shots.parent if shots else Path(os.environ.get("TEMP", "."))
    scratch = base / f"fake_engine_{os.getpid()}"
    scratch.mkdir(parents=True, exist_ok=True)
    crash_dump = scratch / "crash.dmp"
    log_file = open(log_path, "w", encoding="utf-8") if mode == "window" and log_path else sys.stdout

    def log(msg):
        print(msg, file=log_file, flush=True)

    pipe = Pipe(pipe_name)
    log(f"fake: pipe {pipe_name} made")
    pipe.accept()
    reply(pipe, {"ok": True, "hello": {
        "mode": mode, "scratch": str(scratch), "pid": os.getpid(), "timeout": SESSION_TIMEOUT_S,
        "log": str(log_path) if log_file is not sys.stdout else "",
        "crash_dump": str(crash_dump)}})
    pending = b""
    while True:
        data = pipe.read()
        if not data:
            log("fake: client went away")
            return 0
        pending += data
        while b"\n" in pending:
            raw, pending = pending.split(b"\n", 1)
            line = raw.decode("utf-8")
            log(f"> {line}")
            verb, _, rest = line.partition(" ")
            if verb == "fake-hang":
                time.sleep(HANG_CAP_S)
                return 0
            if verb == "fake-close":
                # Drop the pipe but keep running: the helper must kill us.
                k32.CloseHandle(pipe.h)
                time.sleep(HANG_CAP_S)
                return 0
            if verb == "fake-exit":
                parts = rest.split()
                code = int(parts[0]) if parts else 1
                if "dump" in parts[1:]:
                    crash_dump.write_bytes(b"MDMP")
                log(f"fake: exiting with code {code}")
                os._exit(code)
            if verb == "fake-big":
                answer(pipe, BIG_TEXT)
                continue
            answer(pipe, line)
            if line == "quit":
                return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
