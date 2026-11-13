// SPDX-License-Identifier: CC0-1.0
// SCC切替・長い追跡・非負成分で、全確保位置の失敗・出力保証・解放を確認する。
#ifndef SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#define SIRKIT_TEST_NEGATIVE_PATH_SOURCE "../../../Graph/Bellman-Ford_SFPA.cpp"
#endif
#include SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace {
std::int64_t allocation_budget = -1;
std::size_t live_allocations = 0, live_bytes = 0;
std::uint64_t allocation_calls = 0;
struct AllocationHeader { void* raw; std::size_t bytes; };

void* allocate_test(std::size_t bytes, std::size_t alignment) {
    ++allocation_calls;
    if (allocation_budget == 0) throw std::bad_alloc();
    if (allocation_budget > 0) --allocation_budget;
    alignment = std::max(alignment, alignof(std::max_align_t));
    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (alignment > maximum - sizeof(AllocationHeader)) throw std::bad_alloc();
    const auto extra = sizeof(AllocationHeader) + alignment - 1;
    const auto requested = std::max(bytes, std::size_t{1});
    if (requested > maximum - extra) throw std::bad_alloc();
    void* raw = std::malloc(requested + extra);
    if (!raw) throw std::bad_alloc();
    void* cursor = static_cast<std::byte*>(raw) + sizeof(AllocationHeader);
    auto space = requested + alignment - 1;
    void* pointer = std::align(alignment, requested, cursor, space);
    if (!pointer) { std::free(raw); throw std::bad_alloc(); }
    auto* header = static_cast<AllocationHeader*>(pointer) - 1;
    header->raw = raw; header->bytes = bytes;
    ++live_allocations; live_bytes += bytes;
    return pointer;
}
void free_test(void* pointer) noexcept {
    if (!pointer) return;
    auto* header = static_cast<AllocationHeader*>(pointer) - 1;
    --live_allocations; live_bytes -= header->bytes;
    std::free(header->raw);
}
void require(bool condition) {
    if (!condition) throw std::runtime_error("SPFA SCC allocation check failed");
}
std::uint64_t fingerprint(const WGraph& graph) {
    std::uint64_t value = graph.size();
    for (const auto& row : graph) {
        value = value * 1000003 + row.size();
        for (const auto& edge : row)
            value = (value ^ static_cast<std::uint64_t>(edge.to)) * 1000033 + static_cast<std::uint64_t>(edge.w);
    }
    return value;
}
struct Statistics { std::size_t failures = 0; std::uint64_t allocations = 0; };
struct Work {
    std::vector<std::int64_t> distance;
    std::vector<int> sources, negative;
    bool negative_result = false;
#ifndef SKIP_RUNTIME_CHECKS
    std::vector<std::uint8_t> checked;
#endif
    Work(int n, bool direct) : distance(direct ? std::size_t(n) : 2, direct ? INF : 321) {
        if (!direct) { distance[1] = 654; return; }
        distance[0] = 0;
        for (int i = 0; i < n; ++i) sources.push_back(i);
#ifndef SKIP_RUNTIME_CHECKS
        checked.assign(std::size_t(n), 0);
#endif
    }
};
template<bool Direct> void invoke(const WGraph& graph, Work& work) {
    if constexpr (Direct) {
        reinsirk::detail::SPFAComponents(graph, work.sources, work.distance, INF,
            -static_cast<std::int64_t>(graph.size()) * 1'000'000'000, work.negative
#ifndef SKIP_RUNTIME_CHECKS
            , work.checked
#endif
        );
    } else {
#ifdef SIRKIT_TEST_SPFA_FORCE_UNBOUNDED
        work.negative_result = reinsirk::detail::SPFARun<false, false>(graph, work.distance, 0);
#else
        work.negative_result = SPFA(graph, work.distance, 0);
#endif
    }
}
template<bool Direct> void check_result(const WGraph& graph, const Work& work,
                                      const std::vector<std::int64_t>& expected) {
    const auto lower = -static_cast<std::int64_t>(graph.size()) * 1'000'000'000;
    require(work.distance.size() == graph.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        require(work.distance[i] == (Direct && expected[i] == -INF ? lower : expected[i]));
    if constexpr (!Direct)
        require(work.negative_result == (std::find(expected.begin(), expected.end(), -INF) != expected.end()));
}
template<bool Direct> Statistics inject(const WGraph& graph, const std::vector<std::int64_t>& expected) {
    const auto n = static_cast<int>(graph.size());
    const auto graph_hash = fingerprint(graph);
    const auto initial_live = live_allocations, initial_bytes = live_bytes;
    Statistics result;
    bool finished = false;
    for (std::int64_t budget = 0; budget < 1024 && !finished; ++budget) {
        {
            Work work(n, Direct);
            const auto calls_before = allocation_calls;
            bool failed = false;
            allocation_budget = budget;
            try { invoke<Direct>(graph, work); }
            catch (const std::bad_alloc&) { failed = true; }
            allocation_budget = -1;
            if (failed) {
                ++result.failures;
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr (!Direct)
                    require(work.distance == std::vector<std::int64_t>({321, 654}));
#endif
                // 公開APIは同じ出力配列で再試行。内部ヘルパーは新しい作業状態で再試行する。
                if constexpr (!Direct) { invoke<Direct>(graph, work); check_result<Direct>(graph, work, expected); }
                else { Work retry(n, true); invoke<Direct>(graph, retry); check_result<Direct>(graph, retry, expected); }
            } else {
                result.allocations = allocation_calls - calls_before;
                require(result.allocations == static_cast<std::uint64_t>(budget));
                check_result<Direct>(graph, work, expected);
                finished = true;
            }
        }
        require(fingerprint(graph) == graph_hash);
        require(live_allocations == initial_live && live_bytes == initial_bytes);
    }
    require(finished && result.failures > 0);
    require(result.failures == result.allocations);
    return result;
}
} // namespace

