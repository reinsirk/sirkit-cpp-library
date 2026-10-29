// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/implicit_treap.md
//
// API一覧
// [lazy_implicit_treap]
// 可変長列。通常操作は期待O(log N)、挿入は償却期待O(log N)、eraseは期待O(削除数 + log N)。
// 領域O(最大要素数)。ノード配列の再確保は保持スロット数に比例する。
// op は結合的、e は単位元。mapping は集約に分配し、composition(f,g) は g の後に f。
// op・mapping等の計算時間をO(1)として記す。
// 非可換 op では順方向・逆方向を保持する。添字と区間は 0 始まり・半開区間。
// 第8引数 Commutative=true は op の可換性を指定する。既知の既定 op は自動認識する。
// lazy_implicit_treap() は空列、(vector<S>) は O(N) で列を構築する。
// コピーは O(スロット数) で独立した所有。コピー代入失敗時は元の列を保持する。
// ムーブ元は空列、swap は O(1)。
// size() / empty() は要素数・空判定を O(1) で返す。
// clear() は O(スロット数) で要素を破棄し、確保済み容量を保持する。
// reserve(n) は容量を確保する。int の範囲を超える要求は例外。
// get(pos) / set(pos,value) は要素の取得・置換。0 <= pos < size()。
// front() / back() は先頭・末尾の値を返す。空でないこと。期待O(log N)。
// insert_at(pos,value) は位置 pos の直前へ挿入。0 <= pos <= size()。
// push_back(value) / push_front(value) は末尾・先頭へ追加する。
// erase(l,r) は区間を削除。不正な区間は検査有効時に何もしない。
// erase_prod(l,r) は区間の集約を返して削除する。空区間はe()。期待O(削除数 + log N)。
// erase_at(pos) は1要素を削除。0 <= pos < size()。
// pop_back() / pop_front() は末尾・先頭を削除。空列なら何もしない。
// apply(l,r,f) は区間に作用 f を適用する。空区間は何もしない。
// prod(l,r) は順序を保って集約し、空区間は e()。all_prod() は O(1)。
// reverse(l,r) は区間を反転。rotate(l,m,r) は [l,m),[m,r) を交換。l <= m <= r。
// move_range(l,r,pos): [l,r)を移動前の隙間posの直前へ移す。0<=pos<=size()。期待O(log N)。
// l<=pos<=r または空区間なら不変。例: [a,b,c,d,e]でmove_range(1,3,5)は[a,d,e,b,c]。
// to_vector() は現在の列を O(N) で返す。
// 以下の比較 API は列全体が operator< による昇順であることが前提。
// lower_bound(value) / upper_bound(value) は値以上・値より大きい最初の添字、なければ size()。
// get_rank(value) は厳密に小さい要素数、count(value) は < による等価要素数。
// contains(value) は lower_bound 位置を == で判定。== がない型は < による等価判定。
// insert_sorted(value) は昇順を維持して追加。erase_val(value) は contains と同じ一致判定で1個削除。
// erase_max() は最大要素を1個削除。空列なら何もしない。
// max_right(l,g) / max_right<g>(l) は g(prod(l,r)) が真となる最大 r。
// min_left(r,g) / min_left<g>(r) は g(prod(l,r)) が真となる最小 l。
// 境界探索は g(e()) が真で区間伸長に単調であること。検査省略時の不正入力は未定義。
//
// [indexed_implicit_treap / 第9引数 TrackIds=true]
// 同じ先頭8テンプレート引数でID追跡を有効にする別名。既定falseではID状態・親の更新を持たない。
// IDは値と独立した非負int。生存要素間で一意。既存の挿入・vector構築はIDなし(-1)。
// insert_at_with_id(pos,id,value): posの直前へID付きで挿入。位置の範囲はinsert_atと同じ。
// insert_after(id,new_id,value): 存在するidの直後へnew_id付きで挿入。
// insert_before(id,new_id,value): 存在するidの直前へnew_id付きで挿入。
// position_of(id): 現在の0始まり位置。IDがなければ-1。contains_id(id): 存在判定O(1)。
// get_id(pos): 位置posのID、匿名要素は-1。0<=pos<size()。
// get_by_id(id) / set_by_id(id,value): 存在するIDの値を取得・置換。期待O(log N)。IDは保持する。
// erase_id(id): IDの要素を削除してtrue。不在はfalse。期待O(log N)。
// prod_between(a,b) / erase_between(a,b): 2つのIDの間を集約／集約して削除。端点は含めない。
// IDの前後は任意。同じID・隣接要素ならe()。双方のIDが存在すること。
// prod_betweenは期待O(log N)、erase_betweenは期待O(削除数 + log N)。
// reserve_ids(n): ID 0..n-1 の索引を準備する。O(追加枠数)、再確保時O(現在のID索引枠数)。
// ID取得・位置特定は期待O(log N)、挿入は償却期待O(log N)。索引拡張・再確保の費用は別途かかる。
// 領域はノード数＋最大IDに比例。set/applyはIDを変えず、reverse/rotateでも位置を追跡する。
// 削除されたIDは再利用可。clearはIDも消す。コピーはID索引も独立、move/swapはIDを列と共に移す。
// 検査時は負の挿入ID・重複IDと、存在が前提の操作への不存在IDを拒否。コピーはID索引枠数ぶんもかかる。
//
// [既定の作用: RangeAddMin]
// 区間加算・最小値。値と加算後の値は INF 以下、各演算は int64_t に収まること。
// op(a,b)/e(): 最小値/単位元。mapping(f,x): 加算。composition(f,g)/id(): 加算の合成/0。
//
// [既定の作用: RangeAddSum]
// 区間加算・和。各要素は S{値, 1} とし、演算結果は int64_t に収まること。
// op(a,b)/e(): 和と個数の合計/空。mapping(f,x): 各要素へ加算。composition(f,g)/id(): 合成/0。

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_RANGE_ADD_MIN_DEFINED
#define SIRKIT_RANGE_ADD_MIN_DEFINED
struct RangeAddMin {
    using S = std::int64_t;
    using F = std::int64_t;
    static constexpr S INF = INT64_C(8000000000000000000);
    static S op(S a, S b) { return std::min(a, b); }
    static S e() { return INF; }
    static S mapping(F f, S x) { return x + f; }
    static F composition(F f, F g) { return f + g; }
    static F id() { return 0; }
};
#endif

