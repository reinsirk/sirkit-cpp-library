// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::size_t length = 0;
    std::string text;
    verify_in >> length >> text;
    const sirkit::SuffixTree tree(text);
    verify_out << tree.longest_nonoverlapping_repeat().length << '\n';
}
