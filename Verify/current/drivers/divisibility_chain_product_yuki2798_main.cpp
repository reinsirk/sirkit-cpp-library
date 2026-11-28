// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    std::uint64_t value{};
    verify_in >> value;
    verify_out << sirkit::divisibility_chain_product_count(value) << '\n';
}
