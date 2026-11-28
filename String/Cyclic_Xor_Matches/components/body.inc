#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_CYCLIC_XOR_MATCHES_DEFINED
#define SIRKIT_CYCLIC_XOR_MATCHES_DEFINED

struct CyclicXorMatch
{
    int shift;
    std::uint64_t mask;
};

namespace reinsirk::detail
{
inline void CheckCyclicXorInputLength(std::size_t length)
{
    if (length > static_cast<std::size_t>(std::numeric_limits<int>::max() / 2))
    {
        throw std::length_error("cyclic_xor_matches: input too long");
    }
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<CyclicXorMatch> cyclic_xor_matches(
    std::span<const std::uint64_t> source, std::span<const std::uint64_t> target)
{
    const auto n = source.size();
#ifndef SKIP_RUNTIME_CHECKS
    if (target.size() != n)
    {
        throw std::invalid_argument("cyclic_xor_matches: mismatched lengths");
    }
#endif
    if (n == 0)
    {
        return {};
    }
    reinsirk::detail::CheckCyclicXorInputLength(n);
    std::vector<std::uint64_t> pattern(n), text(2 * n - 1);
    // 隣接要素どうしのXORを取ると、全要素に共通のmaskが相殺される。
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto next = i + 1 == n ? 0 : i + 1;
        pattern[i] = target[i] ^ target[next];
        text[i] = source[i] ^ source[next];
    }
    std::copy_n(text.begin(), n - 1, text.begin() + static_cast<std::ptrdiff_t>(n));
    const auto shifts = ZMatches(pattern, text);
    std::vector<CyclicXorMatch> matches;
    matches.reserve(shifts.size());
    for (const int shift : shifts)
    {
        matches.push_back({shift, source[static_cast<std::size_t>(shift)] ^ target.front()});
    }
    return matches;
}

#endif
