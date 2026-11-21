"""Make a comment-free copy of one commit for the blind auditors.

    py -I tools/blind_copy/make_copy.py <commit> <dest> [--preamble <path>] [--leaks]
    py -I tools/blind_copy/make_copy.py --in-place <folder> [--leaks]

The first form exports the commit's tracked files with `git archive` and writes
<dest>/code/, an empty <dest>/out/ and <dest>/manifest.tsv. The second strips the
code files of a checked-out worktree where they stand. The spec is task T2 of the
stand-in recheck plan (docs/superpowers/plans/2026-11-20-stand-in-recheck.md).

Stripping keeps every line break, so a line number in the copy is the line number
at the commit. --leaks prints every comment marker still left in a stripped file,
with the line it sits on and the kind of text the scanner found it in.

Exit codes: 0 done; 1 a leak sits outside a literal; 2 a file the lists do not
cover, a tracked symbolic link in a copied folder, or bad arguments; 3 a stripped
file changed its line count or would not scan.
"""

import argparse
import ast
import io
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tokenize

COPIED_DIRS = ("src", "tests", "tools", "testdata", "assets", "resource")
COPIED_ROOT_FILES = ("CMakeLists.txt",)

C_EXTS = {".cpp", ".h", ".hpp", ".c", ".rc", ".hlsl"}
JS_EXTS = {".ts", ".mjs"}
PY_EXTS = {".py"}
PS_EXTS = {".ps1"}
CMAKE_EXTS = {".cmake"}
CMAKE_NAMES = {"cmakelists.txt"}
XML_EXTS = {".xml", ".manifest"}
HASH_LINE_EXTS = {".dict", ".txt"}
JSON_EXTS = {".json"}
LEFT_OUT_EXTS = {".md", ".html", ".gitattributes", ".gitignore"}
DATA_EXTS = {
    ".jsonl", ".ini", ".csv", ".tsv", ".chart", ".mid", ".sng", ".srb",
    ".png", ".jpg", ".ico", ".ttf", ".obj", ".ogg", ".flac", ".wav", ".opus", ".mp3",
    ".db", ".bin", ".sig", ".log", ".reg", ".after-run", ".gitkeep",
    # The repo's .exe is a saved release installer (testdata/update/real) and
    # its .cs files are Clone Hero's C# (docs/clone-hero, testdata/ch_runs);
    # neither is Hydra code.
    ".exe", ".cs",
}

STRIPPED, COPIED, LEFT_OUT, UNKNOWN = "stripped", "copied", "left out", "unknown"


class StripError(Exception):
    pass


# ---------------------------------------------------------------- classification

def extension(name):
    low = name.lower()
    if low.startswith(".") and "." not in low[1:]:
        return low
    dot = low.rfind(".")
    return low[dot:] if dot > 0 else ""


def language(path):
    """The scanner a code file uses, or None for a file that is not code."""
    name = path.rsplit("/", 1)[-1]
    if name.lower() in CMAKE_NAMES:
        return "cmake"
    ext = extension(name)
    if ext in C_EXTS:
        return "c"
    if ext in JS_EXTS:
        return "js"
    if ext in PY_EXTS:
        return "py"
    if ext in PS_EXTS:
        return "ps1"
    if ext in CMAKE_EXTS:
        return "cmake"
    if ext in XML_EXTS:
        return "xml"
    if ext in HASH_LINE_EXTS:
        return "hashlines"
    if ext in JSON_EXTS:
        return "json"
    return None


def in_copied_folders(path):
    return path in COPIED_ROOT_FILES or path.split("/", 1)[0] in COPIED_DIRS


def classify(path):
    """(state, detail) for one repo-relative path with forward slashes."""
    if not in_copied_folders(path):
        return LEFT_OUT, "outside the copied folders"
    lang = language(path)
    if lang:
        return STRIPPED, lang
    ext = extension(path.rsplit("/", 1)[-1])
    if ext in LEFT_OUT_EXTS:
        return LEFT_OUT, ext
    if ext in DATA_EXTS:
        return COPIED, "data"
    return UNKNOWN, ext or "(no extension)"


