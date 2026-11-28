#include "../Algorithm/Longest_Bitonic_Subsequence.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector values{1, 4, 2, 5, 3, 2};
    const auto result = sirkit::longest_bitonic_subsequence(values);
    assert(result.indices.size() == 5);
    assert(result.indices[result.peak] == 3);
}