#ifndef SIRKIT_RANGE_ADD_SUM_DEFINED
#define SIRKIT_RANGE_ADD_SUM_DEFINED
struct RangeAddSum {
    struct S {
        std::int64_t sum, size;
        friend bool operator<(S a, S b) { return a.sum < b.sum; }
        friend bool operator==(S a, S b) { return a.sum == b.sum; }
    };
    using F = std::int64_t;
    static S op(S a, S b) { return {a.sum + b.sum, a.size + b.size}; }
    static S e() { return {0, 0}; }
    static S mapping(F f, S x) { return {x.sum + f * x.size, x.size}; }
    static F composition(F f, F g) { return f + g; }
    static F id() { return 0; }
};
#endif

#ifndef SIRKIT_IMPLICIT_TREAP_DEFINED
#define SIRKIT_IMPLICIT_TREAP_DEFINED
template<class S = RangeAddMin::S,
         S (*op)(S, S) = RangeAddMin::op,
         S (*e)() = RangeAddMin::e,
         class F = RangeAddMin::F,
         S (*mapping)(F, S) = RangeAddMin::mapping,
         F (*composition)(F, F) = RangeAddMin::composition,
         F (*id)() = RangeAddMin::id,
         bool Commutative = false, bool TrackIds = false>
class lazy_implicit_treap {
    static constexpr bool commutative_ = Commutative || [] {
        if constexpr (std::is_same_v<S, RangeAddMin::S>) return op == RangeAddMin::op;
        else if constexpr (std::is_same_v<S, RangeAddSum::S>) return op == RangeAddSum::op;
        else return false;
    }();
    struct ReverseAggregate {
        S value;
        explicit ReverseAggregate(S&& x) : value(std::move(x)) {}
    };
    struct EmptyAggregate { explicit EmptyAggregate(S&&) {} };
    struct IdLink { int parent = -1, identifier = -1; };
    struct NoIdLink {};
    struct IdIndex { std::vector<int> slots; };
    struct NoIdIndex {};
    struct Node {
        S value, forward;
        [[no_unique_address]] std::conditional_t<commutative_, EmptyAggregate, ReverseAggregate> backward;
        F action;
        std::uint64_t priority;
        int left = -1, right = -1, length = 1;
        bool pending = false, reversed = false;
        [[no_unique_address]] std::conditional_t<TrackIds, IdLink, NoIdLink> link;
        Node(S x, std::uint64_t p)
            : value(x), forward(x), backward(std::move(x)), action(id()), priority(p) {}
    };
    struct CompactNodeLayout {
        S value, forward;
        [[no_unique_address]] std::conditional_t<commutative_, EmptyAggregate, ReverseAggregate> backward;
        F action;
        int left = -1, right = -1, length = 1;
        std::uint32_t priority_flags;
        [[no_unique_address]] std::conditional_t<TrackIds, IdLink, NoIdLink> link;
        CompactNodeLayout(S x, std::uint64_t p)
            : value(x), forward(x), backward(std::move(x)), action(id()),
              priority_flags(static_cast<std::uint32_t>(p >> 32) & ~std::uint32_t{3}) {}
    };
    // 64バイトに収まる単純な型だけを整列し、完全な優先度は補助配列に保持する。
    static constexpr bool compact_nodes_ = sizeof(CompactNodeLayout) == 64 &&
        std::is_trivially_copyable_v<S> && std::is_trivially_copyable_v<F> && !TrackIds;
    struct alignas(sizeof(CompactNodeLayout) == 64 ? 64 : alignof(CompactNodeLayout)) CompactNode : CompactNodeLayout {
        using CompactNodeLayout::CompactNodeLayout;
    };
    using StoredNode = std::conditional_t<compact_nodes_, CompactNode, Node>;
    struct PriorityIndex { std::vector<std::uint64_t> slots; };
    struct NoPriorityIndex {};
    std::vector<StoredNode> nodes_;
    [[no_unique_address]] std::conditional_t<compact_nodes_, PriorityIndex, NoPriorityIndex> priorities_;
    int root_ = -1, free_ = -1;
    std::uint64_t random_ = seed();
    [[no_unique_address]] std::conditional_t<TrackIds, IdIndex, NoIdIndex> ids_;