# ---------------------------------------------------------------- scanners
#
# Each scanner splits a text into segments (kind, start, end, label). kind is
# "code", "comment" or "literal"; label names the comment form or the literal.

JS_REGEX_KEYWORDS = {"return", "typeof", "instanceof", "in", "of", "new", "delete", "void",
                     "throw", "case", "do", "else", "yield", "await"}
RAW_PREFIXES = {"R", "u8R", "uR", "UR", "LR"}


def _ident(ch):
    return ch.isalnum() or ch == "_" or ch == "$"


def _line_end(text, i):
    j = text.find("\n", i)
    return len(text) if j < 0 else j


def _quoted_end(text, i, quote, stop_at_newline):
    """End (exclusive) of a backslash-escaped quoted literal opened at i."""
    j = i + 1
    n = len(text)
    while j < n:
        ch = text[j]
        if ch == "\\":
            j += 2
            continue
        if ch == quote:
            return j + 1
        if ch == "\n" and stop_at_newline:
            return j
        j += 1
    return n


def _line_comment_end(text, i):
    """End of a // comment; a backslash just before the line break continues it."""
    j = i
    while True:
        e = _line_end(text, j)
        k = e - 1
        if k >= 0 and text[k] == "\r":
            k -= 1
        if e < len(text) and k >= i and text[k] == "\\":
            j = e + 1
            continue
        return e


def _regex_end(text, i):
    """End of a JS regex literal opened at i, or -1 if the line has no closing slash."""
    j = i + 1
    in_class = False
    n = len(text)
    while j < n:
        ch = text[j]
        if ch == "\n":
            return -1
        if ch == "\\":
            j += 2
            continue
        if in_class:
            if ch == "]":
                in_class = False
        elif ch == "[":
            in_class = True
        elif ch == "/":
            j += 1
            while j < n and _ident(text[j]):
                j += 1
            return j
        j += 1
    return -1


def _template_end(text, i):
    """From just after a backtick or a closing }, the end of the template text.

    Returns (end, opens_expr): opens_expr is True when the text stops at a ${.
    """
    j = i
    n = len(text)
    while j < n:
        ch = text[j]
        if ch == "\\":
            j += 2
            continue
        if ch == "`":
            return j + 1, False
        if ch == "$" and text.startswith("{", j + 1):
            return j + 2, True
        j += 1
    raise StripError("unterminated template literal")


