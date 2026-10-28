// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <memory>
#include <vector>

int main() {
    int n{}, start{};
    if (!(verify_in >> n >> start)) return 0;
    std::vector<std::shared_ptr<Node>> nodes(n);
    for (auto& node : nodes) node = make_node();
    for (int i = 0; i < n; ++i) {
        int next{};
        verify_in >> next;
        nodes[i]->n = nodes[next - 1];
    }
    auto head = nodes[start - 1];
    break_cycle(head);
    verify_out << chain_size(head) << '\n';

    // 始点から到達しない成分の循環も切ってから解放する。
    for (auto& node : nodes) node->n.reset();
}
