#include "../Graph/Enumerate_Topological_Orders.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 2}};
    const auto orders = sirkit::enumerate_topological_orders(3, edges, 4);
    assert(orders.size() == 3);
    assert(orders[0] != orders[1]);
}
