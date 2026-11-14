#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: ローカル独立検査

// Library: DataStructure/Node.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Node.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// [速度優先版 Node・単方向連結列]
// Node::nと各関数のノード引数/戻り値はNode*。自動所有・参照カウント・自動解放はない。
// make_node(next=nullptr): 共有bump poolから1ノード確保。償却O(1)、deleteしない。
// make_chain(count,tail=nullptr): 4096個以下は共有pool、大きい列はnew Node[count]で連続確保しtailへ接続。O(count+1)。
// 単一スレッド専用。共有poolは4096ノードずつ拡張し、既存アドレスを保つ。個別deleteは禁止、領域はプロセス終了まで保持。
// chain_size/chain_tail/clone_chain/reverse_chainは循環なしが前提。サイズはsize_t以内。
// chain_size(head): ノード数。chain_tail(head): 末尾（空ならnullptr）。ともにO(k)。
// chain_nth(head,index): index番目、末尾を超えればnullptr。O(min(index+1,k))、循環時はO(index+1)。
// push_front_node(head): make_nodeで先頭追加、O(1)。pop_front_node(head): 次へ進み、空ならfalse、O(1)。
// reverse_chain(head): リンクを逆転して新先頭を返す。O(k)、共有参照にも変更が見える。
// clone_chain(head): 独立した同長の連続領域を返す。2回走査でO(k+1)、追加確保は高々1回。
// cycle_entry(head)/has_cycle(head): 循環入口/有無、O(k)。break_cycle(head): 循環の末尾を切り、切ればtrue、O(k)。
// clear_chain(head): head=nullptrとするだけ、O(1)。全APIで例外後の回復・入力検査は省く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NODE_DEFINED
#define SIRKIT_NODE_DEFINED
struct Node {
    Node* n;
    explicit Node(Node* next = nullptr) noexcept : n(next) {}
};
#endif

#ifndef SIRKIT_FAST_NODE_POOL_DEFINED
#define SIRKIT_FAST_NODE_POOL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline Node* node_pool_next = nullptr;
inline Node* node_pool_end = nullptr;
inline Node* allocate_node_chunk(std::size_t count) {
    if (!node_pool_next || static_cast<std::size_t>(node_pool_end - node_pool_next) < count) {
        node_pool_next = new Node[4096];
        node_pool_end = node_pool_next + 4096;
    }
    Node* first = node_pool_next;
    node_pool_next += count;
    return first;
}
}
#endif

#ifndef SIRKIT_MAKE_NODE_DEFINED
#define SIRKIT_MAKE_NODE_DEFINED
inline Node* make_node(Node* next = nullptr) {
    Node* node = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::allocate_node_chunk(1);
    node->n = next;
    return node;
}
#endif

#ifndef SIRKIT_MAKE_CHAIN_DEFINED
#define SIRKIT_MAKE_CHAIN_DEFINED
inline Node* make_chain(std::size_t count, Node* tail = nullptr) {
    if (count == 0) return tail;
    Node* first = count <= 4096 ? /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::allocate_node_chunk(count) : new Node[count];
    for (std::size_t i = 1; i < count; ++i) first[i - 1].n = first + i;
    first[count - 1].n = tail;
    return first;
}
#endif

#ifndef SIRKIT_CHAIN_SIZE_DEFINED
#define SIRKIT_CHAIN_SIZE_DEFINED
inline std::size_t chain_size(const Node* head) noexcept {
    std::size_t count = 0;
    for (; head; head = head->n) ++count;
    return count;
}
#endif

#ifndef SIRKIT_CHAIN_TAIL_DEFINED
#define SIRKIT_CHAIN_TAIL_DEFINED
inline Node* chain_tail(Node* head) noexcept {
    if (head) while (head->n) head = head->n;
    return head;
}
#endif

#ifndef SIRKIT_CHAIN_NTH_DEFINED
#define SIRKIT_CHAIN_NTH_DEFINED
inline Node* chain_nth(Node* head, std::size_t index) noexcept {
    for (; head && index; --index) head = head->n;
    return head;
}
#endif

#ifndef SIRKIT_PUSH_FRONT_NODE_DEFINED
#define SIRKIT_PUSH_FRONT_NODE_DEFINED
inline void push_front_node(Node*& head) { head = make_node(head); }
#endif

#ifndef SIRKIT_POP_FRONT_NODE_DEFINED
#define SIRKIT_POP_FRONT_NODE_DEFINED
inline bool pop_front_node(Node*& head) noexcept {
    if (!head) return false;
    head = head->n;
    return true;
}
#endif

#ifndef SIRKIT_REVERSE_CHAIN_DEFINED
#define SIRKIT_REVERSE_CHAIN_DEFINED
inline Node* reverse_chain(Node* head) noexcept {
    Node* previous = nullptr;
    while (head) {
        Node* next = head->n;
        head->n = previous;
        previous = head;
        head = next;
    }
    return previous;
}
#endif

#ifndef SIRKIT_CLONE_CHAIN_DEFINED
#define SIRKIT_CLONE_CHAIN_DEFINED
inline Node* clone_chain(const Node* head) { return make_chain(chain_size(head)); }
#endif

#ifndef SIRKIT_CYCLE_ENTRY_DEFINED
#define SIRKIT_CYCLE_ENTRY_DEFINED
inline Node* cycle_entry(Node* head) noexcept {
    Node* slow = head;
    Node* fast = head;
    do {
        if (!fast || !fast->n) return nullptr;
        slow = slow->n;
        fast = fast->n->n;
    } while (slow != fast);
    for (slow = head; slow != fast; slow = slow->n, fast = fast->n) {}
    return slow;
}
#endif

#ifndef SIRKIT_HAS_CYCLE_DEFINED
#define SIRKIT_HAS_CYCLE_DEFINED
inline bool has_cycle(Node* head) noexcept {
    Node* slow = head;
    Node* fast = head;
    while (fast && fast->n) {
        slow = slow->n; fast = fast->n->n;
        if (slow == fast) return true;
    }
    return false;
}
#endif

#ifndef SIRKIT_BREAK_CYCLE_DEFINED
#define SIRKIT_BREAK_CYCLE_DEFINED
inline bool break_cycle(Node* head) noexcept {
    Node* entry = cycle_entry(head);
    if (!entry) return false;
    Node* last = entry;
    while (last->n != entry) last = last->n;
    last->n = nullptr;
    return true;
}
#endif

#ifndef SIRKIT_CLEAR_CHAIN_DEFINED
#define SIRKIT_CLEAR_CHAIN_DEFINED
inline void clear_chain(Node*& head) noexcept { head = nullptr; }
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

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
