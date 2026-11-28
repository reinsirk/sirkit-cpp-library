// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    using Tree = sirkit::TreePathAffineSum<VerifyMatrixField>;
    std::vector<VerifyMatrixField> values(n);
    for (auto &value : values)
    {
        std::uint32_t input{};
        verify_in >> input;
        value = input;
    }
    std::vector<Tree::Edge> edges(n - 1);
    for (auto &edge : edges)
    {
        std::uint32_t mul{}, add{};
        verify_in >> edge.u >> edge.v >> mul >> add;
        edge.toward_u = edge.toward_v = {mul, add};
    }
    Tree tree(values, edges);
    for (int i = 0; i < q; ++i)
    {
        int kind{}, index{};
        std::uint32_t x{};
        verify_in >> kind >> index >> x;
        if (kind == 0)
        {
            tree.set_vertex(index, x);
        }
        else
        {
            std::uint32_t y{};
            verify_in >> y;
            tree.set_edge(index, {x, y});
        }
        verify_out << tree.all_prod().get() << '\n';
    }
}
