#include "../Graph/Automaton_Shortest_Path.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::AutomatonPathEdge> edges{{0, 2, 0, 1}, {0, 1, 0, 1}, {1, 2, 0, 1}};
    auto step = [](int state, int) { return 1 - state; };
    auto even = [](int state) { return state == 0; };
    const auto answer = sirkit::automaton_shortest_path(3, edges, 0, 2, 2, 0, step, even);
    assert(answer && answer->distance == 2);
}
