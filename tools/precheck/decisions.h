// The decisions check 3 reads: which paragraphs of the docs record a
// decision, the name each one goes by, and the numbers each one holds. Also
// the one reading of "what number does this text spell", which check 3 uses
// on code literals too, so a doc number and a code number meet as one value.
#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "git_text.h"

namespace precheck {

// Is this byte a word character as the port reads \w? ASCII letters, digits
// and the underscore (codelex's identifier rule). A UTF-8 byte never is.
bool word_byte(char c);
// Is the byte before `pos` in `s` a word byte? False at the start.
bool word_byte_before(const std::string& s, size_t pos);

// The key a number is compared by: its absolute value, written as a whole
// number when it is one (below kWholeNumberLimit in decisions.cpp), otherwise
// with 10 significant digits.
std::string number_canon(double v);

// The value a doc number or a code literal spells: digit separators (, ' _
// and space) dropped, 0x hex read as a signed 64-bit value, a C++ suffix
// (u, l, f) dropped, then a plain decimal. Nothing when it spells none.
std::optional<double> number_of(const std::string& text);

// Every number a doc text writes, by number_canon: 1,000,000, 1'000'000,
// 0xFFFF, 500.0, and "2^46" as the power it names.
std::set<std::string> doc_numbers(const std::string& text);

// A doc's paragraphs: split at blank lines and, unless `whole_lists`, before
// each list item, so a number and a word meet only inside one item. The
// D-records keep their lists whole: a ruling's list items share its words.
// Pieces that are only whitespace are dropped.
std::vector<std::string> split_paras(const std::string& text, bool whole_lists);

// The name a decision record goes by ("D49", "ADR 14"), so a citation in code
// and a record read from the docs meet under one key. `digits` is the
// record's number as written ("0014").
std::string record_name(bool adr, const std::string& digits);

// One decision paragraph: the record it belongs to, its lowercase words and
// its numbers.
struct DecisionPara {
    std::string record;
    std::set<std::string> words;
    std::set<std::string> numbers;
};
DecisionPara make_para(const std::string& record, const std::string& text);

// Every decision paragraph at each of `revs` (the range's tip and its left
// side; empty and repeated revisions are skipped): docs/adr/*.md and
// CONTEXT.md whole, the "User decisions" block and "Test limits" sections of
// docs/superpowers/plans/*.md, and the D-records of docs/audit/*decisions*.md.
std::vector<DecisionPara> decision_paras(Repo& repo, const std::vector<std::string>& revs);

// The plan reader decision_paras uses, on one plan's text: the paragraphs of
// its "User decisions" block and its "Test limits" sections.
std::vector<std::string> plan_decision_paras(const std::string& text);

}  // namespace precheck
