// SPDX-License-Identifier: CC0-1.0
#include "../String/Shortest_Absent_Subsequence.cpp"
#include <cassert>

int main()
{
    assert(sirkit::shortest_absent_subsequence("abba", "ab") == "aaa");
    assert(sirkit::shortest_absent_subsequence_length("abba", "ab") == 3);
}
