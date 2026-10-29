"""Find where Clone Hero keeps the playing chart's checksum (overlay spike,
question 2).

Plain version: give it the chart's MD5 hash. While the song plays it searches
the game's writable memory once for every form of that hash
(il2cpp_strings.encode_forms) and prints each hit. A hit that looks like the
text of a .NET string is then traced back: the finder looks for a pointer to
that string object in the engine object, then in the objects the engine's
pointers lead to (walk_pointers owns how far). A path found that way is a
cheap reader for a future overlay. A raw-bytes hit is traced back another
way, because the Il2Cpp dump says the game keeps the checksum as raw bytes in
a SongEntry that a static GlobalVariables instance points to: one more search
of memory looks for pointers to each candidate SongEntry. Without a path, a
hash that stays at the same
address is the fallback. So the search runs a second time, RESCAN_GAP_S
later, and says which hits held still and which moved.

Every step prints how long it took and the song clock's state across it
(clock_state names the states). A frozen clock can mean a pause or the song's
end; the clock alone does not tell them apart. Candidate offsets go into this
report and the spike's write-up, not into constants.py.

    py tools\\ch_probe\\experiments\\find_checksum.py --hash 234C917654CC7257E2BAFCF7D761304B

Run it, then start the song. It opens the game read-only and never writes.
"""

from __future__ import annotations

import argparse
import ctypes
import os
import struct
import sys
import time
from dataclasses import dataclass
from typing import (Callable, Dict, Iterable, List, Optional, Sequence, TextIO,
                    Tuple)

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C, engine_finder  # noqa: E402
from tools.ch_probe import il2cpp_strings as S  # noqa: E402
from tools.ch_probe.process import open_process  # noqa: E402

# How many bytes before each hit the report prints, to show what holds it.
CONTEXT_BYTES = 32

# The pointer walk: how many objects past the engine it follows, how many
# objects it reads in all before it gives up, and how much of each it reads.
MAX_HOPS = 2
MAX_OBJECTS_READ = 4096
OBJECT_READ_SIZE = 0x400

# How long after the first search the second one runs.
RESCAN_GAP_S = 10.0

# The top of user-mode memory on 64-bit Windows: where the region walk stops,
# and one of the tests pointer_slots uses to tell a pointer from other data.
USER_SPACE_END = 0x7FFFFFFFFFFF
POINTER_SIZE = 8

# Exit codes.
EXIT_OK = 0
EXIT_NO_SONG = 2


# --- the region walk -----------------------------------------------------------

def live_rw_regions(proc) -> Iterable[Tuple[int, int]]:
    """Yield (base, size) for every committed read-write region the engine
    scan would read. LIVE-ONLY: calls VirtualQueryEx on the real process.

    A copy of the region walk in engine_finder.scan_for_engine, kept apart
    so Phase 0 does not refactor engine_finder.py; the spike plan lists the
    duplication under Out of scope. Its size limits are the original's."""
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    handle = ctypes.c_void_p(proc.handle)
    mbi = engine_finder.MEMORY_BASIC_INFORMATION()
    mbi_size = ctypes.sizeof(mbi)

    addr = 0
    while addr < USER_SPACE_END:
        ret = k32.VirtualQueryEx(
            handle, ctypes.c_void_p(addr), ctypes.byref(mbi), mbi_size
        )
        if ret == 0:
            break

        if (mbi.State == engine_finder.MEM_COMMIT
                and engine_finder.is_rw(mbi.Protect) and mbi.RegionSize > 0):
            base = mbi.BaseAddress or 0
            size = mbi.RegionSize
            # Skip tiny regions and absurdly large ones
            if 0x100 <= size <= 256 * 1024 * 1024:
                yield base, size

        next_addr = (mbi.BaseAddress or 0) + mbi.RegionSize
        if next_addr <= addr:
            break
        addr = next_addr


# --- the direct search -------------------------------------------------------------

@dataclass(frozen=True)
class Hit:
    """One place the hash sits: its address, which form matched, and the
    bytes just before it inside the same region."""

    address: int
    form: S.HashForm
    before: bytes


def hits_in_data(base: int, data: bytes,
                 forms: Sequence[S.HashForm]) -> List[Hit]:
    """Every match of every form in one region's bytes, by address. The bytes
    before a hit stop at the region's start, so no hit reaches into another
    region."""
    hits: List[Hit] = []
    for form in forms:
        pos = data.find(form.pattern)
        while pos != -1:
            hits.append(Hit(base + pos, form,
                            data[max(0, pos - CONTEXT_BYTES):pos]))
            pos = data.find(form.pattern, pos + 1)
    hits.sort(key=lambda h: (h.address, h.form.name))
    return hits


