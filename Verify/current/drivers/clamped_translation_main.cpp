// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int n{};
    verify_in >> n;
    sirkit::ClampedTranslation filter;
    for (int i = 0; i < n; ++i)
    {
        std::int64_t value{};
        int type{};
        verify_in >> value >> type;
        if (type == 1)
        {
            filter.then_add(value);
        }
        else if (type == 2)
        {
            filter.then_chmax(value);
        }
        else
        {
            filter.then_chmin(value);
        }
    }
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        std::int64_t value{};
        verify_in >> value;
        verify_out << filter.apply(value) << '\n';
    }
}
