// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::EytzingerSearch data(std::move(values));
    for (int i = 0; i < q; ++i)
    {
        int threshold{};
        verify_in >> threshold;
        verify_out << data.size() - data.lower_bound(threshold) << '\n';
    }
}