@dataclass
class ScanResult:
    hits: List[Hit]
    regions: int
    bytes_scanned: int


def scan_for_hash(proc, forms: Sequence[S.HashForm], *,
                  regions: Callable = live_rw_regions) -> ScanResult:
    """Read every region `regions(proc)` names once and collect the hits. A
    region that cannot be read is skipped, as the engine scan does."""
    hits: List[Hit] = []
    seen = set()
    count = 0
    total = 0
    for base, size in regions(proc):
        try:
            data = proc.read(base, size)
        except OSError:
            continue  # unreadable region
        count += 1
        total += len(data)
        for hit in hits_in_data(base, data, forms):
            key = (hit.address, hit.form.name)
            if key not in seen:
                seen.add(key)
                hits.append(hit)
    return ScanResult(hits, count, total)


def compare_passes(first: Sequence[Hit],
                   second: Sequence[Hit]) -> Tuple[List[Hit], List[Hit], List[Hit]]:
    """(held, moved, new): first-pass hits whose address still holds the same
    form, first-pass hits that are gone from their address, and second-pass
    hits at addresses the first pass did not have."""
    first_keys = {(h.address, h.form.name) for h in first}
    second_keys = {(h.address, h.form.name) for h in second}
    held = [h for h in first if (h.address, h.form.name) in second_keys]
    moved = [h for h in first if (h.address, h.form.name) not in second_keys]
    new = [h for h in second if (h.address, h.form.name) not in first_keys]
    return held, moved, new


# --- the pointer walk ------------------------------------------------------------

def pointer_slots(raw: bytes) -> Iterable[Tuple[int, int]]:
    """(offset, value) for each 8-byte slot of an object's bytes whose value
    could be a pointer into user-mode memory."""
    usable = len(raw) - len(raw) % POINTER_SIZE
    for offset in range(0, usable, POINTER_SIZE):
        (value,) = struct.unpack_from("<Q", raw, offset)
        if value and value % POINTER_SIZE == 0 and value <= USER_SPACE_END:
            yield offset, value


@dataclass
class WalkResult:
    paths: List[Tuple[Tuple[int, ...], int]]   # (offsets from the engine, target)
    objects_read: int
    hit_cap: bool


def walk_pointers(proc, engine: int, targets: Iterable[int], *,
                  max_hops: int = MAX_HOPS,
                  max_objects: int = MAX_OBJECTS_READ) -> WalkResult:
    """Look for pointers from the engine object to any of `targets`: in the
    engine's own first OBJECT_READ_SIZE bytes, then in each object those bytes
    point to, and so on `max_hops` objects out. Each object is read once, on
    the shortest route to it. Stops after `max_objects` successful reads."""
    wanted = set(targets)
    paths: List[Tuple[Tuple[int, ...], int]] = []
    visited = {engine}
    frontier: List[Tuple[int, Tuple[int, ...]]] = [(engine, ())]
    objects_read = 0
    for depth in range(max_hops + 1):
        following: List[Tuple[int, Tuple[int, ...]]] = []
        for obj, route in frontier:
            if objects_read >= max_objects:
                return WalkResult(paths, objects_read, True)
            try:
                raw = proc.read(obj, OBJECT_READ_SIZE)
            except OSError:
                continue  # not a readable object
            objects_read += 1
            for offset, value in pointer_slots(raw):
                if value in wanted:
                    paths.append((route + (offset,), value))
                elif depth < max_hops and value not in visited:
                    visited.add(value)
                    following.append((value, route + (offset,)))
        frontier = following
    return WalkResult(paths, objects_read, False)


def format_path(offsets: Sequence[int], text: str) -> str:
    """`engine+0xAA -> +0xBB -> +0xCC = <text>`."""
    steps = [f"engine+{offsets[0]:#x}"] + [f"+{o:#x}" for o in offsets[1:]]
    return " -> ".join(steps) + f" = {text}"


def read_string_object(proc, addr: int) -> Optional[str]:
    """The text of the System.String at `addr`, or None when the bytes there
    are not one or cannot be read."""
    try:
        length = S.string_length(proc.read(addr, S.STRING_CHARS_OFFSET))
        if length is None:
            return None
        return S.decode_il2cpp_string(
            proc.read(addr, S.string_object_size(length)))
    except OSError:
        return None


