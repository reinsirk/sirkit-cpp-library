// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::string text;
    std::size_t queries = 0;
    verify_in >> text >> queries;
    const sirkit::SubstringIndex index(text);
    std::string pattern;
    for (std::size_t query = 0; query < queries; ++query)
    {
        verify_in >> pattern;
        verify_out << index.occurrences(pattern).size() << '\n';
    }
}
