"""Tests for dump_facts (the dump.cs reader) and dump_build (the dumper runner).

No real dump: the reader runs on the hand-written dump.cs fragment below.
The runner tests start the real Il2CppDumper (dump_build.DEFAULT_DUMPER).
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_facts as BF  # noqa: E402
from tools.ch_mcp import dump_build as B  # noqa: E402
from tools.ch_mcp import dump_facts as D  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402

ENGINE = "ʺʺʸʶʿʵˀʸʲˀʲ"
ENGINE_CTOR_RVA = 0x20DF680

FRAGMENT = """\
// Image 0: Assembly-CSharp.dll - 0

// Namespace:
public interface ʼʽʾʿ // TypeDefIndex: 10
{
	// Methods

	// RVA: -1 Offset: -1 Slot: 0
	public abstract void ʳʴʵ(int ʶʷ);
}

// Namespace:
[RequireComponent(typeof(NoteAnimator))]
public class GameManager : MonoBehaviour // TypeDefIndex: 449
{
	// Fields
	private static GameManager ʷʲʺʷʻ; // 0x0
	public double songLength; // 0x50
	public double songTime; // 0x70
	[HideInInspector]
	public bool isSongOver; // 0x91
	public ScoreManager scoreManager; // 0xB8
	public bool isSongPlaying; // 0xC0

	// Properties
	public static GameManager ˁʹʷʺʻ { get; }

	// Methods

	[IteratorStateMachine(typeof(GameManager.ʻʵˀʸ))]
	// RVA: 0xBF3A0 Offset: 0xBE7A0 VA: 0x1800BF3A0
	private IEnumerator ʾʳʸʴ() { }

	// RVA: 0xC05A0 Offset: 0xBF9A0 VA: 0x1800C05A0
	private void Update() { }
}

// Namespace:
public class ScoreManager : MonoBehaviour // TypeDefIndex: 592
{
	// Fields
	private ʹʺʻ[] ʼʽʾ; // 0x38
}

// Namespace:
public class DrumsPlayer : BasePlayer // TypeDefIndex: 156
{
}

// Namespace: Unity.IL2CPP.Metadata
internal abstract class __Il2CppFullySharedGenericType : ValueType // TypeDefIndex: 16063
{}

// Namespace:
internal sealed class <PrivateImplementationDetails> // TypeDefIndex: 16000
{
	// Fields
	internal static readonly <PrivateImplementationDetails>.__StaticArrayInitTypeSize=20 0D58 /*Metadata offset 0x853AA0*/; // 0x14
}

// Namespace:
public sealed class ʺʺʸʶʿʵˀʸʲˀʲ : ʿʺʸʺʼ<ʾʸʸʻ>, ʼʽʾʿ // TypeDefIndex: 8255
{
	// Fields
	private const double ʵʴʺʹ = 0.25;
	private double ʷʷʽʹ; // 0x2E0

	// Methods

	// RVA: 0x20DDDA0 Offset: 0x20DD1A0 VA: 0x1820DDDA0 Slot: 19
	protected override double ʹʹʴˁ(double ʿʳʹʷ) { }

	// RVA: 0x20DF680 Offset: 0x20DEA80 VA: 0x1820DF680
	public void .ctor(ʲʽʶ.ʸʲʻ ʶˁʾ, Dictionary<string, int> ʼˀʲ, int ʷʾʲ = 50) { }
	/* GenericInstMethod :
	|
	|-RVA: 0x974380 Offset: 0x973780 VA: 0x180974380
	|-ʿʺʸʺʼ<int>..ctor
	*/
}
"""

# A second class whose constructor sits at the engine's address.
SECOND_CTOR = """\
// Namespace:
public class ʻʻʻ // TypeDefIndex: 9000
{
	// Methods

