// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Grid_Connectivity_Frontier.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_connectivity_frontier.md
//
// API一覧
// GridConnectivityFrontier(h): 1<=h<=14の盤面で、列ごとに上から走査する境界を扱う。
// State{}: まだ黒マスのない境界。Stateは同じ高さで生成した値を使う。
// place(state, row, black, mark=false): 1マス処理して{state,closed_group}。消えた成分がなければclosed_group=0。
// label(state,row): 白は0、黒は連結成分の番号。印のある成分はkMarkedGroup。O(1)。
// 黒マス1個に印を付けると、併合で印を引き継ぐ。placeはO(h)時間・領域。
// grid_corner_connected_colorings(h,w,mod): 左上と右下が黒で上下左右に連結な塗り方の数をmodで返す。
// H=min(h,w), W=max(h,w)、途中の境界状態数の最大をSとすると期待O(H^2 W S)時間、O(S)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_CONNECTIVITY_FRONTIER_DEFINED
#define SIRKIT_GRID_CONNECTIVITY_FRONTIER_DEFINED

class GridConnectivityFrontier
{
public:
    using State = std::uint64_t;
    static constexpr int kMaxHeight = 14;
    static constexpr int kMarkedGroup = 15;

    struct Transition
    {
        State state;
        int closed_group;
    };

    explicit GridConnectivityFrontier(int height)
        : height_(height)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (height < 1 || height > kMaxHeight)
        {
            throw std::invalid_argument("GridConnectivityFrontier: height out of range");
        }
#endif
    }

    [[nodiscard]] int label(State state, int row) const
    {
        CheckRow(row);
        return Decode(state, row);
    }

    [[nodiscard]] Transition place(State state, int row, bool black, bool mark = false) const
    {
        CheckRow(row);
        std::array<int, kMaxHeight> groups{};
        const auto end = groups.begin() + height_;
        for (int i = 0; i < height_; ++i)
        {
            groups[i] = Decode(state, i);
        }
        const int above = row == 0 ? 0 : groups[row - 1];
        const int previous = groups[row];
#ifndef SKIP_RUNTIME_CHECKS
        if (mark && (!black || (previous != kMarkedGroup && above != kMarkedGroup &&
                                std::find(groups.begin(), end, kMarkedGroup) != end)))
        {
            throw std::invalid_argument("GridConnectivityFrontier: cannot mark this cell");
        }
#endif
        int closed = 0;
        if (black)
        {
            const int group = mark || previous == kMarkedGroup || above == kMarkedGroup
                                  ? kMarkedGroup
                              : previous != 0 ? previous
                              : above != 0    ? above
                                              : row + 1;
            for (auto it = groups.begin(); it != end; ++it)
            {
                if (*it != 0 && (*it == previous || *it == above))
                {
                    *it = group;
                }
            }
            groups[row] = group;
        }
        else
        {
            groups[row] = 0;
            if (previous != 0 && std::find(groups.begin(), end, previous) == end)
            {
                closed = previous;
            }
        }

        // 普通の成分番号は境界に現れる最初の行から決める。
        std::array<int, kMarkedGroup + 1> canonical{};
        canonical[kMarkedGroup] = kMarkedGroup;
        State next = 0;
        for (int i = 0; i < height_; ++i)
        {
            const int group = groups[i];
            if (group == 0)
            {
                continue;
            }
            if (canonical[group] == 0)
            {
                canonical[group] = i + 1;
            }
            next |= static_cast<State>(canonical[group]) << (4 * i);
        }
        return {next, closed};
    }

private:
    int height_;

    [[nodiscard]] static int Decode(State state, int row) noexcept
    {
        return static_cast<int>((state >> (4 * row)) & 15U);
    }

    void CheckRow([[maybe_unused]] int row) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (row < 0 || row >= height_)
        {
            throw std::out_of_range("GridConnectivityFrontier: row out of range");
        }
#endif
    }
};

[[nodiscard]] inline std::uint32_t grid_corner_connected_colorings(int height, int width,
                                                                   std::uint32_t modulus = 1000000007)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (height < 1 || width < 1 || modulus == 0)
    {
        throw std::invalid_argument("grid_corner_connected_colorings: invalid dimension or modulus");
    }
#endif
    const int smaller = std::min(height, width), larger = std::max(height, width);
    const GridConnectivityFrontier grid(smaller);
    std::unordered_map<GridConnectivityFrontier::State, std::uint32_t> current{{0, 1 % modulus}}, next;
    for (int column = 0; column < larger; ++column)
    {
        for (int row = 0; row < smaller; ++row)
        {
            const bool first = column == 0 && row == 0;
            const bool last = column + 1 == larger && row + 1 == smaller;
            next.clear();
            const auto add = [&](GridConnectivityFrontier::State state, std::uint32_t count)
            {
                auto &value = next[state];
                const std::uint64_t sum = static_cast<std::uint64_t>(value) + count;
                value = static_cast<std::uint32_t>(sum >= modulus ? sum - modulus : sum);
            };
            for (const auto &[state, count] : current)
            {
                const auto black = grid.place(state, row, true, first);
                if (!last || grid.label(black.state, row) == GridConnectivityFrontier::kMarkedGroup)
                {
                    add(black.state, count);
                }
                if (!first && !last)
                {
                    const auto white = grid.place(state, row, false);
                    if (white.closed_group != GridConnectivityFrontier::kMarkedGroup)
                    {
                        add(white.state, count);
                    }
                }
            }
            current.swap(next);
        }
    }
    std::uint64_t answer = 0;
    for (const auto &[state, count] : current)
    {
        answer += count;
        if (answer >= modulus)
        {
            answer -= modulus;
        }
    }
    return static_cast<std::uint32_t>(answer);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
