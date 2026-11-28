#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

#ifndef SIRKIT_PAIRWISE_COPRIME_PROGRESSION
#define SIRKIT_PAIRWISE_COPRIME_PROGRESSION

namespace reinsirk::detail
{
constexpr std::array<std::uint32_t, 27> kCoprimeProgressionPrimes{
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103};
// 53以下の全素数がstepを割ることは64ビットの範囲では起こらない。
static_assert(std::numeric_limits<std::uint64_t>::max() / 53 < 614889782588491410ULL);
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint64_t max_pairwise_coprime_progression_length(std::uint64_t step)
{
    if (step == 0)
    {
        throw std::invalid_argument("pairwise coprime progression: step must be positive");
    }
    const auto missing = std::ranges::find_if(detail::kCoprimeProgressionPrimes,
                                              [step](std::uint32_t prime)
                                              {
                                                  return step % prime != 0;
                                              });
    // 2p項並べると、pで割り切れる位置が少なくとも2回現れる。
    return 2 * *missing - 1;
}

// productはstepと互いに素。各p | productでは、初項はstepと合同だからpの倍数はp項目だけ。
[[nodiscard]] inline std::optional<BigInteger<10>> pairwise_coprime_progression_start(std::uint64_t length, std::uint64_t step)
{
    if (length > max_pairwise_coprime_progression_length(step))
    {
        return std::nullopt;
    }
    BigInteger<10> product{1};
    for (const auto prime : detail::kCoprimeProgressionPrimes)
    {
        if (prime >= length)
        {
            break;
        }
        if (step % prime != 0)
        {
            product *= BigInteger<10>(prime);
        }
    }
    return BigInteger<10>(std::to_string(step)) + product;
}
#endif