	// RVA: 0x20DF680 Offset: 0x20DEA80 VA: 0x1820DF680
	public void .ctor() { }
}
"""


def by_name(classes, name):
    return next(c for c in classes if c["name"] == name)


class ParseDumpTests(unittest.TestCase):
    def setUp(self):
        self.classes = D.parse_dump(FRAGMENT)

    def test_game_manager_fields_come_out_with_their_offsets(self):
        offsets = {f["name"]: f["offset"] for f in by_name(self.classes, "GameManager")["fields"]}
        self.assertEqual(offsets["songTime"], 0x70)
        self.assertEqual(offsets["isSongOver"], 0x91)
        self.assertEqual(offsets["isSongPlaying"], 0xC0)

    def test_scrambled_class_is_flagged_scrambled(self):
        self.assertFalse(by_name(self.classes, ENGINE)["readable"])
        self.assertTrue(by_name(self.classes, "GameManager")["readable"])

    def test_constants_and_properties_are_not_fields(self):
        engine = by_name(self.classes, ENGINE)
        self.assertEqual([f["offset"] for f in engine["fields"]], [0x2E0])
        names = [f["name"] for f in by_name(self.classes, "GameManager")["fields"]]
        self.assertNotIn("ˁʹʷʺʻ", names)

    def test_static_field_is_marked_static(self):
        fields = by_name(self.classes, "GameManager")["fields"]
        self.assertTrue(fields[0]["static"])
        self.assertFalse(fields[1]["static"])

    def test_methods_carry_rva_return_and_parameter_types(self):
        ctor = next(m for m in by_name(self.classes, ENGINE)["methods"] if m["name"] == ".ctor")
        self.assertEqual(ctor["rva"], ENGINE_CTOR_RVA)
        self.assertEqual(ctor["return"], "void")
        self.assertEqual(ctor["params"], ["ʲʽʶ.ʸʲʻ", "Dictionary<string, int>", "int"])

    def test_method_with_no_code_keeps_minus_one(self):
        method = by_name(self.classes, "ʼʽʾʿ")["methods"][0]
        self.assertEqual(method["rva"], -1)

    def test_generic_instance_lines_are_not_methods(self):
        names = [m["name"] for m in by_name(self.classes, ENGINE)["methods"]]
        self.assertEqual(names, ["ʹʹʴˁ", ".ctor"])

    def test_first_listed_type_is_the_base_unless_it_is_an_interface(self):
        engine = by_name(self.classes, ENGINE)
        self.assertEqual(engine["base"], "ʿʺʸʺʼ<ʾʸʸʻ>")
        self.assertEqual(engine["interfaces"], ["ʼʽʾʿ"])
        self.assertEqual(by_name(self.classes, "GameManager")["base"], "MonoBehaviour")

    def test_interface_listed_first_is_not_a_base(self):
        classes = D.parse_dump(FRAGMENT + "// Namespace: \npublic class Foo : ʼʽʾʿ // TypeDefIndex: 11\n{\n}\n")
        foo = by_name(classes, "Foo")
        self.assertIsNone(foo["base"])
        self.assertEqual(foo["interfaces"], ["ʼʽʾʿ"])

    def test_empty_class_on_one_brace_line_closes(self):
        cls = by_name(self.classes, "__Il2CppFullySharedGenericType")
        self.assertEqual(cls["namespace"], "Unity.IL2CPP.Metadata")
        self.assertEqual((cls["fields"], cls["methods"]), ([], []))
        self.assertEqual(self.classes[-1]["typedef_index"], 8255)

    def test_field_with_a_metadata_offset_note_keeps_its_offset(self):
        # The fragment's one field line there ends in a /*Metadata offset ...*/ note before its "// 0x14".
        fields = by_name(self.classes, "<PrivateImplementationDetails>")["fields"]
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual((fields[0]["static"], fields[0]["offset"]), (True, 0x14))

    def test_method_without_rva_line_fails_loudly_naming_its_line(self):
        method_line = "\tpublic void F() { }"
        head = ["// Namespace: ", "public class X // TypeDefIndex: 1", "{", "\t// Methods"]
        broken = head + [method_line, "}"]
        with self.assertRaises(D.FactsError) as caught:
            D.parse_dump("\n".join(broken) + "\n")
        message = str(caught.exception)
        # The refusal points at the method's own line, by number and by text.
        self.assertIn(f"line {broken.index(method_line) + 1}:", message)
        self.assertIn(method_line.strip(), message)
        # The same method with its RVA line before it is read, so only the missing line refuses.
        fixed = head + ["\t// RVA: 0xC05A0 Offset: 0xBF9A0 VA: 0x1800C05A0", method_line, "}"]
        method = by_name(D.parse_dump("\n".join(fixed) + "\n"), "X")["methods"][0]
        self.assertEqual((method["name"], method["rva"]), ("F", 0xC05A0))


class SignatureTests(unittest.TestCase):
    def test_ctor_rva_finds_the_one_engine_class(self):
        facts = D.build_facts(FRAGMENT, "1.1.0.6142-final", ENGINE_CTOR_RVA, "dump.cs")
        # The MCP reads the engine back through engine_class; the class it names
        # must be the one parsed class whose constructor sits at the address it gives.
        name, rva = BF.engine_class(facts)
        self.assertEqual(name, ENGINE)
        engine_cls = by_name(facts["classes"], name)
        self.assertIn(rva, [m["rva"] for m in engine_cls["methods"] if m["name"] == ".ctor"])
        self.assertEqual(facts["signatures"]["drums_engine"]["typedef_index"], engine_cls["typedef_index"])

    def test_written_facts_refuse_a_table_run_for_another_version(self):
        with tempfile.TemporaryDirectory() as tmp:
            dump = os.path.join(tmp, "dump.cs")
            with open(dump, "w", encoding="utf-8") as f:
                f.write(FRAGMENT)
            path, _ = D.write_facts(dump, "1.1.0.6142-final", ENGINE_CTOR_RVA)
            classes, _, _ = D.load_classes(path, "1.1.0.6142-final")
            self.assertIn(ENGINE, [c["name"] for c in classes])
            with self.assertRaises(D.FactsError) as caught:
                D.load_classes(path, "v1.0.0.4045-master")
            self.assertIn(path, str(caught.exception))
            self.assertIn("v1.0.0.4045-master", str(caught.exception))

    def assert_refusal_names_the_engine_and_its_match_count(self, text, rva, count):
        with self.assertRaises(D.FactsError) as caught:
            D.build_facts(text, "v", rva, "dump.cs")
        message = str(caught.exception)
        # The refusal is about the engine, and its one number is how many classes matched.
        self.assertIn("engine", message)
        self.assertEqual(re.findall(r"\d+", message), [str(count)])

    def test_ctor_rva_matching_no_class_refuses(self):
        # No constructor in FRAGMENT sits at 0x1234.
        self.assert_refusal_names_the_engine_and_its_match_count(FRAGMENT, 0x1234, 0)

    def test_ctor_rva_matching_two_classes_refuses(self):
        # SECOND_CTOR adds a second constructor at the engine's address.
        self.assert_refusal_names_the_engine_and_its_match_count(FRAGMENT + SECOND_CTOR, ENGINE_CTOR_RVA, 2)

    def test_game_manager_signature_lists_readable_fields_only(self):
        facts = D.build_facts(FRAGMENT, "v", ENGINE_CTOR_RVA, "dump.cs")
        gm = facts["signatures"]["GameManager"]
        self.assertEqual(
            [(f["name"], f["offset"]) for f in gm["readable_fields"]],
            [("songLength", 0x50), ("songTime", 0x70), ("isSongOver", 0x91),
             ("scoreManager", 0xB8), ("isSongPlaying", 0xC0)],
        )
        self.assertEqual(gm["typedef_index"], 449)
        self.assertEqual(facts["signatures"]["ScoreManager"]["typedef_index"], 592)
        self.assertEqual(facts["signatures"]["DrumsPlayer"]["typedef_index"], 156)

    def test_write_facts_puts_facts_json_beside_dump_cs(self):
        with tempfile.TemporaryDirectory() as tmp:
            dump = os.path.join(tmp, "dump.cs")
            with open(dump, "w", encoding="utf-8") as f:
                f.write(FRAGMENT)
            out_path, _ = D.write_facts(dump, "v", ENGINE_CTOR_RVA)
            # The file goes under the name build_facts reads a build's facts by.
            self.assertEqual(out_path, os.path.join(tmp, BF.FACTS_FILE))
            self.assertTrue(os.path.isfile(out_path))


class DumpBuildTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.game = os.path.join(self.tmp, "game")
        os.makedirs(os.path.join(self.game, "Clone Hero_Data", "il2cpp_data", "Metadata"))
        for rel in ("GameAssembly.dll", os.path.join("Clone Hero_Data", "il2cpp_data", "Metadata", "global-metadata.dat")):
            with open(os.path.join(self.game, rel), "wb") as f:
                f.write(b"x")
        self.out = os.path.join(self.tmp, "out")

    def tearDown(self):
        self._tmp.cleanup()

    def test_output_folder_inside_the_repo_is_refused(self):
        # The real runner. The refusal comes before the dumper starts, so the
        # folder is never made; without the refusal, run_dump would make it.
        inside = os.path.join(_REPO_ROOT, "ch-dumps", "6142")
        with self.assertRaises(B.DumpError) as caught:
            B.run_dump(self.game, inside)
        self.assertFalse(os.path.exists(inside))
        # The refusal names the folder and the checkout that holds it.
        self.assertIn(inside, str(caught.exception))
        self.assertIn(_REPO_ROOT, str(caught.exception))

    def test_cap_reports_the_cap_and_leaves_the_output_folder(self):
        # The real dumper on the real game folder. A whole run took 7.1 s
        # (tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md),
        # so a 1 s cap stops it; a run that ends inside the cap returns and fails here.
        cap_s = 1
        with self.assertRaises(B.DumpError) as caught:
            B.run_dump(G.GAME_DIR, self.out, cap_s=cap_s)
        self.assertIn(f" {cap_s} s", str(caught.exception))
        self.assertTrue(os.path.isdir(self.out))

    def test_three_done_lines_and_a_crash_exit_count_as_good(self):
        # The real dumper on the real game folder, with the default cap. Its
        # end-of-run key wait crashes when it has no stdin, so a good run exits
        # non-zero and still returns here; a run left waiting on a key would
        # hit the cap and raise instead.
        sizes, exit_code = B.run_dump(G.GAME_DIR, self.out)
        self.assertNotEqual(exit_code, 0)
        for name in B.REQUIRED_FILES:
            self.assertGreater(os.path.getsize(os.path.join(self.out, name)), 0, name)
        # Each reported size is the size of the file the run left.
        on_disk = {name: os.path.getsize(os.path.join(self.out, name))
                   for name in B.REPORTED_FILES if os.path.isfile(os.path.join(self.out, name))}
        self.assertEqual(sizes, on_disk)

    def failed_real_run(self):
        """The real dumper on setUp's game folder, whose one-byte files are no
        game: it stops before its first stage, prints no Done! line and writes
        no output file."""
        with self.assertRaises(B.DumpError) as caught:
            B.run_dump(self.game, self.out)
        return str(caught.exception).splitlines()

    def test_fewer_than_three_done_lines_fail_and_name_the_missing_stage(self):
        lines = self.failed_real_run()
        self.assertTrue(any(f"0 of {len(B.STAGES)} {B.DONE_LINE}" in line for line in lines), lines)
        for stage in B.STAGES:
            self.assertTrue(any(repr(stage) in line and B.DONE_LINE in line for line in lines), (stage, lines))

    def test_missing_output_file_fails(self):
        lines = self.failed_real_run()
        for name in B.REQUIRED_FILES:
            self.assertTrue(any(name in line and self.out in line for line in lines), (name, lines))


# ---- the facts tables (--tsv) ------------------------------------------------

# A tiny dump for the tables: a readable class, a scrambled engine whose
# fields partly live on its base class, and an address two methods share.
SP_BASE = "ʿʶʴ"
SP_AMOUNT = "ʵʼʿ"
ENGINE_SCORE = "ʿʾʹ"
SHARED_A = "ʲʳ"

TINY_DUMP = """\
// Namespace:
public class GameManager : MonoBehaviour // TypeDefIndex: 449
{
	// Fields
	public double songTime; // 0x70

