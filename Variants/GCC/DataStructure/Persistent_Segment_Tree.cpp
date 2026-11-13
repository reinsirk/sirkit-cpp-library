// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Persistent_Segment_Tree.cpp
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/persistent_segment_tree
// 参考: https://nyaannyaan.github.io/library/segment-tree/persistent-segment-tree.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_segment_tree.md
//
// API一覧
// [PersistentSegmentTree<T,Op>] Op(const T&,const T&)は結合的、identityは両側単位元。
// T/Opはコピー可能。Tの破棄は例外を送出しない。演算・コピーの費用をO(1)として記す。
// (n,op,identity)/(values,op,identity): 単位元列/配列を構築。O(1)/O(n)。
// size()/empty()/allocated_nodes(): 長さ/空か/格納ノード数、O(1)。
// Root: コピー可能な不変ハンドル。同じオブジェクトの有効な根だけを渡す。
// empty_root()/latest_root(): 全要素単位元の根/直近のbuild・set・transformの根、O(1)。
// build(values): 長さsize()の列の根を追加し返す。以前の根を保持、O(n)時間・追加領域。
// set(root,i,value)/transform(root,i,f): 点代入/f(旧値)の根を返す。O(log(n+1))時間・追加領域。
// get(root,i): 点の値。prod(root,l,r)/all_prod(root): 左から右への半開区間積/全体積。
// get/prodはO(log(n+1))、all_prodはO(1)。空区間の積は単位元。
// prodは共通の経路から区間両端へ分かれ、反復処理で順序を保って集約する。追加領域O(1)。
// max_right(root,l,pred)/min_left(root,r,pred): pred(積)が真の最大右端/最小左端。
// pred(identity)==true、区間を伸ばすと真から偽へ一度だけ変化する前提。O(log(n+1))。
// reserve(nodes): 総ノード容量を予約。再確保時O(allocated_nodes())。ハンドルは失効しない。
// reset(n)/clear(): 全根を無効化して単位元列/長さ0へ戻す。旧ノード数に比例する破棄費用。
// コピーは独立。remap_from(original,root): 直接コピー元のコピー時に存在した根を移す、O(1)。
// 元のRootはコピー先へ直接渡せない。latest_root()/empty_root()はコピー先の根を返す。
// moveは根を移動先へ引き継ぐ。move元には破棄・代入だけ行う。swapは根も交換する。
// 更新・構築の失敗時は追加ノードを取り消し、旧根とlatest_rootを維持する。
// SKIP_RUNTIME_CHECKSで根の所属・添字・長さ・述語の単位元検査と所属用状態を除く。

#include <concepts>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#if !defined(SKIP_RUNTIME_CHECKS) && !defined(SIRKIT_PERSISTENT_TREE_OWNER_DEFINED)
#define SIRKIT_PERSISTENT_TREE_OWNER_DEFINED
namespace reinsirk::detail {
inline std::uint64_t persistent_tree_owner() {
    static std::atomic<std::uint64_t> next{1};
    auto value = next.load(std::memory_order_relaxed);
    for (;;) {
        if (value == std::numeric_limits<std::uint64_t>::max())
            throw std::length_error("PersistentSegmentTree owner limit");
        if (next.compare_exchange_weak(value, value + 1, std::memory_order_relaxed)) return value;
    }
}
}
#endif

#ifndef SIRKIT_PERSISTENT_SEGMENT_TREE_DEFINED
#define SIRKIT_PERSISTENT_SEGMENT_TREE_DEFINED
template<class T, class Op>
class PersistentSegmentTree {
    static_assert(std::copyable<T> && std::copy_constructible<Op>);
    static_assert(std::is_invocable_r_v<T, const Op&, const T&, const T&>);
    static_assert(std::is_nothrow_destructible_v<T>);
public:
    class Root {
        friend class PersistentSegmentTree;
        std::size_t index_ = 0;
#ifndef SKIP_RUNTIME_CHECKS
        std::uint64_t owner_ = 0;
        Root(std::size_t index, std::uint64_t owner) : index_(index), owner_(owner) {}
#else
        explicit Root(std::size_t index) : index_(index) {}
#endif
    public:
        Root() = default;
        friend bool operator==(const Root&, const Root&) = default;
    };
private:
    struct Node {
        T value;
        std::size_t left, right;
    };
    struct State {
        std::size_t n, latest = 0;
        Op op;
        T identity;
        std::vector<Node> nodes;
#ifndef SKIP_RUNTIME_CHECKS
        std::uint64_t owner = reinsirk::detail::persistent_tree_owner();
        std::uint64_t copied_owner = 0;
        std::size_t copied_nodes = 0;
#endif
        State(std::size_t count, Op operation, T unit)
            : n(count), op(std::move(operation)), identity(std::move(unit)) {}
        State(const State& other)
            : n(other.n), latest(other.latest), op(other.op), identity(other.identity), nodes(other.nodes) {
#ifndef SKIP_RUNTIME_CHECKS
            copied_owner = other.owner; copied_nodes = other.nodes.size();
#endif
        }
    };
    std::unique_ptr<State> state_;

