// SPDX-License-Identifier: CC0-1.0
// solve準備の確保失敗後、閾値をまたぐ追加・再解法・所有権操作を独立した割当評価と照合する。
#ifndef SIRKIT_PROJECT_TEST_HEADER
#define SIRKIT_PROJECT_TEST_HEADER "../../../Graph/Project_Selection.cpp"
#endif
#include SIRKIT_PROJECT_TEST_HEADER
#include SIRKIT_PROJECT_TEST_HEADER

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <utility>

namespace {
std::int64_t allocation_budget = -1;
std::size_t live_allocations = 0;
std::size_t checks = 0;
std::size_t failed_preparations = 0;
void require(bool value) {
    ++checks;
    if (!value) std::abort();
}
}

[[gnu::noinline]] void* operator new(std::size_t bytes) {
    if (allocation_budget == 0) throw std::bad_alloc();
    if (allocation_budget > 0) --allocation_budget;
    if (void* value = std::malloc(bytes ? bytes : 1)) {
        ++live_allocations;
        return value;
    }
    throw std::bad_alloc();
}
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete(void* value) noexcept {
    if (value) --live_allocations;
    std::free(value);
}
void operator delete[](void* value) noexcept { ::operator delete(value); }
void operator delete(void* value, std::size_t) noexcept { ::operator delete(value); }
void operator delete[](void* value, std::size_t) noexcept { ::operator delete(value); }

namespace {
struct Energy {
    bool prefix_reward = false;
    int second_reward = 0;
    int third_reward = 0;

    std::int64_t evaluate(bool first, bool second, bool third, bool fourth) const {
        return (first ? 3 : 5) + (second ? 4 : 0)
             - (prefix_reward && second ? 2 : 0)
             - (third ? second_reward : 0) - (fourth ? third_reward : 0);
    }
    std::int64_t minimum() const {
        auto result = std::numeric_limits<std::int64_t>::max();
        for (unsigned mask = 0; mask != 16; ++mask)
            result = std::min(result, evaluate(mask & 1, mask & 2, mask & 4, mask & 8));
        return result;
    }
};

template <class Cost>
void base_terms(TwoVariableSubmodularOpt<Cost>& model) {
    model.add_single_cost(0, Cost{5}, Cost{0});
    model.add_single_cost(0, Cost{0}, Cost{3});
    model.add_single_cost(1, Cost{0}, Cost{4});
}

template <class Cost>
void verify(TwoVariableSubmodularOpt<Cost>& model, const Energy& energy) {
    const auto expected = energy.minimum();
    require(model.solve() == expected);
    require(model.solve() == expected);
    const auto assignment = model.reconstruct();
    require(assignment.size() == static_cast<std::size_t>(model.variable_count()));
    require(energy.evaluate(assignment[0], assignment[1], assignment[2], assignment[3]) == expected);
    const auto result = model.solve_with_assignment();
    require(result.first == expected);
    require(energy.evaluate(result.second[0], result.second[1], result.second[2], result.second[3]) == expected);
}

template <class Cost>
void cross_threshold(TwoVariableSubmodularOpt<Cost>& model, Energy energy) {
    require(model.node_count() < 65536);
    unsigned turn = 0;
    while (model.node_count() < 65536) {
        const int variable = 2 + static_cast<int>(turn++ % 2);
        model.add_all_true_profit({variable}, Cost{1});
        if (variable == 2) ++energy.second_reward;
        else ++energy.third_reward;
    }
    require(model.node_count() == 65536);
    verify(model, energy);
    // 大規模経路へ入った後も残余網を使って項を追加できる。
    model.add_all_true_profit({3}, Cost{1});
    ++energy.third_reward;
    verify(model, energy);
}

template <class Cost>
void one_case(int shape, int budget, int action) {
    const int n = shape == 0 ? 65532 : shape == 1 ? 65533 : 65531;
    TwoVariableSubmodularOpt<Cost> model(n);
    base_terms(model);
    Energy energy;
    if (shape == 2) {
        verify(model, energy);
        model.add_all_true_profit({1}, Cost{2});
        energy.prefix_reward = true;
    }
    const auto nodes = model.node_count();
    const auto edges = model.edge_count();
    bool failed = false;
    allocation_budget = budget;
    try { (void)model.solve(); }
    catch (const std::bad_alloc&) { failed = true; ++failed_preparations; }
    allocation_budget = -1;
    require(failed == (budget < 4)); // level/cursor/queue/stackの4回の予約。
    require(model.variable_count() == n && model.node_count() == nodes && model.edge_count() == edges);

    if (action == 1) {
        auto copy = model;
        cross_threshold(copy, energy);
        cross_threshold(model, energy);
    } else if (action == 2) {
        auto moved = std::move(model);
        require(model.variable_count() == 0 && model.solve() == 0);
        cross_threshold(moved, energy);
    } else if (action == 3) {
        allocation_budget = 0;
        model.clear();
        allocation_budget = -1;
        require(model.variable_count() == n && model.edge_count() == 0);
        base_terms(model);
        energy = {};
        cross_threshold(model, energy);
    } else {
        if (action == 4) verify(model, energy);
        cross_threshold(model, energy);
    }
}

template <class Cost>
void all_cases() {
    for (int shape = 0; shape != 3; ++shape) {
        for (int budget = 0; budget != 5; ++budget) one_case<Cost>(shape, budget, 0);
        // 新しいlevel要素が確保済みでcursor確保に失敗した状態を共有・移動・初期化・再解法する。
        for (int action = 1; action != 5; ++action) one_case<Cost>(shape, 1, action);
    }
}
} // namespace

int main() {
    const auto before = live_allocations;
    all_cases<std::int8_t>();
    all_cases<std::int64_t>();
    require(failed_preparations == 48);
    require(live_allocations == before);
    std::cout << "PASS " << checks << " failed-prepare checks, " << failed_preparations
              << " allocation failures, live_delta=0\n";
}