	// Methods

	// RVA: 0xC05A0 Offset: 0xBF9A0 VA: 0x1800C05A0
	private void Update() { }
}

// Namespace:
public class ʿʶʴ // TypeDefIndex: 8288
{
	// Fields
	private long ʵʼʿ; // 0x68
}

// Namespace:
public class ʺʺʸ : ʿʶʴ // TypeDefIndex: 8255
{
	// Fields
	private uint ʻʼ; // 0x2C0

	// Methods

	// RVA: 0x20DF680 Offset: 0x20DEA80 VA: 0x1820DF680
	public void .ctor() { }

	// RVA: 0x20DEEC0 Offset: 0x20DE2C0 VA: 0x1820DEEC0
	public void ʿʾʹ() { }

	// RVA: 0x20AAAA0 Offset: 0x20A9EA0 VA: 0x1820AAAA0
	public void ʲʳ() { }
}

// Namespace:
public class Other // TypeDefIndex: 9000
{
	// Methods

	// RVA: 0x20AAAA0 Offset: 0x20A9EA0 VA: 0x1820AAAA0
	public void Shared() { }
}

// Namespace:
public class ʿʺʸ<TInputs> // TypeDefIndex: 9100
{
	// Methods

	// RVA: -1 Offset: -1
	protected void .ctor(int x) { }
	/* GenericInstMethod :
	|
	|-RVA: 0x100EC60 Offset: 0x100E060 VA: 0x18100EC60
	|-ʿʺʸ<__Il2CppFullySharedGenericStructType>..ctor
	|
	|-RVA: 0x100E960 Offset: 0x100DD60 VA: 0x18100E960
	|-ʿʺʸ<ʻʹʻ>..ctor
	|-ʿʺʸ<ʽʽʲ>..ctor
	*/

