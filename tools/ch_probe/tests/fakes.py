"""One fake game for the probe's tests: a real Process over fake memory.

A test says what the game's memory holds; process.Process does the address
maths (resolve) and the decoding, exactly as it does against the live game.
So no fake repeats either, and a wrong offset in the code under test reads an
address nobody mapped and fails loudly.

Imported two ways, like engine.py: as tools.ch_probe.tests.fakes from the repo
root, and as a top-level `fakes` when a test puts the ch_probe folder and this
folder on sys.path.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Sequence, Tuple, Union

try:
    from .. import process
except ImportError:  # pragma: no cover - top-level import, ch_probe on sys.path
    import process  # type: ignore[no-redef]


@dataclass
class FakeMemory:
    """What the fake process saw: every read as (address, size), in order,
    and every write by address."""

    reads: List[Tuple[int, int]] = field(default_factory=list)
    writes: Dict[int, bytes] = field(default_factory=dict)


def fake_process(
    memory: Optional[Dict[int, bytes]],
    module_base: int,
    *,
    doubles: Optional[Dict[int, float]] = None,
    dwords: Optional[Dict[int, int]] = None,
    consts: Optional[Dict[int, Union[float, bytes]]] = None,
    clocks: Optional[Dict[int, Sequence[float]]] = None,
) -> Tuple["process.Process", FakeMemory]:
    """Build a real Process whose reader answers from fake memory.

    `memory` maps a live address to raw bytes. `doubles` and `dwords` map a
    live address to a value the game stores there. `consts` maps an RVA to an
    .rdata value (a double, or 8 raw bytes kept as given); it is placed at the
    Process's own resolve() of that RVA. `clocks` maps a live address to a list
    of doubles served one per read, the last one repeating, the way a song
    clock moves between two reads.

    A read of an address nobody mapped raises OSError, as the live reader
    does. Returns the Process and the FakeMemory that records its traffic.
    """
    seen = FakeMemory()
    cells: Dict[int, bytes] = dict(memory or {})
    for addr, value in (doubles or {}).items():
        cells[addr] = struct.pack("<d", value)
    for addr, value in (dwords or {}).items():
        cells[addr] = struct.pack("<I", value)
    ticks = {addr: list(values) for addr, values in (clocks or {}).items()}

    def reader(addr: int, size: int) -> bytes:
        seen.reads.append((addr, size))
        seq = ticks.get(addr)
        if seq is not None:
            value = seq.pop(0) if len(seq) > 1 else seq[0]
            return struct.pack("<d", value)[:size]
        raw = cells.get(addr)
        if raw is None:
            raise OSError(f"nothing mapped at {addr:#x} in the fake memory")
        return raw[:size]

    def writer(addr: int, data: bytes) -> None:
        seen.writes[addr] = data

    proc = process.Process(module_base, reader, writer)
    for rva, value in (consts or {}).items():
        cells[proc.resolve(rva)] = (
            value if isinstance(value, bytes) else struct.pack("<d", value))
    return proc, seen