def scan_c(text, js=False):
    segs = []
    n = len(text)
    i = 0
    start = 0
    word = ""          # identifier or number token just before i, if adjacent
    last = None        # "word", "punct" or "value": the last significant code token
    last_punct = ""
    templates = []     # brace depth at each open ${ of a template
    depth = 0
    word_before = ""   # the last word token, kept across spaces for the regex test

    def flush(upto):
        if upto > start:
            segs.append(("code", start, upto, ""))

    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if ch == "/" and nxt == "/":
            flush(i)
            e = _line_comment_end(text, i)
            segs.append(("comment", i, e, "line"))
            i = start = e
            word = ""
            continue
        if ch == "/" and nxt == "*":
            flush(i)
            e = text.find("*/", i + 2)
            if e < 0:
                raise StripError("unterminated /* comment")
            segs.append(("comment", i, e + 2, "block"))
            i = start = e + 2
            word = ""
            continue
        if js and ch == "/":
            regex_ok = (last is None or (last == "punct" and last_punct not in ")]}")
                        or (last == "word" and word_before in JS_REGEX_KEYWORDS))
            if regex_ok:
                e = _regex_end(text, i)
                if e > 0:
                    flush(i)
                    segs.append(("literal", i, e, "regex"))
                    i = start = e
                    last, word = "value", ""
                    continue
        if ch == '"':
            if not js and word in RAW_PREFIXES:
                flush(i - len(word))
                m = re.match(r'"([^()\\\s]{0,16})\(', text[i:i + 19])
                if not m:
                    raise StripError("raw string without a delimiter")
                close = ")" + m.group(1) + '"'
                e = text.find(close, i + m.end())
                if e < 0:
                    raise StripError("unterminated raw string")
                segs.append(("literal", i - len(word), e + len(close), "raw string"))
                i = start = e + len(close)
                last, word = "value", ""
                continue
            flush(i)
            e = _quoted_end(text, i, '"', stop_at_newline=True)
            segs.append(("literal", i, e, "string"))
            i = start = e
            last, word = "value", ""
            continue
        if ch == "'":
            if not js and word and word[0].isdigit() and i > 0 and _ident(text[i - 1]):
                word += ch
                i += 1
                continue
            flush(i)
            e = _quoted_end(text, i, "'", stop_at_newline=True)
            segs.append(("literal", i, e, "string" if js else "char"))
            i = start = e
            last, word = "value", ""
            continue
        if js and ch == "`":
            flush(i)
            e, opens = _template_end(text, i + 1)
            segs.append(("literal", i, e, "template"))
            if opens:
                templates.append(depth)
            i = start = e
            word = ""
            last, last_punct = ("punct", "{") if opens else ("value", last_punct)
            continue
        if js and ch == "}" and templates and depth == templates[-1]:
            templates.pop()
            flush(i)
            e, opens = _template_end(text, i + 1)
            segs.append(("literal", i, e, "template"))
            if opens:
                templates.append(depth)
            i = start = e
            word = ""
            last, last_punct = ("punct", "{") if opens else ("value", last_punct)
            continue
        if _ident(ch) or (ch == "." and word and word[0].isdigit()):
            word = word + ch if (i > 0 and (_ident(text[i - 1]) or text[i - 1] in ".'")
                                 and word) else ch
            last = "word"
            word_before = word
        else:
            if not ch.isspace():
                last, last_punct = "punct", ch
                if ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
            word = ""
        if last == "word":
            word_before = word
        i += 1
    flush(n)
    return segs


def scan_ps(text):
    segs = []
    n = len(text)
    i = 0
    start = 0

    def flush(upto):
        if upto > start:
            segs.append(("code", start, upto, ""))

    while i < n:
        ch = text[i]
        if ch == "`":
            i += 2
            continue
        if text.startswith("<#", i):
            flush(i)
            e = text.find("#>", i + 2)
            if e < 0:
                raise StripError("unterminated <# comment")
            segs.append(("comment", i, e + 2, "block"))
            i = start = e + 2
            continue
        if ch == "#" and (i == 0 or text[i - 1] in " \t\r\n;|&(){}=,"):
            e = _line_end(text, i)
            if e > i and text[e - 1] == "\r":
                e -= 1
            flush(i)
            segs.append(("comment", i, e, "line"))
            i = start = e
            continue
        if ch == "@" and i + 1 < n and text[i + 1] in "'\"":
            rest = text[i + 2:_line_end(text, i)]
            if rest.strip() == "":
                q = text[i + 1]
                m = re.compile(r"\n[ \t]*" + re.escape(q) + "@").search(text, i + 2)
                if not m:
                    raise StripError("unterminated here-string")
                flush(i)
                segs.append(("literal", i, m.end(), "here-string"))
                i = start = m.end()
                continue
        if ch == "'":
            flush(i)
            j = i + 1
            while j < n:
                if text[j] == "'":
                    if text.startswith("'", j + 1):
                        j += 2
                        continue
                    break
                j += 1
            e = min(j + 1, n)
            segs.append(("literal", i, e, "string"))
            i = start = e
            continue
        if ch == '"':
            flush(i)
            j = i + 1
            while j < n:
                if text[j] == "`":
                    j += 2
                    continue
                if text[j] == '"':
                    if text.startswith('"', j + 1):
                        j += 2
                        continue
                    break
                j += 1
            e = min(j + 1, n)
            segs.append(("literal", i, e, "string"))
            i = start = e
            continue
        i += 1
    flush(n)
    return segs


