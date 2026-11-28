// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Edit_Alignment.cpp"

#include <array>
#include <cassert>
#include <random>
#include <string>
#include <vector>

namespace
{
template <class A, class B, class Equal>
void check(const A &a, const B &b, Equal equal)
{
    const auto result = sirkit::edit_alignment(std::span{a}, std::span{b}, equal);
    std::vector<std::vector<std::size_t>> distance(a.size() + 1, std::vector<std::size_t>(b.size() + 1));
    for (std::size_t i = 0; i <= a.size(); ++i)
    {
        for (std::size_t j = 0; j <= b.size(); ++j)
        {
            if (i == 0 || j == 0)
            {
                distance[i][j] = i + j;
            }
            else
            {
                distance[i][j] = std::min({distance[i - 1][j] + 1, distance[i][j - 1] + 1,
                                           distance[i - 1][j - 1] + !equal(a[i - 1], b[j - 1])});
            }
        }
    }
    assert(result.distance == distance.back().back());
    std::size_t i = 0, j = 0, edits = 0;
    for (const auto operation : result.steps)
    {
        switch (operation)
        {
        case sirkit::EditAction::kMatch:
            assert(i < a.size() && j < b.size() && equal(a[i], b[j]));
            ++i;
            ++j;
            break;
        case sirkit::EditAction::kReplace:
            assert(i < a.size() && j < b.size() && !equal(a[i], b[j]));
            ++i;
            ++j;
            ++edits;
            break;
        case sirkit::EditAction::kErase:
            assert(i < a.size());
            ++i;
            ++edits;
            break;
        case sirkit::EditAction::kInsert:
            assert(j < b.size());
            ++j;
            ++edits;
            break;
        }
    }
    assert(i == a.size() && j == b.size() && edits == result.distance);
}
}

int main()
{
    const std::equal_to<> equal;
    check(std::string{}, std::string{}, equal);
    check(std::string{"a"}, std::string{}, equal);
    check(std::string{}, std::string{"abc"}, equal);
    check(std::string{"x"}, std::string{"z"}, equal);
    assert(sirkit::edit_alignment("kitten", "sitting").distance == 3);
    const std::array left{2, 5};
    const std::array right{1, 0, 2, 7, 2, 5};
    check(left, right, [](int x, int y)
          {
              return x / 2 == y;
          });
    using sirkit::detail::checked_edit_alignment_size;
    assert(checked_edit_alignment_size(2, 4, 6) == 6);
    for (const auto final : {0U, 3U})
    {
        bool caught = false;
        try
        {
            static_cast<void>(checked_edit_alignment_size(1, 3, final));
        }
        catch (const std::length_error &)
        {
            caught = true;
        }
        assert(caught);
    }
    std::mt19937 random(204177);
    for (int trial = 0; trial < 870; ++trial)
    {
        std::vector<int> a(random() % 28), b(random() % 28);
        for (auto &x : a)
        {
            x = static_cast<int>(random() % 9);
        }
        for (auto &x : b)
        {
            x = static_cast<int>(random() % 9);
        }
        check(a, b, equal);
    }
}
