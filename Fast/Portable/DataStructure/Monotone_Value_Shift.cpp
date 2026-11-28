// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Monotone_Value_Shift.cpp
// 参考（値の範囲で区間を選ぶ用途）: https://atcoder.jp/contests/abc389/tasks/abc389_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monotone_value_shift.md
//
// API一覧
// MonotoneValueShift(values): 昇順のint64_t列を保持。O(n+1)時間・領域。n<=INT_MAX、空構築可。
// size()/empty(): 長さ/空か。O(1)。get(i): 0始まりの値。O(log(n+1)+1)。
// lower_bound(value)/upper_bound(value): value以上/より大きい最初の位置。なければn。
// count_between(low,high): 閉区間[low,high]内の値の個数。low<=high。
// shift_between(low,high,delta): 変更前の値が[low,high]に入る全要素をdeltaずらす。昇順とint64_t範囲を保つこと。
// 境界探索・個数・更新はO(log(n+1)+1)。values(): 現在値を全て返す。O(n+1)時間・領域。
// 検査有効時は不正区間・昇順を崩す更新・数値範囲超過で例外。更新失敗時は元の値を保つ。
//
// [同梱のSegment Tree]
// [segtree<S, op, e>（ACL形式、sirkit内に定義）]
// segtree()/segtree(int n)/segtree(vector<S>): 空列/n個の単位元/列から構築。0<=n<=INT_MAX、O(n+1)時間・領域。
// S op(S,S)は結合的、S e()は左右の単位元。同じ呼び出し形式の関数オブジェクトも可。演算/コピーはO(1)。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()の半開区間。
// set(int p,S x): 点代入、O(log(n+1))。get(int p): 点のS値、O(1)。prod(int l,int r): 左からの区間積、O(log(n+1))。
// all_prod(): 全積のS値、O(1)。空区間・空列の積はe()。積は非可換でもよい。
// max_right<pred>(int l)/min_left<pred>(int r): bool pred(S)を使う境界探索。述語を第2引数に渡す形式も可。戻り値はint、O(log(n+1))。
// 探索はpred(e())==trueで、区間を延ばすと真から偽へ一度だけ変化すること。述語に副作用を持たせない。
// [追加機能]
// size()/empty(): intの要素数/空か、O(1)。transform(int p,f): 点をf(現在値)へ更新、O(log(n+1))。
// values(): 列のコピー、O(n+1)。assign(vector<S>)/reset(int n)/reset(): 列/n個/現在の長さの単位元へ再構築、O(旧n+n+1)。
// clear()/swap(other): 容量を保持して空へ/交換、O(旧n+1)/O(1)。コピーは独立、移動元は空。値の演算等が投げる場合は内容の強い保証なし。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstdint>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HLD_SEGMENT_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_SEGMENT_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldSegmentAccess; }
#endif

#ifndef SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
#define SIRKIT_SEGMENT_ARRAY_CAPACITY_DEFINED
namespace detail {
inline std::size_t CheckedSegmentLeafBase(std::size_t count, std::size_t limit, const char* error) {
    if (count == 0) return 1;
    if (count > std::bit_floor(limit)) throw std::length_error(error);
    return std::bit_ceil(count);
}

inline int CheckedSegmentArrayCount(std::size_t count, const char* error) {
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error(error);
    return static_cast<int>(count);
}
}
#endif