def scan_cmake(text):
    segs = []
    n = len(text)
    i = 0
    start = 0

    def flush(upto):
        if upto > start:
            segs.append(("code", start, upto, ""))

    while i < n:
        ch = text[i]
        if ch == "\\":
            i += 2
            continue
        if ch == "#":
            flush(i)
            m = re.match(r"#\[(=*)\[", text[i:])
            if m:
                close = "]" + m.group(1) + "]"
                e = text.find(close, i + m.end())
                if e < 0:
                    raise StripError("unterminated bracket comment")
                segs.append(("comment", i, e + len(close), "block"))
                i = start = e + len(close)
                continue
            e = _line_end(text, i)
            if e > i and text[e - 1] == "\r":
                e -= 1
            segs.append(("comment", i, e, "line"))
            i = start = e
            continue
        if ch == '"':
            flush(i)
            e = _quoted_end(text, i, '"', stop_at_newline=False)
            segs.append(("literal", i, e, "quoted argument"))
            i = start = e
            continue
        if ch == "[" and (i == 0 or text[i - 1] in " \t\r\n("):
            m = re.match(r"\[(=*)\[", text[i:])
            if m:
                close = "]" + m.group(1) + "]"
                e = text.find(close, i + m.end())
                if e < 0:
                    raise StripError("unterminated bracket argument")
                flush(i)
                segs.append(("literal", i, e + len(close), "bracket argument"))
                i = start = e + len(close)
                continue
        i += 1
    flush(n)
    return segs


XML_OPENERS = re.compile(r"<!--|<!\[CDATA\[|<\?")
XML_CLOSERS = {"<!--": ("-->", "comment", "removed"), "<![CDATA[": ("]]>", "literal", "CDATA"),
               "<?": ("?>", "literal", "processing instruction")}


def scan_xml(text):
    segs = []
    start = 0
    pos = 0
    while True:
        m = XML_OPENERS.search(text, pos)
        if not m:
            break
        close, kind, label = XML_CLOSERS[m.group(0)]
        e = text.find(close, m.end())
        if e < 0:
            raise StripError("unterminated %s" % m.group(0))
        if m.start() > start:
            segs.append(("code", start, m.start(), ""))
        segs.append((kind, m.start(), e + len(close), label))
        start = pos = e + len(close)
    if len(text) > start:
        segs.append(("code", start, len(text), ""))
    return segs


SCANNERS = {"c": scan_c, "js": lambda t: scan_c(t, js=True), "ps1": scan_ps, "cmake": scan_cmake,
            "xml": scan_xml}

HASH_LINE = re.compile(r"^[ \t]*#[^\r\n]*", re.M)
JSON_COMMENT_KEY = re.compile(r'("_comment"\s*:\s*)"(?:[^"\\\r\n]|\\.)*"')


def _newlines(s):
    return "".join(ch for ch in s if ch in "\r\n")


def strip_segments(text, segs):
    out = []
    for kind, a, b, label in segs:
        if kind != "comment":
            out.append(text[a:b])
            continue
        keep = _newlines(text[a:b])
        if label == "removed":
            out.append(keep)
        elif label == "line":
            while out:
                tail = out[-1].rstrip(" \t")
                if tail:
                    out[-1] = tail
                    break
                out.pop()
            out.append(keep)
        else:
            out.append(keep or " ")
    return "".join(out)


# ---------------------------------------------------------------- Python

def _line_offsets(text):
    offs = [0]
    for line in text.split("\n")[:-1]:
        offs.append(offs[-1] + len(line) + 1)
    return offs


def _docstring_owners(tree):
    kinds = (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)
    for node in ast.walk(tree):
        if isinstance(node, kinds) and node.body:
            first = node.body[0]
            if (isinstance(first, ast.Expr) and isinstance(first.value, ast.Constant)
                    and isinstance(first.value.value, str)):
                yield node, first


