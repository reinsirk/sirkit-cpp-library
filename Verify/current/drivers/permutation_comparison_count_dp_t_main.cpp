// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{};
    std::string comparisons;
    verify_in >> n >> comparisons;
    verify_out << sirkit::count_permutations_by_comparisons<1'000'000'007>(comparisons) << '\n';
}
