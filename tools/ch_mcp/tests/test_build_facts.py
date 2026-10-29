"""Tests for build_facts.py.

Most tests run on a hand-written facts dict and temp folders that stand in
for the dumps folder and the game folder, so they never touch the game. One
test reads the real 6142 facts file when this machine has it.
"""

from __future__ import annotations

import json
import os

import pytest

from tools.ch_mcp import build_facts as B
from tools.ch_mcp import game_files
from tools.ch_probe import constants as C

ENGINE = "ʺʺʸʶʿʵˀʸʲˀʲ"
BASE = "ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>"
TWIN = "ʷʲʺʷʻʳʾʶˁˀʷ"

V6142 = "v1.1.0.6142-final"
F6142 = "1.1.0.6142-final"


def _method(name, rva):
    return {"name": name, "return": "void", "params": [], "static": False, "rva": rva}


def _facts(version=F6142):
    return {
        "game_version": version,
        "dump_file": "dump.cs",
        "classes": [
            {"name": ENGINE, "namespace": "", "kind": "class", "base": BASE,
             "methods": [_method(".ctor", 0x20DF680), _method("points", 0x20DEEC0),
                         _method("dynamics", 0x20DE770),
                         _method("twin_a", 0x500), _method("twin_b", 0x500)]},
            {"name": BASE, "namespace": "", "kind": "class", "base": "Object",
             "methods": [_method("process", -1), _method("check", -1)]},
            {"name": TWIN, "namespace": "a", "kind": "class", "methods": [_method("x", 0x600)]},
            {"name": TWIN, "namespace": "b", "kind": "class", "methods": [_method("x", 0x600)]},
        ],
        "signatures": {"drums_engine": {"name": ENGINE, "base": BASE, "method_count": 5,
                                        "ctor_rva": 0x20DF680}},
    }


def _game(tmp_path, version):
    game = tmp_path / "game"
    path = B.version_file(str(game))
    os.makedirs(os.path.dirname(path))
    with open(path, "w", encoding="utf-8") as f:
        json.dump({"version": version, "hash": "abc"}, f)
    return str(game)


def _dumps(tmp_path, folder, facts):
    root = tmp_path / "dumps"
    (root / folder).mkdir(parents=True)
    with open(root / folder / B.FACTS_FILE, "w", encoding="utf-8") as f:
        json.dump(facts, f, ensure_ascii=False)
    return str(root)


def test_build_number_is_the_fourth_dotted_number():
    assert B.build_number(V6142) == "6142"
    assert B.build_number("v1.0.0.4045-master") == "4045"


def test_build_number_refuses_a_version_without_one():
    with pytest.raises(B.UnknownBuild) as err:
        B.build_number("v1.1-final")
    assert str(err.value) == ("unknown build v1.1-final: no facts for it; run dump_build.py and "
                              "dump_facts.py (no build number in the version string)")


def test_same_build_drops_one_leading_v():
    assert B.same_build(V6142, F6142)
    assert B.same_build(F6142, V6142)
    assert B.same_build(V6142, V6142)
    assert not B.same_build("v1.0.0.4045-master", F6142)
    assert not B.same_build("vv1.1.0.6142-final", F6142)


def test_current_build_reads_the_version_as_written(tmp_path):
    assert B.current_build(_game(tmp_path, V6142)) == V6142


def test_version_file_sits_in_the_real_game_folder():
    assert B.VERSION_FILE == os.path.join(game_files.GAME_DIR, "Clone Hero_Data", "version.json")


def test_load_facts_loads_a_matching_file(tmp_path):
    game = _game(tmp_path, V6142)
    dumps = _dumps(tmp_path, "6142", _facts())
    facts = B.load_facts(game, dumps)
    assert facts["game_version"] == F6142
    assert B.engine_class(facts)[0] == ENGINE


def test_load_facts_refuses_a_build_with_no_folder(tmp_path):
    game = _game(tmp_path, "v1.1.0.6200-final")
    dumps = _dumps(tmp_path, "6142", _facts())
    with pytest.raises(B.UnknownBuild) as err:
        B.load_facts(game, dumps)
    missing = os.path.join(dumps, "6200", "facts.json")
    assert str(err.value) == ("unknown build v1.1.0.6200-final: no facts for it; run dump_build.py "
                              f"and dump_facts.py ({missing} does not exist)")
    assert err.value.version == "v1.1.0.6200-final"


def test_load_facts_refuses_facts_of_another_version(tmp_path):
    game = _game(tmp_path, V6142)
    dumps = _dumps(tmp_path, "6142", _facts("1.1.0.6143-final"))
    with pytest.raises(B.UnknownBuild) as err:
        B.load_facts(game, dumps)
    found = os.path.join(dumps, "6142", "facts.json")
    assert str(err.value) == ("unknown build v1.1.0.6142-final: no facts for it; run dump_build.py "
                              f"and dump_facts.py ({found} describes 1.1.0.6143-final)")


def test_load_facts_refuses_facts_with_no_version(tmp_path):
    game = _game(tmp_path, V6142)
    facts = _facts()
    del facts["game_version"]
    dumps = _dumps(tmp_path, "6142", facts)
    with pytest.raises(B.UnknownBuild):
        B.load_facts(game, dumps)


def test_engine_class_gives_the_name_and_ctor_rva():
    assert B.engine_class(_facts()) == (ENGINE, 0x20DF680)


def test_method_name_by_rva_names_the_one_match():
    assert B.method_name_by_rva(_facts(), ENGINE, 0x20DEEC0) == "points"
    assert B.method_names_by_rva(_facts(), ENGINE, [0x20DE770, 0x20DEEC0]) == ["dynamics", "points"]


def test_method_name_by_rva_refuses_no_match():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva(_facts(), ENGINE, 0x123)
    assert str(err.value) == f"rva 0x123 matched 0 methods of {ENGINE}"


def test_method_name_by_rva_refuses_two_matches():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva(_facts(), ENGINE, 0x500)
    assert str(err.value) == f"rva 0x500 matched 2 methods of {ENGINE}"


def test_method_name_by_rva_refuses_a_class_two_entries_carry():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva(_facts(), TWIN, 0x600)
    assert str(err.value) == f"class {TWIN} matched 2 classes"


def test_method_name_by_rva_refuses_a_class_none_carries():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva(_facts(), "Nobody", 0x20DEEC0)
    assert str(err.value) == "class Nobody matched 0 classes"


def test_method_name_by_rva_refuses_a_base_class_method_as_zero_matches():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva(_facts(), BASE, 0x100E6E0)
    assert str(err.value) == f"rva 0x100E6E0 matched 0 methods of {BASE}"


def test_both_refusals_share_one_base():
    assert issubclass(B.UnknownBuild, B.BuildFactsError)
    assert issubclass(B.FactsLookupError, B.BuildFactsError)


REAL_FACTS = os.path.join(B.DUMPS_ROOT, "6142", B.FACTS_FILE)


def test_real_6142_facts_give_the_probe_engine_ctor():
    if not os.path.isfile(REAL_FACTS):
        pytest.skip(f"no real facts file at {REAL_FACTS}")
    with open(REAL_FACTS, encoding="utf-8") as f:
        facts = json.load(f)
    _, ctor_rva = B.engine_class(facts)
    assert ctor_rva == C.RVA_DRUMS_ENGINE_CTOR
