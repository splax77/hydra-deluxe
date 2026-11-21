"""Tests for build_facts.py.

Most tests run on a hand-written facts dict and temp folders that stand in
for the dumps folder and the game folder, so they never touch the game. The
build-number and version-file tests read real copies under testdata/ch_runs, and
one reads this machine's installed version file. One test reads the real 6142
facts file when this machine has it.
"""

from __future__ import annotations

import json
import os

import pytest

from tools.ch_mcp import build_facts as B
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


_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
# The real 1.1 build's facts cut to its drum engine class, under the build-number folder
# the dumps use (from hydra-data/ch-dumps/6142/facts.json), and the installed game's real
# version file (C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\version.json).
REAL_6142_DUMPS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-6142")
REAL_6142_GAME = os.path.join(REAL_6142_DUMPS, "game")
# The real 1.0 build's facts, cut (from hydra-data/ch-dumps/4045/facts.json).
REAL_4045_FACTS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-4045", "facts.json")


def _read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def test_build_number_names_the_dump_folder_of_the_real_build():
    version = B.current_build(REAL_6142_GAME)
    path = B.facts_path(version, REAL_6142_DUMPS)
    assert os.path.isfile(path), f"{version} gives {path}, which is not the real dump folder"
    described = _read_json(path)["game_version"]
    # The game writes its version with a leading v and the facts without one; both
    # spellings must name the same folder.
    assert described != version
    assert B.build_number(described) == B.build_number(version)
    other = _read_json(REAL_4045_FACTS)["game_version"]
    assert B.build_number(other) != B.build_number(version)


def test_build_number_refuses_a_version_without_one():
    version = "v1.1-final"
    with pytest.raises(B.UnknownBuild) as err:
        B.build_number(version)
    assert err.value.version == version
    message = str(err.value)
    assert message.startswith(str(B.UnknownBuild(version)) + " (")
    assert "build number" in message


def test_same_build_drops_one_leading_v():
    assert B.same_build(V6142, F6142)
    assert B.same_build(F6142, V6142)
    assert B.same_build(V6142, V6142)
    assert not B.same_build("v1.0.0.4045-master", F6142)
    assert not B.same_build("vv1.1.0.6142-final", F6142)


def test_current_build_reads_the_version_as_written(tmp_path):
    assert B.current_build(_game(tmp_path, V6142)) == V6142


def test_version_file_sits_in_the_real_game_folder():
    # The copy keeps the real game folder's layout, and this machine's game folder holds
    # the file itself; current_build reads it when given no folder.
    assert os.path.isfile(B.version_file(REAL_6142_GAME))
    assert os.path.isfile(B.VERSION_FILE), f"no version file at {B.VERSION_FILE}"
    B.build_number(B.current_build())


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


def test_load_facts_refuses_facts_that_are_not_json(tmp_path):
    game = _game(tmp_path, V6142)
    dumps = _dumps(tmp_path, "6142", _facts())
    found = os.path.join(dumps, "6142", B.FACTS_FILE)
    with open(found, "w", encoding="utf-8") as f:
        f.write("{not json")
    with pytest.raises(B.UnknownBuild) as err:
        B.load_facts(game, dumps)
    assert f"{found} cannot be read: JSONDecodeError" in str(err.value)


def test_load_facts_refuses_facts_that_are_not_an_object(tmp_path):
    game = _game(tmp_path, V6142)
    dumps = _dumps(tmp_path, "6142", [1, 2])
    with pytest.raises(B.UnknownBuild) as err:
        B.load_facts(game, dumps)
    assert str(err.value).endswith("facts.json is not a facts object)")


def test_a_missing_version_file_is_a_plain_refusal(tmp_path):
    game = str(tmp_path / "game")
    with pytest.raises(B.VersionFileError) as err:
        B.current_build(game)
    assert str(err.value).startswith(
        f"cannot read the game's version from {B.version_file(game)}: FileNotFoundError")
    with pytest.raises(B.VersionFileError):
        B.load_facts(game, str(tmp_path / "dumps"))


@pytest.mark.parametrize("content", ["{broken", "{}", "[]", '{"version": 6142}'])
def test_a_version_file_with_no_text_version_is_a_plain_refusal(tmp_path, content):
    game = str(tmp_path / "game")
    path = B.version_file(game)
    os.makedirs(os.path.dirname(path))
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    with pytest.raises(B.VersionFileError) as err:
        B.current_build(game)
    assert path in str(err.value)


def test_engine_class_refuses_facts_with_no_drums_engine():
    facts = _facts()
    del facts["signatures"]["drums_engine"]
    with pytest.raises(B.FactsLookupError) as err:
        B.engine_class(facts)
    assert str(err.value) == "the facts hold no drums_engine name and ctor_rva"
    with pytest.raises(B.FactsLookupError):
        B.engine_class({"game_version": F6142})


def test_method_name_by_rva_refuses_facts_with_no_classes():
    with pytest.raises(B.FactsLookupError) as err:
        B.method_name_by_rva({"game_version": F6142}, ENGINE, 0x20DEEC0)
    assert str(err.value) == f"class {ENGINE} matched 0 classes"


def test_all_refusals_share_one_base():
    assert issubclass(B.UnknownBuild, B.BuildFactsError)
    assert issubclass(B.FactsLookupError, B.BuildFactsError)
    assert issubclass(B.VersionFileError, B.BuildFactsError)


REAL_FACTS = os.path.join(B.DUMPS_ROOT, "6142", B.FACTS_FILE)


def test_real_6142_facts_give_the_probe_engine_ctor():
    if not os.path.isfile(REAL_FACTS):
        pytest.skip(f"no real facts file at {REAL_FACTS}")
    with open(REAL_FACTS, encoding="utf-8") as f:
        facts = json.load(f)
    _, ctor_rva = B.engine_class(facts)
    assert ctor_rva == C.RVA_DRUMS_ENGINE_CTOR
