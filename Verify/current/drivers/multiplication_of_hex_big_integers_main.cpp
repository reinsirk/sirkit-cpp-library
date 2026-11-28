// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    int cases{};
    verify_in >> cases;
    std::string left, right;
    while (cases-- > 0)
    {
        verify_in >> left >> right;
        const sirkit::BigInteger<16> a(left), b(right);
        verify_out << (a * b).to_string() << '\n';
    }
}
