// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    const auto boundaries = lyndon_factorization(text);
    for (std::size_t i = 0; i < boundaries.size(); ++i)
    {
        verify_out << boundaries[i] << (i + 1 == boundaries.size() ? '\n' : ' ');
    }
}
