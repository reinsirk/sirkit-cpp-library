// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{};
    std::size_t k{};
    std::string text;
    verify_in >> n >> k >> text;
    verify_out << sirkit::lexicographic_subsequence(text, k) << '\n';
}
