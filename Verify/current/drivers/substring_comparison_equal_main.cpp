// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>

int main()
{
    std::size_t n = 0, q = 0;
    std::string text;
    verify_in >> n >> q >> text;
    const sirkit::SubstringComparison index(text);
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t a = 0, b = 0, c = 0, d = 0;
        verify_in >> a >> b >> c >> d;
        verify_out << (index.equal(a - 1, c - 1, b - a + 1) ? "Yes\n" : "No\n");
    }
}
