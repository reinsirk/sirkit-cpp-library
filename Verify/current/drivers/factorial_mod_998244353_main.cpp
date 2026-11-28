// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main()
{
    int size{};
    verify_in >> size;
    std::vector<std::uint64_t> queries(size);
    for (auto &query : queries)
    {
        verify_in >> query;
    }
    for (const auto factorial : sirkit::factorials_mod_998244353(queries))
    {
        verify_out << factorial << '\n';
    }
}
