// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Top_K_Cartesian_Sums.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/top_k_cartesian_sums.md
// 参考（上位K個を順に残す考え方）: https://img.atcoder.jp/abc123/editorial.pdf
//
// API一覧
// top_k_cartesian_sums(groups,k[,order]): 各組から1個ずつ選んだ和の上位k個を返す。同値も選び方の個数分残す。
// CartesianSumOrder::largestが既定（大きい順）。smallestは小さい順。全体がk通り未満なら得られる分だけ返す。
// 要素はint64_t。返す和がint64_tを超えるとoverflow_error。途中の部分和は広い型で扱う。
// 組数をD、総要素数をS、最長組をMとして期待O(S+D*k*log(k+1))時間、O(M+k+1)領域。
// k=0または空の組があれば空列。組が0個でk>=1なら{0}。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOP_K_CARTESIAN_SUMS_DEFINED
#define SIRKIT_TOP_K_CARTESIAN_SUMS_DEFINED
enum class CartesianSumOrder
{
    largest,
    smallest,
};

namespace detail
{
struct CartesianWideSum
{
    static constexpr auto kSign = std::uint64_t{1} << 63;
    static constexpr auto kAllBits = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t high = 0;
    std::uint64_t low = 0;

    [[nodiscard]] static CartesianWideSum From(std::int64_t weight) noexcept
    {
        return {weight < 0 ? kAllBits : 0, static_cast<std::uint64_t>(weight)};
    }

    [[nodiscard]] CartesianWideSum add(CartesianWideSum other) const noexcept
    {
        const auto combined = low + other.low;
        return {high + other.high + static_cast<std::uint64_t>(combined < low), combined};
    }

    [[nodiscard]] bool less(CartesianWideSum other) const noexcept
    {
        if (high != other.high)
        {
            return (high ^ kSign) < (other.high ^ kSign);
        }
        return low < other.low;
    }

    [[nodiscard]] std::int64_t to_signed() const
    {
        const auto extension = (low & kSign) != 0 ? kAllBits : 0;
        if (high != extension)
        {
            throw std::overflow_error("top_k_cartesian_sums: result exceeds int64_t");
        }
        return std::bit_cast<std::int64_t>(low);
    }
};

template <bool Largest>
bool BetterCartesianSum(CartesianWideSum first, CartesianWideSum second) noexcept
{
    if constexpr (Largest)
    {
        return second.less(first);
    }
    else
    {
        return first.less(second);
    }
}

template <bool Largest>
std::vector<CartesianWideSum> MergeCartesianSums(std::span<const CartesianWideSum> first,
                                                 std::span<const CartesianWideSum> second, std::size_t k)
{
    if (first.size() > second.size())
    {
        std::swap(first, second);
    }
    const auto length = second.size() > k / first.size() ? k : first.size() * second.size();
    struct Candidate
    {
        CartesianWideSum value;
        std::size_t row = 0, column = 0;
    };
    const auto lower_priority = [](const Candidate &a, const Candidate &b)
    {
        return BetterCartesianSum<Largest>(b.value, a.value);
    };
    std::vector<Candidate> heap;
    heap.reserve(first.size());
    for (std::size_t row = 0; row < first.size(); ++row)
    {
        heap.push_back({first[row].add(second.front()), row, 0});
    }
    std::priority_queue queue(lower_priority, std::move(heap));
    std::vector<CartesianWideSum> result;
    result.reserve(length);
    // 各行の未使用の先頭だけをヒープに置く。
    while (result.size() < length)
    {
        auto next = queue.top();
        queue.pop();
        result.push_back(next.value);
        ++next.column;
        if (next.column < second.size())
        {
            queue.push({first[next.row].add(second[next.column]), next.row, next.column});
        }
    }
    return result;
}

template <bool Largest>
std::vector<std::int64_t> TopKCartesianSums(std::span<const std::vector<std::int64_t>> groups, std::size_t k)
{
    if (k == 0)
    {
        return {};
    }
    std::vector<CartesianWideSum> prefix(1);
    for (const auto &group : groups)
    {
        if (group.empty())
        {
            return {};
        }
        std::vector<CartesianWideSum> sorted;
        sorted.reserve(group.size());
        for (const auto value : group)
        {
            sorted.push_back(CartesianWideSum::From(value));
        }
        if (sorted.size() > k)
        {
            std::nth_element(sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(k), sorted.end(), BetterCartesianSum<Largest>);
            sorted.resize(k);
        }
        std::sort(sorted.begin(), sorted.end(), BetterCartesianSum<Largest>);
        prefix = MergeCartesianSums<Largest>(prefix, sorted, k);
    }
    std::vector<std::int64_t> result;
    result.reserve(prefix.size());
    for (const auto value : prefix)
    {
        result.push_back(value.to_signed());
    }
    return result;
}
}

inline std::vector<std::int64_t> top_k_cartesian_sums(std::span<const std::vector<std::int64_t>> groups,
                                                      std::size_t k, CartesianSumOrder order = CartesianSumOrder::largest)
{
    if (order == CartesianSumOrder::largest)
    {
        return detail::TopKCartesianSums<true>(groups, k);
    }
    return detail::TopKCartesianSums<false>(groups, k);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
