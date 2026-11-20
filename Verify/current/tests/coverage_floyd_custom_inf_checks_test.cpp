// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Floyd-Warshall.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void check_wide_distance(std::int64_t inf) {
    using Matrix = std::vector<std::vector<std::int64_t>>;
    const std::int64_t weight = inf - 1;
    for (const auto edge_weight : {weight, -weight}) {
        // 大きな辺は1本だけにし、広幅演算が必要でも結果はinfの範囲に収める。
        Matrix input(5, std::vector<std::int64_t>(5, inf));
        input[0][1] = edge_weight;
        auto expected = input;
        for (std::size_t v = 0; v < expected.size(); ++v) expected[v][v] = 0;
        Matrix distance = input, with_path = input;
        std::vector<std::vector<int>> next;
        assert(!sirkit::FloydWarshall(distance, inf) && distance == expected);
        assert(!sirkit::FloydWarshall(with_path, next, inf) && with_path == expected);
        assert((sirkit::FloydWarshallPath(next, 0, 1) == std::vector{0, 1}));
        assert(sirkit::FloydWarshallPath(next, 0, 2).empty());
    }
}

} // namespace

int main() {
    using Matrix = std::vector<std::vector<std::int64_t>>;
    constexpr auto kMaxVertices = static_cast<std::size_t>(std::numeric_limits<int>::max());
    sirkit::detail::CheckFloydVertexCount(kMaxVertices);
    rejects<std::length_error>([] { sirkit::detail::CheckFloydVertexCount(kMaxVertices + 1); });

    constexpr std::int64_t inf = 100;
    const Matrix input{{inf, 7, 20}, {inf, inf, -3}, {inf, inf, inf}};
    const Matrix expected{{0, 7, 4}, {inf, 0, -3}, {inf, inf, 0}};
    const std::vector<int> path{0, 1, 2};
    Matrix distance = input, with_path = input;
    std::vector<std::vector<int>> next;
    assert(!sirkit::FloydWarshall(distance, inf) && distance == expected);
    assert(!sirkit::FloydWarshall(with_path, next, inf) && with_path == expected);
    assert(sirkit::FloydWarshallPath(next, 0, 2) == path);
    assert(sirkit::FloydWarshallPath(next, 2, 0).empty());

    Matrix singleton{{inf}};
    assert(!sirkit::FloydWarshall(singleton, next, inf) && singleton[0][0] == 0);
    assert(sirkit::FloydWarshallPath(next, 0, 0) == std::vector{0});
    singleton = {{-1}};
    assert(sirkit::FloydWarshall(singleton, next, inf) && singleton[0][0] == -inf);
    assert(sirkit::FloydWarshallPath(next, 0, 0).empty());
    rejects<std::invalid_argument>([&] { (void)sirkit::FloydWarshall(distance, 0); });
    rejects<std::invalid_argument>([&] { (void)sirkit::FloydWarshall(distance, next, -1); });
#ifndef SKIP_RUNTIME_CHECKS
    Matrix malformed{{}};
    rejects<std::invalid_argument>([&] { (void)sirkit::FloydWarshall(malformed); });
    for (std::int64_t invalid : {-inf, inf + 1}) {
        singleton = {{invalid}};
        rejects<std::out_of_range>([&] { (void)sirkit::FloydWarshall(singleton, next, inf); });
    }
    rejects<std::out_of_range>([&] { (void)sirkit::FloydWarshallPath(next, -1, 0); });
    rejects<std::out_of_range>([&] { (void)sirkit::FloydWarshallPath(next, 0, -1); });
    rejects<std::out_of_range>([] { (void)sirkit::FloydWarshallPath({{0, -2}, {-1, 1}}, 0, 1); });
    rejects<std::out_of_range>([] { (void)sirkit::FloydWarshallPath({{0, 2}, {-1, 1}}, 0, 1); });
    rejects<std::invalid_argument>([] { (void)sirkit::FloydWarshallPath({{0, 0}, {-1, 1}}, 0, 1); });
#endif
    check_wide_distance(sirkit::kInf);
    check_wide_distance(std::numeric_limits<std::int64_t>::max());
    for (auto weight : {std::int64_t{7}, std::numeric_limits<std::int64_t>::max() / 3}) {
        constexpr auto kWideInf = std::numeric_limits<std::int64_t>::max();
        // 非負の2頂点グラフで、通常幅の加算を安全とする限度をまたぐ。
        Matrix cost{{0, weight}, {kWideInf, 0}};
        const auto expected_cost = cost;
        assert(!sirkit::FloydWarshall(cost, next, kWideInf) && cost == expected_cost);
        assert((sirkit::FloydWarshallPath(next, 0, 1) == std::vector{0, 1}));
    }

    Matrix chain(65, std::vector<std::int64_t>(65, sirkit::kInf));
    for (int v = 0; v != 64; ++v) chain[v][v + 1] = 3;
    assert(!sirkit::FloydWarshall(chain, next));
    assert(chain[0][64] == 192 && sirkit::FloydWarshallPath(next, 0, 64).size() == 65);
}
