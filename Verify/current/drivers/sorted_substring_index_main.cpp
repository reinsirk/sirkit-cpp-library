// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>

int main()
{
    int n{}, q{};
    std::string text;
    verify_in >> n >> text >> q;
    sirkit::SortedSubstringIndex<> index(text);
    for (int query = 0; query < q; ++query)
    {
        int type{}, left{};
        verify_in >> type >> left;
        if (type == 1)
        {
            char byte{};
            verify_in >> byte;
            index.set(left - 1, byte);
        }
        else
        {
            int right{};
            verify_in >> right;
            verify_out << (index.is_sorted_slice(left - 1, right) ? "Yes\n" : "No\n");
        }
    }
}
