// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Grid_Pattern_Matching.cpp"

#include <array>
#include <cassert>
#include <random>
#include <span>
#include <string>

using Positions = std::vector<std::pair<std::size_t, std::size_t>>;
using Grid = std::vector<std::string>;

static Positions brute(const Grid &text, const Grid &pattern)
{
    Positions positions;
    if (text.empty() || pattern.empty() || pattern.front().empty() ||
        text.size() < pattern.size() || text.front().size() < pattern.front().size())
    {
        return positions;
    }
    for (std::size_t y = 0; y + pattern.size() <= text.size(); ++y)
    {
        for (std::size_t x = 0; x + pattern.front().size() <= text.front().size(); ++x)
        {
            bool same = true;
            for (std::size_t i = 0; i < pattern.size(); ++i)
            {
                for (std::size_t j = 0; j < pattern[i].size(); ++j)
                {
                    same &= text[y + i][x + j] == pattern[i][j];
                }
            }
            if (same)
            {
                positions.emplace_back(y, x);
            }
        }
    }
    return positions;
}

struct RowsByValue
{
    Grid rows;

    std::size_t size() const
    {
        return rows.size();
    }

    std::string operator[](std::size_t i) const
    {
        return rows[i];
    }
};

int main()
{
    const Grid sample{"ababa", "babab", "ababa"};
    const Grid fragment{"aba", "bab"};
    assert((sirkit::find_grid_pattern(sample, fragment) == Positions{{0, 0}, {0, 2}, {1, 1}}));
    assert(sirkit::find_grid_pattern(Grid{}, fragment).empty());
    assert(sirkit::find_grid_pattern(sample, Grid{}).empty());
    assert(sirkit::find_grid_pattern(sample, Grid(2, "")).empty());
    assert(sirkit::find_grid_pattern(Grid(1, ""), Grid{"a"}).empty());
    assert(sirkit::find_grid_pattern(Grid{"a"}, Grid{"a", "a"}).empty());
    assert(sirkit::find_grid_pattern(sample, Grid{"ababab"}).empty());
    assert((sirkit::find_grid_pattern(sample, sample) == Positions{{0, 0}}));
    assert((sirkit::find_grid_pattern(Grid{"abaabaaba"}, Grid{"aba"}) == Positions{{0, 0}, {0, 3}, {0, 6}}));
    assert((sirkit::find_grid_pattern(Grid{"aaaaaa"}, Grid{"aaa"}) == Positions{{0, 0}, {0, 1}, {0, 2}, {0, 3}}));

    const Grid column{"a", "a", "a", "b", "a", "a", "b", "a", "b", "a", "a", "c", "a"};
    const Grid column_pattern{"a", "a", "b", "a"};
    assert((sirkit::find_grid_pattern(column, column_pattern) == Positions{{1, 0}, {4, 0}}));

    std::string alphabet;
    for (unsigned int value = 0; value != 256; ++value)
    {
        alphabet.push_back(static_cast<char>(value));
    }
    const std::array<std::string_view, 2> bytes{alphabet, alphabet};
    const std::string_view snippet = std::string_view{alphabet}.substr(127, 6);
    const std::array<std::string_view, 2> byte_pattern{snippet, snippet};
    assert((sirkit::find_grid_pattern(std::span{bytes}, byte_pattern) == Positions{{0, 127}}));
    const std::array<const char *, 2> literals{"bcd", "xyz"};
    assert((sirkit::find_grid_pattern(RowsByValue{{"abcde", "wxyzz"}}, literals) == Positions{{0, 1}}));

#ifndef SKIP_RUNTIME_CHECKS
    bool bad_text = false;
    try
    {
        (void)sirkit::find_grid_pattern(Grid{"ab", "x"}, fragment);
    }
    catch (const std::invalid_argument &)
    {
        bad_text = true;
    }
    assert(bad_text);

    bool bad_pattern = false;
    try
    {
        (void)sirkit::find_grid_pattern(sample, Grid{"ab", "x"});
    }
    catch (const std::invalid_argument &)
    {
        bad_pattern = true;
    }
    assert(bad_pattern);
#endif

    std::mt19937 rng(91283);
    const auto random_grid = [&rng](std::size_t height, std::size_t width, unsigned int sigma) {
        Grid grid(height, std::string(width, '\0'));
        for (auto &row : grid)
        {
            for (auto &byte : row)
            {
                byte = static_cast<char>(rng() % sigma);
            }
        }
        return grid;
    };
    for (int test = 0; test < 6000; ++test)
    {
        const auto height = rng() % 10;
        const auto width = rng() % 10;
        const auto sigma = 1 + rng() % 12;
        const auto text = random_grid(height, width, sigma);
        auto pattern = random_grid(rng() % 7, rng() % 7, sigma);
        if (height != 0 && width != 0 && test % 2 == 0)
        {
            const auto y = rng() % height;
            const auto x = rng() % width;
            const auto r = 1 + rng() % (height - y);
            const auto c = 1 + rng() % (width - x);
            pattern.resize(r);
            for (std::size_t i = 0; i < r; ++i)
            {
                pattern[i] = text[y + i].substr(x, c);
            }
        }
        assert(sirkit::find_grid_pattern(text, pattern) == brute(text, pattern));
    }
}