#ifndef SIRKIT_SEGMENT_TREE_DEFINED
#define SIRKIT_SEGMENT_TREE_DEFINED
template<class S, auto op, auto e>
class segtree {
    static_assert(std::is_invocable_r_v<S, decltype(op), S, S>, "segtree: op(S,S) must return S");
    static_assert(std::is_invocable_r_v<S, decltype(e)>, "segtree: e() must return S");
    using HldSegmentTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldSegmentAccess;
    S identity_;
    std::size_t n_ = 0, base_ = 1;
    std::vector<S> data_;
    static std::size_t leaf_base(std::size_t n) {
        return detail::CheckedSegmentLeafBase(n, std::vector<S>{}.max_size() / 2, "segtree: size");
    }
    static std::size_t checked_size(int n) {
        return static_cast<std::size_t>(n);
    }
    static int vector_size(const std::vector<S>& values) {
        return detail::CheckedSegmentArrayCount(values.size(), "segtree: int size overflow");
    }
    void check_index(int i) const {
        (void)i;
    }
    void check_range(int l, int r) const {
        (void)l; (void)r;
    }
    static unsigned aligned_node(unsigned value) {
        if (value == 0) std::unreachable();
        return value >> std::countr_zero(value);
    }
    void rebuild() {
        for (auto p = base_; p > 1;) {
            --p;
            data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]);
        }
    }
    void update_carried_ancestors(std::size_t p) {
        if (p <= 1) return;
        S value = std::as_const(data_)[p];
        while (p > 1) {
            const S sibling = std::as_const(data_)[p ^ 1];
            const S left = (p & 1) ? sibling : value;
            const S right = (p & 1) ? value : sibling;
            value = std::invoke(op, left, right);
            p >>= 1;
            data_[p] = std::as_const(value);
        }
    }
    void update_ancestors(std::size_t p) {
        if constexpr (std::is_trivially_copyable_v<S> && std::is_copy_constructible_v<S> &&
                      std::is_copy_assignable_v<S> && sizeof(S) <= 16 &&
                      std::is_invocable_r_v<S, decltype(op), const S&, const S&>) {
#if defined(__clang__)
            if constexpr (sizeof(S) > 8) {
                // 照会を含む呼び出し側のコードが膨らむのを抑える。
                [[clang::noinline]] update_carried_ancestors(p);
                return;
            }
#endif
            update_carried_ancestors(p);
        } else {
            for (p >>= 1; p; p >>= 1) data_[p] = std::invoke(op, data_[2 * p], data_[2 * p + 1]);
        }
    }
