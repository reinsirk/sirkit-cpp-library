// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::size_t q{};
    std::string s, t;
    verify_in >> q >> s >> t;
    const sirkit::PrefixSubstringLCS lcs(s, t);
    while (q-- != 0)
    {
        std::size_t a{}, b{}, c{};
        verify_in >> a >> b >> c;
        verify_out << lcs.length(a, b, c) << '\n';
    }
}
