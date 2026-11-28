#include "../Graph/Johnson_Shortest_Paths.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::JohnsonEdge> edges{{0, 1, -3}, {1, 2, 4}};
    int rows = 0;
    auto consume = [&](int source, auto row)
    {
        if (source == 0)
        {
            assert(row[2] == 1);
        }
        ++rows;
    };
    const bool ok = sirkit::for_each_johnson_distance_row(3, edges, consume);
    assert(ok && rows == 3);
}
