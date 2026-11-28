// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <string>

int main()
{
    std::uint64_t copies{};
    std::string cycle, target;
    verify_in >> copies >> cycle >> target;
    const sirkit::RepeatedSubsequenceIndex index(cycle);
    verify_out << index.max_repetition(copies, target) << '\n';
}
