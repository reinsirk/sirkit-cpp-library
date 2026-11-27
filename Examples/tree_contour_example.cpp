// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Contour.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const sirkit::TreeContourIndex index(3, edges);
    sirkit::VertexAddContourSum<> sums(index);
    sums.add(2, 7);
    assert(sums.sum(0, 1, 3) == 7);
    assert(sums.sum(0, 0, 2) == 0);
    sirkit::ContourAddVertexGet<> values(index);
    values.add_range(0, 1, 3, 7);
    assert(values.get(2) == 7 && values.get(0) == 0);
}
