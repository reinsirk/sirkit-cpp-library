// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<sirkit::RunLengthZ::Run> runs(n);
    for (auto &run : runs)
    {
        verify_in >> run.count;
    }
    std::string text;
    verify_in >> text;
    for (std::size_t i = 0; i < n; ++i)
    {
        runs[i].byte = static_cast<std::uint8_t>(text[i]);
    }
    verify_out << sirkit::RunLengthZ(runs).sum_prefix_lengths() << '\n';
}
