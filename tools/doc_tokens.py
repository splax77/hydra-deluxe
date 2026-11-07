"""Find the names a markdown document mentions, and sort each by its shape.

This is the one tokeniser for the scripts that check documents against the repo:
check_handoff.py uses it, and the pointer checker imports it too. It reads text
only and never calls git; whether a name exists is the caller's question.

The interface:
  tokens(text)        -> list of Token(line, text, backticked), in document order
  shape(token_text)   -> "commit", "branch", "path", "function" or None
  is_commit, is_branch_shape, is_path, is_function: the four shape tests shape() uses
  strip_line_ref(t)   -> the path part of a token such as "src/a.cpp:142"
  function_name(t)    -> the one identifier to search for in a function token
  has_separator(t), is_absolute(t), absolute_paths(line)
  KNOWN_EXTENSIONS    -> the file extensions that make a bare name a path

A branch is known by shape here only by its prefix (BRANCH_PREFIXES); a caller
that can ask git should also treat any name git knows as a branch, before the
path test, because a branch name can hold a "/".
"""

import os
import re
from collections import namedtuple

Token = namedtuple("Token", "line text backticked")

# Extensions of files people write and commit. Build outputs and runtime files
# (.exe, .db, .ini) are left out on purpose: a bare "hydra.db" in a handoff names
# a file on someone's disk, not one the repo can show.
KNOWN_EXTENSIONS = (
    ".md", ".py", ".ps1", ".psm1", ".sh", ".bat", ".cmd",
    ".cpp", ".cc", ".c", ".h", ".hpp", ".inl",
    ".json", ".jsonl", ".txt", ".yml", ".yaml", ".toml", ".cmake", ".iss",
    ".js", ".ts", ".html", ".css", ".xml", ".csv", ".svg", ".png",
)

BRANCH_PREFIXES = ("claude/", "hx-")

_HEX = re.compile(r"^[0-9a-fA-F]{7,40}$")
_IDENT = r"[A-Za-z_][A-Za-z0-9_]*"
_CALL = re.compile(r"^(?:%s::)*(%s)\(.*\)$" % (_IDENT, _IDENT))
_QUALIFIED = re.compile(r"^(?:%s::)+(%s)$" % (_IDENT, _IDENT))
_LINE_REF = re.compile(r"(?::\d+(?:-\d+)?|#L\d+(?:-L?\d+)?)$")
_DRIVE = re.compile(r"^[A-Za-z]:[\\/]")
# Characters that mark a placeholder or a pattern, never a real name.
_PLACEHOLDER = set("<>*?{}$%|")

_SPAN = re.compile(r"(`+)(.+?)\1")
# Bare hex: bounded so a UUID piece ("6777b546-e751") or a "0x" number is not taken.
_BARE_HEX = re.compile(r"(?<![\w\-])[0-9a-f]{7,40}(?![\w\-])")
# Bare paths and absolute paths: never taken from the middle of a word or a URL.
_BARE_PATH = re.compile(r"(?<![\w`:/\\])(?:[A-Za-z]:)?[\w.~\-]*[\\/][\w.\-\\/]*\.[A-Za-z][A-Za-z0-9]*")
_ABS_PATH = re.compile(r"(?<![\w/])[A-Za-z]:[\\/][^\s`'\"()<>|*?,;]*")
# A hex word right after these words is an id, not a commit ("session e3dd3907").
_ID_WORD = re.compile(r"\b(?:session|agent)(?:\s+id)?\s*:?\s*`?$", re.IGNORECASE)
_TRAIL = ".,;:)]}'\""


def strip_line_ref(t):
    """"src/a.cpp:142" -> "src/a.cpp"; a drive letter's colon is kept."""
    return _LINE_REF.sub("", t)


def has_separator(t):
    return "/" in t or "\\" in t


def is_absolute(t):
    return bool(_DRIVE.match(t)) or t.startswith("~")


def is_commit(t):
    return bool(_HEX.match(t))


def is_branch_shape(t):
    return t.startswith(BRANCH_PREFIXES) and " " not in t and not (set(t) & _PLACEHOLDER)


def is_path(t):
    p = strip_line_ref(t)
    if not p or (set(p) & _PLACEHOLDER) or "://" in p:
        return False
    if " " in p and not is_absolute(p):
        return False  # a command or a sentence, not a name
    if has_separator(p):
        return True
    return os.path.splitext(p)[1].lower() in KNOWN_EXTENSIONS


def is_function(t):
    return bool(_CALL.match(t) or _QUALIFIED.match(t))


def function_name(t):
    """The last identifier of a function token: "app::Clock::now()" -> "now"."""
    m = _CALL.match(t) or _QUALIFIED.match(t)
    return m.group(1) if m else None


def shape(t):
    """The shape of one token; None for prose. Functions are tested before paths so that a
    call holding a path argument stays a call."""
    if is_commit(t):
        return "commit"
    if is_branch_shape(t):
        return "branch"
    if is_function(t):
        return "function"
    if is_path(t):
        return "path"
    return None


def absolute_paths(line):
    """Every absolute Windows path written on one line, bare or in backticks."""
    return [m.group(0).rstrip(_TRAIL) for m in _ABS_PATH.finditer(line)]


def _is_id(before):
    return bool(_ID_WORD.search(before))


def tokens(text):
    """Every backticked span, plus every bare hex word and bare path, with its 1-based line.

    Inside a fenced code block nothing is in backticks, so only bare words are read
    there. A hex word written as a session or agent id is left out. A bare hex word
    with no digit, or with no letter, is left out too: English words ("defaced") and
    plain numbers take that shape, so a bare hash that happens to be all digits is
    missed. In backticks every hex word counts.
    """
    out = []
    fenced = False
    for n, line in enumerate(text.splitlines(), 1):
        if line.lstrip().startswith(("```", "~~~")):
            fenced = not fenced
            continue
        rest = line
        if not fenced:
            for m in _SPAN.finditer(line):
                t = m.group(2).strip()
                if t and not (is_commit(t) and _is_id(line[:m.start()])):
                    out.append(Token(n, t, True))
            rest = _SPAN.sub(lambda m: " " * len(m.group(0)), line)
        for m in _BARE_HEX.finditer(rest):
            t = m.group(0)
            if re.search(r"\d", t) and re.search(r"[a-f]", t) and not _is_id(rest[:m.start()]):
                out.append(Token(n, t, False))
        for m in _BARE_PATH.finditer(rest):
            t = m.group(0).rstrip(_TRAIL)
            if is_path(t):
                out.append(Token(n, t, False))
    return out
