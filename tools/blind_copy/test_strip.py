"""Tests for tools/blind_copy/make_copy.py, the comment strip script.

Run with: py -I tools/blind_copy/test_strip.py
"""

import ast
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
SCRIPT = os.path.join(HERE, "make_copy.py")
sys.path.insert(0, HERE)

import make_copy as mc  # noqa: E402


def strip(text, lang):
    return mc.strip_text(text, lang)


class CTraps(unittest.TestCase):
    def test_comment_markers_inside_strings(self):
        self.assertEqual(strip('s = "//"; t = "/*"; // c\n', "c"), 's = "//"; t = "/*";\n')

    def test_raw_string_holding_slashes_and_close_quote(self):
        src = 'auto r = R"x(a // b )" c /* d)x"; // gone\n'
        self.assertEqual(strip(src, "c"), 'auto r = R"x(a // b )" c /* d)x";\n')

    def test_prefixed_raw_string(self):
        self.assertEqual(strip('auto r = u8R"(//)"; // gone\n', "c"), 'auto r = u8R"(//)";\n')

    def test_quote_char_literals(self):
        src = "char a = '\"'; char b = '\\''; // it's \"\n"
        self.assertEqual(strip(src, "c"), "char a = '\"'; char b = '\\'';\n")

    def test_digit_separator(self):
        self.assertEqual(strip("int n = 1'000; // it's\nint m = 0x1'F;\n", "c"),
                         "int n = 1'000;\nint m = 0x1'F;\n")

    def test_backslash_continued_line_comment(self):
        self.assertEqual(strip("int a; // one \\\nstill comment\nint b;\n", "c"),
                         "int a;\n\nint b;\n")

    def test_backslash_continued_line_comment_crlf(self):
        self.assertEqual(strip("int a; // one \\\r\nstill\r\nint b;\r\n", "c"),
                         "int a;\r\n\r\nint b;\r\n")

    def test_empty_block_comment_becomes_a_space(self):
        self.assertEqual(strip("a/**/b\n", "c"), "a b\n")

    def test_block_comment_keeps_its_line_breaks(self):
        self.assertEqual(strip("x /* one\ntwo\nthree */ y\n", "c"), "x \n\n y\n")

    def test_unterminated_block_comment_stops(self):
        with self.assertRaises(mc.StripError):
            strip("x /* never closed\n", "c")


class JsTraps(unittest.TestCase):
    def test_template_with_expression_holding_a_comment(self):
        src = "const t = `a // ${ {b: 1}.b /* in */ } c /* d */`; // gone\n"
        self.assertEqual(strip(src, "js"), "const t = `a // ${ {b: 1}.b   } c /* d */`;\n")

    def test_regex_literal_holding_slashes(self):
        src = "const r = /\\/\\/[/*]/g; // gone\n"
        self.assertEqual(strip(src, "js"), "const r = /\\/\\/[/*]/g;\n")

    def test_regex_after_return(self):
        self.assertEqual(strip("return /a\\/\\/b/.test(s); // gone\n", "js"),
                         "return /a\\/\\/b/.test(s);\n")

    def test_division_is_not_a_regex(self):
        self.assertEqual(strip("const q = x / 2 / y; // gone\n", "js"), "const q = x / 2 / y;\n")

    def test_single_quoted_js_string(self):
        self.assertEqual(strip("const s = 'it//s'; // gone\n", "js"), "const s = 'it//s';\n")


class PythonTraps(unittest.TestCase):
    def test_hash_in_string_and_fstring(self):
        src = 'x = "#a"  # c\ny = f"{x}#b"  # d\n'
        self.assertEqual(strip(src, "py"), 'x = "#a"\ny = f"{x}#b"\n')

    def test_docstrings_on_module_class_and_function(self):
        src = ('"""Mod."""\n'
               "class A:\n"
               '    """Cls."""\n'
               "    x = 1\n"
               "    def f(self):\n"
               '        """Fn.\n'
               "\n"
               '        more"""\n'
               "        return 1\n")
        want = ("\n"
                "class A:\n"
                "\n"
                "    x = 1\n"
                "    def f(self):\n"
                "\n"
                "\n"
                "\n"
                "        return 1\n")
        self.assertEqual(strip(src, "py"), want)

    def test_function_holding_only_a_docstring_gets_pass(self):
        src = 'def g():\n    """Only.\n\n    two"""\n'
        self.assertEqual(strip(src, "py"), "def g():\n    pass\n\n\n")

    def test_one_line_docstring_body_gets_pass(self):
        self.assertEqual(strip('def g(): """Only."""\n', "py"), "def g(): pass\n")

    def test_string_that_is_not_a_docstring_is_kept(self):
        src = 'def h():\n    x = 1\n    "not a docstring"\n    s = """kept"""\n'
        self.assertEqual(strip(src, "py"), src)


