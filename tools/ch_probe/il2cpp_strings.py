"""Pure helpers for finding a chart's checksum in the game's memory.

Plain version: the game may keep the playing chart's MD5 checksum as raw
bytes, as hex text in a .NET string, or as hex text in a plain byte string.
encode_forms turns one hash into every byte pattern a memory search should
look for. decode_il2cpp_string reads back a .NET string object the way the
game's Il2Cpp runtime lays it out, so a search hit can be checked and printed.

Nothing here touches the game; experiments/find_checksum.py does the reading.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import List, Optional

from tools.ch_probe.process import decode_u32

# An Il2Cpp System.String object: an object header, the character count as a
# signed 32-bit number at +0x10, then the characters as UTF-16LE from +0x14.
STRING_LENGTH_OFFSET = 0x10
STRING_CHARS_OFFSET = 0x14

# A string longer than this is taken as a misread, not a real string. A hash
# is 32 characters; this only has to reject garbage lengths.
MAX_STRING_LENGTH = 1024

# The names of the forms encode_forms returns, as the finder prints them.
FORM_RAW = "raw bytes"
FORM_UTF16_UPPER = "UTF-16 upper hex"
FORM_UTF16_LOWER = "UTF-16 lower hex"
FORM_ASCII_UPPER = "ASCII upper hex"
FORM_ASCII_LOWER = "ASCII lower hex"

_HASH_HEX = re.compile(r"[0-9a-fA-F]{32}")


@dataclass(frozen=True)
class HashForm:
    """One way the hash can sit in memory. `utf16` says whether a hit can be
    the characters of a System.String, which the pointer walk follows."""

    name: str
    pattern: bytes
    utf16: bool


def encode_forms(hash_hex: str) -> List[HashForm]:
    """Every byte pattern to search for one MD5 hash, given as 32 hex digits
    in either case: the 16 raw bytes, the upper- and lower-case hex as
    UTF-16LE (a .NET string's characters), and both as ASCII. Raises
    ValueError for anything that is not 32 hex digits."""
    text = hash_hex.strip()
    if not _HASH_HEX.fullmatch(text):
        raise ValueError(
            f"{hash_hex!r} is not an MD5 hash (32 hex digits)")
    upper, lower = text.upper(), text.lower()
    return [
        HashForm(FORM_RAW, bytes.fromhex(text), False),
        HashForm(FORM_UTF16_UPPER, upper.encode("utf-16-le"), True),
        HashForm(FORM_UTF16_LOWER, lower.encode("utf-16-le"), True),
        HashForm(FORM_ASCII_UPPER, upper.encode("ascii"), False),
        HashForm(FORM_ASCII_LOWER, lower.encode("ascii"), False),
    ]


def string_object_size(length: int) -> int:
    """How many bytes a System.String of `length` characters spans from its
    object start to its last character."""
    return STRING_CHARS_OFFSET + 2 * length


def string_length(raw: bytes) -> Optional[int]:
    """The character count of the System.String whose object starts at
    raw[0], or None when raw is too short for the header or the count is
    below zero or over MAX_STRING_LENGTH."""
    if len(raw) < STRING_CHARS_OFFSET:
        return None
    # Read unsigned: a count below zero reads as 2**31 or more, so the one
    # upper bound turns it away too.
    length = decode_u32(raw[STRING_LENGTH_OFFSET:STRING_CHARS_OFFSET])
    if length > MAX_STRING_LENGTH:
        return None
    return length


def decode_il2cpp_string(raw: bytes) -> Optional[str]:
    """The text of an Il2Cpp System.String whose object starts at raw[0].
    Returns None when the bytes cannot be one: a bad length (string_length),
    fewer character bytes than the length says, or characters that are not
    valid UTF-16."""
    length = string_length(raw)
    if length is None:
        return None
    end = string_object_size(length)
    if len(raw) < end:
        return None
    try:
        return raw[STRING_CHARS_OFFSET:end].decode("utf-16-le")
    except UnicodeDecodeError:
        return None
