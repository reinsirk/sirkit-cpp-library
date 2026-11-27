// SPDX-License-Identifier: CC0-1.0
#include "../Math/Combinatorial_Sequences.cpp"
#include <cassert>

int main()
{
    assert((sirkit::partition_numbers(5) == std::vector<std::uint32_t>{1, 1, 2, 3, 5, 7}));
    assert((sirkit::bell_numbers(4) == std::vector<std::uint32_t>{1, 1, 2, 5, 15}));
    assert(sirkit::stirling_first_row(4)[2] == 11);
    assert(sirkit::stirling_second_row(4)[2] == 7);
    assert(sirkit::derangement_numbers(4)[4] == 9);
    assert(sirkit::bernoulli_numbers(2)[1] == 499122176);
    assert(sirkit::sum_of_powers(3, 2) == 14);
    const std::vector<std::size_t> values{0, 1, 2};
    assert(sirkit::subset_sum_counts(values, 3)[3] == 2);
}
