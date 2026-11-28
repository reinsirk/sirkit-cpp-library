// SPDX-License-Identifier: CC0-1.0
#include "../Math/Modular_Shift_Mex.cpp"

#include <iostream>

int main()
{
    sirkit::ModularShiftMex values(2, 3, 1);
    values.insert(0);
    values.insert(2);
    std::cout << values.sum() << '\n'; // 3
    values.erase(0);
    values.insert(1);
    std::cout << values.sum() << '\n'; // 1
}
