// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Cartesian_Tree.cpp
// 参考: https://nyaannyaan.github.io/library/tree/cartesian-tree.hpp.html
// 最小元を根とする区間分割と単調stackの構成を参考に独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cartesian_tree.md
// API一覧
// CartesianTree(): 空。CartesianTree(span[,compare]) / CartesianTree(vector[,compare]): 値列から構築。
// build(span[,compare]) / build(vector[,compare]): 木を再構築。既定compareはstd::less<>。
//   strict weak orderingのcompare(a,b)がtrueならaを優先。同値なら左の添字が先祖になる。
//   頂点番号は入力の添字。入力値/comparatorは保持せず、入力を書き換えない。vector<bool>は対象外。
//   長さn<=INT32_MAX、時間O(n)（比較O(n)回）・所有O(n)・追加作業O(n)。再帰なし。
// size()/empty(): 頂点数/空か、root(): 根のstd::int32_t添字（空なら-1）。各O(1)。
// parent(v)/left_child(v)/right_child(v): std::int32_t添字、根の親/存在しない子は-1。
// children(v): {左の子,右の子}。parents()/left_children()/right_children(): const span。各O(1)。
// clear(): 空にする。copyはO(n)で独立、代入/buildは失敗時に旧木とviewを維持する。
//   comparatorの外部状態は戻さない。比較中に本体/入力を変更・移動しない。
// move元は空/root=-1。swapはO(1)。viewはbuild成功/clear/代入/所有権移動まで有効。
// 番号/負数の検査はSKIP_RUNTIME_CHECKSで除外。添字容量保護は常に維持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CARTESIAN_TREE_DEFINED
#define SIRKIT_CARTESIAN_TREE_DEFINED
class CartesianTree {
    std::vector<std::int32_t> parents_, left_, right_;
    std::int32_t root_ = -1;
    void check_vertex(std::int32_t vertex) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
            throw std::out_of_range("CartesianTree vertex");
#else
        (void)vertex;
#endif
    }
public:
    CartesianTree() = default;
    template<class T, std::size_t Extent, class Compare = std::less<>>
    explicit CartesianTree(std::span<T, Extent> values, Compare&& compare = Compare{}) {
        build(values, std::forward<Compare>(compare));
    }
    template<class T, class Allocator, class Compare = std::less<>>
    requires (!std::is_same_v<T, bool>)
    explicit CartesianTree(const std::vector<T, Allocator>& values, Compare&& compare = Compare{}) {
        build(values, std::forward<Compare>(compare));
    }
    CartesianTree(const CartesianTree&) = default;
    CartesianTree& operator=(const CartesianTree& other) {
        if (this != &other) { CartesianTree copy(other); swap(copy); }
        return *this;
    }
    CartesianTree(CartesianTree&& other) noexcept { swap(other); }
    CartesianTree& operator=(CartesianTree&& other) noexcept {
        if (this != &other) { CartesianTree moved(std::move(other)); swap(moved); }
        return *this;
    }
    void swap(CartesianTree& other) noexcept {
        parents_.swap(other.parents_); left_.swap(other.left_); right_.swap(other.right_);
        std::swap(root_, other.root_);
    }
    friend void swap(CartesianTree& a, CartesianTree& b) noexcept { a.swap(b); }
    template<class T, std::size_t Extent, class Compare = std::less<>>
    void build(std::span<T, Extent> values, Compare&& compare = Compare{}) {
        const auto n = values.size();
        if (n > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
            throw std::length_error("CartesianTree index capacity");
        CartesianTree next;
        next.parents_.assign(n, -1); next.left_.assign(n, -1); next.right_.assign(n, -1);
        std::vector<std::int32_t> stack; stack.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            const auto vertex = static_cast<std::int32_t>(i);
            std::int32_t removed = -1;
            while (!stack.empty() && std::invoke(compare, std::as_const(values[i]),
                    std::as_const(values[static_cast<std::size_t>(stack.back())]))) {
                removed = stack.back(); stack.pop_back();
            }
            if (!stack.empty()) {
                const auto parent = static_cast<std::size_t>(stack.back());
                next.parents_[i] = stack.back(); next.right_[parent] = vertex;
            }
            if (removed != -1) {
                next.parents_[static_cast<std::size_t>(removed)] = vertex; next.left_[i] = removed;
            }
            stack.push_back(vertex);
        }
        if (!stack.empty()) next.root_ = stack.front();
        swap(next);
    }
    template<class T, class Allocator, class Compare = std::less<>>
    requires (!std::is_same_v<T, bool>)
    void build(const std::vector<T, Allocator>& values, Compare&& compare = Compare{}) {
        build(std::span<const T>(values.data(), values.size()), std::forward<Compare>(compare));
    }
    std::size_t size() const noexcept { return parents_.size(); }
    bool empty() const noexcept { return parents_.empty(); }
    std::int32_t root() const noexcept { return root_; }
    std::int32_t parent(std::int32_t vertex) const { check_vertex(vertex); return parents_[static_cast<std::size_t>(vertex)]; }
    std::int32_t left_child(std::int32_t vertex) const { check_vertex(vertex); return left_[static_cast<std::size_t>(vertex)]; }
    std::int32_t right_child(std::int32_t vertex) const { check_vertex(vertex); return right_[static_cast<std::size_t>(vertex)]; }
    std::pair<std::int32_t, std::int32_t> children(std::int32_t vertex) const {
        check_vertex(vertex); const auto i = static_cast<std::size_t>(vertex); return {left_[i], right_[i]};
    }
    std::span<const std::int32_t> parents() const noexcept { return parents_; }
    std::span<const std::int32_t> left_children() const noexcept { return left_; }
    std::span<const std::int32_t> right_children() const noexcept { return right_; }
    void clear() noexcept { parents_.clear(); left_.clear(); right_.clear(); root_ = -1; }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
