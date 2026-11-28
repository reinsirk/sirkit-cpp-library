// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Alien_Trick.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/alien_trick.md
// 参考（AtCoder Algorithm Lectures）: https://info.atcoder.jp/entry/algorithm_lectures/alien_trick
//
// API一覧
// CardinalityOptimum<Cost>{cost,least,most}: ペナルティ込みの最小値と、その最小値を達成する個数の最小・最大。
// choose_cardinality_optimum(a,b): 目的値が小さい方を返す。同値なら個数の両端を統合。O(1)。
// alien_minimum_cost(k,lower,upper,evaluate): 個数kでの最小費用を、整数ペナルティの閉区間で二分探索する。
// evaluate(p)は元の費用+p*個数の最小値をCardinalityOptimumで返す。個数ごとの元の最小費用は整数・離散凸。
// 区間内にkを最適にするpがなければnullopt。範囲R=upper-lower+1に対してO(log(R+1))回評価、O(1)補助領域。
// lower>upperまたはleast>mostはinvalid_argument。返値の復元に使う加減算・積がCostに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ALIEN_TRICK_DEFINED
#define SIRKIT_ALIEN_TRICK_DEFINED

template <class Cost>
struct CardinalityOptimum
{
    Cost cost{};
    std::size_t least = 0;
    std::size_t most = 0;
};

template <class Cost>
[[nodiscard]] CardinalityOptimum<Cost> choose_cardinality_optimum(const CardinalityOptimum<Cost> &first,
                                                                  const CardinalityOptimum<Cost> &second)
{
    if (first.cost < second.cost)
    {
        return first;
    }
    if (second.cost < first.cost)
    {
        return second;
    }
    return {first.cost, std::min(first.least, second.least), std::max(first.most, second.most)};
}

template <class Evaluate>
[[nodiscard]] auto alien_minimum_cost(std::size_t count, std::int64_t lower, std::int64_t upper, Evaluate &&evaluate)
    -> std::optional<decltype(evaluate(lower).cost)>
{
    if (lower > upper)
    {
        throw std::invalid_argument("alien_minimum_cost: invalid penalty interval");
    }
    for (;;)
    {
        const auto penalty = std::midpoint(lower, upper);
        const auto optimum = evaluate(penalty);
        if (optimum.least > optimum.most)
        {
            throw std::invalid_argument("alien_minimum_cost: invalid optimal count interval");
        }
        if (optimum.least <= count && count <= optimum.most)
        {
            using Cost = decltype(optimum.cost);
            return optimum.cost - static_cast<Cost>(penalty) * static_cast<Cost>(count);
        }
        if (lower == upper)
        {
            return std::nullopt;
        }
        if (optimum.least > count)
        {
            lower = penalty + 1;
        }
        else
        {
            upper = penalty;
        }
    }
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
