#include "../Graph/Topological_Order_Counter.cpp"

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{1, 0}, {1, 2}};
    const sirkit::TopologicalOrderCounter orders(3, edges);
    assert(orders.count() == 2);
    const std::vector<int> first{1, 0, 2};
    assert(orders.order_at(0) == first);
    assert(orders.rank(first) == std::uint64_t{0});
    assert(!orders.order_at(2));
}
