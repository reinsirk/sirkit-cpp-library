// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    int cases = 0;
    verify_in >> cases;
    std::string left, right;
    while (cases-- > 0)
    {
        verify_in >> left >> right;
        const sirkit::BigInteger<10> a(left), b(right);
        const auto [quotient, remainder] = a.divmod(b);
        verify_out << quotient.to_string() << ' ' << remainder.to_string() << '\n';
    }
}