    Root handle(std::size_t index) const {
#ifndef SKIP_RUNTIME_CHECKS
        return Root(index, state_->owner);
#else
        return Root(index);
#endif
    }
    std::size_t checked_root(const Root& root) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (root.owner_ != state_->owner || root.index_ > state_->nodes.size())
            throw std::invalid_argument("PersistentSegmentTree root");
#endif
        return root.index_;
    }
    void check_index(std::size_t index) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= size()) throw std::out_of_range("PersistentSegmentTree index");
#else
        static_cast<void>(index);
#endif
    }
    void check_range(std::size_t left, std::size_t right) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size()) throw std::out_of_range("PersistentSegmentTree range");
#else
        static_cast<void>(left); static_cast<void>(right);
#endif
    }
    const T& value(std::size_t index) const {
        return index == 0 ? state_->identity : state_->nodes[index - 1].value;
    }
    std::size_t child(std::size_t index, bool right) const {
        return index == 0 ? 0 : (right ? state_->nodes[index - 1].right : state_->nodes[index - 1].left);
    }
    std::size_t append(T aggregate, std::size_t left, std::size_t right) {
        if (state_->nodes.size() == state_->nodes.max_size())
            throw std::length_error("PersistentSegmentTree node limit");
        state_->nodes.push_back(Node{std::move(aggregate), left, right});
        return state_->nodes.size();
    }
    std::size_t merge(std::size_t left, std::size_t right) {
        T aggregate = std::invoke(std::as_const(state_->op), value(left), value(right));
        return append(std::move(aggregate), left, right);
    }
    void rollback(std::size_t count) noexcept {
        while (state_->nodes.size() > count) state_->nodes.pop_back();
    }
    std::size_t build_impl(std::span<const T> values, std::size_t left, std::size_t right) {
        if (left == right) return 0;
        if (right - left == 1) return append(values[left], 0, 0);
        const auto middle = left + (right - left) / 2;
        const auto a = build_impl(values, left, middle);
        const auto b = build_impl(values, middle, right);
        return merge(a, b);
    }
    template<class F>
    std::size_t transform_impl(std::size_t index, std::size_t position, F& function,
                               std::size_t left, std::size_t right) {
        if (right - left == 1) return append(std::invoke(function, value(index)), 0, 0);
        const auto middle = left + (right - left) / 2;
        auto a = child(index, false), b = child(index, true);
        if (position < middle) a = transform_impl(a, position, function, left, middle);
        else b = transform_impl(b, position, function, middle, right);
        return merge(a, b);
    }
    T prod_impl(std::size_t index, std::size_t begin, std::size_t end,
                std::size_t left, std::size_t right) const {
        if (begin == end || index == 0) return state_->identity;
        std::size_t middle;
        for (;;) {
            if (begin <= left && right <= end) return value(index);
            middle = left + (right - left) / 2;
            if (end <= middle) { index = child(index, false); right = middle; }
            else if (middle <= begin) { index = child(index, true); left = middle; }
            else break;
            if (index == 0) return state_->identity;
        }
        T first = state_->identity, second = state_->identity;
        auto current = child(index, false);
        auto lo = left, hi = middle;
        while (current != 0) {
            if (begin <= lo) {
                first = std::invoke(std::as_const(state_->op), value(current), first);
                break;
            }
            const auto split = lo + (hi - lo) / 2;
            if (begin < split) {
                const auto sibling = child(current, true);
                if (sibling != 0) first = std::invoke(std::as_const(state_->op), value(sibling), first);
                current = child(current, false); hi = split;
            } else { current = child(current, true); lo = split; }
        }
        current = child(index, true); lo = middle; hi = right;
        while (current != 0) {
            if (hi <= end) {
                second = std::invoke(std::as_const(state_->op), second, value(current));
                break;
            }
            const auto split = lo + (hi - lo) / 2;
            if (split < end) {
                const auto sibling = child(current, false);
                if (sibling != 0) second = std::invoke(std::as_const(state_->op), second, value(sibling));
                current = child(current, true); lo = split;
            } else { current = child(current, false); hi = split; }
        }
        return std::invoke(std::as_const(state_->op), first, second);
    }
    template<class Pred>
    std::size_t right_impl(std::size_t index, std::size_t begin, Pred& pred,
                           T& aggregate, std::size_t left, std::size_t right) const {
        if (right <= begin) return right;
        if (begin <= left) {
            T next = std::invoke(std::as_const(state_->op), aggregate, value(index));
            if (std::invoke(pred, std::as_const(next))) { aggregate = std::move(next); return right; }
            if (right - left == 1) return left;
        }
        const auto middle = left + (right - left) / 2;
        const auto result = right_impl(child(index, false), begin, pred, aggregate, left, middle);
        if (result < middle) return result;
        return right_impl(child(index, true), begin, pred, aggregate, middle, right);
    }
    template<class Pred>
    std::size_t left_impl(std::size_t index, std::size_t end, Pred& pred,
                          T& aggregate, std::size_t left, std::size_t right) const {
        if (end <= left) return left;
        if (right <= end) {
            T next = std::invoke(std::as_const(state_->op), value(index), aggregate);
            if (std::invoke(pred, std::as_const(next))) { aggregate = std::move(next); return left; }
            if (right - left == 1) return right;
        }
        const auto middle = left + (right - left) / 2;
        const auto result = left_impl(child(index, true), end, pred, aggregate, middle, right);
        if (result > middle) return result;
        return left_impl(child(index, false), end, pred, aggregate, left, middle);
    }
