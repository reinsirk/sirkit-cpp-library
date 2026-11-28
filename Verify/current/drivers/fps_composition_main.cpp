// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t size{};
    verify_in >> size;
    std::vector<std::uint32_t> outer(size), inner(size);
    for (auto &value : outer)
    {
        verify_in >> value;
    }
    for (auto &value : inner)
    {
        verify_in >> value;
    }
    const auto composed = sirkit::fps_compose(sirkit::FormalPowerSeries(std::move(outer)),
                                              sirkit::FormalPowerSeries(std::move(inner)), size);
    for (std::size_t i = 0; i < size; ++i)
    {
        verify_out << composed[i] << (i + 1 == size ? '\n' : ' ');
    }
}
