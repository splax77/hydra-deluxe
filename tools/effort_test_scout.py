"""Checks scout answers from the effort test against the question's key.

An answer passes when its text holds the key's path and the key's quoted line, exactly. The
line's own indent does not count, and a path may sit inside a longer absolute path, because a
scout may quote either form. Case counts in both.

Run it with:
    py -I tools/effort_test_scout.py <questions.json> <question id> <answer.txt> [<answer.txt> ...]

It prints one line per answer file and exits 0 once every file is checked, whatever the results.
It exits 2 when the question or a file cannot be read.
"""

import argparse
import json
import re
import sys


def find_question(path, question_id):
    with open(path, encoding="utf-8") as f:
        questions = json.load(f)
    for question in questions:
        if question["id"] == question_id:
            if not question["path"] or not question["line_text"].strip():
                raise ValueError("question %s has an empty key" % question_id)
            return question
    raise ValueError("no question %s in %s" % (question_id, path))


def has_path(answer, path):
    name = re.escape(path.replace("\\", "/"))
    return bool(re.search(r"(?<![\w.-])" + name + r"(?![\w-]|\.\w)", answer.replace("\\", "/")))


def has_line(answer, line_text):
    return line_text.strip() in answer


def check(question, answer):
    path_ok = has_path(answer, question["path"])
    line_ok = has_line(answer, question["line_text"])
    verdict = "pass" if path_ok and line_ok else "fail"
    return "%s (path %s, line %s)" % (verdict, "found" if path_ok else "not found",
                                      "found" if line_ok else "not found")


def main():
    parser = argparse.ArgumentParser(description="Check scout answers against the key.")
    parser.add_argument("questions", help="the test's questions.json")
    parser.add_argument("id", help="the question's id, such as Q1")
    parser.add_argument("answers", nargs="+", help="text files, one scout's answer each")
    args = parser.parse_args()
    try:
        question = find_question(args.questions, args.id)
        lines = []
        for path in args.answers:
            with open(path, encoding="utf-8") as f:
                lines.append("%s: %s" % (path, check(question, f.read())))
    except (OSError, ValueError, KeyError, TypeError) as e:
        print("cannot check: %s" % e, file=sys.stderr)
        return 2
    for line in lines:
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
