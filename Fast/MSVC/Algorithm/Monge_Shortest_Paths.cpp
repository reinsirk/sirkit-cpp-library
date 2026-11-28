// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Monge_Shortest_Paths.cpp
// 参考（Larmore・Schieberのオンライン行列探索）: https://web.cs.unlv.edu/larmore/Research/ltol.02.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monge_shortest_paths.md
//
// API一覧
// monge_shortest_paths(n,cost,initial={}): 0..n-1の頂点でj<iの辺j→iを持つDAGの頂点0からの最短路。
// cost(a,c)+cost(b,d)<=cost(a,d)+cost(b,c)（a<b<c<d）が必要。
// cost(j,i)はj<iかつjの距離を確定したあとだけ呼ぶ。iの呼び出し順は単調とは限らない。
// distances[i]は最短距離、predecessors[i]は同値のとき最小の直前頂点。始点の直前頂点は-1。
// cost・加算・比較がO(1)なら時間・領域ともO(n+1)。加算と比較は正確に行えること。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONGE_SHORTEST_PATHS_DEFINED
#define SIRKIT_MONGE_SHORTEST_PATHS_DEFINED

template<class T>
struct MongeShortestPathResult
{
    std::vector<T> distances;
    std::vector<int> predecessors;
};

namespace detail
{
template<class T, class Cost>
class OnlineMongeRows
{
public:
    OnlineMongeRows(int rows, const std::vector<T> &distance, Cost &cost)
        : distance_(distance), cost_(cost)
    {
        int stride = 1;
        for (;;)
        {
            levels_.emplace_back(rows, stride);
            if (rows < 2)
            {
                break;
            }
            rows /= 2;
            stride *= 2;
        }
    }

    T evaluate(int from, int to) const
    {
        return distance_[from] + std::invoke(cost_, from, to);
    }

    int next()
    {
        return NextRow(0);
    }

private:
    struct Column
    {
        int local;
        int original;
    };

    struct Level
    {
        int rows;
        int stride;
        int next_row = 0;
        int previous = 0;
        int pending = 0;
        std::vector<Column> reduced;

        Level(int row_count, int row_stride) : rows(row_count), stride(row_stride)
        {
            reduced.reserve(rows / 2);
        }
    };

    const std::vector<T> &distance_;
    Cost &cost_;
    std::vector<Level> levels_;

    int OriginalColumn(int level, int column) const
    {
        return level == 0 ? column : levels_[level - 1].reduced[column].original;
    }

    bool Better(int level, int row, int current, int candidate) const
    {
        const int to = (row + 1) * levels_[level].stride;
        return evaluate(OriginalColumn(level, candidate), to) <
               evaluate(OriginalColumn(level, current), to);
    }

    void ReduceColumns(int level, int row)
    {
        auto &state = levels_[level];
        const int first = row == 0 ? 0 : row - 1;
        for (int candidate = first; candidate <= row; ++candidate)
        {
            while (!state.reduced.empty())
            {
                const int k = static_cast<int>(state.reduced.size()) - 1;
                // 下位のk行は元の2k+1行で、2k列までを使う。
                // 新しい列が範囲外なら、それまでに確定した列を取り除かない。
                if (candidate > 2 * k || !Better(level, 2 * k + 1, state.reduced.back().local, candidate))
                {
                    break;
                }
                state.reduced.pop_back();
            }
            if (state.reduced.size() < static_cast<std::size_t>(state.rows / 2))
            {
                state.reduced.push_back({candidate, OriginalColumn(level, candidate)});
            }
        }
    }

    int NextRow(int level)
    {
        auto &state = levels_[level];
        const int row = state.next_row++;
        if (row % 2 != 0)
        {
            // 直前の処理では未確定だった最後の列だけを比べる。
            state.previous = Better(level, row, state.pending, row) ? row : state.pending;
            return state.previous;
        }

        int upper = row;
        if (row + 1 < state.rows)
        {
            ReduceColumns(level, row);
            const int reduced = NextRow(level + 1);
            upper = state.reduced[reduced].local;
        }
        state.pending = upper;
        // 前の行と、現在の列だけを使った次の行の答えが探索範囲を挟む。
        // 各段でこの範囲を左から右へ詰めるため、走査の合計は行数に比例する。
        int best = state.previous;
        for (int column = best + 1; column <= upper; ++column)
        {
            if (Better(level, row, best, column))
            {
                best = column;
            }
        }
        state.previous = best;
        return best;
    }
};
} // namespace detail

template<class Cost, class T = std::remove_cvref_t<std::invoke_result_t<Cost &, int, int>>>
MongeShortestPathResult<T> monge_shortest_paths(int n, Cost cost, std::type_identity_t<T> initial = {})
{
    MongeShortestPathResult<T> result;
    if (n == 0)
    {
        return result;
    }
    result.distances.reserve(n);
    result.predecessors.reserve(n);
    result.distances.push_back(std::move(initial));
    result.predecessors.push_back(-1);
    detail::OnlineMongeRows<T, Cost> rows(n - 1, result.distances, cost);
    for (int to = 1; to < n; ++to)
    {
        const int from = rows.next();
        result.predecessors.push_back(from);
        result.distances.push_back(rows.evaluate(from, to));
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
