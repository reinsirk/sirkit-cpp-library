// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Functional_Graph_Fold.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/functional_graph_fold.md
//
// API一覧
// FunctionalGraphFold<T, Combine>(next, label, identity, combine={}):
//   頂点vからnext[v]へ進む辺にlabel[v]を置く。combineは結合的でidentityは両側単位元。
//   前処理と領域はO(n log(n+1))。積と値のコピーをO(1)とする。
// walk(v, k): k本の辺を進んだ頂点と、通った辺ラベルの順序付き積を返す。
//   kはuint64_t。0本では{v, identity}。O(log(n+1)+log(k+1))。
// size(): 頂点数。O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FUNCTIONAL_GRAPH_FOLD_DEFINED
#define SIRKIT_FUNCTIONAL_GRAPH_FOLD_DEFINED

namespace detail
{
inline int functional_graph_fold_vertex_count(std::size_t n)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("FunctionalGraphFold: too many vertices");
    }
    return static_cast<int>(n);
}
} // namespace detail

template <class T, class Combine = std::plus<T>>
class FunctionalGraphFold
{
    struct Jump
    {
        int end;
        T product;
    };

public:
    FunctionalGraphFold(std::span<const int> next, std::span<const T> label, T identity, Combine combine = {})
        : identity_(std::move(identity)), combine_(std::move(combine))
    {
        const int n = detail::functional_graph_fold_vertex_count(next.size());
        if (next.size() != label.size())
        {
            throw std::invalid_argument("FunctionalGraphFold: inconsistent label count");
        }
        build_periods(next, n);
        const unsigned levels = std::bit_width(static_cast<unsigned>(n));
        jumps_.reserve(next.size() * levels);
        for (int v = 0; v < n; ++v)
        {
            jumps_.push_back({next[v], label[v]});
        }
        for (unsigned level = 1; level < levels; ++level)
        {
            const std::size_t base = static_cast<std::size_t>(n) * (level - 1);
            for (int v = 0; v < n; ++v)
            {
                const auto &first = jumps_[base + v];
                const auto &second = jumps_[base + first.end];
                jumps_.push_back({second.end, combine_(first.product, second.product)});
            }
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(depth_.size());
    }

    [[nodiscard]] std::pair<int, T> walk(int from, std::uint64_t steps) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= size())
        {
            throw std::out_of_range("FunctionalGraphFold: invalid start vertex");
        }
#endif
        T product = identity_;
        if (steps <= depth_.size())
        {
            append(from, steps, product);
            return {from, std::move(product)};
        }

        const auto tail = static_cast<std::uint64_t>(depth_[from]);
        append(from, tail, product);
        steps -= tail;
        const auto period = static_cast<std::uint64_t>(period_[from]);
        T cycle = identity_;
        append(from, period, cycle);
        for (std::uint64_t copies = steps / period; copies != 0;)
        {
            if (copies & 1)
            {
                product = combine_(product, cycle);
            }
            copies >>= 1;
            if (copies != 0)
            {
                cycle = combine_(cycle, cycle);
            }
        }
        append(from, steps % period, product);
        return {from, std::move(product)};
    }

private:
    void build_periods(std::span<const int> next, int n)
    {
        std::vector<int> indegree(n);
        for (const int v : next)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (v < 0 || v >= n)
            {
                throw std::out_of_range("FunctionalGraphFold: invalid destination");
            }
#endif
            ++indegree[v];
        }
        std::vector<int> peeled;
        peeled.reserve(n);
        for (int v = 0; v < n; ++v)
        {
            if (indegree[v] == 0)
            {
                peeled.push_back(v);
            }
        }
        for (std::size_t i = 0; i < peeled.size(); ++i)
        {
            const int to = next[peeled[i]];
            if (--indegree[to] == 0)
            {
                peeled.push_back(to);
            }
        }

        depth_.resize(n);
        period_.resize(n);
        for (int start = 0; start < n; ++start)
        {
            if (indegree[start] != 0 && period_[start] == 0)
            {
                int length = 1;
                for (int v = next[start]; v != start; v = next[v])
                {
                    ++length;
                }
                int v = start;
                do
                {
                    period_[v] = length;
                    v = next[v];
                } while (v != start);
            }
        }
        for (auto it = peeled.rbegin(); it != peeled.rend(); ++it)
        {
            const int v = *it;
            depth_[v] = depth_[next[v]] + 1;
            period_[v] = period_[next[v]];
        }
    }

    void append(int &from, std::uint64_t steps, T &product) const
    {
        while (steps != 0)
        {
            const auto level = std::countr_zero(steps);
            const auto &jump = jumps_[depth_.size() * level + from];
            product = combine_(product, jump.product);
            from = jump.end;
            steps &= steps - 1;
        }
    }

    T identity_;
    [[no_unique_address]] Combine combine_;
    std::vector<int> depth_;
    std::vector<int> period_;
    std::vector<Jump> jumps_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
