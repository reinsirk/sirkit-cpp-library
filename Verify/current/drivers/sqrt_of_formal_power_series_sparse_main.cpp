// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{}, k{};
    verify_in >> n >> k;
    std::vector<sirkit::SparseFpsTerm> terms(k);
    for (auto &[degree, coefficient] : terms)
    {
        verify_in >> degree >> coefficient;
    }
    const auto result = sirkit::sparse_fps_sqrt(n, terms);
    if (!result)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    for (std::size_t i = 0; i < result->size(); ++i)
    {
        verify_out << (*result)[i] << (i + 1 == result->size() ? '\n' : ' ');
    }
}
