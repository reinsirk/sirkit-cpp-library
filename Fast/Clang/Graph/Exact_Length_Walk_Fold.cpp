// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Exact_Length_Walk_Fold.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exact_length_walk_fold.md
//
// API一覧
// ExactLengthWalkArc<T>{from,to,weight}: 重み付き有向辺。頂点番号は0始まり。
// fold_exact_length_walks(n,edges,source,k,zero,one,add,multiply):
//   sourceからちょうどk本の辺を通る全移動を頂点別に集約し、vector<T>で返す。
//   各移動は辺重みを通過順にmultiplyし、同じ頂点への値をaddする。自己辺・多重辺も可。
//   add・multiplyは半環の演算。zero・oneはそれぞれの単位元。Tはzeroと==で比較できること。
//   k<=n: O((n+m)(k+1))時間、O(n)追加領域。k>n: O(m+n^3 log(k+1))時間、O(n^2)追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXACT_LENGTH_WALK_FOLD_DEFINED
#define SIRKIT_EXACT_LENGTH_WALK_FOLD_DEFINED

template <typename T>
struct ExactLengthWalkArc
{
    int from;
    int to;
    T weight;
};

namespace detail
{
inline std::size_t walk_fold_matrix_area(std::size_t n, std::size_t limit)
{
    if (n != 0 && n > limit / n)
    {
        throw std::length_error("fold_exact_length_walks: matrix size overflow");
    }
    return n * n;
}
} // namespace detail

template <typename T, typename Add, typename Multiply>
[[nodiscard]] std::vector<T> fold_exact_length_walks(
    int n, std::span<const ExactLengthWalkArc<std::type_identity_t<T>>> edges, int source,
    std::uint64_t k, T zero, T one, Add add, Multiply multiply)
{
    if (n < 0)
    {
        throw std::length_error("fold_exact_length_walks: negative vertex count");
    }

    std::vector<T> values(n, zero);
    if (k == 0)
    {
        values[source] = one;
        return values;
    }
    if (edges.empty())
    {
        return values;
    }
    values[source] = one;
    if (k <= static_cast<std::uint64_t>(n))
    {
        std::vector<T> next(n, zero);
        while (k-- != 0)
        {
            std::fill(next.begin(), next.end(), zero);
            for (const auto &edge : edges)
            {
                const auto &left = std::as_const(values)[edge.from];
                if (!(left == zero))
                {
                    next[edge.to] = add(std::as_const(next)[edge.to], multiply(left, edge.weight));
                }
            }
            values.swap(next);
        }
        return values;
    }

    const auto width = static_cast<std::size_t>(n);
    const auto area = detail::walk_fold_matrix_area(width, values.max_size());
    std::vector<T> power(area, zero);
    for (const auto &[u, v, weight] : edges)
    {
        const auto position = static_cast<std::size_t>(u) * width + v;
        power[position] = add(std::as_const(power)[position], weight);
    }
    auto apply_power = [&](const std::vector<T> &left)
    {
        std::vector<T> product(left.size(), zero);
        for (std::size_t row = 0; row < left.size(); row += width)
        {
            for (std::size_t middle = 0; middle < width; ++middle)
            {
                const auto &coefficient = left[row + middle];
                if (coefficient == zero)
                {
                    continue;
                }
                const auto right_row = middle * width;
                for (std::size_t column = 0; column < width; ++column)
                {
                    const auto position = row + column;
                    product[position] = add(std::as_const(product)[position],
                                            multiply(coefficient, std::as_const(power)[right_row + column]));
                }
            }
        }
        return product;
    };
    while (k != 0)
    {
        if ((k & 1U) != 0)
        {
            values = apply_power(values);
        }
        k >>= 1;
        if (k != 0)
        {
            power = apply_power(power);
        }
    }
    return values;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
