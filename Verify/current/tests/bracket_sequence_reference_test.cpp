// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Bracket_Sequence.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
void check_query(const sirkit::BracketSequence &brackets, const std::string &text, int left, int right)
{
    int matched = 0, opening = 0, closing = 0;
    int first_close = brackets.size();
    for (int i = left; i < right; ++i)
    {
        if (text[static_cast<std::size_t>(i)] == '(')
        {
            ++opening;
        }
        else if (opening != 0)
        {
            --opening;
            ++matched;
        }
        else
        {
            ++closing;
            first_close = std::min(first_close, i);
        }
    }
    const auto result = brackets.query(left, right);
    assert(result.matched_pairs == matched);
    assert(result.unmatched_open == opening);
    assert(result.unmatched_close == closing);
    assert(result.balanced() == (opening + closing == 0));
    assert(result.matched_length() == 2 * matched);
    assert(result.minimum_insertions() == opening + closing);
    if (right == brackets.size())
    {
        assert(brackets.first_unmatched_close(left) == first_close);
    }
    if (left == 0)
    {
        int pending = 0, last_open = -1;
        for (int i = right; i > 0;)
        {
            --i;
            if (text[static_cast<std::size_t>(i)] == ')')
            {
                ++pending;
            }
            else if (pending == 0)
            {
                last_open = i;
                break;
            }
            else
            {
                --pending;
            }
        }
        assert(brackets.last_unmatched_open(right) == last_open);
    }
}
} // namespace

int main()
{
    const sirkit::BracketSequence empty("");
    check_query(empty, "", 0, 0);
    std::mt19937 rng(5133021);
    for (const int length : {1, 2, 3, 31, 32, 53, 64, 65, 128, 129})
    {
        std::string text(static_cast<std::size_t>(length), '(');
        sirkit::BracketSequence brackets(text);
        for (int trial = 0; trial < 400; ++trial)
        {
            assert(brackets.size() == length);
            const auto first = static_cast<int>(rng() % static_cast<unsigned>(length));
            const auto second = static_cast<int>(rng() % static_cast<unsigned>(length));
            auto &a = text[static_cast<std::size_t>(first)];
            auto &b = text[static_cast<std::size_t>(second)];
            assert(brackets.get(first) == a);
            if (trial % 2 == 0)
            {
                a = rng() % 2 == 0 ? '(' : ')';
                brackets.set(first, a);
            }
            else
            {
                std::swap(a, b);
                brackets.swap(first, second);
            }
            check_query(brackets, text, std::min(first, second), std::max(first, second));
            check_query(brackets, text, first, length);
            check_query(brackets, text, 0, second + 1);
        }
    }

    bool invalid = false;
    try
    {
        const sirkit::BracketSequence checked("(b)");
    }
    catch (const std::invalid_argument &)
    {
        invalid = true;
    }
    assert(invalid);
    sirkit::BracketSequence valid("()");
    invalid = false;
    try
    {
        valid.swap(0, 2);
    }
    catch (const std::out_of_range &)
    {
        invalid = true;
    }
    assert(invalid && valid.query(0, 2).balanced());
}
