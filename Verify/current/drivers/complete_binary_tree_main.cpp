// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int tests{};
    verify_in >> tests;
    while (tests-- > 0)
    {
        std::uint64_t n{}, vertex{}, distance{};
        verify_in >> n >> vertex >> distance;
        const sirkit::CompleteBinaryTree tree(n);
        verify_out << tree.count_at_distance(vertex, distance) << '\n';
    }
}
