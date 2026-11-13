// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Cartesian_Tree.cpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <span>
#include <utility>
#include <vector>
int main() {
    const std::vector<std::int64_t> values{4,2,2,5,1,8,1};
    CartesianTree tree(values);
    std::cout << tree.size() << ' ' << tree.root() << '\n';
    for (auto p : tree.parents()) std::cout << p << ' ';
    std::cout << '\n';
    for (std::int32_t v = 0; v < static_cast<std::int32_t>(tree.size()); ++v) {
        const auto [left,right] = tree.children(v);
        std::cout << left << ',' << right << ' ';
    }
    std::cout << '\n';
    CartesianTree maximum(values,std::greater<>{});
    std::cout << maximum.root() << ' ' << maximum.left_child(maximum.root()) << ' ' << maximum.right_child(maximum.root()) << '\n';
    auto copy = tree;
    copy.build(std::span(values).first(4));
    std::cout << copy.root() << ' ' << tree.root() << '\n';
    CartesianTree moved(std::move(copy));
    std::cout << copy.empty() << ' ' << copy.root() << ' ' << moved.size() << '\n';
    swap(moved,copy);
    copy.clear(); tree.build(std::span<const std::int64_t>{});
    std::cout << tree.empty() << ' ' << copy.empty() << ' ' << moved.empty() << '\n';
}
