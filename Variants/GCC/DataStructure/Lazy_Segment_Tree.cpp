// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Lazy_Segment_Tree.cpp
// 参考（モノイドと作用）: https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design
// 参考（公開API）: https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lazy_segment_tree.md
//
// API一覧
// [lazy_segtree<S,op,e,F,mapping,composition,id>]
// lazy_segtree()/lazy_segtree(int n)/lazy_segtree(vector<S>): 空列/e()のn要素/列から構築。O(n+1)時間・領域。
// op(S,S)は結合的でe()が左右の単位元。mapping(F,S)は作用、composition(newer,older)はolderの後にnewerを適用し、id()は恒等作用。
// mapping(F,S,size_t length)も拡張として使用可。両方呼べる場合は2引数を使う。演算・コピーはO(1)とする。
// 添字・区間端・nはint、0<=n<=INT_MAXで、内部容量にも収まること。区間は0始まりの半開。size()/empty(): 要素数/空か、O(1)。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// set(int p,S x)/apply(int p,F f)/apply(int l,int r,F f): 点代入/点作用/区間作用。get(int p)/prod(int l,int r): 点の値/左からの区間積。O(log(n+1))。
// all_prod(): 全積のS値、O(1)。空区間・空列の積はe()。照会でも遅延伝播で内部状態を変更する。
// max_right<pred>(int l)/min_left<pred>(int r): 関数bool pred(S)で境界探索。max_right(l,pred)/min_left(r,pred): 関数オブジェクト版。戻り値int、O(log(n+1))。
// pred(e())==true、同じ値に同じ結果を返し、区間を延ばすと真から偽へ一度だけ変わること。単調性のために全要素を走査しない。
// values(): 全点を伝播して列を返す。O(n+1)。assign(vector<S>)/reset(int n)/reset(): 列を再構築/n要素/現在の長さを単位元へ戻す。O(旧n+n+1)。
// clear()/swap(other): 空へ戻す/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空。演算等が例外を投げる場合は内容の強い保証なし。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LAZY_SEGMENT_TREE_DEFINED
#define SIRKIT_LAZY_SEGMENT_TREE_DEFINED
template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class lazy_segtree {
    static_assert(std::is_invocable_r_v<S, decltype(op), const S&, const S&>);
    static_assert(std::is_invocable_r_v<S, decltype(e)>);
    static_assert(std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>
        || std::is_invocable_r_v<S, decltype(mapping), const F&, const S&, std::size_t>);
    static_assert(std::is_invocable_r_v<F, decltype(composition), const F&, const F&>);
    static_assert(std::is_invocable_r_v<F, decltype(id)>);
    S value_identity_;
    F update_identity_;
    std::size_t n_ = 0, base_ = 1, height_ = 0;
    std::vector<S> data_;
    std::vector<F> lazy_;
    std::vector<std::uint8_t> pending_;
    static std::size_t leaf_base(std::size_t n) {
        if (!n) return 1;
        const auto limit = std::min({std::vector<S>{}.max_size() / 2,
                                     std::vector<F>{}.max_size(), std::vector<std::uint8_t>{}.max_size()});
        std::size_t base = 1;
        while (base < n) {
            if (base > limit / 2) throw std::length_error("lazy_segtree: size");
            base *= 2;
        }
        if (base > limit) throw std::length_error("lazy_segtree: size");
        return base;
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("lazy_segtree: size");
#endif
        return static_cast<std::size_t>(n);
    }
    static int vector_size(const std::vector<S>& values) {
        if (values.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("lazy_segtree: size");
        return static_cast<int>(values.size());
    }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || static_cast<std::size_t>(i) >= n_) throw std::out_of_range("lazy_segtree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || static_cast<std::size_t>(r) > n_) throw std::out_of_range("lazy_segtree: range");
#else
        (void)l; (void)r;
#endif
    }
    static std::size_t aligned_node(std::size_t value) {
        if (value == 0) std::unreachable();
        return value >> std::countr_zero(value);
    }
    void pull(std::size_t p) { data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]); }
    void act(std::size_t p, const F& f, std::size_t length) {
        if constexpr (std::is_invocable_r_v<S, decltype(mapping), const F&, const S&>)
            data_[p] = std::invoke(mapping, f, data_[p]);
        else data_[p] = std::invoke(mapping, f, data_[p], length);
        if (p < base_) {
            if (pending_[p]) lazy_[p] = std::invoke(composition, f, lazy_[p]);
            else { lazy_[p] = f; pending_[p] = 1; }
        }
    }
    void push(std::size_t p, std::size_t length) {
        if (!pending_[p]) return;
        act(2 * p, lazy_[p], length / 2);
        act(2 * p + 1, lazy_[p], length / 2);
        pending_[p] = 0;
    }
    void push_path(std::size_t leaf) {
        for (auto level = height_; level; --level) push(leaf >> level, std::size_t{1} << level);
    }
    void push_boundary(std::size_t l, std::size_t r) {
        for (auto level = height_; level; --level) {
            const auto length = std::size_t{1} << level;
            if (l & (length - 1)) push(l >> level, length);
            if (r & (length - 1)) push((r - 1) >> level, length);
        }
    }
    void pull_path(std::size_t leaf) {
        for (leaf >>= 1; leaf; leaf >>= 1) pull(leaf);
    }
