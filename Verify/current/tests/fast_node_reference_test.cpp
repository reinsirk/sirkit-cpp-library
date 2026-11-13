// SPDX-License-Identifier: CC0-1.0
#ifndef FAST_NODE_SOURCE
#define FAST_NODE_SOURCE "../../../Fast/GCC/DataStructure/Node.cpp"
#endif
#include FAST_NODE_SOURCE
#include FAST_NODE_SOURCE
#include <algorithm>
#include <cstdint>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
namespace {
std::uint64_t checks = 0, graphs = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }
int by_visits(const std::vector<int>& next, int start, std::vector<int>& order) {
    std::vector<int> first(next.size(), -1);
    order.clear();
    while (start >= 0 && first[start] < 0) {
        first[start] = static_cast<int>(order.size()); order.push_back(start); start = next[start];
    }
    return start;
}
int by_floyd(const std::vector<int>& next, int start) {
    int a = start, b = start;
    do {
        if (b < 0 || next[b] < 0) return -1;
        a = next[a]; b = next[next[b]];
    } while (a != b);
    for (a = start; a != b; a = next[a], b = next[b]) {}
    return a;
}
void test_chain(std::size_t n) {
    Node* h = make_chain(n); Node* original = h;
    std::vector<Node*> nodes;
    for (Node* p = h; p; p = p->n) nodes.push_back(p);
    check(nodes.size() == n); check(chain_size(h) == n);
    check(chain_tail(h) == (n ? nodes.back() : nullptr)); check(!has_cycle(h)); check(cycle_entry(h) == nullptr); check(!break_cycle(h));
    if (n) {
        check(chain_nth(h, 0) == nodes[0]); check(chain_nth(h, n - 1) == nodes.back());
        for (std::size_t i = 1; i < n; ++i) check(nodes[i] == nodes[0] + i);
    }
    check(chain_nth(h, n) == nullptr); check(chain_nth(h, n + 1) == nullptr);
    Node* copy = clone_chain(h); check(chain_size(copy) == n);
    Node* p = copy;
    for (auto* expected : nodes) { check(p != expected); check(!std::binary_search(nodes.begin(), nodes.end(), p, std::less<Node*>{})); p = p->n; }
    check(p == nullptr);
    h = reverse_chain(h); check(h == (n ? nodes.back() : nullptr)); check(chain_size(h) == n);
    for (std::size_t i = 0; i < n; ++i) check(nodes[i]->n == (i ? nodes[i - 1] : nullptr));
    if (n) check(original->n == nullptr);
    h = reverse_chain(h); check(h == original);
    for (std::size_t i = 0; i < n; ++i) check(nodes[i]->n == (i + 1 < n ? nodes[i + 1] : nullptr));
    Node* alias = h; push_front_node(h); check(h && h->n == alias); Node* added = h;
    check(pop_front_node(h)); check(h == alias); check(added->n == alias);
    clear_chain(h); check(h == nullptr); check(chain_size(alias) == n); clear_chain(copy); check(copy == nullptr);
}
}
int main() {
    Node* empty = nullptr;
    check(make_chain(0) == nullptr); check(make_node()->n == nullptr); check(!pop_front_node(empty));
    Node* tail = make_chain(3); check(make_chain(0, tail) == tail);
    Node* prefix = make_chain(2, tail); check(prefix->n->n == tail); check(chain_size(prefix) == 5);
    for (std::size_t n : {0, 1, 2, 3, 7, 8, 31, 32, 33, 65, 1024, 100000, 1000000}) test_chain(n);
    std::mt19937_64 random(0xa66f3172);
    for (const int n : {0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 65, 129}) {
        Node* allocated = make_chain(n);
        std::vector<Node*> nodes;
        for (Node* p = allocated; p; p = p->n) nodes.push_back(p);
        std::vector<int> next(n), order;
        for (int rep = 0; rep < 1000; ++rep) {
            ++graphs;
            for (int i = 0; i < n; ++i) {
                next[i] = static_cast<int>(random() % (n + 1)) - 1;
                nodes[i]->n = next[i] < 0 ? nullptr : nodes[next[i]];
            }
            const int start = static_cast<int>(random() % (n + 1)) - 1;
            Node* head = start < 0 ? nullptr : nodes[start];
            const int entry = by_visits(next, start, order); check(by_floyd(next, start) == entry);
            check(cycle_entry(head) == (entry < 0 ? nullptr : nodes[entry])); check(has_cycle(head) == (entry >= 0));
            const auto index = static_cast<std::size_t>(random() % 1000);
            int expected = start;
            for (std::size_t i = 0; i < index && expected >= 0; ++i) expected = next[expected];
            check(chain_nth(head, index) == (expected < 0 ? nullptr : nodes[expected]));
            Node* alias = head; Node* old_next = head ? head->n : nullptr;
            check(pop_front_node(head) == (alias != nullptr)); check(head == old_next); if (alias) check(alias->n == old_next);
            head = alias; push_front_node(head); Node* front = head;
            check(head != alias && head->n == alias); check(pop_front_node(head)); check(head == alias); check(front->n == alias);
            const auto old_links = next;
            if (entry >= 0) next[order.back()] = -1;
            check(break_cycle(head) == (entry >= 0)); check(!has_cycle(head)); check(cycle_entry(head) == nullptr); check(!break_cycle(head));
            for (int i = 0; i < n; ++i) check(nodes[i]->n == (next[i] < 0 ? nullptr : nodes[next[i]]));
            check(chain_size(head) == order.size()); check(chain_tail(head) == (order.empty() ? nullptr : nodes[order.back()]));
            Node* clone = clone_chain(head); check(chain_size(clone) == order.size());
            Node* cursor = clone;
            for (auto i : order) { check(cursor && std::find(nodes.begin(), nodes.end(), cursor) == nodes.end()); cursor = cursor->n; }
            check(cursor == nullptr);
            Node* reversed = reverse_chain(head);
            for (std::size_t i = 0; i < order.size(); ++i) check(nodes[order[i]]->n == (i ? nodes[order[i - 1]] : nullptr));
            check(reversed == (order.empty() ? nullptr : nodes[order.back()]));
            head = reverse_chain(reversed); check(head == alias);
            clear_chain(head); check(head == nullptr);
            for (int i = 0; i < n; ++i) check(nodes[i]->n == (next[i] < 0 ? nullptr : nodes[next[i]]));
            clear_chain(clone); check(clone == nullptr);
            (void)old_links;
        }
    }
    std::printf("fast Node graphs=%ju checks=%ju\n", static_cast<std::uintmax_t>(graphs), static_cast<std::uintmax_t>(checks));
}
