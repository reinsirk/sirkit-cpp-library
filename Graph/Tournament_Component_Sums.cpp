// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tournament_component_sums.md
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/tournament
//
// API一覧
// tournament_component_sums<T>(n): 頂点 0..n-1 のトーナメント全体を対象にする。
//   戻り値 [m] は、小さい番号から大きい番号への辺がちょうど m 本ある全グラフでの強連結成分数の和。
//   全 m=0..n(n-1)/2 を返す。n=0 は {0}。T は 0,1 から構築でき、加算・乗算ができる型。
//   O((n+1)^5) 時間、O((n+1)^3) 領域。除算は不要。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOURNAMENT_COMPONENT_SUMS_DEFINED
#define SIRKIT_TOURNAMENT_COMPONENT_SUMS_DEFINED

template <class T>
[[nodiscard]] std::vector<T> tournament_component_sums(int n)
{
    if (n < 0)
    {
        throw std::length_error("tournament_component_sums: negative vertex count");
    }
    if (n == 0)
    {
        return {T{0}};
    }

    const auto size = static_cast<std::size_t>(n);
    const auto total_edges = size * (size - 1) / 2;
    const auto half = size / 2;
    std::vector<std::vector<T>> splits(half + 1);
    splits[0].push_back(T{1});
    for (std::size_t length = 1; length <= size; ++length)
    {
        for (auto k = std::min(length, half); k != 0; --k)
        {
            std::vector<T> updated(k * (length - k) + 1, T{0});
            std::copy(splits[k - 1].begin(), splits[k - 1].end(), updated.begin());
            for (std::size_t i = 0; i < splits[k].size(); ++i)
            {
                updated[i + k] = updated[i + k] + splits[k][i];
            }
            splits[k].swap(updated);
        }
    }

    std::vector<T> binomial{T{1}};
    const auto expand_binomial = [&](std::size_t degree)
    {
        while (binomial.size() <= degree)
        {
            binomial.push_back(T{0});
            for (auto i = binomial.size() - 1; i != 0; --i)
            {
                binomial[i] = binomial[i] + binomial[i - 1];
            }
        }
    };

    // 各真の接頭辞が強連結成分境界を一つ数える。全体のトーナメント数を足すと成分数の和になる。
    std::vector<T> answer(total_edges + 1, T{0});
    for (auto k = half; k != 0; --k)
    {
        expand_binomial(total_edges - k * (size - k));
        const T multiplier = k * 2 == size ? T{1} : T{1} + T{1};
        for (std::size_t i = 0; i < splits[k].size(); ++i)
        {
            const T ways = multiplier * splits[k][i];
            for (std::size_t j = 0; j < binomial.size(); ++j)
            {
                answer[i + j] = answer[i + j] + ways * binomial[j];
            }
        }
    }
    expand_binomial(total_edges);
    for (std::size_t i = 0; i <= total_edges; ++i)
    {
        answer[i] = answer[i] + binomial[i];
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