class CMakeTraps(unittest.TestCase):
    def test_hash_in_quoted_argument(self):
        self.assertEqual(strip('set(A "x#y") # c\n', "cmake"), 'set(A "x#y")\n')

    def test_bracket_comment(self):
        self.assertEqual(strip("#[[ a\nb ]]set(B 1)\n", "cmake"), "\nset(B 1)\n")

    def test_bracket_argument_and_escaped_hash(self):
        src = "set(C [=[ # kept ]=] a\\#b) # gone\n"
        self.assertEqual(strip(src, "cmake"), "set(C [=[ # kept ]=] a\\#b)\n")


class PowerShellTraps(unittest.TestCase):
    def test_hash_inside_here_string(self):
        src = "$h = @'\n# not a comment\n'@\n# gone\n$x = 1 # gone\n"
        self.assertEqual(strip(src, "ps1"), "$h = @'\n# not a comment\n'@\n\n$x = 1\n")

    def test_hash_inside_quoted_strings(self):
        src = "$a = 'x#y' # gone\n$b = \"p#q`\"#r\" # gone\n"
        self.assertEqual(strip(src, "ps1"), "$a = 'x#y'\n$b = \"p#q`\"#r\"\n")

    def test_block_comment(self):
        self.assertEqual(strip("<# help\nmore #>\nGet-Item x <# y #> z\n", "ps1"),
                         "\n\nGet-Item x   z\n")

    def test_hash_inside_a_bare_word_is_kept(self):
        self.assertEqual(strip("Write-Output a#b\n", "ps1"), "Write-Output a#b\n")

    def test_requires_line_is_kept(self):
        self.assertEqual(strip("#Requires -Version 7\n# gone\n", "ps1"), "#Requires -Version 7\n\n")


class EachFileType(unittest.TestCase):
    """One case per extension the script strips, through its own classification."""

    SAMPLES = {
        "src/a.cpp": ('int x = 1; // c\n', 'int x = 1;\n'),
        "src/a.h": ('#pragma once /* c */\n', '#pragma once  \n'),
        "src/a.hpp": ('// only\nint y;\n', '\nint y;\n'),
        "src/a.c": ('char *p = "/*"; /* c */\n', 'char *p = "/*";  \n'),
        "src/a.rc": ('IDI_ICON ICON "a//b.ico" // c\n', 'IDI_ICON ICON "a//b.ico"\n'),
        "assets/s.hlsl": ('float4 c; /* c */ // d\n', 'float4 c;\n'),
        "tools/a.ts": ("let s = `//`; // c\n", "let s = `//`;\n"),
        "tools/a.mjs": ("let r = /\\/\\//; // c\n", "let r = /\\/\\//;\n"),
        "tools/a.py": ("x = '#'  # c\n", "x = '#'\n"),
        "tools/a.ps1": ("$x = '#' # c\n", "$x = '#'\n"),
        "tools/a.cmake": ('set(X "#") # c\n', 'set(X "#")\n'),
        "tests/CMakeLists.txt": ('add(T "#") # c\n', 'add(T "#")\n'),
    }

    def test_each_code_type_strips(self):
        for path, (src, want) in self.SAMPLES.items():
            with self.subTest(path=path):
                state, lang = mc.classify(path)
                self.assertEqual(state, mc.STRIPPED)
                self.assertEqual(mc.strip_bytes(src.encode(), lang), want.encode())

    def test_byte_order_mark_is_kept(self):
        self.assertEqual(mc.strip_bytes(b"\xef\xbb\xbfx = 1  # c\n", "py"), b"\xef\xbb\xbfx = 1\n")