	// RVA: -1 Offset: -1 Slot: 25
	protected virtual bool Check() { }
	/* GenericInstMethod :
	|
	|-RVA: 0x100DAB0 Offset: 0x100CEB0 VA: 0x18100DAB0
	|-ʿʺʸ<ʻʹʻ>.Check
	|
	|-RVA: 0x100DAE0 Offset: 0x100CEE0 VA: 0x18100DAE0
	|-Box<int>.Check<object>
	|
	|-RVA: -1 Offset: -1
	|-ʿʺʸ<object>.Check
	*/
}
"""

TINY_CONSTANTS = """\
RVA_DRUMS_ENGINE_CTOR = 0x20DF680
RVA_BASE_ENGINE_CTOR_DRUMS = 0x100E960
RVA_CONST_NORMAL_BACK = 0x31406E0
OFF_SP_AMOUNT = 0x68
"""

TINY_VERSION = "1.1.0.6142-final"
# MD5 of the five bytes b"hello", the temp DLL's whole content.
HELLO_MD5 = "5d41402abc4b2a76b9719d911017c592"


# The real 4045 facts.json's GameManager record, copied whole from
# hydra-data/ch-dumps/4045/facts.json.
REAL_4045_FACTS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-4045", "facts.json")


class _TableFixture(unittest.TestCase):
    """A temp folder with the tiny dump, constants, DLL and a pages folder."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.dump = os.path.join(self.tmp, "dump.cs")
        with open(self.dump, "w", encoding="utf-8") as f:
            f.write(TINY_DUMP)
        self.constants = os.path.join(self.tmp, "constants.py")
        with open(self.constants, "w", encoding="utf-8") as f:
            f.write(TINY_CONSTANTS)
        self.dll = os.path.join(self.tmp, "GameAssembly.dll")
        with open(self.dll, "wb") as f:
            f.write(b"hello")
        self.pages = os.path.join(self.tmp, "pages")
        os.makedirs(self.pages)

    def tearDown(self):
        self._tmp.cleanup()

    def page(self, text, name="page.md"):
        with open(os.path.join(self.pages, name), "w", encoding="utf-8") as f:
            f.write(text)

    def table(self):
        return D.build_table(self.dump, self.pages, self.dll, version=TINY_VERSION,
                             constants_path=self.constants)

    def rows(self, text):
        return [line.split("\t") for line in text.splitlines() if not line.startswith("#")][1:]