public:
    PersistentSegmentTree(std::size_t n, Op op, T identity)
        : state_(std::make_unique<State>(n, std::move(op), std::move(identity))) {}
    PersistentSegmentTree(std::span<const T> values, Op op, T identity)
        : PersistentSegmentTree(values.size(), std::move(op), std::move(identity)) { build(values); }
    PersistentSegmentTree(const std::vector<T>& values, Op op, T identity)
        : PersistentSegmentTree(std::span<const T>(values), std::move(op), std::move(identity)) {}
    PersistentSegmentTree(const PersistentSegmentTree& other)
        : state_(std::make_unique<State>(*other.state_)) {}
    PersistentSegmentTree& operator=(const PersistentSegmentTree& other) {
        if (this != &other) { auto next = std::make_unique<State>(*other.state_); state_.swap(next); }
        return *this;
    }
    PersistentSegmentTree(PersistentSegmentTree&&) noexcept = default;
    PersistentSegmentTree& operator=(PersistentSegmentTree&&) noexcept = default;
    void swap(PersistentSegmentTree& other) noexcept { state_.swap(other.state_); }
    friend void swap(PersistentSegmentTree& a, PersistentSegmentTree& b) noexcept { a.swap(b); }
    std::size_t size() const noexcept { return state_->n; }
    bool empty() const noexcept { return size() == 0; }
    std::size_t allocated_nodes() const noexcept { return state_->nodes.size(); }
    Root empty_root() const { return handle(0); }
    Root latest_root() const { return handle(state_->latest); }
    Root remap_from(const PersistentSegmentTree& original, const Root& root) const {
#ifndef SKIP_RUNTIME_CHECKS
        original.checked_root(root);
        if (state_->copied_owner != original.state_->owner || root.index_ > state_->copied_nodes)
            throw std::invalid_argument("PersistentSegmentTree copied root");
#else
        static_cast<void>(original);
#endif
        return handle(root.index_);
    }
    void reserve(std::size_t nodes) { state_->nodes.reserve(nodes); }
    void reset(std::size_t n) {
        auto next = std::make_unique<State>(n, state_->op, state_->identity);
        state_.swap(next);
    }
    void clear() { reset(0); }
    Root build(std::span<const T> values) {
#ifndef SKIP_RUNTIME_CHECKS
        if (values.size() != size()) throw std::invalid_argument("PersistentSegmentTree build length");
#endif
        const auto before = allocated_nodes();
        try { state_->latest = build_impl(values, 0, size()); }
        catch (...) { rollback(before); throw; }
        return latest_root();
    }
    Root build(const std::vector<T>& values) { return build(std::span<const T>(values)); }
    template<class F>
    Root transform(const Root& root, std::size_t position, F function) {
        const auto index = checked_root(root); check_index(position);
        const auto before = allocated_nodes();
        try { state_->latest = transform_impl(index, position, function, 0, size()); }
        catch (...) { rollback(before); throw; }
        return latest_root();
    }
    Root set(const Root& root, std::size_t position, const T& replacement) {
        return transform(root, position, [&replacement](const T&) -> const T& { return replacement; });
    }
    T get(const Root& root, std::size_t position) const {
        auto index = checked_root(root); check_index(position);
        std::size_t left = 0, right = size();
        while (right - left > 1) {
            const auto middle = left + (right - left) / 2;
            if (position < middle) { index = child(index, false); right = middle; }
            else { index = child(index, true); left = middle; }
        }
        return value(index);
    }
    T prod(const Root& root, std::size_t left, std::size_t right) const {
        const auto index = checked_root(root); check_range(left, right);
        return prod_impl(index, left, right, 0, size());
    }
    T all_prod(const Root& root) const { return value(checked_root(root)); }
    template<class Pred>
    std::size_t max_right(const Root& root, std::size_t left, Pred pred) const {
        const auto index = checked_root(root); check_range(left, size());
#ifndef SKIP_RUNTIME_CHECKS
        if (!std::invoke(pred, std::as_const(state_->identity)))
            throw std::invalid_argument("PersistentSegmentTree predicate identity");
#endif
        if (left == size()) return size();
        T aggregate = state_->identity;
        return right_impl(index, left, pred, aggregate, 0, size());
    }
    template<class Pred>
    std::size_t min_left(const Root& root, std::size_t right, Pred pred) const {
        const auto index = checked_root(root); check_range(0, right);
#ifndef SKIP_RUNTIME_CHECKS
        if (!std::invoke(pred, std::as_const(state_->identity)))
            throw std::invalid_argument("PersistentSegmentTree predicate identity");
#endif
        if (right == 0) return 0;
        T aggregate = state_->identity;
        return left_impl(index, right, pred, aggregate, 0, size());
    }
};
#endif