class Classification(unittest.TestCase):
    def test_states(self):
        self.assertEqual(mc.classify("src/x.md"), (mc.LEFT_OUT, ".md"))
        self.assertEqual(mc.classify("tools/page.html"), (mc.LEFT_OUT, ".html"))
        self.assertEqual(mc.classify("tests/shards.txt"), (mc.COPIED, "data"))
        self.assertEqual(mc.classify("testdata/fuzz/mid/.gitkeep"), (mc.COPIED, "data"))
        self.assertEqual(mc.classify("CMakeLists.txt"), (mc.STRIPPED, "cmake"))
        self.assertEqual(mc.classify("docs/a.cpp"), (mc.LEFT_OUT, "outside the copied folders"))
        self.assertEqual(mc.classify("third_party/x/y.cpp"),
                         (mc.LEFT_OUT, "outside the copied folders"))
        self.assertEqual(mc.classify("src/app/x.manifest"), (mc.UNKNOWN, ".manifest"))
        self.assertEqual(mc.classify("tools/noext"), (mc.UNKNOWN, "(no extension)"))


class Leaks(unittest.TestCase):
    def test_marker_in_a_string_names_the_string(self):
        self.assertEqual(mc.leaks('a = "//";\nb = "/*";\n', "c"),
                         [(1, "//", "string"), (2, "/*", "string")])

    def test_python_marker_in_a_string(self):
        self.assertEqual(mc.leaks('x = "#"\n', "py"), [(1, "#", "string")])

    def test_unstripped_comment_is_reported_as_comment(self):
        self.assertEqual(mc.leaks("x = 1 # c\n", "cmake"), [(1, "#", "comment")])


def _clear_read_only(func, path, _exc):
    """git marks its object files read-only, which stops a delete on Windows."""
    os.chmod(path, stat.S_IWRITE)
    func(path)


def git(repo, *args):
    env = {k: v for k, v in os.environ.items() if not k.startswith("GIT_")}
    r = subprocess.run(["git", "-C", repo, "-c", "user.name=t", "-c", "user.email=t@t", *args],
                       capture_output=True, text=True, env=env)
    if r.returncode != 0:
        raise RuntimeError("git %s failed: %s" % (" ".join(args), r.stderr))
    return r.stdout.strip()


def run(*args):
    return subprocess.run([sys.executable, "-I", SCRIPT, *args], capture_output=True, text=True,
                          encoding="utf-8", errors="replace")


