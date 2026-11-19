// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

int main() {
    static_assert(sizeof(std::size_t) == 4);
    // i386では行spanの容量をintの頂点数で超えられる。辺も頂点配列も確保しない。
    constexpr int vertices = std::numeric_limits<int>::max();
    const std::span<const std::tuple<int, int, std::int64_t>> edges;
    std::vector<std::int64_t> distances{-1};
    std::vector<int> path{-1};
    bool threw = false;
    try {
        sirkit::Dijkstra(vertices, edges, distances, 0, 0, path);
    } catch (const std::length_error& error) {
        threw = true;
        assert(std::string_view(error.what()) == "Dijkstra: CSR capacity exceeded");
    }
    assert(threw && distances == std::vector<std::int64_t>{-1} && path == std::vector<int>{-1});
}
