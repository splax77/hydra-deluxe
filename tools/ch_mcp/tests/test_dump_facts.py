"""Tests for dump_facts (the dump.cs reader) and dump_build (the dumper runner).

No real dump and no real dumper: the reader runs on the hand-written dump.cs
fragment below, and the runner gets a fake subprocess.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import types
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import dump_build as B  # noqa: E402
from tools.ch_mcp import dump_facts as D  # noqa: E402

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

THREE_STAGES = (
    "Initializing metadata...\nDumping...\nDone!\nGenerate struct...\nDone!\n"
    "Generate dummy dll...\nDone!\nPress any key to exit...\n"
)
TWO_STAGES = "Initializing metadata...\nDumping...\nDone!\nGenerate struct...\nDone!\nGenerate dummy dll...\n"
CRASH = "Unhandled exception. System.InvalidOperationException: Cannot read keys\n"


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
        field = by_name(self.classes, "<PrivateImplementationDetails>")["fields"][0]
        self.assertEqual(
            field,
            {"name": "0D58", "type": "<PrivateImplementationDetails>.__StaticArrayInitTypeSize=20",
             "static": True, "offset": 0x14},
        )

    def test_method_without_rva_line_fails_loudly(self):
        broken = "// Namespace: \npublic class X // TypeDefIndex: 1\n{\n\t// Methods\n\tpublic void F() { }\n}\n"
        with self.assertRaisesRegex(D.FactsError, "method with no RVA line"):
            D.parse_dump(broken)


class SignatureTests(unittest.TestCase):
    def test_ctor_rva_finds_the_one_engine_class(self):
        facts = D.build_facts(FRAGMENT, "1.1.0.6142-final", ENGINE_CTOR_RVA, "dump.cs")
        engine = facts["signatures"]["drums_engine"]
        self.assertEqual(engine["name"], ENGINE)
        self.assertEqual(engine["typedef_index"], 8255)
        self.assertEqual(engine["ctor_rva"], ENGINE_CTOR_RVA)
        self.assertEqual(facts["game_version"], "1.1.0.6142-final")

    def test_ctor_rva_matching_no_class_refuses(self):
        with self.assertRaisesRegex(D.FactsError, r"^engine ctor RVA matched 0 classes$"):
            D.build_facts(FRAGMENT, "v", 0x1234, "dump.cs")

    def test_ctor_rva_matching_two_classes_refuses(self):
        with self.assertRaisesRegex(D.FactsError, r"^engine ctor RVA matched 2 classes$"):
            D.build_facts(FRAGMENT + SECOND_CTOR, "v", ENGINE_CTOR_RVA, "dump.cs")

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
            self.assertEqual(out_path, os.path.join(tmp, "facts.json"))
            self.assertTrue(os.path.isfile(out_path))


class FakeRunner:
    """Stands in for subprocess.run: records the call, writes the files a run leaves."""

    def __init__(self, stdout="", returncode=0, files=B.REPORTED_FILES, timeout=False):
        self.stdout = stdout
        self.returncode = returncode
        self.files = files
        self.timeout = timeout
        self.calls = []

    def __call__(self, cmd, **kwargs):
        self.calls.append((cmd, kwargs))
        if self.timeout:
            raise subprocess.TimeoutExpired(cmd, kwargs["timeout"])
        out_dir = cmd[-1]
        for name in self.files:
            with open(os.path.join(out_dir, name), "w", encoding="utf-8") as f:
                f.write(name)
        return types.SimpleNamespace(returncode=self.returncode, stdout=self.stdout, stderr=CRASH)


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
        runner = FakeRunner(THREE_STAGES)
        inside = os.path.join(_REPO_ROOT, "ch-dumps", "6142")
        with self.assertRaisesRegex(B.DumpError, "inside the git checkout"):
            B.run_dump(self.game, inside, runner=runner)
        self.assertEqual(runner.calls, [])
        self.assertFalse(os.path.exists(inside))

    def test_cap_reports_the_cap_line(self):
        runner = FakeRunner(timeout=True)
        with self.assertRaises(B.DumpError) as caught:
            B.run_dump(self.game, self.out, runner=runner)
        self.assertEqual(str(caught.exception), "Il2CppDumper did not finish in 600 s; output folder left as is")

    def test_three_done_lines_and_a_crash_exit_count_as_good(self):
        runner = FakeRunner(THREE_STAGES, returncode=0xE0434352)
        sizes, exit_code = B.run_dump(self.game, self.out, runner=runner)
        self.assertEqual(sorted(sizes), sorted(B.REPORTED_FILES))
        self.assertEqual(exit_code, 0xE0434352)
        _, kwargs = runner.calls[0]
        self.assertIs(kwargs["stdin"], subprocess.DEVNULL)
        self.assertEqual(kwargs["env"]["DOTNET_ROLL_FORWARD"], "Major")
        self.assertEqual(kwargs["timeout"], 600)

    def test_fewer_than_three_done_lines_fail_and_name_the_missing_stage(self):
        runner = FakeRunner(TWO_STAGES, returncode=1)
        with self.assertRaises(B.DumpError) as caught:
            B.run_dump(self.game, self.out, runner=runner)
        message = str(caught.exception)
        self.assertIn("printed 2 of 3 Done! lines", message)
        self.assertIn("stage 'Generate dummy dll...' never printed Done!", message)

    def test_missing_output_file_fails(self):
        runner = FakeRunner(THREE_STAGES, files=("dump.cs", "il2cpp.h", "script.json"))
        with self.assertRaisesRegex(B.DumpError, "stringliteral.json is missing"):
            B.run_dump(self.game, self.out, runner=runner)


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


class FactsTableTests(unittest.TestCase):
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

    def test_cited_name_is_found(self):
        self.page("# P\n\nThe clock is `GameManager.songTime`.\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn(["6142", "field", "449", "GameManager", "songTime", "", "0x70"], self.rows(text))

    def test_cited_address_is_found_in_any_class(self):
        self.page("# P\n\nScore is added at `0x20DEEC0`.\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        self.assertIn(["6142", "method", "8255", "", ENGINE_SCORE, "0x20DEEC0", ""], self.rows(text))

    def test_missing_fact_is_an_error_naming_page_line_and_span(self):
        self.page("# P\n\nfine\n`GameManager.songTimer` and `0x20DEEC1`\n")
        _, errors = self.table()
        self.assertEqual(len(errors), 2)
        self.assertTrue(all("page.md:4:" in e for e in errors), errors)
        self.assertIn("`GameManager.songTimer`", errors[0])
        self.assertIn("`0x20DEEC1`", errors[1])

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
        self.assertIn(["6142", "field", "8255", "", SP_AMOUNT, "", "0x68"], self.rows(text))

    def test_constants_give_method_and_offset_rows_and_skip_data_addresses(self):
        text, errors = self.table()
        self.assertEqual(errors, [])
        rows = self.rows(text)
        self.assertIn(["6142", "method", "8255", "", ".ctor", "0x20DF680", ""], rows)
        self.assertIn(["6142", "field", "8255", "", SP_AMOUNT, "", "0x68"], rows)
        self.assertIn(["6142", "method", "9100", "", ".ctor", "0x100E960", ""], rows)
        self.assertEqual(len(rows), 3)

    def test_an_address_only_in_a_generic_instantiation_block_is_a_method_row(self):
        self.page("# P\n\n`0x100DAB0` and `0x100DAE0`\n")
        text, errors = self.table()
        self.assertEqual(errors, [])
        rows = self.rows(text)
        # An obfuscated class name is blank, as for any other row; the readable one shows.
        self.assertIn(["6142", "method", "9100", "", "Check", "0x100DAB0", ""], rows)
        self.assertIn(["6142", "method", "9100", "Box<int>", "Check<object>", "0x100DAE0", ""], rows)

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
        facts = {"game_version": TINY_VERSION, "classes": D.parse_dump(TINY_DUMP)}
        path = os.path.join(self.tmp, "facts.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(facts, f, ensure_ascii=False)
        self.page("# P\n\n`0x100DAB0`\n")
        _, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertTrue(any("0x100DAB0" in e for e in errors), errors)

    def test_header_md5_comes_from_the_dll_bytes(self):
        text, _ = self.table()
        self.assertIn(f"# version: {TINY_VERSION}", text.splitlines())
        self.assertIn(f"# GameAssembly.dll md5: {HELLO_MD5}", text.splitlines())

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
        facts = {"game_version": "v1.0.0.4045-master", "classes": [
            {"name": "GameManager", "namespace": "", "kind": "class", "typedef_index": 562,
             "readable": True, "base": None, "interfaces": [],
             "fields": [], "methods": [{"name": "Update", "return": "void", "params": [],
                                        "static": False, "rva": None}]}]}
        path = os.path.join(self.tmp, "facts.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(facts, f)
        self.page("# P\n\n## Build 1.0.0.4045\n\n`GameManager.Update`\n\n## Back\n\n`0x20DEEC0`\n")
        text, errors = D.build_table(path, self.pages, self.dll, constants_path=self.constants)
        self.assertEqual(errors, [])
        self.assertEqual(self.rows(text), [["4045", "method", "562", "GameManager", "Update", "", ""]])
        self.assertIn("# version: v1.0.0.4045-master", text.splitlines())


if __name__ == "__main__":
    unittest.main()
