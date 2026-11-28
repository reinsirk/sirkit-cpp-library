// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>
#include <vector>

int main()
{
    std::string alphabet;
    std::size_t n{};
    verify_in >> alphabet >> n;
    std::vector<std::string> names(n);
    for (auto &name : names)
    {
        verify_in >> name;
    }
    for (const auto i : sirkit::string_radix_order(names, alphabet))
    {
        verify_out << names[i] << '\n';
    }
}
