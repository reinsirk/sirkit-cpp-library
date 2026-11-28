// SPDX-License-Identifier: CC0-1.0
#include "../Graph/DAG_Grundy.cpp"

#include <cassert>
#include <vector>

int main()
{
    const sirkit::Graph moves{{}, {0}, {0, 1}};
    const auto grundy = sirkit::dag_grundy_numbers(moves);
    assert((grundy && *grundy == std::vector{0, 1, 2}));
    assert(((*grundy)[1] ^ (*grundy)[2]) == 3);
}