# --- the SongEntry holder search ------------------------------------------------
#
# The Il2Cpp dump (Task I's il2cpp-fields.md, from dump.cs) names a field
# SongEntry.checksum holding 16 raw bytes, and a GlobalVariables instance
# whose currentSongEntry field points to the playing SongEntry. Both offsets
# are read off the dump only, not confirmed live.

SONG_ENTRY_CHECKSUM_OFFSET = 0x38       # SongEntry.checksum
GLOBALS_CURRENT_SONG_OFFSET = 0x28      # GlobalVariables.currentSongEntry


def holders_in_data(base: int, data: bytes,
                    targets: Iterable[int]) -> List[Tuple[int, int]]:
    """(holder address, target) for every 8-aligned slot in one region's
    bytes that holds a pointer equal to one of `targets`."""
    found: List[Tuple[int, int]] = []
    for target in targets:
        needle = struct.pack("<Q", target)
        pos = data.find(needle)
        while pos != -1:
            if (base + pos) % POINTER_SIZE == 0:
                found.append((base + pos, target))
            pos = data.find(needle, pos + 1)
    return found


def scan_for_holders(proc, targets: Iterable[int], *,
                     regions: Callable = live_rw_regions) -> List[Tuple[int, int]]:
    """One pass over the regions `regions(proc)` names, collecting every
    slot that points to one of `targets`, each holder once."""
    wanted = list(dict.fromkeys(targets))
    seen = set()
    found: List[Tuple[int, int]] = []
    for base, size in regions(proc):
        try:
            data = proc.read(base, size)
        except OSError:
            continue  # unreadable region
        for holder, target in holders_in_data(base, data, wanted):
            if holder not in seen:
                seen.add(holder)
                found.append((holder, target))
    found.sort()
    return found


# --- the song clock around each step --------------------------------------------

CLOCK_MOVING = "moving"
CLOCK_FROZEN = "frozen"
CLOCK_STOPPED = "stopped (the engine object could not be read)"


def clock_state(before: Optional[float], after: Optional[float]) -> str:
    """Whether the song clock moved across a step. None is a clock read that
    failed. Moving uses the same threshold the engine finder uses."""
    if before is None or after is None:
        return CLOCK_STOPPED
    if abs(after - before) > C.ENGINE_CLOCK_MOVED_S:
        return CLOCK_MOVING
    return CLOCK_FROZEN


def _read_clock(proc, engine: int) -> Optional[float]:
    try:
        return proc.read_double(engine + C.OFF_SONG_CLOCK)
    except OSError:
        return None


# --- the report ------------------------------------------------------------------

def _print_hits(hits: Sequence[Hit], out: TextIO) -> None:
    if not hits:
        out.write("  no hits\n")
    for hit in hits:
        out.write(f"  {hit.address:#x}  {hit.form.name:17}  "
                  f"before: {hit.before.hex(' ')}\n")


def run(
    argv: Optional[Sequence[str]] = None,
    *,
    open_proc: Callable = open_process,
    find_engine: Callable = engine_finder.find_live_engine_capped,
    regions: Callable = live_rw_regions,
    sleep: Callable[[float], None] = time.sleep,
    now: Callable[[], float] = time.perf_counter,
    out: Optional[TextIO] = None,
) -> int:
    """The whole finder. LIVE-ONLY by default: open_proc, find_engine and
    regions reach the running game, and the tests pass fakes for all three.
    Returns the exit code."""
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--hash", required=True,
                        help="the chart's MD5 checksum, 32 hex digits")
    args = parser.parse_args(argv)
    try:
        forms = S.encode_forms(args.hash)
    except ValueError as e:
        parser.error(str(e))

    out.write("Connecting to Clone Hero (read-only)...\n")
    proc = open_proc(read_only=True)
    try:
        proc.verify_targets()
        out.write("  Waiting for a playing song...\n")
        try:
            engine = find_engine(proc, [engine_finder.normal_pattern(proc)],
                                 out=out)
        except engine_finder.EngineSearchTimeout as e:
            out.write(f"\n{e}\n")
            return EXIT_NO_SONG
        out.write(f"\n  Engine at {engine:#x}\n")

        first = _timed_scan(proc, engine, forms, regions, now, out,
                            "First search")
        _pointer_report(proc, engine, first.hits, now, out)
        _song_entry_report(proc, engine, first.hits, regions, now, out)

        out.write(f"\nWaiting {RESCAN_GAP_S:g} s for the second search...\n")
        sleep(RESCAN_GAP_S)
        second = _timed_scan(proc, engine, forms, regions, now, out,
                             "Second search")
        held, moved, new = compare_passes(first.hits, second.hits)
        out.write(f"\nSame addresses {RESCAN_GAP_S:g} s later: "
                  f"{len(held)} held, {len(moved)} moved, {len(new)} new\n")
        for hit in held:
            out.write(f"  held   {hit.address:#x}  {hit.form.name}\n")
        for hit in moved:
            out.write(f"  moved  {hit.address:#x}  {hit.form.name}\n")
        for hit in new:
            out.write(f"  new    {hit.address:#x}  {hit.form.name}\n")
        return EXIT_OK
    finally:
        proc.close()


