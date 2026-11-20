// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_DISJOINT_TEST_SOURCE
#define SIRKIT_DISJOINT_TEST_SOURCE "../../../DataStructure/Disjoint_Sparse_Table.cpp"
#endif
#include SIRKIT_DISJOINT_TEST_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }

template<class T>
struct ConstSum {
    T operator()(const T& a, const T& b) const { return a + b; }
    // const照会がmutableな2要素を渡す実装はコンパイル時に拒否する。
    T operator()(T&, T&) const = delete;
};
struct MoveOnlyOp {
    std::unique_ptr<int> state;
    MoveOnlyOp() = default;
    MoveOnlyOp(const MoveOnlyOp&) = delete;
    MoveOnlyOp(MoveOnlyOp&&) = default;
    MoveOnlyOp& operator=(const MoveOnlyOp&) = delete;
    MoveOnlyOp& operator=(MoveOnlyOp&&) = default;
    std::int32_t operator()(const std::int32_t& a, const std::int32_t& b) const { return a + b; }
};
static_assert(!std::is_copy_constructible_v<DisjointSparseTable<std::int32_t, MoveOnlyOp>>);
static_assert(!std::is_copy_constructible_v<DisjointSparseTable<std::int16_t, MoveOnlyOp>>);

template<class T>
void run(std::size_t n, std::mt19937_64& random) {
    using Tree = DisjointSparseTable<T, ConstSum<T>>;
    static_assert(std::is_copy_constructible_v<Tree> && std::is_copy_assignable_v<Tree>);
    static_assert(std::is_same_v<decltype(std::declval<const Tree&>().get(0)), const T&>);
    static_assert(std::is_same_v<decltype(std::declval<const Tree&>().values()), std::vector<T>>);
    std::vector<T> values(n);
    for (auto& x : values) x = static_cast<T>(random() % 97 + 1);
    const auto verify = [&](const Tree& tree, const std::vector<T>& expected) {
        check(tree.size() == expected.size() && tree.empty() == expected.empty());
        check(tree.values() == expected);
        std::vector<std::int64_t> prefix(expected.size() + 1);
        for (std::size_t i = 0; i < expected.size(); ++i) {
            prefix[i + 1] = prefix[i] + static_cast<std::int64_t>(expected[i]);
            check(tree.get(i) == expected[i]);
            check(tree.prod(i, i + 1) == expected[i]);
        }
        check(tree.all_prod() == static_cast<T>(prefix.back()));
        check(tree.prod(0, 0) == T{} && tree.prod(expected.size(), expected.size()) == T{});
        for (int trial = 0; trial < 80; ++trial) {
            auto l = static_cast<std::size_t>(random() % (expected.size() + 1));
            auto r = static_cast<std::size_t>(random() % (expected.size() + 1));
            if (l > r) std::swap(l, r);
            check(tree.prod(l, r) == static_cast<T>(prefix[r] - prefix[l]));
        }
    };
    Tree source(values);
    const Tree& constant = source;
    verify(constant, values);
    Tree copied(constant), assigned;
    assigned = constant;
    if (n) {
        check(&copied.get(0) != &constant.get(0));
        check(&assigned.get(0) != &constant.get(0));
        check(&copied.get(0) != &assigned.get(0));
    }
    source.clear();
    verify(source, {});
    verify(copied, values);
    verify(assigned, values);
    Tree moved(std::move(copied));
    verify(copied, {});
    verify(moved, values);
    copied.assign(values);
    verify(copied, values);
    const std::vector<T> replacement{T{8}, T{9}};
    assigned.assign(replacement);
    moved.swap(assigned);
    verify(moved, replacement);
    verify(assigned, values);
    source = std::move(assigned);
    verify(source, values);
    verify(assigned, {});
    auto* same = &source;
    source = *same;
    source = std::move(*same);
    verify(source, values);
    auto detached = [&] {
        Tree original(values);
        Tree duplicate(original);
        if (n) check(&duplicate.get(0) != &original.get(0));
        return duplicate;
    }();
    // コピー元の寿命を終えてから、全ての取得を再実行する。
    verify(detached, values);
}

template<class T>
void type_cases(std::mt19937_64& random) {
    for (const auto n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 127, 128, 129,
                        1023, 1024, 1025, 4095, 4096, 4097, 8191, 8192, 8193})
        run<T>(static_cast<std::size_t>(n), random);
}
}

int main() {
    std::mt19937_64 random(0x79ac85196519ULL);
    type_cases<std::int32_t>(random);
    type_cases<std::uint32_t>(random);
    type_cases<std::int64_t>(random);
    type_cases<std::uint64_t>(random);
    std::printf("Disjoint owned rows/const/copy checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