    static std::uint64_t seed() {
        return static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
    }
    std::uint64_t priority() {
        // 平方根に由来する定数を選んだ局所的な線形合同法。下位ビットを添字に使わない。
        random_ = random_ * UINT64_C(0x6a09e667f3bcc909) + UINT64_C(0xbb67ae8584caa73b);
        return random_;
    }
    // 上位ビットが一致する場合も64ビット値で順序を確定する。
    bool higher_priority(int a, int b) const {
        if constexpr (compact_nodes_) {
            const auto x = nodes_[a].priority_flags, y = nodes_[b].priority_flags;
            // 異なる上位キーの差は4以上で、下位2ビットのフラグに左右されない。
            if ((x ^ y) >= 4) return x > y;
            return priorities_.slots[a] > priorities_.slots[b];
        } else return nodes_[a].priority > nodes_[b].priority;
    }
    bool first_has_priority(int a, int b) const {
        if constexpr (compact_nodes_) return !higher_priority(b, a);
        else return nodes_[a].priority >= nodes_[b].priority;
    }
    bool build_priority_less(int a, int b) const {
        if constexpr (compact_nodes_) return priorities_.slots[a] < priorities_.slots[b];
        else return nodes_[a].priority < nodes_[b].priority;
    }
    bool has_action(int p) const {
        if constexpr (compact_nodes_) return (nodes_[p].priority_flags & 1) != 0;
        else return nodes_[p].pending;
    }
    void set_action_pending(int p, bool value) {
        if constexpr (compact_nodes_)
            nodes_[p].priority_flags = (nodes_[p].priority_flags & ~std::uint32_t{1}) | std::uint32_t(value);
        else nodes_[p].pending = value;
    }
    bool has_reversal(int p) const {
        if constexpr (compact_nodes_) return (nodes_[p].priority_flags & 2) != 0;
        else return nodes_[p].reversed;
    }
    void toggle_reversal(int p) {
        if constexpr (compact_nodes_) nodes_[p].priority_flags ^= 2;
        else nodes_[p].reversed = !nodes_[p].reversed;
    }
    void clear_reversal(int p) {
        if constexpr (compact_nodes_) nodes_[p].priority_flags &= ~std::uint32_t{2};
        else nodes_[p].reversed = false;
    }
    static constexpr bool deferred_actions_ = compact_nodes_ && sizeof(S) >= 16 && sizeof(F) >= 16;
    int length(int p) const { return p < 0 ? 0 : nodes_[p].length; }
    S forward(int p) const { return p < 0 ? e() : nodes_[p].forward; }
    S backward(int p) const {
        if constexpr (commutative_) return forward(p);
        else return p < 0 ? e() : nodes_[p].backward.value;
    }
    void pull(int p) {
        StoredNode& n = nodes_[p];
        if constexpr (TrackIds) {
            if (n.left >= 0) nodes_[n.left].link.parent = p;
            if (n.right >= 0) nodes_[n.right].link.parent = p;
        }
        n.length = 1 + length(n.left) + length(n.right);
        n.forward = op(op(forward(n.left), n.value), forward(n.right));
        if constexpr (!commutative_)
            n.backward.value = op(op(backward(n.right), n.value), backward(n.left));
    }
    void act(int p, F f) {
        if (p < 0) return;
        StoredNode& n = nodes_[p];
        // 対象型では要素自身への作用をpushまでまとめ、集約には直ちに反映する。
        if constexpr (!deferred_actions_) n.value = mapping(f, n.value);
        n.forward = mapping(f, n.forward);
        if constexpr (!commutative_) n.backward.value = mapping(f, n.backward.value);
        if constexpr (compact_nodes_) {
            n.action = has_action(p) ? composition(f, n.action) : f;
            set_action_pending(p, true);
        } else {
            n.action = n.pending ? composition(f, n.action) : f;
            n.pending = true;
        }
    }
    void flip(int p) {
        if (p < 0) return;
        StoredNode& n = nodes_[p];
        if constexpr (!compact_nodes_) std::swap(n.left, n.right);
        if constexpr (!commutative_) std::swap(n.forward, n.backward.value);
        if constexpr (compact_nodes_) toggle_reversal(p);
        else n.reversed = !n.reversed;
    }
    void push(int p) {
        StoredNode& n = nodes_[p];
        if constexpr (compact_nodes_) {
            if (has_action(p)) {
                if constexpr (deferred_actions_) n.value = mapping(n.action, n.value);
                act(n.left, n.action);
                act(n.right, n.action);
                set_action_pending(p, false);
            }
            if (has_reversal(p)) {
                std::swap(n.left, n.right);
                flip(n.left);
                flip(n.right);
                clear_reversal(p);
            }
        } else {
            if (n.pending) {
                act(n.left, n.action);
                act(n.right, n.action);
                n.pending = false;
            }
            if (n.reversed) {
                flip(n.left);
                flip(n.right);
                n.reversed = false;
            }
        }
    }
    std::pair<int, int> split(int p, int k) {
        if (p < 0) return {-1, -1};
        if constexpr (compact_nodes_) {
            if (k == 0) return {-1, p};
            if (k == length(p)) return {p, -1};
        }
        push(p);
        const int left_size = length(nodes_[p].left);
        if (k <= left_size) {
            auto [a, b] = split(nodes_[p].left, k);
            nodes_[p].left = b;
            pull(p);
            return {a, p};
        }
        auto [a, b] = split(nodes_[p].right, k - left_size - 1);
        nodes_[p].right = a;
        pull(p);
        return {p, b};
    }
    int merge(int a, int b) {
        if (a < 0) return b;
        if (b < 0) return a;
        if (first_has_priority(a, b)) {
            push(a);
            nodes_[a].right = merge(nodes_[a].right, b);
            pull(a);
            return a;
        }
        push(b);
        nodes_[b].left = merge(a, nodes_[b].left);
        pull(b);
        return b;
    }
    std::pair<int, int> split_sorted(int p, const S& value) {
        if (p < 0) return {-1, -1};
        push(p);
        if (nodes_[p].value < value) {
            auto [a, b] = split_sorted(nodes_[p].right, value);
            nodes_[p].right = a;
            pull(p);
            return {p, b};
        }
        auto [a, b] = split_sorted(nodes_[p].left, value);
        nodes_[p].left = b;
        pull(p);
        return {a, p};
    }
    int insert_sorted_node(int p, int fresh) {
        if (p < 0) return fresh;
        if (higher_priority(fresh, p)) {
            auto [a, b] = split_sorted(p, nodes_[fresh].value);
            nodes_[fresh].left = a;
            nodes_[fresh].right = b;
            pull(fresh);
            return fresh;
        }
        push(p);
        if (nodes_[p].value < nodes_[fresh].value)
            nodes_[p].right = insert_sorted_node(nodes_[p].right, fresh);
        else nodes_[p].left = insert_sorted_node(nodes_[p].left, fresh);
        pull(p);
        return p;
    }
    int insert_node(int p, int fresh, int position) {
        if (p < 0) return fresh;
        if (higher_priority(fresh, p)) {
            auto [left, right] = split(p, position);
            nodes_[fresh].left = left;
            nodes_[fresh].right = right;
            pull(fresh);
            return fresh;
        }
        push(p);
        const int left_size = length(nodes_[p].left);
        if (position <= left_size) nodes_[p].left = insert_node(nodes_[p].left, fresh, position);
        else nodes_[p].right = insert_node(nodes_[p].right, fresh, position - left_size - 1);
        pull(p);
        return p;
    }
    int allocate(S value) {
        if (free_ >= 0) {
            int p = free_;
            free_ = nodes_[p].left;
            if constexpr (compact_nodes_) {
                const auto fresh_priority = priority();
                priorities_.slots[p] = fresh_priority;
                nodes_[p] = StoredNode(std::move(value), fresh_priority);
            } else nodes_[p] = Node(std::move(value), priority());
            return p;
        }
        if (nodes_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("lazy_implicit_treap capacity");
        if constexpr (compact_nodes_) {
            priorities_.slots.push_back(priority());
            try { nodes_.emplace_back(std::move(value), priorities_.slots.back()); }
            catch (...) { priorities_.slots.pop_back(); throw; }
        } else nodes_.emplace_back(std::move(value), priority());
        return static_cast<int>(nodes_.size()) - 1;
    }
    void recycle(int p) {
        if constexpr (TrackIds) {
            if (nodes_[p].link.identifier >= 0) ids_.slots[nodes_[p].link.identifier] = -1;
        }
        if constexpr (!compact_nodes_) nodes_[p] = StoredNode(e(), 0);
        nodes_[p].left = free_;
        free_ = p;
    }
    S erase_value_node(int& p, int position, int& removed) {
        push(p);
        const int k = length(nodes_[p].left);
        if (position == k) {
            S result = nodes_[p].value;
            removed = p;
            p = merge(nodes_[p].left, nodes_[p].right);
            return result;
        }
        S result = position < k ? erase_value_node(nodes_[p].left, position, removed)
                                : erase_value_node(nodes_[p].right, position - k - 1, removed);
        pull(p);
        return result;
    }
    int erase_node(int p, int position, int& removed) {
        push(p);
        const int left_size = length(nodes_[p].left);
        if (position < left_size) nodes_[p].left = erase_node(nodes_[p].left, position, removed);
        else if (position > left_size) nodes_[p].right = erase_node(nodes_[p].right, position - left_size - 1, removed);
        else {
            const int result = merge(nodes_[p].left, nodes_[p].right);
            removed = p;
            return result;
        }
        pull(p);
        return p;
    }
    void release(int p) {
        if (p < 0) return;
        release(nodes_[p].left);
        release(nodes_[p].right);
        // 所有する値を解放し、スロットを再利用する。
        recycle(p);
    }
    void check_position(int p, bool end_allowed = false) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (p < 0 || p > size() || (!end_allowed && p == size()))
            throw std::out_of_range("lazy_implicit_treap position");
#else
        (void)p;
        (void)end_allowed;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size())
            throw std::out_of_range("lazy_implicit_treap range");
#else
        (void)l;
        (void)r;
#endif
    }
    void apply_range(int p, int l, int r, const F& f) {
        if (l == 0 && r == length(p)) { act(p, f); return; }
        push(p);
        const int k = length(nodes_[p].left);
        if (l < k) apply_range(nodes_[p].left, l, std::min(r, k), f);
        if (l <= k && k < r) nodes_[p].value = mapping(f, nodes_[p].value);
        if (r > k + 1) apply_range(nodes_[p].right, std::max(0, l - k - 1), r - k - 1, f);
        if constexpr (compact_nodes_) {
            StoredNode& n = nodes_[p];
            n.forward = op(op(forward(n.left), n.value), forward(n.right));
            if constexpr (!commutative_)
                n.backward.value = op(op(backward(n.right), n.value), backward(n.left));
        } else pull(p);
    }
    // 対象の可換集約は、並べ替えても部分木全体の値と要素数が変わらない。
    void reverse_range(int p, int l, int r) {
        if (l == 0 && r == length(p)) { flip(p); return; }
        push(p);
        const int k = length(nodes_[p].left);
        if (r <= k) reverse_range(nodes_[p].left, l, r);
        else if (l > k) reverse_range(nodes_[p].right, l - k - 1, r - k - 1);
        else {
            auto [a, b] = split(nodes_[p].left, l);
            auto [c, d] = split(nodes_[p].right, r - k - 1);
            flip(b);
            flip(c);
            nodes_[p].left = merge(a, c);
            nodes_[p].right = merge(b, d);
        }
        if constexpr (!compact_nodes_ || !commutative_) pull(p);
    }
    void rotate_range(int p, int l, int m, int r) {
        push(p);
        const int k = length(nodes_[p].left);
        if (r <= k) rotate_range(nodes_[p].left, l, m, r);
        else if (l > k) rotate_range(nodes_[p].right, l - k - 1, m - k - 1, r - k - 1);
        else if (m <= k) {
            auto [a, bc] = split(nodes_[p].left, l);
            auto [b, c] = split(bc, m - l);
            auto [d, tail] = split(nodes_[p].right, r - k - 1);
            nodes_[p].left = merge(a, c);
            nodes_[p].right = merge(d, merge(b, tail));
        } else {
            auto [a, b] = split(nodes_[p].left, l);
            auto [c, de] = split(nodes_[p].right, m - k - 1);
            auto [d, tail] = split(de, r - m);
            nodes_[p].left = merge(merge(a, d), b);
            nodes_[p].right = merge(c, tail);
        }
        if constexpr (!compact_nodes_ || !commutative_) pull(p);
    }
    void push_query(int p) {
        if constexpr (compact_nodes_) {
            if ((nodes_[p].priority_flags & 3) != 0) push(p);
        } else push(p);
    }
    S query_prefix(int p, int r) {
        if (r == length(p)) return nodes_[p].forward;
        if constexpr (compact_nodes_) push_query(p);
        else push(p);
        const int k = length(nodes_[p].left);
        if (r <= k) return query_prefix(nodes_[p].left, r);
        S result = k ? op(nodes_[nodes_[p].left].forward, nodes_[p].value) : nodes_[p].value;
        if (r > k + 1) result = op(std::move(result), query_prefix(nodes_[p].right, r - k - 1));
        return result;
    }
    S query_suffix(int p, int l) {
        if (l == 0) return nodes_[p].forward;
        if constexpr (compact_nodes_) push_query(p);
        else push(p);
        const int k = length(nodes_[p].left);
        if (l > k) return query_suffix(nodes_[p].right, l - k - 1);
        S result = l < k ? op(query_suffix(nodes_[p].left, l), nodes_[p].value) : nodes_[p].value;
        if (nodes_[p].right >= 0) result = op(std::move(result), nodes_[nodes_[p].right].forward);
        return result;
    }
    S query(int p, int l, int r) {
        if (l == 0) return query_prefix(p, r);
        if (r == length(p)) return query_suffix(p, l);
        if constexpr (compact_nodes_) push_query(p);
        else push(p);
        const int k = length(nodes_[p].left);
        if (r <= k) return query(nodes_[p].left, l, r);
        if (l > k) return query(nodes_[p].right, l - k - 1, r - k - 1);
        S result = l < k ? op(query_suffix(nodes_[p].left, l), nodes_[p].value) : nodes_[p].value;
        if (r > k + 1) result = op(std::move(result), query_prefix(nodes_[p].right, r - k - 1));
        return result;
    }
    template<class G>
    bool right_walk(int p, int start, int offset, S& acc, G& g, int& result) {
        if (p < 0 || offset + length(p) <= start) return true;
        if (start <= offset) {
            S next = op(acc, nodes_[p].forward);
            if (g(next)) { acc = std::move(next); return true; }
        }
        push(p);
        const int middle = offset + length(nodes_[p].left);
        if (!right_walk(nodes_[p].left, start, offset, acc, g, result)) return false;
        if (start <= middle) {
            S next = op(acc, nodes_[p].value);
            if (!g(next)) { result = middle; return false; }
            acc = std::move(next);
        }
        return right_walk(nodes_[p].right, start, middle + 1, acc, g, result);
    }
    template<class G>
    bool left_walk(int p, int finish, int offset, S& acc, G& g, int& result) {
        if (p < 0 || finish <= offset) return true;
        if (offset + length(p) <= finish) {
            S next = op(nodes_[p].forward, acc);
            if (g(next)) { acc = std::move(next); return true; }
        }
        push(p);
        const int middle = offset + length(nodes_[p].left);
        if (!left_walk(nodes_[p].right, finish, middle + 1, acc, g, result)) return false;
        if (middle < finish) {
            S next = op(nodes_[p].value, acc);
            if (!g(next)) { result = middle + 1; return false; }
            acc = std::move(next);
        }
        return left_walk(nodes_[p].left, finish, offset, acc, g, result);
    }
    void flatten(int p, std::vector<S>& out) {
        if (p < 0) return;
        push(p);
        flatten(nodes_[p].left, out);
        out.push_back(nodes_[p].value);
        flatten(nodes_[p].right, out);
    }
    static bool equal(const S& a, const S& b) {
        if constexpr (requires { static_cast<bool>(a == b); }) return static_cast<bool>(a == b);
        else return !(a < b) && !(b < a);
    }
    int bound(S value, bool upper) {
        int p = root_, offset = 0, result = size();
        while (p >= 0) {
            push(p);
            const int k = offset + length(nodes_[p].left);
            const bool before = upper ? !(value < nodes_[p].value) : nodes_[p].value < value;
            if (before) { offset = k + 1; p = nodes_[p].right; }
            else { result = k; p = nodes_[p].left; }
        }
        return result;
    }
    void reset_root_parent() {
        if constexpr (TrackIds) if (root_ >= 0) nodes_[root_].link.parent = -1;
    }
    int id_node(int element_id) const requires (TrackIds) {
        return element_id >= 0 && static_cast<std::size_t>(element_id) < ids_.slots.size() ? ids_.slots[element_id] : -1;
    }
    int subtree_offset(int p) requires (TrackIds) {
        const int parent = nodes_[p].link.parent;
        int offset = parent < 0 ? 0 : subtree_offset(parent);
        // 祖先の反転を先に伝えてから、現在の左右関係を読む。
        if (parent >= 0 && nodes_[parent].right == p)
            offset += length(nodes_[parent].left) + 1;
        push(p);
        return offset;
    }
    int require_id_position(int element_id) requires (TrackIds) {
        const int pos = position_of(element_id);
#ifndef SKIP_RUNTIME_CHECKS
        if (pos < 0) throw std::out_of_range("lazy_implicit_treap missing ID");
#endif
        return pos;
    }
    std::pair<int, int> between_range(int first_id, int second_id) requires (TrackIds) {
        int first = require_id_position(first_id), second = require_id_position(second_id);
        if (first > second) std::swap(first, second);
        return {first == second ? first : first + 1, second};
    }
