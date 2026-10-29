"""Tests for dump_facts (the dump.cs reader) and dump_build (the dumper runner).

No real dump and no real dumper: the reader runs on the hand-written dump.cs
fragment below, and the runner gets a fake subprocess.
"""

from __future__ import annotations

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


if __name__ == "__main__":
    unittest.main()
