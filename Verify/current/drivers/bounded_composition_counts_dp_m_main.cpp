// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{}, sum{};
    verify_in >> n >> sum;
    std::vector<std::size_t> maxima(n);
    for (auto &limit : maxima)
    {
        verify_in >> limit;
    }
    verify_out << sirkit::bounded_composition_counts(maxima, sum, 1'000'000'007).back() << '\n';
}
