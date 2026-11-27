// SPDX-License-Identifier: CC0-1.0
#include "../Math/Summatory_Totient.cpp"
#include <cassert>

int main()
{
    assert(sirkit::summatory_totient(5) == 10);
    assert(sirkit::summatory_totient(5, 3) == 1);
}
