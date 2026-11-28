// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    std::uint64_t n = 0;
    verify_in >> n;
    std::string merged, word;
    std::vector<std::size_t> starts(n), lengths(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> word;
        starts[i] = merged.size();
        lengths[i] = word.size();
        merged += word;
        merged.push_back('\0');
    }
    const sirkit::SubstringComparison index(merged);
    std::uint64_t q = 0, seed = 0, delta = 0;
    verify_in >> q >> seed >> delta;
    const auto width = n - 1;
    const auto modulus = n * width;
    std::uint64_t total = 0;
    for (std::uint64_t k = 0; k < q; ++k)
    {
        const auto i = seed / width;
        const auto slot = seed % width;
        const auto j = slot + (i <= slot);
        const auto common = index.common_prefix(starts[i], starts[j]);
        total += std::min({lengths[i], lengths[j], common});
        seed += delta;
        if (seed >= modulus)
        {
            seed -= modulus;
        }
    }
    verify_out << total << '\n';
}
