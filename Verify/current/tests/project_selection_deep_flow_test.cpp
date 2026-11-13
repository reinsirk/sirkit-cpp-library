// SPDX-License-Identifier: CC0-1.0
// 深さ64の境界で、分岐・合流・逆辺・増分追加を独立Edmonds–Karpと照合する。
#ifndef SIRKIT_PROJECT_TEST_HEADER
#define SIRKIT_PROJECT_TEST_HEADER "../../../Graph/Project_Selection.cpp"
#endif
#include SIRKIT_PROJECT_TEST_HEADER
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
std::uint64_t checks = 0;
void require(bool condition) {
    ++checks;
    if (!condition) {
        std::cerr << "project selection deep-flow check failed: " << checks << '\n';
        std::abort();
    }
}
struct Arc { int from, to; std::int64_t capacity; bool hard; };

template<class Cost> class Case {
    int n_;
    std::vector<Arc> arcs_;
    std::int64_t finite_ = 0;
public:
    TwoVariableSubmodularOpt<Cost> model;
    explicit Case(int n) : n_(n), model(n) {}
    void source(int v, int capacity) {
        model.add_single_cost(v, Cost(capacity), Cost{0});
        arcs_.push_back({n_, v, capacity, false}); finite_ += capacity;
    }
    void sink(int v, int capacity) {
        model.add_single_cost(v, Cost{0}, Cost(capacity));
        arcs_.push_back({v, n_ + 1, capacity, false}); finite_ += capacity;
    }
    void link(int from, int to, int capacity = -1) {
        if (capacity < 0) model.add_psp_constraint(from, to);
        else model.add_psp_penalty(from, to, Cost(capacity));
        arcs_.push_back({from, to, capacity, capacity < 0});
        if (capacity >= 0 && from != to) finite_ += capacity;
    }
    void force(int v, bool value) {
        model.force(v, value);
        arcs_.push_back({value ? n_ : v, value ? v : n_ + 1, 0, true});
    }
    void verify() {
        // 参照側は毎回元の項から再構築し、有限容量総和+1を厳密制約に使う。
        const int count = n_ + 2, source = n_, sink = n_ + 1;
        const auto index = [count](int from, int to) { return std::size_t(from) * count + to; };
        std::vector<std::int64_t> residual(std::size_t(count) * count);
        std::vector<std::vector<int>> adjacent(count);
        const std::int64_t hard = finite_ + 1;
        for (const auto& arc : arcs_) {
            if (arc.from == arc.to) continue;
            residual[index(arc.from, arc.to)] += arc.hard ? hard : arc.capacity;
            adjacent[arc.from].push_back(arc.to);
            adjacent[arc.to].push_back(arc.from);
        }
        std::int64_t flow = 0;
        std::vector<int> parent(count), queue(count);
        for (;;) {
            std::fill(parent.begin(), parent.end(), -1);
            parent[source] = source;
            queue[0] = source;
            int head = 0, tail = 1;
            while (head != tail && parent[sink] < 0) {
                const int from = queue[head++];
                for (int to : adjacent[from]) {
                    if (parent[to] >= 0 || residual[index(from, to)] == 0) continue;
                    parent[to] = from;
                    queue[tail++] = to;
                }
            }
            if (parent[sink] < 0) break;
            std::int64_t amount = hard;
            for (int to = sink; to != source; to = parent[to])
                amount = std::min(amount, residual[index(parent[to], to)]);
            for (int to = sink; to != source; to = parent[to]) {
                residual[index(parent[to], to)] -= amount;
                residual[index(to, parent[to])] += amount;
            }
            flow += amount;
            if (flow >= hard) break;
        }
        if (flow >= hard) {
            bool failed = false;
            try { (void)model.solve(); } catch (const std::domain_error&) { failed = true; }
            require(failed);
            return;
        }
        require(model.solve() == flow);
        require(model.solve() == flow);
        const auto assignment = model.reconstruct();
        std::int64_t cost = 0;
        for (const auto& arc : arcs_) {
            const bool from = arc.from == source || (arc.from != sink && assignment[arc.from]);
            const bool to = arc.to == source || (arc.to != sink && assignment[arc.to]);
            if (from && !to) {
                require(!arc.hard);
                cost += arc.capacity;
            }
        }
        require(cost == flow);
    }
};

template<class Cost> void branches(std::uint64_t seed) {
    constexpr std::array lengths{61, 62, 63, 64, 65, 66, 129};
    std::array<int, lengths.size()> begin{}, end{};
    int n = 0;
    for (std::size_t i = 0; i < lengths.size(); ++i) {
        begin[i] = n; n += lengths[i]; end[i] = n - 1;
    }
    Case<Cost> test(n);
    for (std::size_t i = 0; i < lengths.size(); ++i)
        for (int v = begin[i]; v != end[i]; ++v) test.link(v, v + 1);
    test.verify();
    for (std::size_t i = 0; i < lengths.size(); ++i) {
        test.source(begin[i], 1 + int(i % 3));
        test.sink(end[i], 1 + int((i + 1) % 3));
        test.verify();
    }
    std::mt19937_64 random(seed);
    for (int step = 0; step < 36; ++step) {
        const int from = int(random() % n), to = int(random() % n);
        test.link(from, to, step % 4 == 0 ? -1 : 1 + int(random() % 2));
        if (step % 3 == 2) test.verify();
    }
    test.source(begin.back(), 3);
    test.sink(end.front(), 3);
    test.verify();
    test.force(begin.back(), true);
    test.verify();
    test.force(end.back(), false);
    test.verify();
    test.verify();
}

template<class Cost> void reverse_path() {
    // 先に A->X を流した後、B->X->A->Y の逆辺を通らなければ流量2にならない。
    for (int length : {61, 62, 63, 64, 65, 129}) {
        const int a = length, b = length + 1, x = length + 2, y = length + 3;
        Case<Cost> test(length + 4);
        test.source(0, 1);
        for (int v = 0; v + 1 < length; ++v) test.link(v, v + 1);
        test.link(length - 1, a);
        test.link(a, x, 1);
        test.link(a, y, 1);
        test.sink(x, 1);
        test.sink(y, 1);
        test.verify();
        test.source(b, 1);
        test.link(b, x, 1);
        test.verify();
    }
}
template<class Cost> void fork_join() {
    // 反復DFSのフレーム内で、複数の子から部分流量を集約する。
    for (int length : {63, 64, 65, 130}) {
        const int first = length, second = length + 1, third = length + 2, join = length + 3;
        Case<Cost> test(length + 4);
        test.source(0, 6);
        for (int v = 0; v + 1 < length; ++v) test.link(v, v + 1);
        test.link(length - 1, first, 1);
        test.link(length - 1, second, 2);
        test.link(length - 1, third, 3);
        test.link(first, join);
        test.link(second, join);
        test.link(third, join);
        test.sink(join, 5);
        test.verify();
        test.sink(join, 1);
        test.verify();
        test.source(0, 2);
        test.link(length - 1, first, 2);
        test.sink(first, 2);
        test.verify();
    }
}
template<class Cost> void run() {
    for (std::uint64_t seed : {20261025, 64, 129, 777}) branches<Cost>(seed);
    reverse_path<Cost>();
    fork_join<Cost>();
}
}
int main() {
    run<std::int8_t>(); run<std::int16_t>(); run<std::int32_t>(); run<std::int64_t>();
    std::cout << checks << " independent deep-flow checks passed\n";
}
