// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Sliding_Window_Extrema.cpp"

#include <array>
#include <cassert>
#include <random>
#include <span>

using Values = std::vector<int>;
using Matrix = std::vector<Values>;

template <class Compare>
static std::vector<std::size_t> brute_indices(const Values &values, std::size_t width, const Compare &compare)
{
    std::vector<std::size_t> answer;
    if (width == 0 || width > values.size())
    {
        return answer;
    }
    for (std::size_t i = 0; i + width <= values.size(); ++i)
    {
        std::size_t pick = i;
        for (std::size_t j = i + 1; j < i + width; ++j)
        {
            if (compare(values[j], values[pick]))
            {
                pick = j;
            }
        }
        answer.push_back(pick);
    }
    return answer;
}

template <class Compare>
static Matrix brute_rectangles(const Matrix &matrix, std::size_t height, std::size_t width, const Compare &compare)
{
    if (matrix.empty() || height == 0 || width == 0 || height > matrix.size() || width > matrix.front().size())
    {
        return {};
    }
    Matrix answer(matrix.size() - height + 1);
    for (std::size_t y = 0; y < answer.size(); ++y)
    {
        for (std::size_t x = 0; x + width <= matrix.front().size(); ++x)
        {
            int best = matrix[y][x];
            for (std::size_t dy = 0; dy < height; ++dy)
            {
                for (std::size_t dx = 0; dx < width; ++dx)
                {
                    if (compare(matrix[y + dy][x + dx], best))
                    {
                        best = matrix[y + dy][x + dx];
                    }
                }
            }
            answer[y].push_back(best);
        }
    }
    return answer;
}

int main()
{
    const Values a{1, 7, 7, 4, 8, 1, 6};
    assert((sirkit::sliding_window_extrema(a, 3) == Values{1, 4, 4, 1, 1}));
    assert((sirkit::sliding_window_argextrema(a, 3, std::greater<>{}) == std::vector<std::size_t>{1, 1, 4, 4, 4}));
    assert(sirkit::sliding_window_extrema(a, 0).empty());
    assert(sirkit::sliding_window_extrema(a, a.size() + 1).empty());
    assert(sirkit::sliding_window_argextrema(Values{}, 1).empty());
    assert(sirkit::sliding_window_extrema(std::span{a}, 1) == a);
    const int raw[] = {4, 2, 8, 1};
    assert((sirkit::sliding_window_extrema(raw, std::size(raw)) == Values{1}));
    assert((sirkit::sliding_window_extrema(std::views::iota(0, 9), 4) == Values{0, 1, 2, 3, 4, 5}));
    const std::vector<bool> flags{true, true, false, true};
    assert((sirkit::sliding_window_extrema(flags, 2) == std::vector<bool>{true, false, false}));

    const Matrix m{{1, 9, 3}, {6, 2, 7}, {4, 8, 5}};
    assert((sirkit::sliding_window_extrema_2d(m, 2, 2) == Matrix{{1, 2}, {2, 2}}));
    assert((sirkit::sliding_window_extrema_2d(m, 2, 2, std::greater<>{}) == Matrix{{9, 9}, {8, 8}}));
    assert(sirkit::sliding_window_extrema_2d(m, 1, 1) == m);
    assert(sirkit::sliding_window_extrema_2d(m, 0, 2).empty());
    assert(sirkit::sliding_window_extrema_2d(m, 2, 0).empty());
    assert(sirkit::sliding_window_extrema_2d(m, 4, 2).empty());
    assert(sirkit::sliding_window_extrema_2d(m, 2, 4).empty());
    assert(sirkit::sliding_window_extrema_2d(Matrix{}, 1, 1).empty());
    assert(sirkit::sliding_window_extrema_2d(Matrix(2), 1, 1).empty());

    struct Item
    {
        int key;
        int identifier;
        bool operator==(const Item &) const = default;
    };
    const auto before = [](const Item &lhs, const Item &rhs) { return lhs.key < rhs.key; };
    const std::array<Item, 4> items{{{2, 0}, {1, 1}, {1, 2}, {0, 3}}};
    assert((sirkit::sliding_window_extrema(items, 2, before) == std::vector<Item>{{1, 1}, {1, 1}, {0, 3}}));
    const std::vector<std::vector<Item>> cells{{{3, 0}, {1, 1}, {1, 2}}, {{1, 3}, {1, 4}, {0, 5}}};
    assert((sirkit::sliding_window_extrema_2d(cells, 2, 2, before) ==
            std::vector<std::vector<Item>>{{{1, 1}, {0, 5}}}));

#ifndef SKIP_RUNTIME_CHECKS
    bool checked = false;
    try
    {
        (void)sirkit::sliding_window_extrema_2d(Matrix{{1, 2}, {3}}, 1, 1);
    }
    catch (const std::invalid_argument &)
    {
        checked = true;
    }
    assert(checked);
#endif

    std::mt19937 rng(1873);
    for (int trial = 0; trial < 3600; ++trial)
    {
        Values row(rng() % 60);
        for (int &value : row)
        {
            value = static_cast<int>(rng() % 17) - 8;
        }
        const auto width = rng() % (row.size() + 2);
        assert(sirkit::sliding_window_argextrema(row, width) == brute_indices(row, width, std::less<>{}));
        assert(sirkit::sliding_window_argextrema(row, width, std::greater<>{}) == brute_indices(row, width, std::greater<>{}));
    }
    for (int trial = 0; trial < 2400; ++trial)
    {
        const auto width = rng() % 12;
        Matrix matrix(rng() % 11, Values(width));
        for (auto &row : matrix)
        {
            for (int &value : row)
            {
                value = static_cast<int>(rng() % 17) - 8;
            }
        }
        const auto window_height = rng() % (matrix.size() + 2);
        const auto window_width = rng() % (width + 2);
        assert(sirkit::sliding_window_extrema_2d(matrix, window_height, window_width) ==
               brute_rectangles(matrix, window_height, window_width, std::less<>{}));
        assert(sirkit::sliding_window_extrema_2d(matrix, window_height, window_width, std::greater<>{}) ==
               brute_rectangles(matrix, window_height, window_width, std::greater<>{}));
    }
}
