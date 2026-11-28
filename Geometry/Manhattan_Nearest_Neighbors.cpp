// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/manhattan_nearest_neighbors.md
//
// API一覧
// geo::ManhattanPoint: {x,y}を表すstd::array<std::int32_t,2>。
// geo::ManhattanNeighbor{distance,index}: L1距離と入力の0始まりの添字。
// geo::manhattan_nearest_neighbors(points): 各点で自分以外の最近点を入力順に返す。
// 同距離なら最小の添字を選ぶ。同じ座標の別の点は距離0。相手がいないならnullopt。
// 点数nに対しO(n log(n+1))時間・O(n)領域。座標はint32_tの全域を扱う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MANHATTAN_NEAREST_NEIGHBORS_DEFINED
#define SIRKIT_MANHATTAN_NEAREST_NEIGHBORS_DEFINED

namespace geo
{
using ManhattanPoint = std::array<std::int32_t, 2>;

struct ManhattanNeighbor
{
    std::uint64_t distance;
    std::size_t index;
};

inline std::vector<std::optional<ManhattanNeighbor>> manhattan_nearest_neighbors(std::span<const ManhattanPoint> points)
{
    const auto n = points.size();
    std::vector<std::optional<ManhattanNeighbor>> answer(n);
    if (n <= 1)
    {
        return answer;
    }

    struct SweepPoint
    {
        std::int64_t x;
        std::int64_t y;
        std::size_t index;
    };
    using Candidate = std::pair<std::int64_t, std::size_t>;
    constexpr Candidate kEmpty{std::numeric_limits<std::int64_t>::min(), 0};
    std::vector<SweepPoint> events(n);
    std::vector<std::int64_t> ys;
    ys.reserve(n);
    std::vector<Candidate> fenwick(n + 1);

    for (const int sx : {1, -1})
    {
        for (const int sy : {1, -1})
        {
            ys.clear();
            for (std::size_t i = 0; i < n; ++i)
            {
                const auto x = static_cast<std::int64_t>(points[i][0]) * sx;
                const auto y = static_cast<std::int64_t>(points[i][1]) * sy;
                events[i] = {x, y, i};
                ys.push_back(y);
            }

            std::sort(ys.begin(), ys.end());
            ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
            // 同一座標の点は順序も反転する。これで各点から同じ座標の他点も候補になる。
            std::sort(events.begin(), events.end(), [sx](const SweepPoint &a, const SweepPoint &b)
                      {
                          if (std::tie(a.x, a.y) != std::tie(b.x, b.y))
                          {
                              return std::tie(a.x, a.y) < std::tie(b.x, b.y);
                          }
                          return sx > 0 ? a.index < b.index : a.index > b.index;
                      });

            std::fill(fenwick.begin(), fenwick.end(), kEmpty);
            for (const auto &[x, y, index] : events)
            {
                const auto pos = static_cast<std::size_t>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin()) + 1;
                Candidate best = kEmpty;
                // この象限では距離 = (x+y) - (候補のx+候補のy)。
                for (std::size_t j = pos; j > 0; j &= j - 1)
                {
                    best = std::max(best, fenwick[j]);
                }
                if (best.second != 0)
                {
                    const ManhattanNeighbor current{static_cast<std::uint64_t>(x + y - best.first), n - best.second};
                    auto &old = answer[index];
                    if (!old || std::tie(current.distance, current.index) < std::tie(old->distance, old->index))
                    {
                        old = current;
                    }
                }
                const Candidate self{x + y, n - index};
                for (std::size_t j = pos; j <= ys.size(); j += j & (std::size_t{0} - j))
                {
                    fenwick[j] = std::max(fenwick[j], self);
                }
            }
        }
    }
    return answer;
}
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