def remove_docstrings(tree):
    """The tree with each docstring removed and an emptied body given a pass."""
    for node, first in list(_docstring_owners(tree)):
        node.body.pop(0)
        if not node.body:
            node.body.append(ast.Pass())
    return tree


def strip_python(text):
    offs = _line_offsets(text)
    lines = text.split("\n")

    def char_off(row, col):
        return offs[row - 1] + col

    def byte_off(row, col):
        return offs[row - 1] + len(lines[row - 1].encode("utf-8", "surrogatepass")[:col]
                                   .decode("utf-8", "surrogatepass"))

    edits = []
    try:
        toks = list(tokenize.generate_tokens(io.StringIO(text).readline))
        tree = ast.parse(text)
    except (SyntaxError, tokenize.TokenError, ValueError) as e:
        raise StripError("Python does not parse: %s" % e)
    for tok in toks:
        if tok.type == tokenize.COMMENT:
            a = char_off(*tok.start)
            b = char_off(*tok.end)
            while a > offs[tok.start[0] - 1] and text[a - 1] in " \t":
                a -= 1
            edits.append((a, b, ""))
    for node, first in _docstring_owners(tree):
        a = byte_off(first.lineno, first.col_offset)
        b = byte_off(first.end_lineno, first.end_col_offset)
        alone = len(node.body) == 1
        line_start = offs[first.lineno - 1]
        if not alone and text[line_start:a].strip() == "":
            a = line_start
        edits.append((a, b, ("pass" if alone else "") + _newlines(text[a:b])))
    edits.sort()
    out = []
    pos = 0
    for a, b, repl in edits:
        if a < pos:
            raise StripError("overlapping Python edits at offset %d" % a)
        out.append(text[pos:a])
        out.append(repl)
        pos = b
    out.append(text[pos:])
    return "".join(out)


def python_leaks(text):
    """(line, marker, context) for every # left in a Python text."""
    found = []
    for tok in tokenize.generate_tokens(io.StringIO(text).readline):
        if tok.type == tokenize.COMMENT:
            found.append((tok.start[0], "#", "comment"))
        elif "#" in tok.string:
            ctx = "string" if tok.type in (tokenize.STRING, getattr(tokenize, "FSTRING_MIDDLE", -1),
                                           getattr(tokenize, "TSTRING_MIDDLE", -1)) else "code"
            for k, ch in enumerate(tok.string):
                if ch == "#":
                    found.append((tok.start[0] + tok.string.count("\n", 0, k), "#", ctx))
    return found


# ---------------------------------------------------------------- one file

MARKERS = {"c": ("//", "/*"), "js": ("//", "/*"), "ps1": ("#",), "cmake": ("#",),
           "xml": ("<!--",)}


def strip_text(text, lang):
    if lang == "py":
        return strip_python(text)
    if lang == "hashlines":
        return HASH_LINE.sub("", text)
    if lang == "json":
        return JSON_COMMENT_KEY.sub(r'\1""', text)
    return strip_segments(text, SCANNERS[lang](text))


def leaks(text, lang):
    """(line, marker, context) for each comment marker in a stripped text.

    context is the literal the marker sits in, "comment" if the scanner still sees a
    comment there, or "code" for a marker outside both.
    """
    if lang == "py":
        return python_leaks(text)
    if lang == "hashlines":
        return [(text.count("\n", 0, m.start()) + 1, "#", "comment") for m in HASH_LINE.finditer(text)]
    if lang == "json":
        return [(text.count("\n", 0, m.start()) + 1, "_comment", "comment")
                for m in JSON_COMMENT_KEY.finditer(text)
                if m.group(0)[len(m.group(1)):] != '""']
    found = []
    for kind, a, b, label in SCANNERS[lang](text):
        piece = text[a:b]
        for marker in MARKERS[lang]:
            k = piece.find(marker)
            while k >= 0:
                ctx = label if kind == "literal" else kind
                found.append((text.count("\n", 0, a + k) + 1, marker, ctx))
                k = piece.find(marker, k + len(marker))
    return sorted(found)