class WholeRuns(unittest.TestCase):
    FILES = {
        "src/a.cpp": b"int a; // gone\n",
        "tests/t.py": b'"""Doc."""\nx = 1  # gone\n',
        "testdata/in.txt": b"# a data file keeps this\r\n",
        "tools/notes.md": b"# left out\n",
        "docs/page.cpp": b"// outside\n",
        "CMakeLists.txt": b"project(X) # gone\n",
    }

    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.repo = os.path.join(self.tmp, "repo")
        os.makedirs(self.repo)
        git(self.repo, "init", "-q")
        git(self.repo, "config", "core.autocrlf", "false")
        for p, body in self.FILES.items():
            path = os.path.join(self.repo, *p.split("/"))
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "wb") as f:
                f.write(body)
        git(self.repo, "add", "-A")
        git(self.repo, "commit", "-q", "-m", "c")

    def tearDown(self):
        shutil.rmtree(self.tmp, onexc=_clear_read_only)

    def read(self, *parts):
        with open(os.path.join(*parts), "rb") as f:
            return f.read()

    def test_copy(self):
        dest = os.path.join(self.tmp, "dest")
        r = run("--repo", self.repo, "HEAD", dest)
        self.assertEqual(r.returncode, 0, r.stderr)
        code = os.path.join(dest, "code")
        self.assertEqual(self.read(code, "src", "a.cpp"), b"int a;\n")
        self.assertEqual(self.read(code, "tests", "t.py"), b"\nx = 1\n")
        self.assertEqual(self.read(code, "testdata", "in.txt"), self.FILES["testdata/in.txt"])
        self.assertEqual(self.read(code, "CMakeLists.txt"), b"project(X)\n")
        self.assertFalse(os.path.exists(os.path.join(code, "tools", "notes.md")))
        self.assertFalse(os.path.exists(os.path.join(code, "docs")))
        self.assertEqual(os.listdir(os.path.join(dest, "out")), [])
        self.assertFalse(os.path.exists(os.path.join(dest, "BLIND-PREAMBLE.md")))
        rows = self.read(dest, "manifest.tsv").decode().splitlines()
        self.assertEqual(rows[0], "path\tstate\tdetail\tlines")
        self.assertEqual(sorted(rows[1:]), sorted([
            "CMakeLists.txt\tstripped\tcmake\t2",
            "docs/page.cpp\tleft out\toutside the copied folders\t",
            "src/a.cpp\tstripped\tc\t2",
            "testdata/in.txt\tcopied\tdata\t",
            "tests/t.py\tstripped\tpy\t3",
            "tools/notes.md\tleft out\t.md\t",
        ]))

    def test_preamble_is_copied_only_when_given(self):
        pre = os.path.join(self.tmp, "pre.md")
        with open(pre, "wb") as f:
            f.write(b"rules\n")
        dest = os.path.join(self.tmp, "dest")
        r = run("--repo", self.repo, "HEAD", dest, "--preamble", pre)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self.read(dest, "BLIND-PREAMBLE.md"), b"rules\n")

    def test_unknown_extension_stops_and_names_the_file(self):
        with open(os.path.join(self.repo, "src", "x.weird"), "wb") as f:
            f.write(b"?")
        git(self.repo, "add", "-A")
        git(self.repo, "commit", "-q", "-m", "d")
        dest = os.path.join(self.tmp, "dest")
        r = run("--repo", self.repo, "HEAD", dest)
        self.assertEqual(r.returncode, 2)
        self.assertIn("src/x.weird", r.stderr)
        self.assertFalse(os.path.exists(os.path.join(dest, "manifest.tsv")))

    def test_non_empty_destination_is_refused(self):
        dest = os.path.join(self.tmp, "dest")
        os.makedirs(dest)
        with open(os.path.join(dest, "old"), "wb") as f:
            f.write(b"x")
        self.assertNotEqual(run("--repo", self.repo, "HEAD", dest).returncode, 0)

    def test_leak_list_prints_the_line(self):
        with open(os.path.join(self.repo, "src", "a.cpp"), "wb") as f:
            f.write(b'auto s = "//"; // gone\n')
        git(self.repo, "commit", "-q", "-am", "e")
        dest = os.path.join(self.tmp, "dest")
        r = run("--repo", self.repo, "HEAD", dest, "--leaks")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn('src/a.cpp:1: // in string: auto s = "//";', r.stdout)
        self.assertIn("leaks: 1 marker(s), 0 outside a literal", r.stdout)

    def test_in_place_strips_only_code_in_the_copied_folders(self):
        r = run("--in-place", self.repo)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self.read(self.repo, "src", "a.cpp"), b"int a;\n")
        self.assertEqual(self.read(self.repo, "tests", "t.py"), b"\nx = 1\n")
        for p in ("testdata/in.txt", "tools/notes.md", "docs/page.cpp"):
            self.assertEqual(self.read(self.repo, *p.split("/")), self.FILES[p], p)


class WholeCommitProperties(unittest.TestCase):
    """Every code file of a real commit: line count kept, and Python's ast as expected.

    The commit is BLIND_COPY_COMMIT if set, else HEAD of this repo.
    """

    @classmethod
    def setUpClass(cls):
        commit = os.environ.get("BLIND_COPY_COMMIT", "HEAD")
        cls.files = mc.export(REPO, commit)

    def code_files(self):
        for p in sorted(self.files):
            state, lang = mc.classify(p)
            if state == mc.STRIPPED:
                yield p, lang

    def test_some_code_files_exist(self):
        self.assertTrue(any(lang == "py" for _, lang in self.code_files()))
        self.assertTrue(any(lang == "c" for _, lang in self.code_files()))

    def test_line_count_is_kept(self):
        for p, lang in self.code_files():
            with self.subTest(path=p):
                out = mc.strip_bytes(self.files[p], lang)
                self.assertEqual(out.count(b"\n"), self.files[p].count(b"\n"))

    def test_python_ast_equals_original_without_docstrings(self):
        for p, lang in self.code_files():
            if lang != "py":
                continue
            with self.subTest(path=p):
                src = self.files[p].decode("utf-8-sig")
                out = mc.strip_bytes(self.files[p], lang).decode("utf-8-sig")
                want = mc.remove_docstrings(ast.parse(src))
                self.assertEqual(ast.dump(ast.parse(out)), ast.dump(want))


if __name__ == "__main__":
    unittest.main()
