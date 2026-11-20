// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstdint>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

#include "../Graph/LCA.cpp"

namespace {

enum class TreeShape { chain, balanced };

constexpr int kSmallVertices = 4'096;
constexpr int kLargeVertices = 524'288;
constexpr int kQueries = 65'536;
constexpr std::uint32_t kSeed = 20'261'120;
using PairList = std::vector<std::pair<int, int>>;

bool validation_failed = false;

PairList make_edges(int vertices, TreeShape shape) {
    PairList edges;
    edges.reserve(vertices - 1);
    for (int v = 1; v < vertices; ++v) {
        const int parent = shape == TreeShape::chain ? v - 1 : (v - 1) / 2;
        edges.emplace_back(parent, v);
    }
    return edges;
}

PairList make_queries(int vertices) {
    std::mt19937 random(kSeed);
    PairList queries;
    queries.reserve(kQueries);
    for (int i = 0; i < kQueries; ++i) {
        const int u = static_cast<int>(random() % vertices);
        const int v = static_cast<int>(random() % vertices);
        queries.emplace_back(u, v);
    }
    return queries;
}

int reference_lca(TreeShape shape, int u, int v) {
    if (shape == TreeShape::chain) return std::min(u, v);
    while (u != v) {
        if (u < v) std::swap(u, v);
        u = (u - 1) / 2;
    }
    return u;
}

void report_mismatch(benchmark::State& state) {
    validation_failed = true;
    state.SkipWithError("LCA result does not match the independent reference");
}

std::optional<std::int64_t> validate(const sirkit::LCA& index, TreeShape shape, const PairList& queries) {
    std::int64_t sum = 0;
    for (auto [u, v] : queries) {
        const int expected = reference_lca(shape, u, v);
        if (index.get(u, v) != expected) return std::nullopt;
        sum += expected;
    }
    return sum;
}

void LcaBuild(benchmark::State& state, TreeShape shape) {
    const int vertices = static_cast<int>(state.range(0));
    const auto edges = make_edges(vertices, shape);
    if (!validate(sirkit::LCA(vertices, edges), shape, make_queries(vertices))) {
        report_mismatch(state);
        return;
    }

    std::optional<sirkit::LCA> index;
    for (auto _ : state) {
        index.emplace(vertices, edges);
        auto* built = &*index;
        benchmark::DoNotOptimize(built);
        benchmark::ClobberMemory();
        state.PauseTiming();
        index.reset();
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * vertices);
}

void LcaQuery(benchmark::State& state, TreeShape shape) {
    const int vertices = static_cast<int>(state.range(0));
    const auto queries = make_queries(vertices);
    const sirkit::LCA index(vertices, make_edges(vertices, shape));
    const auto expected = validate(index, shape, queries);
    if (!expected) {
        report_mismatch(state);
        return;
    }

    std::int64_t sum = 0;
    while (state.KeepRunningBatch(kQueries)) {
        // 同じ照会列の再計算をコンパイラがループ外へ移さないようにする。
        benchmark::ClobberMemory();
        sum = 0;
        for (auto [u, v] : queries) sum += index.get(u, v);
        benchmark::DoNotOptimize(sum);
    }
    if (sum != *expected) {
        report_mismatch(state);
        return;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_CAPTURE(LcaBuild, chain, TreeShape::chain)
    ->ArgName("vertices")->Arg(kSmallVertices)->Arg(kLargeVertices)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(LcaBuild, balanced, TreeShape::balanced)
    ->ArgName("vertices")->Arg(kSmallVertices)->Arg(kLargeVertices)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(LcaQuery, chain, TreeShape::chain)
    ->ArgName("vertices")->Arg(kSmallVertices)->Arg(kLargeVertices)->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(LcaQuery, balanced, TreeShape::balanced)
    ->ArgName("vertices")->Arg(kSmallVertices)->Arg(kLargeVertices)->Unit(benchmark::kNanosecond);

}  // namespace

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::AddCustomContext("sirkit_lca_sha256", SIRKIT_BENCHMARK_LCA_SHA256);
    benchmark::AddCustomContext("sirkit_compiler", SIRKIT_BENCHMARK_COMPILER);
    benchmark::AddCustomContext("sirkit_build_type", SIRKIT_BENCHMARK_BUILD_TYPE);
#ifdef SKIP_RUNTIME_CHECKS
    benchmark::AddCustomContext("sirkit_runtime_checks", "disabled");
#else
    benchmark::AddCustomContext("sirkit_runtime_checks", "enabled");
#endif
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return validation_failed ? 1 : 0;
}
