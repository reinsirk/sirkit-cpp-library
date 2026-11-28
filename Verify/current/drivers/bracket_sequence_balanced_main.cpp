// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::string text;
    verify_in >> text;
    sirkit::BracketSequence sequence(text);
    while (q-- > 0)
    {
        int type = 0, first = 0, second = 0;
        verify_in >> type >> first >> second;
        if (type == 1)
        {
            sequence.swap(first - 1, second - 1);
        }
        else
        {
            verify_out << (sequence.query(first - 1, second).balanced() ? "Yes\n" : "No\n");
        }
    }
}
