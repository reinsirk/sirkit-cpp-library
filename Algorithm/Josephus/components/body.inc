#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_JOSEPHUS
#define SIRKIT_JOSEPHUS

namespace reinsirk::detail
{
[[nodiscard]] constexpr std::uint64_t josephus_survivor_linear(std::uint64_t count, std::uint64_t step)
{
    std::uint64_t result = 0;
    for (std::uint64_t size = 1; size < count;)
    {
        ++size;
        const auto shift = step % size;
        // result+shiftがuint64_tの範囲外になる場合を先に処理する。
        result = result >= size - shift ? result - (size - shift) : result + shift;
    }
    return result;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint64_t josephus_survivor(std::uint64_t count, std::uint64_t step)
{
    if (count == 0 || step == 0)
    {
        throw std::invalid_argument("josephus_survivor: positive size and step required");
    }
    if (step == 1)
    {
        return count - 1;
    }
    if (step == 2)
    {
        return 2 * (count - std::bit_floor(count));
    }
    if (step > count / std::bit_width(count))
    {
        return detail::josephus_survivor_linear(count, step);
    }
    std::vector<std::uint64_t> sizes;
    while (count >= step)
    {
        sizes.push_back(count);
        count -= count / step;
    }
    auto result = detail::josephus_survivor_linear(count, step);
    for (auto it = sizes.rbegin(); it != sizes.rend(); ++it)
    {
        // 1周で等間隔に除いた番号を元へ戻す。数え直しの起点は残りtail個の直前。
        const auto tail = *it % step;
        if (result < tail)
        {
            result += *it - tail;
        }
        else
        {
            result -= tail;
            result += result / (step - 1);
        }
    }
    return result;
}

[[nodiscard]] inline std::vector<int> josephus_order(int count, std::uint64_t step)
{
    if (count < 0 || step == 0)
    {
        throw std::invalid_argument("josephus_order: nonnegative size and positive step required");
    }
    fenwick_tree<int> alive(std::vector<int>(static_cast<std::size_t>(count), 1));
    std::vector<int> result;
    result.reserve(static_cast<std::size_t>(count));
    std::uint64_t rank = 0;
    for (int remaining = count; remaining != 0; --remaining)
    {
        rank = (rank + (step - 1) % static_cast<std::uint64_t>(remaining)) % static_cast<std::uint64_t>(remaining);
        const auto person = alive.lower_bound(static_cast<int>(rank) + 1);
        result.push_back(person);
        alive.add(person, -1);
    }
    return result;
}
#endif