def decode(data):
    bom = data.startswith(b"\xef\xbb\xbf")
    return bom, data[3 if bom else 0:].decode("utf-8", "surrogateescape")


def encode(bom, text):
    return (b"\xef\xbb\xbf" if bom else b"") + text.encode("utf-8", "surrogateescape")


def strip_bytes(data, lang):
    """Stripped bytes; raises StripError if the line count would change."""
    bom, text = decode(data)
    out = strip_text(text, lang)
    if out.count("\n") != text.count("\n"):
        raise StripError("line count changed from %d to %d"
                         % (text.count("\n") + 1, out.count("\n") + 1))
    return encode(bom, out)


# ---------------------------------------------------------------- runs

def _long(path):
    path = os.path.abspath(path)
    if os.name == "nt" and not path.startswith("\\\\?\\"):
        return "\\\\?\\" + path
    return path


def _git(repo, *args):
    r = subprocess.run(["git", "-C", repo, *args], capture_output=True)
    if r.returncode != 0:
        raise SystemExit("git %s failed: %s" % (" ".join(args), r.stderr.decode(errors="replace")))
    return r.stdout


def stop_on_symlinks(entries):
    """entries: (git mode, path) pairs. A tracked symlink in a copied folder stops the run.

    The blind auditors follow no link on Read, but a Grep from the root could walk through
    one, so the copy must hold none.
    """
    links = sorted(p for mode, p in entries if mode == "120000" and in_copied_folders(p))
    for p in links:
        print("SYMLINK %s: a tracked symbolic link in a copied folder" % p, file=sys.stderr)
    if links:
        print("stopped: %d symbolic link(s); nothing was written" % len(links), file=sys.stderr)
        raise SystemExit(2)


def tree_entries(repo, commit):
    """(mode, path) for every file of the commit."""
    out = _git(repo, "ls-tree", "-r", "-z", commit).decode("utf-8", "surrogateescape")
    entries = []
    for rec in out.split("\0"):
        if rec:
            meta, path = rec.split("\t", 1)
            entries.append((meta.split(" ", 1)[0], path))
    return entries


def export(repo, commit):
    """{path: bytes} for every tracked file of the commit."""
    stop_on_symlinks(tree_entries(repo, commit))
    data = _git(repo, "archive", "--format=tar", commit)
    files = {}
    with tarfile.open(fileobj=io.BytesIO(data)) as tar:
        for m in tar.getmembers():
            if m.isdir():
                continue
            if not m.isfile():
                if in_copied_folders(m.name):
                    raise SystemExit("not a plain file in the commit: %s" % m.name)
                continue
            files[m.name] = tar.extractfile(m).read()
    return files


def stop_on_unknown(paths):
    unknown = [(p, classify(p)[1]) for p in sorted(paths) if classify(p)[0] == UNKNOWN]
    for p, ext in unknown:
        print("UNKNOWN %s: extension %s is on neither the code nor the data list" % (p, ext),
              file=sys.stderr)
    if unknown:
        print("stopped: %d file(s) the lists do not cover; nothing was written" % len(unknown),
              file=sys.stderr)
        raise SystemExit(2)


def strip_all(files):
    """{path: stripped bytes} for the code files; stops on any failure."""
    out = {}
    failed = []
    for p in sorted(files):
        state, lang = classify(p)
        if state != STRIPPED:
            continue
        try:
            out[p] = strip_bytes(files[p], lang)
        except StripError as e:
            failed.append("%s: %s" % (p, e))
    for line in failed:
        print("STRIP FAILED " + line, file=sys.stderr)
    if failed:
        print("stopped: %d file(s) did not strip cleanly; nothing was written" % len(failed),
              file=sys.stderr)
        raise SystemExit(3)
    return out