public:
    segtree() : segtree(0) {}
    explicit segtree(int n)
        : identity_(std::invoke(e)), n_(checked_size(n)), base_(leaf_base(n_)),
          data_(n_ ? 2 * base_ : 0, identity_) {}
    explicit segtree(const std::vector<S>& values) : segtree(vector_size(values)) {
        if (!n_) return;
        std::copy(values.begin(), values.end(), data_.begin() + base_);
        rebuild();
    }
    segtree(const segtree&) = default;
    segtree& operator=(const segtree& other) {
        if (this != &other) { segtree replacement(other); swap(replacement); }
        return *this;
    }
    segtree(segtree&& other)
        : identity_(other.identity_), n_(other.n_), base_(other.base_), data_(std::move(other.data_)) {
        other.n_ = 0; other.base_ = 1; other.data_.clear();
    }
    segtree& operator=(segtree&& other) {
        if (this != &other) { segtree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    int size() const noexcept { return static_cast<int>(n_); }
    bool empty() const noexcept { return n_ == 0; }
    S get(int i) const { check_index(i); return data_[base_ + static_cast<std::size_t>(i)]; }
    void set(int i, S value) {
        check_index(i);
        const auto p = base_ + static_cast<std::size_t>(i);
        data_[p] = std::move(value);
        update_ancestors(p);
    }
    template<class F> void transform(int i, F&& f) {
        check_index(i);
        const auto p = base_ + static_cast<std::size_t>(i);
        data_[p] = std::invoke(std::forward<F>(f), std::as_const(data_)[p]);
        update_ancestors(p);
    }
    S prod(int l, int r) const {
        check_range(l, r);
        if (l == r) return identity_;
        // n<=INT_MAXなので葉番号はunsignedに収まり、両端は常に正。
        unsigned first = static_cast<unsigned>(base_) + static_cast<unsigned>(l);
        unsigned last = static_cast<unsigned>(base_) + static_cast<unsigned>(r);
        auto step = first & (~first + 1);
        if (step > last - first) {
            // firstの整列幅より残りが短ければ、右端の下位bitを落としてfirstへ戻れる。
            S result = data_[aligned_node(last) - 1];
            last -= last & (~last + 1);
            while (first < last) {
                result = std::invoke(op, data_[aligned_node(last) - 1], result);
                last -= last & (~last + 1);
            }
            return result;
        }
        // 左から最大の整列ブロックを選ぶ。最初は節点をコピーし、単位元との積を省く。
        S left = data_[aligned_node(first)];
        first += step;
        if (first == last) return left;
        // 短い残区間では分割点の計算を省く。
        if (last - first <= 32) {
            while (first < last && (step = first & (~first + 1)) <= last - first) {
                left = std::invoke(op, left, data_[aligned_node(first)]);
                first += step;
            }
            if (first == last) return left;
            S right = data_[aligned_node(last) - 1];
            last -= last & (~last + 1);
            while (first < last) {
                right = std::invoke(op, data_[aligned_node(last) - 1], right);
                last -= last & (~last + 1);
            }
            return std::invoke(op, left, right);
        }
        // 左走査が止まる整列境界を先に求め、独立な左右の積を交互に進める。
        const unsigned stop = last & (0U - std::bit_floor((first - 1) ^ (last - 1)));
        if (stop == last) {
            while (first < stop) {
                left = std::invoke(op, left, data_[aligned_node(first)]);
                first += first & (~first + 1);
            }
            return left;
        }
        S right = data_[aligned_node(last) - 1];
        last -= last & (~last + 1);
        while (first < stop && last > stop) {
            left = std::invoke(op, left, data_[aligned_node(first)]);
            first += first & (~first + 1);
            right = std::invoke(op, data_[aligned_node(last) - 1], right);
            last -= last & (~last + 1);
        }
        while (first < stop) {
            left = std::invoke(op, left, data_[aligned_node(first)]);
            first += first & (~first + 1);
        }
        while (last > stop) {
            right = std::invoke(op, data_[aligned_node(last) - 1], right);
            last -= last & (~last + 1);
        }
        return std::invoke(op, left, right);
    }
    S all_prod() const { return n_ ? data_[1] : identity_; }
    template<bool (*Pred)(S)> int max_right(int l) const { return max_right(l, Pred); }
    template<class Pred> int max_right(int l, Pred pred) const {
        check_range(l, l);
        if (static_cast<std::size_t>(l) == n_) return size();
        S accumulated = identity_;
        auto node = base_ + static_cast<std::size_t>(l);
        while (true) {
            while (!(node & 1)) node >>= 1;
            S candidate = std::invoke(op, accumulated, data_[node]);
            if (!std::invoke(pred, candidate)) {
                while (node < base_) {
                    node *= 2;
                    S next = std::invoke(op, accumulated, data_[node]);
                    if (std::invoke(pred, next)) { accumulated = std::move(next); ++node; }
                }
                return static_cast<int>(std::min(n_, node - base_));
            }
            accumulated = std::move(candidate);
            ++node;
            if ((node & (node - 1)) == 0) return size();
        }
    }
    template<bool (*Pred)(S)> int min_left(int r) const { return min_left(r, Pred); }
    template<class Pred> int min_left(int r, Pred pred) const {
        check_range(r, r);
        if (r == 0) return 0;
        S accumulated = identity_;
        auto node = base_ + static_cast<std::size_t>(r);
        while (true) {
            --node;
            while (node > 1 && (node & 1)) node >>= 1;
            S candidate = std::invoke(op, data_[node], accumulated);
            if (!std::invoke(pred, candidate)) {
                while (node < base_) {
                    node = 2 * node + 1;
                    S next = std::invoke(op, data_[node], accumulated);
                    if (std::invoke(pred, next)) { accumulated = std::move(next); --node; }
                }
                return static_cast<int>(node + 1 - base_);
            }
            accumulated = std::move(candidate);
            if ((node & (node - 1)) == 0) return 0;
        }
    }
    std::vector<S> values() const {
        if (!n_) return {};
        return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
    }
    void assign(const std::vector<S>& values) { segtree replacement(values); swap(replacement); }
    void reset(int n) { segtree replacement(n); swap(replacement); }
    void reset() { std::fill(data_.begin(), data_.end(), identity_); }
    void clear() noexcept { data_.clear(); n_ = 0; base_ = 1; }
    void swap(segtree& other) noexcept(std::is_nothrow_swappable_v<S>) {
        using std::swap;
        swap(identity_, other.identity_);
        swap(n_, other.n_); swap(base_, other.base_); data_.swap(other.data_);
    }
};
#endif

#ifndef SIRKIT_MONOTONE_VALUE_SHIFT_DEFINED
#define SIRKIT_MONOTONE_VALUE_SHIFT_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr std::uint64_t AddMonotoneValueDifferences(std::uint64_t left, std::uint64_t right) noexcept
{
    return left + right;
}

inline constexpr std::uint64_t EmptyMonotoneValueDifference() noexcept
{
    return 0;
}
} // namespace reinsirk::detail