public:
    lazy_implicit_treap() = default;
    explicit lazy_implicit_treap(const std::vector<S>& values) {
        reserve(values.size());
        std::vector<int> stack;
        stack.reserve(values.size());
        for (const S& value : values) {
            int p = allocate(value), left = -1;
            while (!stack.empty() && build_priority_less(stack.back(), p)) {
                left = stack.back();
                stack.pop_back();
                pull(left);
            }
            nodes_[p].left = left;
            if (!stack.empty()) nodes_[stack.back()].right = p;
            stack.push_back(p);
        }
        if (!stack.empty()) root_ = stack.front();
        while (!stack.empty()) { pull(stack.back()); stack.pop_back(); }
    }
    lazy_implicit_treap(const lazy_implicit_treap&) = default;
    lazy_implicit_treap& operator=(const lazy_implicit_treap& other) {
        if (this == &other) return *this;
        if constexpr (std::is_nothrow_copy_constructible_v<StoredNode> && std::is_nothrow_copy_assignable_v<StoredNode>) {
            if (other.nodes_.size() <= nodes_.capacity()) {
                if constexpr (TrackIds) {
                    auto next_ids = other.ids_;
                    if constexpr (compact_nodes_) priorities_.slots = other.priorities_.slots;
                    nodes_ = other.nodes_;
                    ids_ = std::move(next_ids);
                } else {
                    if constexpr (compact_nodes_) priorities_.slots = other.priorities_.slots;
                    nodes_ = other.nodes_;
                }
                root_ = other.root_;
                free_ = other.free_;
                random_ = other.random_;
                return *this;
            }
        }
        lazy_implicit_treap temporary(other);
        swap(temporary);
        return *this;
    }
    lazy_implicit_treap(lazy_implicit_treap&& other) noexcept
        : nodes_(std::move(other.nodes_)), root_(std::exchange(other.root_, -1)),
          free_(std::exchange(other.free_, -1)), random_(other.random_), ids_(std::move(other.ids_)) {
        if constexpr (compact_nodes_) priorities_.slots = std::move(other.priorities_.slots);
        other.nodes_.clear();
        if constexpr (compact_nodes_) other.priorities_.slots.clear();
        if constexpr (TrackIds) other.ids_.slots.clear();
    }
    lazy_implicit_treap& operator=(lazy_implicit_treap&& other) noexcept {
        if (this != &other) { lazy_implicit_treap temporary(std::move(other)); swap(temporary); }
        return *this;
    }
    void swap(lazy_implicit_treap& other) noexcept {
        nodes_.swap(other.nodes_);
        if constexpr (compact_nodes_) priorities_.slots.swap(other.priorities_.slots);
        std::swap(root_, other.root_);
        std::swap(free_, other.free_);
        std::swap(random_, other.random_);
        if constexpr (TrackIds) ids_.slots.swap(other.ids_.slots);
    }
    friend void swap(lazy_implicit_treap& a, lazy_implicit_treap& b) noexcept { a.swap(b); }
    int size() const { return length(root_); }
    bool empty() const { return root_ < 0; }
    void clear() {
        nodes_.clear(); root_ = free_ = -1;
        if constexpr (compact_nodes_) priorities_.slots.clear();
        if constexpr (TrackIds) ids_.slots.clear();
    }
    void reserve(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("lazy_implicit_treap capacity");
        nodes_.reserve(n);
        if constexpr (compact_nodes_) priorities_.slots.reserve(n);
    }
    S get(int pos) {
        check_position(pos);
        int p = root_;
        for (;;) {
            push(p);
            int k = length(nodes_[p].left);
            if (pos == k) return nodes_[p].value;
            if (pos < k) p = nodes_[p].left;
            else { pos -= k + 1; p = nodes_[p].right; }
        }
    }
    S front() { return get(0); }
    S back() { return get(size() - 1); }
    void set(int pos, S value) {
        check_position(pos);
        set_value(root_, pos, std::move(value));
    }
