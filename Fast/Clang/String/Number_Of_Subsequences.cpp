// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Number_Of_Subsequences.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/number_of_subsequences.md
//
// API一覧
// count_distinct_subsequences(sequence, modulus=998244353): 空でない相異なる部分列の個数を法modulusで返す。
// 入力は整数を値型とする、長さ既知のランダムアクセス範囲。符号付きの負数や整数型の最大値も可。
// 空列なら0。法はuint64_tで1以上、0の場合はinvalid_argument。
// 要素数Nに対して、最大値と最小値の差がN未満ならO(N)時間、他はO(N log(N+1))時間。O(N)追加領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_DISTINCT_SUBSEQUENCES_DEFINED
#define SIRKIT_COUNT_DISTINCT_SUBSEQUENCES_DEFINED

namespace detail
{
template <std::integral Symbol>
constexpr auto SubsequenceUnsigned(Symbol symbol)
{
    if constexpr (std::same_as<Symbol, bool>)
    {
        return static_cast<unsigned char>(symbol);
    }
    else
    {
        return static_cast<std::make_unsigned_t<Symbol>>(symbol);
    }
}

template <class Sequence, class Index>
std::uint64_t CountSubsequenceCodes(const Sequence &sequence, Index index, std::size_t symbols, std::uint64_t modulus)
{
    std::vector<std::uint64_t> before_previous(symbols);
    std::uint64_t total = 1;
    for (const auto value : sequence)
    {
        auto &previous = before_previous[index(value)];
        // 現在の記号を追加する/しないで倍にし、前回の同じ記号で既に作った部分列を引く。
        const auto gap = modulus - total;
        const auto doubled = total >= gap ? total - gap : total + total;
        const auto next = doubled >= previous ? doubled - previous : modulus - (previous - doubled);
        previous = total;
        total = next;
    }
    return total == 0 ? modulus - 1 : total - 1;
}
} // namespace detail

template <class Sequence>
    requires std::ranges::random_access_range<const Sequence> && std::ranges::sized_range<const Sequence> &&
             std::integral<std::ranges::range_value_t<const Sequence>>
std::uint64_t count_distinct_subsequences(const Sequence &sequence, std::uint64_t modulus = 998'244'353)
{
    if (modulus == 0)
    {
        throw std::invalid_argument("count_distinct_subsequences: modulus must be positive");
    }
    const auto size = static_cast<std::size_t>(std::ranges::size(sequence));
    if (size == 0 || modulus == 1)
    {
        return 0;
    }

    using Symbol = std::ranges::range_value_t<const Sequence>;
    using Difference = std::ranges::range_difference_t<const Sequence>;
    using Unsigned = decltype(detail::SubsequenceUnsigned(Symbol{}));
    const auto [min_value, max_value] = std::ranges::minmax_element(sequence);
    const auto minimum = detail::SubsequenceUnsigned(static_cast<Symbol>(*min_value));
    const auto offset = [minimum](Symbol value)
    {
        return static_cast<Unsigned>(detail::SubsequenceUnsigned(value) - minimum);
    };
    const auto width = offset(static_cast<Symbol>(*max_value));
    if (width < size)
    {
        return detail::CountSubsequenceCodes(sequence, offset, static_cast<std::size_t>(width) + 1, modulus);
    }

    std::vector<std::size_t> code(size);
    std::size_t last_code = 0;
    {
        const auto first = std::ranges::begin(sequence);
        const auto value_at = [&](std::size_t i)
        {
            return static_cast<Symbol>(first[static_cast<Difference>(i)]);
        };
        std::vector<std::size_t> order(size);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::ranges::sort(order, {}, value_at);

        auto previous = value_at(order.front());
        for (const auto i : order)
        {
            const auto value = value_at(i);
            last_code += value != previous;
            code[i] = last_code;
            previous = value;
        }
    }
    return detail::CountSubsequenceCodes(code, std::identity{}, last_code + 1, modulus);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