def print_leaks(stripped):
    total = 0
    outside = 0
    for p in sorted(stripped):
        lang = classify(p)[1]
        bom, text = decode(stripped[p])
        lines = text.split("\n")
        for line, marker, ctx in leaks(text, lang):
            total += 1
            if ctx in ("comment", "code"):
                outside += 1
            print("%s:%d: %s in %s: %s" % (p, line, marker, ctx, lines[line - 1].strip()))
    print("leaks: %d marker(s), %d outside a literal" % (total, outside))
    return outside


def make_copy(repo, commit, dest, preamble=None, show_leaks=False):
    if os.path.exists(dest) and os.listdir(dest):
        raise SystemExit("destination is not empty: %s" % dest)
    files = export(repo, commit)
    stop_on_unknown(files)
    stripped = strip_all(files)
    code = os.path.join(dest, "code")
    os.makedirs(_long(code), exist_ok=True)
    os.makedirs(_long(os.path.join(dest, "out")), exist_ok=True)
    rows = []
    for p in sorted(files):
        state, detail = classify(p)
        lines = ""
        if state == STRIPPED:
            body = stripped[p]
            lines = str(body.count(b"\n") + 1)
        elif state == COPIED:
            body = files[p]
        else:
            body = None
        if body is not None:
            target = _long(os.path.join(code, *p.split("/")))
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target, "wb") as f:
                f.write(body)
        rows.append("%s\t%s\t%s\t%s" % (p, state, detail, lines))
    with open(os.path.join(dest, "manifest.tsv"), "w", encoding="utf-8", newline="\n") as f:
        f.write("path\tstate\tdetail\tlines\n")
        f.write("\n".join(rows) + "\n")
    if preamble:
        shutil.copyfile(preamble, os.path.join(dest, "BLIND-PREAMBLE.md"))
    counts = {}
    for r in rows:
        s = r.split("\t")[1]
        counts[s] = counts.get(s, 0) + 1
    print("manifest: %d files; %s" % (len(rows), ", ".join(
        "%s %d" % (s, counts.get(s, 0)) for s in (STRIPPED, COPIED, LEFT_OUT))))
    return print_leaks(stripped) if show_leaks else 0


def in_place(folder, show_leaks=False):
    staged = [r for r in _git(folder, "ls-files", "-s", "-z").decode("utf-8").split("\0") if r]
    entries = [(r.split(" ", 1)[0], r.split("\t", 1)[1]) for r in staged]
    stop_on_symlinks(entries)
    tracked = sorted({p for _, p in entries})
    tracked = [p for p in tracked if os.path.isfile(_long(os.path.join(folder, *p.split("/"))))]
    stop_on_unknown(tracked)
    code = [p for p in tracked if classify(p)[0] == STRIPPED]
    files = {}
    for p in code:
        with open(_long(os.path.join(folder, *p.split("/"))), "rb") as f:
            files[p] = f.read()
    stripped = strip_all(files)
    changed = 0
    for p, body in stripped.items():
        if body != files[p]:
            with open(_long(os.path.join(folder, *p.split("/"))), "wb") as f:
                f.write(body)
            changed += 1
    print("in place: %d code file(s) read, %d rewritten, all line counts kept"
          % (len(code), changed))
    return print_leaks(stripped) if show_leaks else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("commit", nargs="?")
    ap.add_argument("dest", nargs="?")
    ap.add_argument("--in-place", metavar="FOLDER")
    ap.add_argument("--preamble", metavar="PATH")
    ap.add_argument("--leaks", action="store_true")
    ap.add_argument("--repo", default=os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__)))))
    a = ap.parse_args(argv)
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    if a.in_place:
        if a.commit or a.dest or a.preamble:
            ap.error("--in-place takes only a folder (and --leaks)")
        outside = in_place(a.in_place, a.leaks)
    else:
        if not (a.commit and a.dest):
            ap.error("give a commit and a destination, or --in-place <folder>")
        outside = make_copy(a.repo, a.commit, a.dest, a.preamble, a.leaks)
    return 1 if outside else 0


if __name__ == "__main__":
    sys.exit(main())
