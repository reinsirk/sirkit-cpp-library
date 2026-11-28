// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>

int main()
{
    std::string period, pattern;
    verify_in >> period >> pattern;
    const auto result = sirkit::longest_periodic_pattern_run(period, pattern);
    if (result.repetitions)
    {
        verify_out << *result.repetitions << '\n';
    }
    else
    {
        verify_out << -1 << '\n';
    }
}