void* operator new(std::size_t n) { return allocate_test(n, alignof(std::max_align_t)); }
void* operator new[](std::size_t n) { return allocate_test(n, alignof(std::max_align_t)); }
void* operator new(std::size_t n, std::align_val_t a) { return allocate_test(n, static_cast<std::size_t>(a)); }
void* operator new[](std::size_t n, std::align_val_t a) { return allocate_test(n, static_cast<std::size_t>(a)); }
void* operator new(std::size_t n, const std::nothrow_t&) noexcept { try { return allocate_test(n, alignof(std::max_align_t)); } catch (...) { return nullptr; } }
void* operator new[](std::size_t n, const std::nothrow_t&) noexcept { try { return allocate_test(n, alignof(std::max_align_t)); } catch (...) { return nullptr; } }
void* operator new(std::size_t n, std::align_val_t a, const std::nothrow_t&) noexcept { try { return allocate_test(n, static_cast<std::size_t>(a)); } catch (...) { return nullptr; } }
void* operator new[](std::size_t n, std::align_val_t a, const std::nothrow_t&) noexcept { try { return allocate_test(n, static_cast<std::size_t>(a)); } catch (...) { return nullptr; } }
void operator delete(void* p) noexcept { free_test(p); }
void operator delete[](void* p) noexcept { free_test(p); }
void operator delete(void* p, std::size_t) noexcept { free_test(p); }
void operator delete[](void* p, std::size_t) noexcept { free_test(p); }
void operator delete(void* p, std::align_val_t) noexcept { free_test(p); }
void operator delete[](void* p, std::align_val_t) noexcept { free_test(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { free_test(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { free_test(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { free_test(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { free_test(p); }
void operator delete(void* p, std::align_val_t, const std::nothrow_t&) noexcept { free_test(p); }
void operator delete[](void* p, std::align_val_t, const std::nothrow_t&) noexcept { free_test(p); }

int main() {
    const auto initial_live = live_allocations, initial_bytes = live_bytes;
    Statistics public_total, direct_total;
    std::size_t graphs = 0;
    const auto run_graph = [&](const WGraph& graph, const std::vector<std::int64_t>& expected) {
        const auto public_result = inject<false>(graph, expected), direct_result = inject<true>(graph, expected);
        public_total.failures += public_result.failures; public_total.allocations += public_result.allocations;
        direct_total.failures += direct_result.failures; direct_total.allocations += direct_result.allocations;
        ++graphs;
    };
    for (const int n : {65, 127, 257}) for (int kind = 0; kind < (n == 257 ? 3 : 2); ++kind) {
        WGraph graph(static_cast<std::size_t>(n));
        for (int i = 1; i < n; ++i) graph[0].emplace_back(i, 0);
        if (kind != 1)
            for (int i = 0; i < n; ++i) graph[i].emplace_back((i + 1) % n, -1);
        // 全頂点の負自己辺は、frontierの閾値を持つSCC切替でも毎パス全頂点を更新する。
        if (kind != 0)
            for (int i = 0; i < n; ++i) graph[i].emplace_back(i, -1);
        if (kind == 2)
            for (int i = 0; i < n; ++i) for (int j = 1; j <= 3; ++j)
                graph[i].emplace_back((i * 17 + j * 31) % n, (i + j) % 7);
        run_graph(graph, std::vector<std::int64_t>(graph.size(), -INF));
    }
    {
        constexpr int count = 64; WGraph graph(2 * count + 2); graph[0].emplace_back(1, 0);
        for (int i = 0; i < count; ++i) graph[1].emplace_back(i + 2, 1);
        for (int i = 0; i < count; ++i) graph[1].emplace_back(i + count + 2, 0);
        for (int i = 0; i < count; ++i) {
            graph[i + 2].emplace_back(1, 0); graph[i + count + 2].emplace_back(i + 2, -1);
        }
        std::vector<std::int64_t> expected(graph.size(), -INF); expected[0] = 0;
        run_graph(graph, expected);
    }
    {
        constexpr int size = 128; WGraph graph(size + 1);
        for (int i = size; i >= 1; --i) graph[0].emplace_back(i, size + 1 - i);
        for (int i = 1; i <= size; ++i) graph[i].emplace_back(i == 1 ? size : i - 1, 0);
        graph[size].emplace_back(size - 1, 0);
        std::vector<std::int64_t> expected(graph.size(), 1); expected[0] = 0;
        run_graph(graph, expected);
    }
    {
        constexpr int count = 128; WGraph graph(count + 2); graph[0].emplace_back(1, 0);
        for (int i = 1; i <= count; ++i) graph[0].emplace_back(i + 1, count + 1 - i);
        for (int i = 1; i <= count; ++i) graph[1].emplace_back(i + 1, count);
        for (int i = 1; i <= count; ++i) {
            graph[i + 1].emplace_back(1, 0); if (i >= 2) graph[i + 1].emplace_back(i, 0);
        }
        std::vector<std::int64_t> expected(graph.size(), 1); expected[0] = expected[1] = 0;
        run_graph(graph, expected);
    }
    {
        constexpr int size = 128, count = 64, center = size + 1, entry = size + 2 * count + 2, delay = entry + 1;
        WGraph graph(delay + 1);
        for (int i = size; i >= 1; --i) graph[0].emplace_back(i, size + 1 - i);
        for (int i = 1; i <= size; ++i) graph[i].emplace_back(i == 1 ? size : i - 1, 0);
        graph[size].emplace_back(size - 1, 0); graph[0].emplace_back(delay, 0);
        graph[delay].emplace_back(entry, 0); graph[entry].emplace_back(center, 0);
        for (int i = 0; i < count; ++i) graph[center].emplace_back(size + 2 + i, 1);
        for (int i = 0; i < count; ++i) graph[center].emplace_back(size + count + 2 + i, 0);
        for (int i = 0; i < count; ++i) {
            graph[size + 2 + i].emplace_back(center, 0); graph[size + count + 2 + i].emplace_back(size + 2 + i, -1);
        }
        std::vector<std::int64_t> expected(graph.size(), -INF); expected[0] = expected[entry] = expected[delay] = 0;
        for (int i = 1; i <= size; ++i) expected[i] = 1;
        run_graph(graph, expected);
    }
    require(live_allocations == initial_live && live_bytes == initial_bytes);
    std::cout << "{\"graphs\":" << graphs << ",\"public_injected_failures\":" << public_total.failures
              << ",\"helper_injected_failures\":" << direct_total.failures
              << ",\"successful_public_allocations\":" << public_total.allocations
              << ",\"successful_helper_allocations\":" << direct_total.allocations
              << ",\"retained_allocation_delta\":0,\"retained_byte_delta\":0}\n";
}
