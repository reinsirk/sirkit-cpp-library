// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/Persistent_Segment_Tree.cpp"
#endif
#include HEADER

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

int main() {
    using Value = std::int64_t;
    using Tree = sirkit::PersistentSegmentTree<Value, std::plus<Value>>;
    const std::vector<Value> initial{2, 4, 1, 0, 8, 3, 6};
    Tree tree(initial, {}, 0);
    std::vector<Tree::Root> roots{tree.latest_root(), tree.empty_root()};
    std::vector<std::vector<Value>> values{initial, std::vector<Value>(initial.size())};
    std::mt19937 random(1043);
    for (int i = 0; i < 256; ++i) {
        const std::size_t base = random() % roots.size();
        const std::size_t position = random() % initial.size();
        const Value delta = random() % 11;
        auto next = values[base];
        next[position] += delta;
        roots.push_back(tree.transform(roots[base], position, [delta](Value x) { return x + delta; }));
        values.push_back(std::move(next));
    }
    // 更新後も、分岐元を含むすべての版が自身の配列と一致する。
    for (std::size_t version = 0; version < roots.size(); ++version) {
        const auto& reference = values[version];
        for (std::size_t left = 0; left <= reference.size(); ++left) {
            for (std::size_t right = left; right <= reference.size(); ++right) {
                const auto expected = std::accumulate(reference.begin() + left, reference.begin() + right, Value{0});
                assert(tree.prod(roots[version], left, right) == expected);
            }
        }
    }
}
