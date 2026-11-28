// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Digit_DP.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/digit_dp.md
// 参考（Verify問題）: https://atcoder.jp/contests/dp/tasks/dp_s
//
// API一覧
// digit_dp_counts<Count>(upper,states,initial,transition,radix=10): 0以上upper以下の数を、最終状態別に数える。
// upperは2..36進数の非空文字列。状態は[0,states)。初期状態はinitial。
// transition(state,digit,leading_zero): 次状態またはnullopt。leading_zeroは先頭の0に限りtrue。
// Count{}は0、Count{1}は1。加算できること。桁数LでO((L+1)states*radix)時間・O(states*radix)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DIGIT_DP_DEFINED
#define SIRKIT_DIGIT_DP_DEFINED
namespace detail
{
inline unsigned DecodeDigitDP(char character) noexcept
{
    if (character >= '0' && character <= '9')
    {
        return static_cast<unsigned>(character - '0');
    }
    if (character >= 'A' && character <= 'Z')
    {
        return static_cast<unsigned>(character - 'A' + 10);
    }
    if (character >= 'a' && character <= 'z')
    {
        return static_cast<unsigned>(character - 'a' + 10);
    }
    return 36;
}
}

template <class Count, class Transition>
std::vector<Count> digit_dp_counts(std::string_view upper, std::size_t state_count,
                                   std::size_t initial, const Transition &transition, unsigned radix = 10)
{
    std::vector<Count> below, next;
    std::vector<std::size_t> table;
    const auto max_states = std::min(below.max_size() / 2, table.max_size() / (2 * radix));
    if (state_count > max_states)
    {
        throw std::length_error("digit_dp_counts: state table capacity");
    }
    const auto slots = 2 * state_count;
    table.resize(slots * radix, slots);
    below.resize(slots);
    next.resize(slots);
    // 上側のstates個は、すでに0以外の桁を読んだ状態を持つ。
    for (std::size_t slot = 0; slot < slots; ++slot)
    {
        const bool unstarted = slot < state_count;
        const auto state = unstarted ? slot : slot - state_count;
        for (unsigned digit = 0; digit < radix; ++digit)
        {
            const bool leading_zero = unstarted && digit == 0;
            const std::optional<std::size_t> destination = transition(state, digit, leading_zero);
            if (destination)
            {
                table[slot * radix + digit] = *destination + (leading_zero ? 0 : state_count);
            }
        }
    }
    const auto add_below = [&](std::size_t slot, const Count &value)
    {
        if (slot != slots)
        {
            next[slot] += value;
        }
    };
    const Count one{1};
    std::size_t tight = initial;
    for (const char character : upper)
    {
        const auto limit = detail::DecodeDigitDP(character);
        std::ranges::fill(next, Count{});
        for (std::size_t slot = 0; slot < slots; ++slot)
        {
            for (unsigned digit = 0; digit < radix; ++digit)
            {
                add_below(table[slot * radix + digit], below[slot]);
            }
        }
        // 上限と一致する接頭辞は一つなので、個数を持たず状態だけを追う。
        if (tight != slots)
        {
            for (unsigned digit = 0; digit < limit; ++digit)
            {
                add_below(table[tight * radix + digit], one);
            }
            tight = table[tight * radix + limit];
        }
        below.swap(next);
    }
    if (tight != slots)
    {
        below[tight] += one;
    }
    for (std::size_t state = 0; state < state_count; ++state)
    {
        below[state] += below[state + state_count];
    }
    below.resize(state_count);
    return below;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