private:
    void set_value(int p, int pos, S value) {
        push(p);
        int k = length(nodes_[p].left);
        if (pos == k) nodes_[p].value = std::move(value);
        else if (pos < k) set_value(nodes_[p].left, pos, std::move(value));
        else set_value(nodes_[p].right, pos - k - 1, std::move(value));
        pull(p);
    }
public:
    void insert_at(int pos, S value) {
        check_position(pos, true);
        int p = allocate(std::move(value));
        if (pos == size()) root_ = merge(root_, p);
        else if (pos == 0) root_ = merge(p, root_);
        else root_ = insert_node(root_, p, pos);
        reset_root_parent();
    }
    void push_back(S value) { insert_at(size(), std::move(value)); }
    void push_front(S value) { insert_at(0, std::move(value)); }
    void erase(int l, int r) {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) return;
#endif
        if (l == r) return;
        auto [a, bc] = split(root_, l);
        auto [b, c] = split(bc, r - l);
        root_ = merge(a, c);
        release(b);
        reset_root_parent();
    }
    S erase_prod(int l, int r) {
        check_range(l, r);
        if (l == r) return e();
        if constexpr (!(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S>)) {
            S result = query(root_, l, r);
            erase(l, r);
            return result;
        } else {
            if (r - l == 1) {
                int removed = -1;
                S result = erase_value_node(root_, l, removed);
                recycle(removed);
                reset_root_parent();
                return result;
            }
            auto [a, bc] = split(root_, l);
            auto [b, c] = split(bc, r - l);
            S result = nodes_[b].forward;
            root_ = merge(a, c);
            release(b);
            reset_root_parent();
            return result;
        }
    }
    void erase_at(int pos) {
        check_position(pos);
        int removed = -1;
        root_ = erase_node(root_, pos, removed);
        recycle(removed);
        reset_root_parent();
    }
    void pop_back() { if (!empty()) erase_at(size() - 1); }
    void pop_front() { if (!empty()) erase_at(0); }
    void apply(int l, int r, F f) {
        check_range(l, r);
        if (l != r) apply_range(root_, l, r, f);
    }
    S prod(int l, int r) { check_range(l, r); return l == r ? e() : query(root_, l, r); }
    S all_prod() const { return forward(root_); }
    void reverse(int l, int r) {
        check_range(l, r);
        if (r - l <= 1) return;
        reverse_range(root_, l, r);
        reset_root_parent();
    }
    void rotate(int l, int m, int r) {
        check_range(l, r);
#ifndef SKIP_RUNTIME_CHECKS
        if (m < l || m > r) throw std::out_of_range("lazy_implicit_treap middle");
#endif
        if (l == m || m == r) return;
        rotate_range(root_, l, m, r);
        reset_root_parent();
    }
    void move_range(int l, int r, int pos) {
        check_range(l, r);
        check_position(pos, true);
        if (l == r || (l <= pos && pos <= r)) return;
        if (pos < l) rotate(pos, l, r);
        else rotate(l, r, pos);
    }
    std::vector<S> to_vector() { std::vector<S> out; out.reserve(size()); flatten(root_, out); return out; }
    int lower_bound(S value) { return bound(std::move(value), false); }
    int upper_bound(S value) { return bound(std::move(value), true); }
    int get_rank(S value) { return lower_bound(std::move(value)); }
    int count(S value) { return upper_bound(value) - lower_bound(value); }
    bool contains(S value) {
        int p = lower_bound(value);
        return p < size() && equal(get(p), value);
    }
    void insert_sorted(S value) {
        int p = allocate(std::move(value));
        root_ = insert_sorted_node(root_, p);
        reset_root_parent();
    }
    void reserve_ids(std::size_t n) requires (TrackIds) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1)
            throw std::length_error("lazy_implicit_treap ID capacity");
        if (n > ids_.slots.size()) ids_.slots.resize(n, -1);
    }
    bool contains_id(int element_id) const requires (TrackIds) { return id_node(element_id) >= 0; }
    int position_of(int element_id) requires (TrackIds) {
        const int p = id_node(element_id);
        if (p < 0) return -1;
        const int offset = subtree_offset(p);
        return offset + length(nodes_[p].left);
    }
    int get_id(int pos) requires (TrackIds) {
        check_position(pos);
        int p = root_;
        for (;;) {
            push(p);
            const int k = length(nodes_[p].left);
            if (pos == k) return nodes_[p].link.identifier;
            if (pos < k) p = nodes_[p].left;
            else { pos -= k + 1; p = nodes_[p].right; }
        }
    }
    S get_by_id(int element_id) requires (TrackIds) {
        return get(require_id_position(element_id));
    }
    void set_by_id(int element_id, S value) requires (TrackIds) {
        set(require_id_position(element_id), std::move(value));
    }
    bool erase_id(int element_id) requires (TrackIds) {
        const int pos = position_of(element_id);
        if (pos < 0) return false;
        erase_at(pos);
        return true;
    }
    S prod_between(int first_id, int second_id) requires (TrackIds) {
        const auto [l, r] = between_range(first_id, second_id);
        return prod(l, r);
    }
    S erase_between(int first_id, int second_id) requires (TrackIds) {
        const auto [l, r] = between_range(first_id, second_id);
        return erase_prod(l, r);
    }
    void insert_at_with_id(int pos, int element_id, S value) requires (TrackIds) {
        check_position(pos, true);
#ifndef SKIP_RUNTIME_CHECKS
        if (element_id < 0 || contains_id(element_id))
            throw std::invalid_argument("lazy_implicit_treap duplicate or negative ID");
#endif
        reserve_ids(static_cast<std::size_t>(element_id) + 1);
        const int p = allocate(std::move(value));
        nodes_[p].link.identifier = element_id;
        ids_.slots[element_id] = p;
        if (pos == size()) root_ = merge(root_, p);
        else if (pos == 0) root_ = merge(p, root_);
        else root_ = insert_node(root_, p, pos);
        reset_root_parent();
    }
    void insert_after(int element_id, int new_id, S value) requires (TrackIds) {
        const int pos = position_of(element_id);
#ifndef SKIP_RUNTIME_CHECKS
        if (pos < 0) throw std::out_of_range("lazy_implicit_treap missing ID");
#endif
        insert_at_with_id(pos + 1, new_id, std::move(value));
    }
    void insert_before(int element_id, int new_id, S value) requires (TrackIds) {
        insert_at_with_id(require_id_position(element_id), new_id, std::move(value));
    }
    void erase_val(S value) {
        int p = lower_bound(value);
        if (p < size() && equal(get(p), value)) erase_at(p);
    }
    void erase_max() { pop_back(); }
    template<class G> int max_right(int l, G g) {
        check_position(l, true);
        S acc = e();
#ifndef SKIP_RUNTIME_CHECKS
        if (!g(acc)) throw std::invalid_argument("lazy_implicit_treap predicate identity");
#endif
        int result = size();
        right_walk(root_, l, 0, acc, g, result);
        return result;
    }
    template<bool (*g)(S)> int max_right(int l) { return max_right(l, g); }
    template<class G> int min_left(int r, G g) {
        check_position(r, true);
        S acc = e();
#ifndef SKIP_RUNTIME_CHECKS
        if (!g(acc)) throw std::invalid_argument("lazy_implicit_treap predicate identity");
#endif
        int result = 0;
        left_walk(root_, r, 0, acc, g, result);
        return result;
    }
    template<bool (*g)(S)> int min_left(int r) { return min_left(r, g); }
};
#endif

#ifndef SIRKIT_INDEXED_IMPLICIT_TREAP_DEFINED
#define SIRKIT_INDEXED_IMPLICIT_TREAP_DEFINED
template<class S = RangeAddMin::S,
         S (*op)(S, S) = RangeAddMin::op,
         S (*e)() = RangeAddMin::e,
         class F = RangeAddMin::F,
         S (*mapping)(F, S) = RangeAddMin::mapping,
         F (*composition)(F, F) = RangeAddMin::composition,
         F (*id)() = RangeAddMin::id,
         bool Commutative = false>
using indexed_implicit_treap = lazy_implicit_treap<S, op, e, F, mapping, composition, id, Commutative, true>;
#endif
