// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <tuple>
#include <utility>

namespace {
std::atomic<std::ptrdiff_t> budget{-1};
std::atomic<std::size_t> live{0};
}
void* operator new(std::size_t bytes) {
    if (budget == 0) throw std::bad_alloc();
    if (budget > 0) --budget;
    if (void* p = std::malloc(bytes ? bytes : 1)) { ++live; return p; }
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { if (p) { --live; std::free(p); } }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }

#ifndef LOW_LINK_SOURCE
#define LOW_LINK_SOURCE "../../../Graph/Low_Link.cpp"
#endif
#include LOW_LINK_SOURCE

static auto snapshot(const UndirectedLowLink& g) {
    return std::tuple{g.component_count(), g.bridges(), g.articulation_points(),
                      g.two_edge_components(), g.biconnected_edge_components(),
                      g.biconnected_vertex_components(),
                      g.biconnected_vertex_components(false), g.bridge_forest()};
}

int main() {
    const auto initial = live.load();
    std::size_t failures = 0, successes = 0;
    {
        UndirectedLowLink graph(9);
        for (auto [u, v] : {std::pair{0,1}, {1,2}, {2,0}, {2,3}, {3,4},
                            {3,4}, {4,4}, {6,7}, {8,8}}) graph.add_edge(u, v);
        graph.build(true);
        const auto expected = snapshot(graph);
        for (int operation = 0; operation < 7; ++operation) {
            for (int allowance = 0; allowance < 100; ++allowance) {
                const auto before = live.load();
                budget = allowance;
                try {
                    if (operation == 0) {
                        auto result = graph.two_edge_components();
                        budget = -1; assert(result == std::get<3>(expected));
                    } else if (operation == 1 || operation == 2) {
                        auto result = graph.biconnected_vertex_components(operation == 1);
                        budget = -1;
                        assert(result == (operation == 1 ? std::get<5>(expected) : std::get<6>(expected)));
                    } else if (operation == 3) {
                        auto result = graph.bridge_forest();
                        budget = -1; assert(result == std::get<7>(expected));
                    } else if (operation == 4) {
                        auto copy = graph;
                        budget = -1; assert(snapshot(copy) == expected);
                        copy.reset(); assert(graph.size() == 9);
                    } else {
                        graph.build(operation == 6);
                        budget = -1;
                        graph.build(true);
                    }
                    ++successes;
                } catch (const std::bad_alloc&) {
                    budget = -1; ++failures;
                    assert(snapshot(graph) == expected);
                }
                assert(live == before);
                assert(snapshot(graph) == expected);
            }
        }
    }
    assert(failures > 0 && successes > 0 && live == initial);
    std::cout << "low-link components allocation failures=" << failures
              << " successes=" << successes << " live_delta=" << live.load() - initial << '\n';
}
