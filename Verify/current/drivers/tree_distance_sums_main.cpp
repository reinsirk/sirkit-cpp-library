// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<sirkit::TreeDistanceSumEdge<VerifyMatrixField>> edges(n - 1);
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to;
        --edge.from;
        --edge.to;
    }
    sirkit::TreeDistanceSums<VerifyMatrixField> distance(n, edges);
    for (int i = 0; i < q; ++i)
    {
        int kind{}, u{};
        verify_in >> kind >> u;
        --u;
        if (kind == 1)
        {
            std::uint64_t amount{};
            verify_in >> amount;
            distance.add(u, amount);
        }
        else
        {
            int v{};
            verify_in >> v;
            verify_out << distance.sum_path(u, v - 1).get() << '\n';
        }
    }
}
