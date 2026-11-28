// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Grid_Pattern_Matching.cpp
// 参考: R. S. Bird, Two dimensional pattern matching (1977).
// https://ora.ox.ac.uk/objects/uuid:8831b471-0180-41d7-b60e-f590cd428368
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_pattern_matching.md
//
// API一覧
// find_grid_pattern(text, pattern): 長方形の完全一致の左上座標 {行, 列} を0始まりの行優先順で返す。
// 各行はstd::string_viewへ変換できるバイト列。重なりも列挙する。高さまたは幅が0なら空配列を返す。
// 行の長さが不揃いなら検査有効時にstd::invalid_argument。
// 時間O(H+R+σ(HW+RC)+K)、追加領域O(RC+W+K)。盤面H×W、パターンR×C、一致数K、
// σはパターンに含まれる異なるバイト数（非空なら1..256）。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_PATTERN_MATCHING_DEFINED
#define SIRKIT_GRID_PATTERN_MATCHING_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class Grid>
concept GridPatternRows = requires(const Grid &grid, std::size_t index) {
    static_cast<std::size_t>(grid.size());
    std::string_view{grid[index]};
};

template <GridPatternRows Grid>
std::size_t grid_pattern_width(const Grid &grid)
{
    if (grid.size() == 0)
    {
        return 0;
    }
    const auto width = std::string_view{grid[0]}.size();
#ifndef SKIP_RUNTIME_CHECKS
    for (std::size_t row = 1; row < static_cast<std::size_t>(grid.size()); ++row)
    {
        if (std::string_view{grid[row]}.size() != width)
        {
            throw std::invalid_argument("find_grid_pattern: ragged grid");
        }
    }
#endif
    return width;
}

// 行の幅が共通なので、最長一致の状態番号をそのまま行の識別子にできる。
class GridPatternRowAutomaton
{
    struct Node
    {
        std::size_t first_child = 0;
        std::size_t next_sibling = 0;
        std::size_t failure = 0;
        unsigned char byte = 0;
    };

    std::array<std::size_t, 256> root_children_{};
    std::vector<Node> nodes_{1};

    [[nodiscard]] std::size_t child(std::size_t state, unsigned char byte) const
    {
        if (state == 0)
        {
            return root_children_[byte];
        }
        for (auto next = nodes_[state].first_child; next != 0; next = nodes_[next].next_sibling)
        {
            if (nodes_[next].byte == byte)
            {
                return next;
            }
        }
        return 0;
    }

public:
    std::size_t add(std::string_view row)
    {
        std::size_t state = 0;
        for (const unsigned char byte : row)
        {
            auto next = child(state, byte);
            if (next == 0)
            {
                next = nodes_.size();
                nodes_.push_back({0, nodes_[state].first_child, 0, byte});
                if (state == 0)
                {
                    root_children_[byte] = next;
                }
                else
                {
                    nodes_[state].first_child = next;
                }
            }
            state = next;
        }
        return state;
    }

    void build()
    {
        std::vector<std::size_t> order;
        order.reserve(nodes_.size() - 1);
        for (const auto state : root_children_)
        {
            if (state != 0)
            {
                order.push_back(state);
            }
        }
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const auto state = order[i];
            for (auto next = nodes_[state].first_child; next != 0; next = nodes_[next].next_sibling)
            {
                auto &node = nodes_[next];
                node.failure = step(nodes_[state].failure, node.byte);
                order.push_back(next);
            }
        }
    }

    [[nodiscard]] std::size_t step(std::size_t state, unsigned char byte) const
    {
        auto next = child(state, byte);
        while (next == 0 && state != 0)
        {
            state = nodes_[state].failure;
            next = child(state, byte);
        }
        return next;
    }
};
} // namespace detail

template <detail::GridPatternRows Text, detail::GridPatternRows Pattern>
[[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>> find_grid_pattern(const Text &text, const Pattern &pattern)
{
    const auto text_width = detail::grid_pattern_width(text);
    const auto pattern_width = detail::grid_pattern_width(pattern);
    const auto text_height = static_cast<std::size_t>(text.size());
    const auto pattern_height = static_cast<std::size_t>(pattern.size());
    if (pattern_width == 0 || pattern_height > text_height || pattern_width > text_width)
    {
        return {};
    }

    detail::GridPatternRowAutomaton automaton;
    std::vector<std::size_t> row_ids;
    row_ids.reserve(pattern_height);
    for (std::size_t y = 0; y < pattern_height; ++y)
    {
        row_ids.push_back(automaton.add(std::string_view{pattern[y]}));
    }
    automaton.build();

    std::vector<std::size_t> border(pattern_height);
    for (std::size_t i = 1; i < pattern_height; ++i)
    {
        auto matched = border[i - 1];
        while (matched != 0 && row_ids[matched] != row_ids[i])
        {
            matched = border[matched - 1];
        }
        border[i] = matched + (row_ids[matched] == row_ids[i]);
    }

    const auto first_end = pattern_width - 1;
    std::vector<std::size_t> column_matches(text_width - first_end);
    std::vector<std::pair<std::size_t, std::size_t>> result;
    for (std::size_t y = 0; y < text_height; ++y)
    {
        const auto &row = text[y];
        const std::string_view bytes{row};
        std::size_t state = 0;
        for (std::size_t x = 0; x < first_end; ++x)
        {
            state = automaton.step(state, bytes[x]);
        }
        for (std::size_t x = first_end; x < text_width; ++x)
        {
            state = automaton.step(state, bytes[x]);
            auto &matched = column_matches[x - first_end];
            while (matched != 0 && row_ids[matched] != state)
            {
                matched = border[matched - 1];
            }
            matched += (row_ids[matched] == state);
            if (matched == pattern_height)
            {
                result.emplace_back(y - (pattern_height - 1), x - first_end);
                matched = border[matched - 1];
            }
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