def _timed_scan(proc, engine: int, forms, regions, now, out,
                label: str) -> ScanResult:
    """One direct search, with its time and the clock's state across it."""
    out.write(f"\n{label}: {len(forms)} forms of the hash\n")
    clock_before = _read_clock(proc, engine)
    start = now()
    result = scan_for_hash(proc, forms, regions=regions)
    took = now() - start
    state = clock_state(clock_before, _read_clock(proc, engine))
    out.write(f"  {result.regions} regions, {result.bytes_scanned} bytes, "
              f"{len(result.hits)} hits, took {took * 1000:.0f} ms, "
              f"clock {state}\n")
    _print_hits(result.hits, out)
    return result


def _pointer_report(proc, engine: int, hits: Sequence[Hit], now,
                    out: TextIO) -> WalkResult:
    """Walk from the engine to the string object behind each UTF-16 hit and
    print every path found, or the not-found line."""
    targets: Dict[int, None] = {}
    for hit in hits:
        if hit.form.utf16:
            targets[hit.address - S.STRING_CHARS_OFFSET] = None
    out.write(f"\nPointer walk: {len(targets)} candidate string objects, "
              f"up to {MAX_HOPS} hops and {MAX_OBJECTS_READ} objects\n")
    clock_before = _read_clock(proc, engine)
    start = now()
    result = walk_pointers(proc, engine, targets)
    took = now() - start
    state = clock_state(clock_before, _read_clock(proc, engine))
    for offsets, target in result.paths:
        text = read_string_object(proc, target)
        shown = text if text is not None else f"<no string at {target:#x}>"
        out.write(f"  {format_path(offsets, shown)}\n")
    if not result.paths:
        out.write(f"  pointer path not found within {MAX_HOPS} hops\n")
    cap_note = ", stopped at the object cap" if result.hit_cap else ""
    out.write(f"  read {result.objects_read} objects{cap_note}, "
              f"took {took * 1000:.0f} ms, clock {state}\n")
    return result


def _song_entry_report(proc, engine: int, hits: Sequence[Hit], regions, now,
                       out: TextIO) -> None:
    """For each raw-bytes hit, take the SongEntry it would sit in, search
    memory once for pointers to those objects, and print each holder with the
    GlobalVariables instance it would belong to."""
    entries: Dict[int, int] = {}   # raw hit address -> candidate SongEntry
    for hit in hits:
        if hit.form.name == S.FORM_RAW:
            entries[hit.address] = hit.address - SONG_ENTRY_CHECKSUM_OFFSET
    out.write(f"\nSongEntry holders: {len(entries)} candidate SongEntry "
              "objects from the raw-bytes hits\n")
    if not entries:
        out.write("  no raw-bytes hits, so no holder search\n")
        return
    clock_before = _read_clock(proc, engine)
    start = now()
    holders = scan_for_holders(proc, entries.values(), regions=regions)
    took = now() - start
    state = clock_state(clock_before, _read_clock(proc, engine))
    for raw_hit, entry in entries.items():
        mine = [h for h, target in holders if target == entry]
        for holder in mine:
            out.write(f"  rawhit {raw_hit:#x} -> SongEntry {entry:#x} <- held "
                      f"at {holder:#x} (candidate GlobalVariables = "
                      f"{holder - GLOBALS_CURRENT_SONG_OFFSET:#x})\n")
        if not mine:
            out.write(f"  rawhit {raw_hit:#x} -> SongEntry {entry:#x} <- "
                      "no pointer to it found\n")
    out.write(f"  {len(holders)} holders, took {took * 1000:.0f} ms, "
              f"clock {state}\n")


def main() -> None:
    sys.exit(run())


if __name__ == "__main__":
    main()
