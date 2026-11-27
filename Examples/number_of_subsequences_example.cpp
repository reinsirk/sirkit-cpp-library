// SPDX-License-Identifier: CC0-1.0
#include "../String/Number_Of_Subsequences.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<int> sequence{1, 2, 1};
    assert(sirkit::count_distinct_subsequences(sequence) == 6);
    assert(sirkit::count_distinct_subsequences(sequence, 5) == 1);
}
