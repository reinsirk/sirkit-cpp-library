// SPDX-License-Identifier: CC0-1.0
// 添字で指定した入口・切断辺・所有者数を照合する。
#ifndef SIRKIT_NODE_SOURCE
#define SIRKIT_NODE_SOURCE "../../../DataStructure/Node.cpp"
#endif
#include SIRKIT_NODE_SOURCE
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace {
std::size_t checks = 0;
void require(bool ok) {
    ++checks;
    if (!ok) std::abort();
}

void check_cycle(std::size_t prefix, std::size_t period, bool member_argument) {
    std::vector<std::shared_ptr<Node>> nodes(prefix + period);
    for (auto& node : nodes) node = make_node();
    for (std::size_t i = 1; i < nodes.size(); ++i) nodes[i - 1]->n = nodes[i];
    nodes.back()->n = nodes[prefix];
    auto parent = make_node(nodes.front());
    auto head = nodes.front();
    auto shared_head = head;
    std::vector<std::int64_t> owners;
    for (const auto& node : nodes) owners.push_back(node.use_count());
    require(has_cycle(head));
    require(cycle_entry(head) == nodes[prefix].get());
    require(break_cycle(member_argument ? parent->n : head));
    require(head == shared_head && parent->n == head);
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        require(nodes[i]->n == (i + 1 < nodes.size() ? nodes[i + 1] : nullptr));
        require(nodes[i].use_count() == owners[i] - (i == prefix));
    }
    require(!has_cycle(head));
    require(cycle_entry(head) == nullptr);
    require(chain_size(head) == nodes.size());
    require(!break_cycle(head));
}

void check_owning_member(std::size_t period) {
    auto head = make_chain(period);
    Node* first = head.get();
    Node* last = nullptr;
    std::vector<std::weak_ptr<Node>> weak;
    for (auto p = head; p; p = p->n) {
        last = p.get();
        weak.push_back(p);
    }
    last->n = head;
    head.reset();
    require(has_cycle(last->n));
    require(cycle_entry(last->n) == first);
    // 切断するshared_ptr自体を引数にし、唯一の循環所有から解放する。
    require(break_cycle(last->n));
    for (const auto& p : weak) require(p.expired());
}
} // namespace

int main() {
    for (std::size_t prefix = 0; prefix <= 128; ++prefix)
        for (std::size_t period = 1; period <= 128; ++period)
            check_cycle(prefix, period, (prefix + period) & 1);
    for (std::size_t n = 0; n <= 256; ++n) {
        auto head = make_chain(n);
        require(!has_cycle(head));
        require(cycle_entry(head) == nullptr);
        require(!break_cycle(head));
        require(chain_size(head) == n);
    }
    for (std::size_t period = 1; period <= 256; ++period) check_owning_member(period);
    std::cout << "PASS " << checks << " Node cycle model checks\n";
}