class FactsTableTests(_TableFixture):
    def test_column_line_ends_with_cited(self):
        span = "GameManager.songTime"
        self.page(f"# P\n\n`{span}`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn("\t".join(D.TABLE_COLUMNS), text.splitlines())
        # Every row has every column, and the last one holds the span as the page wrote it.
        rows = self.rows(text)
        self.assertTrue(rows)
        for row in rows:
            self.assertEqual(len(row), len(D.TABLE_COLUMNS))
        self.assertIn(span, [row[-1] for row in rows])

    def test_cited_name_is_found(self):
        self.page("# P\n\nThe clock is `GameManager.songTime`.\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn(["6142", "field", "449", "GameManager", "songTime", "", "0x70",
                       "GameManager.songTime"], self.rows(text))

    def test_cited_address_is_found_in_any_class(self):
        self.page("# P\n\nScore is added at `0x20DEEC0`.\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn(["6142", "method", "8255", "", ENGINE_SCORE, "0x20DEEC0", "", "0x20DEEC0"],
                      self.rows(text))

    def test_missing_fact_is_an_error_naming_page_line_and_span(self):
        # Neither span is in TINY_DUMP: no member songTimer, no method at 0x20DEEC1.
        spans = ["`GameManager.songTimer`", "`0x20DEEC1`"]
        lines = ["# P", "", "fine", f"{spans[0]} and {spans[1]}"]
        self.page("\n".join(lines) + "\n")
        _, errors = self.table()
        # Both errors point at the page's last line, where the spans are.
        where = f"{D._display_path(os.path.join(self.pages, 'page.md'))}:{len(lines)}:"
        self.assertEqual(len(errors), len(spans), errors)
        for error, span in zip(errors, spans):
            self.assertTrue(error.startswith(where), (where, error))
            self.assertIn(span, error)

    def test_shared_address_gives_one_row_per_method(self):
        self.page("# P\n\n`0x20AAAA0`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        shared = [r for r in self.rows(text) if r[5] == "0x20AAAA0"]
        self.assertEqual(sorted((r[2], r[4]) for r in shared), [("8255", SHARED_A), ("9000", "Shared")])

    def test_offset_of_the_engine_is_found_on_its_base_class(self):
        self.page("# P\n\n`#8255+0x68`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn(["6142", "field", "8255", "", SP_AMOUNT, "", "0x68", "#8255+0x68"], self.rows(text))

    def test_constants_give_method_and_offset_rows_and_skip_data_addresses(self):
        text, errors = self.table()
        self.assertEqual(errors, [])
        rows = self.rows(text)
        self.assertIn(["6142", "method", "8255", "", ".ctor", "0x20DF680", "", "RVA_DRUMS_ENGINE_CTOR"], rows)
        self.assertIn(["6142", "field", "8255", "", SP_AMOUNT, "", "0x68", "OFF_SP_AMOUNT"], rows)
        self.assertIn(["6142", "method", "9100", "", ".ctor", "0x100E960", "", "RVA_BASE_ENGINE_CTOR_DRUMS"], rows)
        self.assertEqual(len(rows), 3)

    def test_an_address_only_in_a_generic_instantiation_block_is_a_method_row(self):
        self.page("# P\n\n`0x100DAB0` and `0x100DAE0`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        rows = self.rows(text)
        # An obfuscated class name is blank, as for any other row; the readable one shows.
        self.assertIn(["6142", "method", "9100", "", "Check", "0x100DAB0", "", "0x100DAB0"], rows)
        self.assertIn(["6142", "method", "9100", "Box<int>", "Check<object>", "0x100DAE0", "", "0x100DAE0"], rows)

    def test_instantiations_are_collected_only_when_asked_and_leave_the_classes_alone(self):
        instances = []
        with_list = D.parse_dump(TINY_DUMP, instances)
        self.assertEqual(with_list, D.parse_dump(TINY_DUMP))
        generic = by_name(with_list, "ʿʺʸ<TInputs>")
        self.assertEqual([m["name"] for m in generic["methods"]], [".ctor", "Check"])
        self.assertEqual(
            sorted((i["rva"], i["class"], i["method"]) for i in instances),
            [(0x100DAB0, "ʿʺʸ<ʻʹʻ>", "Check"), (0x100DAE0, "Box<int>", "Check<object>"),
             (0x100E960, "ʿʺʸ<ʻʹʻ>", ".ctor"), (0x100E960, "ʿʺʸ<ʽʽʲ>", ".ctor"),
             (0x100EC60, "ʿʺʸ<__Il2CppFullySharedGenericStructType>", ".ctor")])
        self.assertTrue(all(i["typedef_index"] == 9100 for i in instances))

    def test_split_instance_uses_the_methods_own_name(self):
        self.assertEqual(D._split_instance("A<B, C>..ctor", ".ctor"), ("A<B, C>", ".ctor"))
        self.assertEqual(D._split_instance("SLImage.M<Color32>", "M"), ("SLImage", "M<Color32>"))
        self.assertEqual(D._split_instance("Outer.Inner<T>.M", "M"), ("Outer.Inner<T>", "M"))
        self.assertEqual(D._split_instance("A.B", "Other"), ("A", "B"))

    def test_a_facts_json_has_no_instantiation_rows(self):
        # The facts.json comes from the real writer. The writer needs every
        # class it signs, so the dump gains ScoreManager, which the tiny dumps lack.
        with open(self.dump, "w", encoding="utf-8") as f:
            f.write(MAP_DUMP + "\n// Namespace: \npublic class ScoreManager : MonoBehaviour"
                    " // TypeDefIndex: 592\n{\n}\n")
        path, _ = D.write_facts(self.dump, TINY_VERSION, ENGINE_CTOR_RVA)
        # Read from the dump.cs itself, the same span is found.
        self.page("# P\n\n`0x100DAB0`\n")
        self.assertEqual(self.table()[1], [])
        _, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertTrue(any("0x100DAB0" in e for e in errors), errors)

    def test_header_md5_comes_from_the_dll_bytes(self):
        text, _ = self.table()
        self.assertIn(f"# GameAssembly.dll md5: {HELLO_MD5}", text.splitlines())
        # The sum is read from the file each time, so new bytes give a new sum.
        with open(self.dll, "wb") as f:
            f.write(b"hello, again")
        with open(self.dll, "rb") as f:
            again = hashlib.md5(f.read()).hexdigest()
        self.assertNotEqual(again, HELLO_MD5)
        self.assertIn(f"# GameAssembly.dll md5: {again}", self.table()[0].splitlines())

    def test_python_name_and_file_names_are_not_game_facts(self):
        self.assertIsNone(D.game_fact("bot_run.launch_command"))
        self.assertIsNone(D.game_fact("song.ini"))
        self.assertIsNone(D.game_fact("dump.cs"))
        self.assertIsNone(D.game_fact("scoredata.bin"))
        self.assertIsNotNone(D.game_fact("GameManager.songTime"))
        self.assertIsNotNone(D.game_fact("0x20DEEC0"))
        self.assertIsNotNone(D.game_fact("#8255+0x68"))
        self.assertIsNotNone(D.game_fact("#8255.Update"))
        self.assertIsNone(D.game_fact("0x2E0"))  # under five hex digits: an offset, not an RVA
        self.page("# P\n\n`bot_run.launch_command` reads `song.ini`.\n")
        _, errors = self.table()
        self.assertEqual(errors, [])

    def test_check_passes_on_a_fresh_table_and_fails_on_a_stale_one(self):
        self.page("# P\n\n`GameManager.Update`\n")
        out = os.path.join(self.tmp, "facts-6142.tsv")
        argv = [self.dump, "--version", TINY_VERSION, "--tsv", self.pages, "--out", out,
                "--dll", self.dll, "--constants", self.constants]
        self.assertEqual(D.main(argv), 0)
        self.assertEqual(D.main(argv + ["--check"]), 0)
        with open(out, encoding="utf-8") as f:
            stale = f.read().replace("0xC05A0", "0xC05A1")
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write(stale)
        self.assertEqual(D.main(argv + ["--check"]), 1)

    def test_a_block_naming_another_build_is_left_to_that_build(self):
        self.page("# P\n\n### F-01 a 1.0 rule\n\nRule: `Old.thing`\nGame code (4045): `Old.thing`\n"
                  "\n## Build 1.0.0.4045\n\n`Older.thing`\n\n## Back\n\n`GameManager.songTime`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn("songTime", text)

    def test_4045_facts_json_is_read_by_name_with_no_address(self):
        # Build 4045 has no method addresses.
        path = REAL_4045_FACTS
        manager = self.real_4045_manager()
        self.assertIsNone(next(m for m in manager["methods"] if m["name"] == "Update")["rva"])
        self.page("# P\n\n## Build 1.0.0.4045\n\n`GameManager.Update`\n\n## Back\n\n`0x20DEEC0`\n")
        text, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertEqual(errors, [])
        # Only the span under the 4045 heading is looked up, and its address column is blank.
        self.assertEqual(self.rows(text), [["4045", "method", str(manager["typedef_index"]), "GameManager",
                                            "Update", "", "", "GameManager.Update"]])

    def test_a_file_under_code_and_a_build_takes_that_build_for_every_span(self):
        lines = ["# Clone Hero 1.0 (build 4045): the notes", "`A.b`", "## Build 1.0.0.9999", "`C.d`"]
        self.assertEqual([b for _, _, b in D.span_builds(lines, "4045")], ["4045", "4045"])
        # With no place, the heading rule still applies and the default build holds elsewhere.
        self.assertEqual([b for _, _, b in D.span_builds(lines)], ["6142", "9999"])
        self.assertEqual(D.place_build(os.path.join(self.pages, "code", "4045", "n.md"), self.pages), "4045")
        self.assertEqual(D.place_build(os.path.join(self.pages, "code", "6142", "sub", "n.md"), self.pages), "6142")
        for outside in (("n.md",), ("code", "names.md"), ("sub", "code", "4045", "n.md")):
            self.assertIsNone(D.place_build(os.path.join(self.pages, *outside), self.pages))

    # A 1.0 note: the page text the code-folder tests below place in different folders.
    NOTE_4045 = "# Clone Hero 1.0 (build 4045): notes\n\nThe game runs `GameManager.Update`.\n"

    def real_4045_manager(self):
        """The GameManager record of REAL_4045_FACTS."""
        with open(REAL_4045_FACTS, encoding="utf-8") as f:
            return by_name(json.load(f)["classes"], "GameManager")

    def test_a_note_under_code_4045_is_looked_up_in_the_4045_table(self):
        manager = self.real_4045_manager()
        os.makedirs(os.path.join(self.pages, "code", "4045"))
        self.page(self.NOTE_4045, name=os.path.join("code", "4045", "note.md"))
        text, errors = D.build_table(REAL_4045_FACTS, self.pages, self.dll, constants_path=self.constants)
        self.assertEqual(errors, [])
        self.assertEqual(self.rows(text), [["4045", "method", str(manager["typedef_index"]), "GameManager",
                                            "Update", "", "", "GameManager.Update"]])

    def test_the_same_note_outside_code_4045_is_left_out_of_the_4045_table(self):
        # Outside code/4045 the page is a 6142 page, so the 4045 run leaves its span alone.
        self.page(self.NOTE_4045)
        text, errors = D.build_table(REAL_4045_FACTS, self.pages, self.dll, constants_path=self.constants)
        self.assertEqual(errors, [])
        self.assertEqual(self.rows(text), [])

    def test_a_note_under_code_6142_stays_6142_under_a_heading_naming_another_build(self):
        manager = by_name(D.parse_dump(TINY_DUMP), "GameManager")
        song_time = next(f for f in manager["fields"] if f["name"] == "songTime")
        os.makedirs(os.path.join(self.pages, "code", "6142"))
        self.page("## Build 1.0.0.4045\n\n`GameManager.songTime`\n", name=os.path.join("code", "6142", "n.md"))
        text, errors = self.table()
        self.assertEqual(errors, [])
        # The 6142 table also holds the constants file's rows, so the page's row is looked for among them.
        self.assertIn(["6142", "field", str(manager["typedef_index"]), "GameManager",
                       "songTime", "", f"0x{song_time['offset']:X}", "GameManager.songTime"], self.rows(text))


# More of the tiny dump for the names map: a namespaced class, and a readable
# class with a scrambled field.
FIRE = "ʸʸʳ"
MAP_DUMP = TINY_DUMP + """\

// Namespace: System
public static class Math // TypeDefIndex: 1721
{
	// Methods

	// RVA: 0x1000000 Offset: 0x1000000 VA: 0x181000000
	public static double Floor(double d) { }
}

// Namespace:
public class DrumsPlayer // TypeDefIndex: 156
{
	// Fields
	private float ʸʸʳ; // 0x40
}
"""

MAP_COLUMNS = "build\ttypedef_index\tclass\tmember\tname\twhat\n"
GOOD_MAP = (
    "# Our names for the tiny dump's scrambled names.\n" + MAP_COLUMNS
    + "6142\t8255\tʺʺʸ\t\tDrumsEngine\tThe drum engine.\n"
    + f"6142\t8255\tʺʺʸ\t{ENGINE_SCORE}\taddScore\tAdds a note's score.\n"
    + f"6142\t8288\t{SP_BASE}\t{SP_AMOUNT}\tspAmount\tThe Star Power meter.\n"
    + f"6142\t156\tDrumsPlayer\t{FIRE}\tfillDeadline\tThe fill deadline.\n"
)


class NamesMapTests(_TableFixture):
    def setUp(self):
        super().setUp()
        with open(self.dump, "w", encoding="utf-8") as f:
            f.write(MAP_DUMP)
        os.makedirs(os.path.join(self.pages, "code"))

    def names(self, text, build="6142"):
        with open(os.path.join(self.pages, "code", f"names-{build}.tsv"), "w", encoding="utf-8") as f:
            f.write(text)

    def table_and_warnings(self):
        warnings = []
        text, errors = D.build_table(self.dump, self.pages, self.dll, version=TINY_VERSION,
                                     constants_path=self.constants, warnings=warnings)
        return text, errors, warnings

    def test_our_names_resolve_to_the_dumps_names_one_row_per_span(self):
        self.names(GOOD_MAP)
        self.page("# P\n\n`#8255.addScore` `DrumsEngine.addScore` `DrumsPlayer.fillDeadline`\n"
                  "`#8255.spAmount` `DrumsEngine.spAmount`\n")
        text, errors, warnings = self.table_and_warnings()
        self.assertEqual(errors, [])
        self.assertEqual(warnings, [])
        rows = self.rows(text)
        for cited in ("#8255.addScore", "DrumsEngine.addScore"):
            self.assertIn(["6142", "method", "8255", "", ENGINE_SCORE, "0x20DEEC0", "", cited], rows)
        self.assertIn(["6142", "field", "156", "DrumsPlayer", FIRE, "", "0x40", "DrumsPlayer.fillDeadline"], rows)
        # A member of the base class, by our name, through the class or its index.
        for cited in ("#8255.spAmount", "DrumsEngine.spAmount"):
            self.assertIn(["6142", "field", "8255", "", SP_AMOUNT, "", "0x68", cited], rows)

    def test_a_namespaced_class_is_its_class_row_cited_as_written(self):
        self.page("# P\n\n`System.Math` and `Math.Floor`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        rows = self.rows(text)
        self.assertIn(["6142", "class", "1721", "Math", "", "", "", "System.Math"], rows)
        self.assertIn(["6142", "method", "1721", "Math", "Floor", "0x1000000", "", "Math.Floor"], rows)

    def test_a_scrambled_span_is_an_error_naming_the_line_and_the_map(self):
        spans = [f"`#8255.{ENGINE_SCORE}`", f"`{SP_BASE}`"]
        self.page(f"# P\n\n`GameManager.songTime`\n{spans[0]} and {spans[1]}\n")
        _, errors = self.table()
        # One error per scrambled span; the readable span on line 3 gives none.
        self.assertEqual(len(errors), len(spans), errors)
        where = f"{D._display_path(os.path.join(self.pages, 'page.md'))}:4: "
        names_map = D._display_path(D.names_path(self.pages, D.DEFAULT_BUILD))
        for error, span in zip(errors, spans):
            self.assertTrue(error.startswith(where + span), (where, error))
            self.assertIn(names_map, error)

    def test_an_uncited_row_is_a_warning_not_an_error(self):
        self.names(GOOD_MAP)
        self.page("# P\n\n`DrumsEngine.addScore`\n")
        _, errors, warnings = self.table_and_warnings()
        self.assertEqual(errors, [])
        self.assertEqual(len(warnings), 2, warnings)
        self.assertTrue(any("`spAmount` is cited by no page" in w for w in warnings), warnings)
        self.assertTrue(any("`fillDeadline` is cited by no page" in w for w in warnings), warnings)

    def test_map_rows_that_do_not_match_the_dump_are_errors(self):
        self.names(MAP_COLUMNS
                   + "6142\t8255\tʻʻ\t\tWrongClass\tNot the class at that index.\n"
                   + f"6142\t8255\tʺʺʸ\tʻʻʻ\tnoSuch\tA member the class lacks.\n"
                   + f"6142\t8255\tʺʺʸ\t{ENGINE_SCORE}\t.ctor\tNot an identifier.\n"
                   + f"6142\t8255\tʺʺʸ\t{SHARED_A}\tUpdate2\tFine on its own.\n"
                   + f"6142\t8255\tʺʺʸ\t{ENGINE_SCORE}\tUpdate2\tThe same name twice in one class.\n"
                   + f"6142\t449\tGameManager\tsongTime\tUpdate\tA real member's name.\n"
                   + "6142\t8288\tʿʶʴ\t\tGameManager\tA real class's name.\n"
                   + "6142\t\tʿʶʴ\t\tNoIndex\tNo TypeDefIndex in a build keyed by it.\n")
        _, errors = self.table()
        joined = "\n".join(errors)
        self.assertIn("TypeDefIndex 8255 is the class `ʺʺʸ` in the dump, not `ʻʻ`", joined)
        self.assertIn("has no member `ʻʻʻ`", joined)
        self.assertIn("the name '.ctor' is not an ASCII identifier", joined)
        self.assertIn("`Update2` names another member of the class too", joined)
        self.assertIn("`Update` is a real member name of the class `GameManager`", joined)
        self.assertIn("`GameManager` is a real class name in the dump", joined)
        self.assertIn("names a class by its TypeDefIndex", joined)
        self.assertTrue(all("names-6142.tsv:" in e for e in errors), errors)

    def test_check_exits_1_on_a_map_error(self):
        self.page("# P\n\n`GameManager.Update`\n")
        out = os.path.join(self.tmp, "facts-6142.tsv")
        argv = [self.dump, "--version", TINY_VERSION, "--tsv", self.pages, "--out", out,
                "--dll", self.dll, "--constants", self.constants]
        self.assertEqual(D.main(argv), 0)
        self.assertEqual(D.main(argv + ["--check"]), 0)
        self.names(MAP_COLUMNS + "6142\t8255\tʺʺʸ\tʻʻʻ\tnoSuch\tA member the class lacks.\n")
        self.assertEqual(D.main(argv + ["--check"]), 1)

    def test_a_bad_column_line_is_an_error(self):
        self.names("build\tclass\tname\n")
        _, errors = self.table()
        self.assertEqual(len(errors), 1, errors)
        self.assertIn("not the column line", errors[0])

    def test_a_4045_map_names_its_class_by_name(self):
        scrambled = "ʶˀʵ"
        facts = {"game_version": "v1.0.0.4045-master", "classes": [
            {"name": scrambled, "namespace": "", "kind": "class", "typedef_index": 3,
             "readable": False, "base": None, "interfaces": [],
             "fields": [], "methods": [{"name": "ʹʹʾ", "return": "void", "params": [],
                                        "static": False, "rva": None}]}]}
        path = os.path.join(self.tmp, "facts.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(facts, f, ensure_ascii=False)
        self.names(MAP_COLUMNS + f"4045\t\t{scrambled}\t\tFillGenerator\tBuilds the fills.\n"
                   + f"4045\t\t{scrambled}\tʹʹʾ\tbuild\tBuilds one fill.\n", build="4045")
        self.page("# P\n\n## Build 1.0.0.4045\n\n`FillGenerator.build`\n")
        text, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertEqual(errors, [])
        self.assertEqual(self.rows(text), [["4045", "method", "3", "", "ʹʹʾ", "", "", "FillGenerator.build"]])
        self.names(MAP_COLUMNS + f"4045\t3\t{scrambled}\t\tFillGenerator\tBuilds the fills.\n", build="4045")
        _, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertTrue(any("typedef_index stays empty" in e for e in errors), errors)
        # facts.json lists no properties, so a member it lacks is a warning; a missing class stays an error.
        self.names(MAP_COLUMNS + f"4045\t\t{scrambled}\t\tFillGenerator\tBuilds the fills.\n"
                   + f"4045\t\t{scrambled}\tʹʹʾ\tbuild\tBuilds one fill.\n"
                   + f"4045\t\t{scrambled}\tʸʶʳ\tWrittenFills\tA property.\n"
                   + "4045\t\tʻʻʻ\tʸʶʳ\tLaneMask\tA class the build lacks.\n", build="4045")
        warnings = []
        _, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants, warnings=warnings)
        self.assertEqual(len(errors), 1, errors)
        self.assertIn("names-4045.tsv:5: the class `ʻʻʻ` matches 0 classes", errors[0])
        self.assertTrue(any("names-4045.tsv:4:" in w and "facts.json has no properties to check it against" in w
                            for w in warnings), warnings)

    def test_a_4045_span_naming_a_mapped_property_gets_an_unlisted_row_and_a_warning(self):
        scrambled, prop = "ʶˀʵ", "ʸʶʳ"
        facts = {"game_version": "v1.0.0.4045-master", "classes": [
            {"name": scrambled, "namespace": "", "kind": "class", "typedef_index": 3,
             "readable": False, "base": None, "interfaces": [],
             "fields": [], "methods": [{"name": "ʹʹʾ", "return": "void", "params": [],
                                        "static": False, "rva": None}]}]}
        path = os.path.join(self.tmp, "facts.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(facts, f, ensure_ascii=False)
        self.names(MAP_COLUMNS + f"4045\t\t{scrambled}\t\tFillGenerator\tBuilds the fills.\n"
                   + f"4045\t\t{scrambled}\tʹʹʾ\tbuild\tBuilds one fill.\n"
                   + f"4045\t\t{scrambled}\t{prop}\tWrittenFills\tA property.\n", build="4045")
        self.page("# P\n\n## Build 1.0.0.4045\n\n`FillGenerator.WrittenFills` and `FillGenerator.build`\n"
                  "`FillGenerator.noSuchThing`\n")
        warnings = []
        text, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants,
                                     warnings=warnings)
        rows = self.rows(text)
        self.assertIn(["4045", "property-unlisted", "3", "", prop, "", "", "FillGenerator.WrittenFills"], rows)
        self.assertIn(["4045", "method", "3", "", "ʹʹʾ", "", "", "FillGenerator.build"], rows)
        # A member with no map row and no facts.json entry stays an error.
        self.assertEqual(len(errors), 1, errors)
        self.assertIn("page.md:6: `FillGenerator.noSuchThing` matches 0 classes and members in build 4045", errors[0])
        self.assertTrue(any("page.md:5: `FillGenerator.WrittenFills` is a property" in w for w in warnings),
                        warnings)
        self.assertFalse(any("`FillGenerator.build`" in w for w in warnings), warnings)

    def test_a_6142_span_naming_a_member_the_dump_lacks_is_an_error_not_an_unlisted_row(self):
        self.names(GOOD_MAP + "6142\t8255\tʺʺʸ\tʻʻʻ\tghost\tA member the class lacks.\n")
        self.page("# P\n\n`DrumsEngine.ghost`\n")
        text, errors, _ = self.table_and_warnings()
        self.assertNotIn("property-unlisted", text)
        self.assertTrue(any("`DrumsEngine.ghost` matches 0 classes and members in build 6142" in e
                            for e in errors), errors)


if __name__ == "__main__":
    unittest.main()
