// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::size_t length = 0, query_count = 0;
    verify_in >> length >> query_count;
    std::string word;
    verify_in >> word;
    sirkit::DynamicSubstringMatching text(word);
    for (std::size_t qi = 0; qi < query_count; ++qi)
    {
        int type = 0;
        verify_in >> type;
        if (type == 1)
        {
            std::size_t position = 0;
            char symbol = '\0';
            verify_in >> position >> symbol;
            text.set(position - 1, symbol);
        }
        else
        {
            verify_in >> word;
            verify_out << (text.contains(word) ? "Yes\n" : "No\n");
        }
    }
}
