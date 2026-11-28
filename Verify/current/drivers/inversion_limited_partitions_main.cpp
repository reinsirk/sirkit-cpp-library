// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t limit{};
    verify_in >> n >> limit;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_inversion_limited_partitions<atcoder::modint1000000007>(values, limit).val() << '\n';
}
