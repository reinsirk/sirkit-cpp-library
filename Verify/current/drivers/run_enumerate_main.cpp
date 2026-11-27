// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    const auto runs = sirkit::run_enumerate(text);
    verify_out << runs.size() << '\n';
    for (const auto &[period, left, right] : runs)
    {
        verify_out << period << ' ' << left << ' ' << right << '\n';
    }
}
