// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{}, m{};
    std::string target, stamp;
    verify_in >> n >> m >> target >> stamp;
    verify_out << (sirkit::pattern_stamping_order(target, stamp) ? "Yes" : "No") << '\n';
}
