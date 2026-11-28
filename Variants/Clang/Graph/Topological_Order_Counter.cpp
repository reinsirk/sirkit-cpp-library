// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Topological_Order_Counter.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/topological_order_counter.md
//
// API一覧
// TopologicalOrderCounter(n,edges): 0≤n≤20 の有向グラフ。各辺は (先に置く頂点,後に置く頂点)。
//   頂点は0始まり。閉路があれば順序は0通り。前計算 O(n2^n+m) 時間、O(2^n+n) 領域。
// size()/count(): 頂点数/順序の総数。O(1)。個数は uint64_t の正確値。
// order_at(k): 辞書順で0始まりの第k番目の順序。範囲外はnullopt。O(n²)。
//   候補を選ぶたびに、現在の接頭辞から始まる完成順序の中に第k候補が残る。
// rank(order): 指定順序の辞書順順位。トポロジカル順序でなければnullopt。O(n²)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOPOLOGICAL_ORDER_COUNTER_DEFINED
#define SIRKIT_TOPOLOGICAL_ORDER_COUNTER_DEFINED

class TopologicalOrderCounter
{
public:
    explicit TopologicalOrderCounter(int n, std::span<const std::pair<int, int>> edges)
        : n_(n)
    {
        if (n < 0 || n > kMaximumVertices)
        {
            throw std::length_error("TopologicalOrderCounter: vertex count must be between 0 and 20");
        }
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (!is_vertex(u) || !is_vertex(v))
            {
                throw std::out_of_range("TopologicalOrderCounter: invalid endpoint");
            }
#endif
            prerequisites_[v] |= bit(u);
        }
        full_ = bit(n) - 1;
        completions_.resize(static_cast<std::size_t>(full_) + 1);
        completions_[full_] = 1;
        for (std::uint32_t mask = full_; mask != 0;)
        {
            --mask;
            std::uint64_t ways = 0;
            for (std::uint32_t left = full_ ^ mask; left != 0; left &= left - 1)
            {
                const int u = std::countr_zero(left);
                if (can_place(mask, u))
                {
                    ways += completions_[mask | bit(u)];
                }
            }
            completions_[mask] = ways;
        }
    }

    int size() const noexcept
    {
        return n_;
    }

    std::uint64_t count() const noexcept
    {
        return completions_[0];
    }

    std::optional<std::vector<int>> order_at(std::uint64_t k) const
    {
        if (k >= count())
        {
            return std::nullopt;
        }
        std::vector<int> order;
        order.reserve(n_);
        std::uint32_t mask = 0;
        while (mask != full_)
        {
            for (std::uint32_t left = full_ ^ mask;; left &= left - 1)
            {
                const int u = std::countr_zero(left);
                if (!can_place(mask, u))
                {
                    continue;
                }
                const std::uint32_t next = mask | bit(u);
                const std::uint64_t ways = completions_[next];
                if (k < ways)
                {
                    order.push_back(u);
                    mask = next;
                    break;
                }
                k -= ways;
            }
        }
        return order;
    }

    std::optional<std::uint64_t> rank(std::span<const int> order) const
    {
        if (order.size() != static_cast<std::size_t>(n_))
        {
            return std::nullopt;
        }
        std::uint32_t mask = 0;
        std::uint64_t position = 0;
        for (int u : order)
        {
            if (!is_vertex(u) || (mask & bit(u)) != 0 || !can_place(mask, u))
            {
                return std::nullopt;
            }
            const std::uint32_t before = (bit(u) - 1) & ~mask;
            for (std::uint32_t left = before; left != 0; left &= left - 1)
            {
                const int v = std::countr_zero(left);
                if (can_place(mask, v))
                {
                    position += completions_[mask | bit(v)];
                }
            }
            mask |= bit(u);
        }
        return position;
    }

private:
    static constexpr int kMaximumVertices = 20;

    static constexpr std::uint32_t bit(int vertex) noexcept
    {
        return std::uint32_t{1} << vertex;
    }

    bool is_vertex(int vertex) const noexcept
    {
        return 0 <= vertex && vertex < n_;
    }

    bool can_place(std::uint32_t prefix, int vertex) const noexcept
    {
        return (prerequisites_[vertex] & prefix) == prerequisites_[vertex];
    }

    int n_;
    std::uint32_t full_ = 0;
    std::array<std::uint32_t, kMaximumVertices> prerequisites_{};
    std::vector<std::uint64_t> completions_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
