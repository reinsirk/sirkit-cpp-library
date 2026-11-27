// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, k{};
    std::uint64_t exponent{};
    verify_in >> n >> k >> exponent;
    std::vector<sirkit::SparseFpsTerm> terms(k);
    for (auto &[degree, coefficient] : terms)
    {
        verify_in >> degree >> coefficient;
    }
    const auto result = sirkit::sparse_fps_pow(n, terms, exponent);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