public:
    lazy_segtree() : lazy_segtree(0) {}
    explicit lazy_segtree(int n)
        : value_identity_(std::invoke(e)), update_identity_(std::invoke(id)), n_(checked_size(n)), base_(leaf_base(n_)),
          data_(n_ ? 2 * base_ : 0, value_identity_), lazy_(n_ ? base_ : 0, update_identity_), pending_(n_ ? base_ : 0, 0) {
        for (auto p = base_; p > 1; p >>= 1) ++height_;
    }
    explicit lazy_segtree(const std::vector<S>& values) : lazy_segtree(vector_size(values)) {
        if (!n_) return;
        std::copy(values.begin(), values.end(), data_.begin() + base_);
        for (auto p = base_; p > 1;) pull(--p);
    }
    lazy_segtree(const lazy_segtree&) = default;
    lazy_segtree& operator=(const lazy_segtree& other) {
        if (this != &other) { lazy_segtree replacement(other); swap(replacement); }
        return *this;
    }
    lazy_segtree(lazy_segtree&& other)
        : value_identity_(other.value_identity_), update_identity_(other.update_identity_),
          n_(other.n_), base_(other.base_), height_(other.height_), data_(std::move(other.data_)),
          lazy_(std::move(other.lazy_)), pending_(std::move(other.pending_)) {
        other.clear();
    }
    lazy_segtree& operator=(lazy_segtree&& other) {
        if (this != &other) { lazy_segtree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    int size() const noexcept { return static_cast<int>(n_); }
    bool empty() const noexcept { return n_ == 0; }
    S get(int i) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf);
        return data_[leaf];
    }
    void set(int i, S value) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); data_[leaf] = value; pull_path(leaf);
    }
    void apply(int i, F f) {
        check_index(i);
        const auto leaf = base_ + i;
        push_path(leaf); act(leaf, f, 1); pull_path(leaf);
    }
    void apply(int l, int r, F f) {
        check_range(l, r);
        if (l == r) return;
        const auto begin = base_ + static_cast<std::size_t>(l);
        const auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        auto left = begin, right = end;
        std::size_t length = 1;
        while (left < right) {
            if (left & 1) act(left++, f, length);
            if (right & 1) act(--right, f, length);
            left >>= 1; right >>= 1; length *= 2;
        }
        for (std::size_t level = 1; level <= height_; ++level) {
            const auto span = std::size_t{1} << level;
            if (begin & (span - 1)) pull(begin >> level);
            if (end & (span - 1)) pull((end - 1) >> level);
        }
    }
    S prod(int l, int r) {
        check_range(l, r);
        if (l == r) return value_identity_;
        auto begin = base_ + static_cast<std::size_t>(l);
        auto end = base_ + static_cast<std::size_t>(r);
        push_boundary(begin, end);
        // コピー専用型に、局所値を返す際の暗黙ムーブを要求しない。
        if constexpr (!std::is_move_constructible_v<S>) {
            S left = value_identity_, right = value_identity_;
            for (; begin < end; begin >>= 1, end >>= 1) {
                if (begin & 1) left = std::invoke(op, left, data_[begin++]);
                if (end & 1) right = std::invoke(op, data_[--end], right);
            }
            return std::invoke(op, left, right);
        } else {
            auto step = begin & (~begin + 1);
            if (step > end - begin) {
                S result = data_[aligned_node(end) - 1];
                end -= end & (~end + 1);
                while (begin < end) {
                    result = std::invoke(op, data_[aligned_node(end) - 1], result);
                    end -= end & (~end + 1);
                }
                return result;
            }
            S left = data_[aligned_node(begin)];
            begin += step;
            while (begin < end && (step = begin & (~begin + 1)) <= end - begin) {
                left = std::invoke(op, left, data_[aligned_node(begin)]);
                begin += step;
            }
            if (begin == end) return left;
            S right = data_[aligned_node(end) - 1];
            end -= end & (~end + 1);
            while (begin < end) {
                right = std::invoke(op, data_[aligned_node(end) - 1], right);
                end -= end & (~end + 1);
            }
            return std::invoke(op, left, right);
        }
    }

    S all_prod() { return n_ ? data_[1] : value_identity_; }
    template<bool (*pred)(S)> int max_right(int l) {
        return max_right(l, pred);
    }
    template<class Pred> int max_right(int l, Pred pred) {
        check_range(l, l);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(value_identity_)) throw std::invalid_argument("lazy_segtree: predicate(identity)");
