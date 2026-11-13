// SPDX-License-Identifier: CC0-1.0
#include <cassert>
#include <iostream>

int main() {
    auto head = make_chain(7);
    assert(chain_size(head) == 7);
    auto middle = chain_nth(head, 3);
    auto tail = chain_tail(head);
    tail->n = middle;
    assert(has_cycle(head) && cycle_entry(head) == middle);
    assert(break_cycle(head) && !has_cycle(head));
    auto copy = clone_chain(head);
    assert(chain_size(copy) == 7 && copy != head);
    head = reverse_chain(head);
    assert(chain_nth(head, 6)->n == nullptr);
    push_front_node(head);
    assert(chain_size(head) == 8 && pop_front_node(head));
    clear_chain(head);
    assert(head == nullptr && chain_size(copy) == 7);
    std::cout << "OK\n";
}
