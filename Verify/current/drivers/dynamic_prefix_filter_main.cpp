// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t q{};
    verify_in >> q;
    sirkit::DynamicPrefixFilter filter;
    std::string word;
    for (std::size_t i = 0; i < q; ++i)
    {
        int type{};
        verify_in >> type >> word;
        if (type == 1)
        {
            filter.block(word);
        }
        else
        {
            filter.insert(word);
        }
        verify_out << filter.allowed_count() << '\n';
    }
}
