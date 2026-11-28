// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Static_Top_Tree.cpp"

#include <array>
#include <cassert>
#include <utility>

struct SumOperations
{
    using Vertex = int;
    using Path = int;
    using Point = int;

    Path vertex(Vertex value, const Point *branches) const noexcept
    {
        return value + (branches ? *branches : 0);
    }

    Point project(Path path) const noexcept
    {
        return path;
    }

    Point rake(Point left, Point right) const noexcept
    {
        return left + right;
    }

    Path compress(Path upper, Path lower) const noexcept
    {
        return upper + lower;
    }
};

int main()
{
    const std::array values{3, 5, 1};
    const std::array edges{std::pair{0, 1}, std::pair{0, 2}};
    sirkit::StaticTopTree<SumOperations> tree(values, edges);
    tree.set(2, 4);
    assert(tree.all_prod() == 12);
}
