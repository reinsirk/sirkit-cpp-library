// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<int> permutation(n);
    for (auto &element : permutation)
    {
        verify_in >> element;
        --element;
    }
    const auto code = sirkit::permutation_lehmer_code(permutation);
    std::vector<std::uint32_t> radices(n), digits(code.begin(), code.end());
    for (std::size_t i = 0; i < n; ++i)
    {
        radices[i] = static_cast<std::uint32_t>(n - i);
    }
    const sirkit::MixedRadix format(radices);
    verify_out << (format.encode(digits) + 1).to_string() << '\n';
}