class MonotoneValueShift
{
    using Value = std::int64_t;
    using Key = std::uint64_t;
    static constexpr Key kSignBit = Key{1} << 63;
    segtree<Key, /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddMonotoneValueDifferences, /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::EmptyMonotoneValueDifference> differences_;

    static constexpr Key key(Value value) noexcept
    {
        return static_cast<Key>(value) ^ kSignBit;
    }

    static constexpr Value original(Key coordinate) noexcept
    {
        return static_cast<Value>(coordinate ^ kSignBit);
    }

    static std::vector<Key> make_differences(std::span<const Value> values)
    {
        static_cast<void>(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedSegmentArrayCount(values.size(), "MonotoneValueShift: length"));
        std::vector<Key> result;
        result.reserve(values.size());
        Key previous = 0;
        for (const auto value : values)
        {
            const auto coordinate = key(value);
            result.push_back(coordinate - previous);
            previous = coordinate;
        }
        return result;
    }

    [[nodiscard]] Key key_at(int position) const
    {
        return differences_.prod(0, position + 1);
    }

    static void check_interval([[maybe_unused]] Value low, [[maybe_unused]] Value high)
    {
    }

public:
    MonotoneValueShift() = default;

    explicit MonotoneValueShift(std::span<const Value> values) : differences_(make_differences(values))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return differences_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return differences_.empty();
    }

    [[nodiscard]] Value get(int position) const
    {
        return original(key_at(position));
    }

    [[nodiscard]] int lower_bound(Value value) const
    {
        const auto target = key(value);
        if (target == 0)
        {
            return 0;
        }
        return differences_.max_right(0, [target](Key prefix) { return prefix < target; });
    }

    [[nodiscard]] int upper_bound(Value value) const
    {
        const auto target = key(value);
        return differences_.max_right(0, [target](Key prefix) { return prefix <= target; });
    }

    [[nodiscard]] int count_between(Value low, Value high) const
    {
        check_interval(low, high);
        return upper_bound(high) - lower_bound(low);
    }

    void shift_between(Value low, Value high, Value delta)
    {
        check_interval(low, high);
        if (delta == 0)
        {
            return;
        }
        const int left = lower_bound(low);
        const int right = upper_bound(high);
        if (left == right)
        {
            return;
        }
        const auto offset = static_cast<Key>(delta);
        // 選んだ区間内の差は変わらない。符号なし演算で両端を更新し終える。
        differences_.set(left, differences_.get(left) + offset);
        if (right != size())
        {
            differences_.set(right, differences_.get(right) - offset);
        }
    }

    [[nodiscard]] std::vector<Value> values() const
    {
        const auto differences = differences_.values();
        std::vector<Value> result;
        result.reserve(differences.size());
        Key coordinate = 0;
        for (const auto difference : differences)
        {
            coordinate += difference;
            result.push_back(original(coordinate));
        }
        return result;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
