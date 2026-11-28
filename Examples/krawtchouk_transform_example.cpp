// SPDX-License-Identifier: CC0-1.0
#include "../Math/Krawtchouk_Transform.cpp"

#include <iostream>

int main()
{
    const auto transformed = sirkit::krawtchouk_transform({1, 0, 0});
    for (const auto coefficient : transformed.coefficients())
    {
        std::cout << coefficient << '\n'; // 1, 2, 1
    }
}