#endif
        if (static_cast<std::size_t>(l) == n_) return size();
        auto node = base_ + l;
        push_path(node);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            while (!(node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, accumulated, data_[node]);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node *= 2; length /= 2;
                    S next = std::invoke(op, accumulated, data_[node]);
                    if (pred(next)) { accumulated = std::move(next); ++node; }
                }
                return static_cast<int>(std::min(n_, node - base_));
            }
            accumulated = std::move(candidate);
            ++node;
            if ((node & (node - 1)) == 0) return size();
        }
    }
    template<bool (*pred)(S)> int min_left(int r) {
        return min_left(r, pred);
    }
    template<class Pred> int min_left(int r, Pred pred) {
        check_range(r, r);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(value_identity_)) throw std::invalid_argument("lazy_segtree: predicate(identity)");
#endif
        if (r == 0) return 0;
        auto node = base_ + r;
        push_path(node - 1);
        std::size_t length = 1;
        S accumulated = value_identity_;
        while (true) {
            --node;
            while (node > 1 && (node & 1)) { node >>= 1; length *= 2; }
            S candidate = std::invoke(op, data_[node], accumulated);
            if (!pred(candidate)) {
                while (node < base_) {
                    push(node, length); node = 2 * node + 1; length /= 2;
                    S next = std::invoke(op, data_[node], accumulated);
                    if (pred(next)) { accumulated = std::move(next); --node; }
                }
                return static_cast<int>(node + 1 - base_);
            }
            accumulated = std::move(candidate);
            if ((node & (node - 1)) == 0) return 0;
        }
    }
    std::vector<S> values() {
        if (!n_) return {};
        for (std::size_t begin = 1, length = base_; begin < base_; begin *= 2, length /= 2)
            for (auto p = begin; p < 2 * begin; ++p) push(p, length);
        return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
    }
    void assign(const std::vector<S>& values) {
        lazy_segtree replacement(values); swap(replacement);
    }
    void reset(int n) {
        lazy_segtree replacement(n); swap(replacement);
    }
    void reset() {
        std::fill(data_.begin(), data_.end(), value_identity_);
        std::fill(lazy_.begin(), lazy_.end(), update_identity_);
        std::fill(pending_.begin(), pending_.end(), 0);
    }
    void clear() noexcept {
        data_.clear(); lazy_.clear(); pending_.clear(); n_ = 0; base_ = 1; height_ = 0;
    }
    void swap(lazy_segtree& other) noexcept(std::is_nothrow_swappable_v<S> && std::is_nothrow_swappable_v<F>) {
        using std::swap;
        swap(value_identity_, other.value_identity_); swap(update_identity_, other.update_identity_);
        swap(n_, other.n_); swap(base_, other.base_); swap(height_, other.height_);
        data_.swap(other.data_); lazy_.swap(other.lazy_); pending_.swap(other.pending_);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
