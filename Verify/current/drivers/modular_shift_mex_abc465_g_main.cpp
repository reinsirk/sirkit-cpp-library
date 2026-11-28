// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint32_t modulus{}, count{}, step{};
    verify_in >> n >> modulus >> step >> count;
    sirkit::ModularShiftMex mex(count, modulus, step);
    std::vector<std::uint32_t> offsets(n);
    std::map<std::uint32_t, std::uint32_t> frequency;
    for (auto &offset : offsets)
    {
        verify_in >> offset;
        if (frequency[offset]++ == 0)
        {
            mex.insert(offset);
        }
    }
    int queries{};
    verify_in >> queries;
    for (int q = 0; q < queries; ++q)
    {
        std::size_t index{};
        std::uint32_t value{};
        verify_in >> index >> value;
        auto &offset = offsets[index - 1];
        if (offset != value)
        {
            const auto previous = frequency.find(offset);
            if (--previous->second == 0)
            {
                frequency.erase(previous);
                mex.erase(offset);
            }
            offset = value;
            if (frequency[offset]++ == 0)
            {
                mex.insert(offset);
            }
        }
        verify_out << mex.sum() << '\n';
    }
}
