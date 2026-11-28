// SPDX-License-Identifier: CC0-1.0
#include "../String/Bracket_Sequence.cpp"
#include <cassert>

int main()
{
    sirkit::BracketSequence brackets("())(");
    assert(brackets.query(0, 2).balanced());
    assert(brackets.query(0, 4).matched_length() == 2);
    assert(brackets.first_unmatched_close(0) == 2);
    brackets.swap(2, 3);
    assert(brackets.query(0, 4).balanced());
}
