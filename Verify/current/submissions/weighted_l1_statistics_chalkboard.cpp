#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 84f8bc285eed84e33f09dfd02012da9e049597de0531e73ff2c4c6caaede7970
// Verify: https://atcoder.jp/contests/abc458/tasks/abc458_d

// Library: DataStructure/Weighted_L1_Statistics.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_l1_statistics.md
//
// API一覧
// WeightedL1Statistics(coordinates={}): 追加できるint64_t座標を登録する。重複可。U座標の登録はO(U log(U+1))。
// insert(x,w=1)/erase(x,w=1): 登録済みのxへuint64_tの重みを加える/引く。不足するeraseはfalseで状態を保つ。
// weight_at(x)/total_weight(): 登録座標の重み/全重み。total_weightはO(1)。全重みがuint64_tを超える追加は例外。
// kth(rank): 重みを個数とみなした0始まりの順位。範囲外はnullopt。
// minimizer_interval(): Σw|x-a|を最小にする閉区間。空集合の関数は全域0で、戻り値はnullopt。
// absolute_deviation(x): 任意のint64_t xに対するΣw|x-a|。uint64_tを超えた場合は例外。
// 異なる座標数Dに対し、上記の個別操作はO(log(D+1))時間。格納領域O(U+1)。コピーは独立、move元は空。
//
// [同梱のFenwick木]
// [fenwick_tree<T>]
// fenwick_tree()/fenwick_tree(int n)/fenwick_tree(vector<T>): 空列/n要素の0/列から構築。O(n+1)時間・領域。
// add(int p,T x): 点加算。sum(int l,int r): 半開区間[l,r)の和をT値で返す。O(log(n+1))、空区間はT{}。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// ACL形式の公開APIをsirkit内に定義する。bool以外の整数は内部を符号なしにして型幅のmod 2^bitで計算する。
// その他のTはT{}が単位元の可換加法群とし、加減算・コピーをO(1)とする。要素数はintの範囲内。
// size()/empty(): 要素数/空か、O(1)。get(int p)/set(int p,T value): 点取得/代入、O(log(n+1))。
// lower_bound(target)/upper_bound(target): sum(0,i+1)>=target/>targetとなる最小int添字、なければsize()。O(log(n+1))。
// 境界探索は全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。全要素の検査はしない。
// values(): 列を復元して返す。O(n+1)時間・追加O(n)。reset(int n)/reset(): n要素/現在の長さを0にする。O(旧n+n+1)。
// clear()/swap(other): 空にして容量保持/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空として使える。
// 添字・区間は既定で検査しSKIP_RUNTIME_CHECKSで除く。容量・添字のオーバーフロー対策は残す。
//
// [同梱の既約分数と固定幅整数]
// [Fraction]
// Fraction(n=0,d=1): int64_tの分子・分母から既約分数を作る。分母は正、0は0/1。O(log M)。
// numerator()/denominator()/sign()/is_integer(): 正規化済みの分子・分母・符号・整数判定。O(1)。
// 四則演算・複合代入・単項+/-: 厳密な既約結果を返す。整数との演算も可。O(log M)。
// ==, <=>: 厳密な等値・大小比較。O(1)。最終結果が表現できても中間積が大きい場合に対応。
// inverse()/abs(): 逆数・絶対値を返す。O(1)。0の逆数はdomain_error。
// floor()/ceil()/trunc(): 床・天井・0方向への整数丸めをint64_tで返す。O(1)。
// explicit浮動小数点変換: 分子/分母を対象の浮動小数点型で計算する。O(1)。
// <<, >>: n/dを出力。入力は整数またはn/d（/は分子の直後）。失敗時は値を保持。
// 0分母・0除算はdomain_error、既約結果の分子/正分母がint64_t範囲外ならoverflow_error。
// これらの数学的・表現上の保護はSKIP_RUNTIME_CHECKS時も有効。Mは整数の絶対値の上限。
// SIRKIT_FRACTION_DISABLE_INT128: GCC/Clangの128ビット拡張を使わず標準C++経路を選ぶ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
namespace detail {
inline std::size_t CheckedFenwickStorageSize(std::size_t n, std::size_t limit) {
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= limit)
        throw std::length_error("fenwick_tree: size");
    return n == 0 ? 0 : n + 1;
}
}

template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("fenwick_tree: negative size");
#endif
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || i >= size()) throw std::out_of_range("fenwick_tree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("fenwick_tree: range");
#else
        (void)l; (void)r;
#endif
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i;;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <compare>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <concepts>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <istream>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <ostream>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_FRACTION_DETAIL_DEFINED
#define SIRKIT_FRACTION_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {

// 引数は0以上2^63以下。奇数化した正の値の差は絶対値が2^63未満。
constexpr std::uint64_t fraction_gcd(std::uint64_t a, std::uint64_t b) {
    if (a == 1 || b == 1) return 1;
    if (a == 0) return b;
    if (b == 0) return a;
    const auto common = std::countr_zero(a | b);
    a >>= std::countr_zero(a);
    b >>= std::countr_zero(b);
    while (a != b) {
        const auto diff = a - b;
        const auto shift = std::countr_zero(diff);
        const auto low = a < b ? a : b;
        const auto mag = diff < (std::uint64_t{1} << 63) ? diff : 0 - diff;
        a = mag >> shift;
        b = low;
    }
    return a << common;
}

struct FractionWide {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FRACTION_DISABLE_INT128)
    __extension__ using Storage = unsigned __int128;
    Storage value = 0;
    constexpr FractionWide(std::uint64_t x = 0) : value(x) {}
    static constexpr FractionWide product(std::uint64_t a, std::uint64_t b) {
        FractionWide r;
        r.value = Storage(a) * b;
        return r;
    }
    friend constexpr FractionWide operator+(FractionWide a, FractionWide b) {
        a.value += b.value;
        return a;
    }
    friend constexpr FractionWide operator-(FractionWide a, FractionWide b) {
        a.value -= b.value;
        return a;
    }
    constexpr auto operator<=>(const FractionWide&) const = default;
    constexpr std::uint64_t low() const { return static_cast<std::uint64_t>(value); }
    constexpr bool fits(std::uint64_t bound) const { return value <= bound; }
    constexpr std::pair<FractionWide, std::uint64_t> divmod(std::uint64_t d) const {
        FractionWide q;
        q.value = value / d;
        return {q, static_cast<std::uint64_t>(value % d)};
    }
#else
    std::uint64_t hi = 0, lo = 0;
    constexpr FractionWide(std::uint64_t x = 0) : lo(x) {}
    static constexpr FractionWide product(std::uint64_t a, std::uint64_t b) {
        constexpr std::uint64_t mask = 0xffffffff;
        const auto a0 = a & mask, a1 = a >> 32;
        const auto b0 = b & mask, b1 = b >> 32;
        const auto t0 = a0 * b0;
        const auto t1 = a1 * b0 + (t0 >> 32);
        const auto t2 = a0 * b1 + (t1 & mask);
        FractionWide r;
        r.hi = a1 * b1 + (t1 >> 32) + (t2 >> 32);
        r.lo = (t2 << 32) | (t0 & mask);
        return r;
    }
    friend constexpr FractionWide operator+(FractionWide a, FractionWide b) {
        const auto old = a.lo;
        a.lo += b.lo;
        a.hi += b.hi + (a.lo < old);
        return a;
    }
    friend constexpr FractionWide operator-(FractionWide a, FractionWide b) {
        const auto old = a.lo;
        a.lo -= b.lo;
        a.hi -= b.hi + (old < b.lo);
        return a;
    }
    constexpr auto operator<=>(const FractionWide&) const = default;
    constexpr std::uint64_t low() const { return lo; }
    constexpr bool fits(std::uint64_t bound) const { return hi == 0 && lo <= bound; }
    constexpr std::pair<FractionWide, std::uint64_t> divmod(std::uint64_t d) const {
        if (hi == 0) return {FractionWide(lo / d), lo % d};
        FractionWide q;
        q.hi = hi / d;
        auto rem = hi % d;
        // 呼び出し側では1 <= d <= 2^63。余りを1bit伸ばしてもuint64_tに収まる。
        for (int bit = 63; bit >= 0; --bit) {
            rem = (rem << 1) | ((lo >> bit) & 1);
            if (rem >= d) {
                rem -= d;
                q.lo |= std::uint64_t{1} << bit;
            }
        }
        return {q, rem};
    }
#endif
};

constexpr std::uint64_t fraction_magnitude(std::int64_t x) {
    const auto u = static_cast<std::uint64_t>(x);
    return x < 0 ? std::uint64_t{0} - u : u;
}

} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_FRACTION_DEFINED
#define SIRKIT_FRACTION_DEFINED
class Fraction {
    using Wide = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FractionWide;
    std::int64_t num_ = 0, den_ = 1;
    static constexpr auto max_ = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());

    static constexpr std::uint64_t magnitude(std::int64_t n) {
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_magnitude(n);
    }
    static constexpr Fraction result(Wide n, Wide d, bool negative) {
        if (!n.fits(max_ + static_cast<unsigned>(negative)) || !d.fits(max_))
            throw std::overflow_error("Fraction: result out of int64_t range");
        Fraction r;
        const auto u = n.low();
        // 負号を指定する呼出しは、分子が0の場合を処理済み。
        r.num_ = negative ? -static_cast<std::int64_t>(u - 1) - 1 : static_cast<std::int64_t>(u);
        r.den_ = static_cast<std::int64_t>(d.low());
        return r;
    }
    static constexpr Fraction combine(const Fraction& a, const Fraction& b, bool subtract) {
        const auto da = static_cast<std::uint64_t>(a.den_);
        const auto db = static_cast<std::uint64_t>(b.den_);
        std::uint64_t g;
        Wide left, right;
        if (da == db) {
            g = da;
            left = Wide(magnitude(a.num_));
            right = Wide(magnitude(b.num_));
        } else {
            g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(da, db);
            left = Wide::product(magnitude(a.num_), db / g);
            right = Wide::product(magnitude(b.num_), da / g);
        }
        bool negative = a.num_ < 0;
        const bool right_negative = (b.num_ < 0) != subtract;
        Wide n;
        if (negative == right_negative) n = left + right;
        else if (left >= right) n = left - right;
        else { n = right - left; negative = right_negative; }
        if (n == Wide{}) return {};
        // 各分数は既約なので、和の分子との追加の共通因子はgの約数だけ。
        const auto h = g == 1 ? 1 : /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(n.divmod(g).second, g);
        if (h != 1) n = n.divmod(h).first;
        return result(n, Wide::product(da / g, db / h), negative);
    }

public:
    constexpr Fraction() = default;
    constexpr Fraction(std::int64_t n, std::int64_t d = 1) {
        if (d == 0) throw std::domain_error("Fraction: zero denominator");
        if (d == 1) { num_ = n; return; }
        if (n == 0) return;
        auto u = magnitude(n), v = magnitude(d);
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(u, v);
        if (g == 1 && d > 0) { num_ = n; den_ = d; return; }
        if (g != 1) { u /= g; v /= g; }
        *this = result(Wide(u), Wide(v), (n < 0) != (d < 0));
    }
    constexpr std::int64_t numerator() const noexcept { return num_; }
    constexpr std::int64_t denominator() const noexcept { return den_; }
    constexpr int sign() const noexcept { return (num_ > 0) - (num_ < 0); }
    constexpr bool is_integer() const noexcept { return den_ == 1; }
    constexpr std::int64_t trunc() const noexcept { return num_ / den_; }
    constexpr std::int64_t floor() const noexcept { return trunc() - (num_ % den_ < 0); }
    constexpr std::int64_t ceil() const noexcept { return trunc() + (num_ % den_ > 0); }
    constexpr Fraction inverse() const {
        if (num_ == 0) throw std::domain_error("Fraction: inverse of zero");
        return result(Wide(static_cast<std::uint64_t>(den_)), Wide(magnitude(num_)), num_ < 0);
    }
    constexpr Fraction abs() const {
        return result(Wide(magnitude(num_)), Wide(static_cast<std::uint64_t>(den_)), false);
    }
    template<std::floating_point T>
    explicit constexpr operator T() const noexcept { return static_cast<T>(num_) / static_cast<T>(den_); }
    constexpr Fraction operator+() const noexcept { return *this; }
    constexpr Fraction operator-() const {
        return result(Wide(magnitude(num_)), Wide(static_cast<std::uint64_t>(den_)), num_ > 0);
    }
    friend constexpr Fraction operator+(const Fraction& a, const Fraction& b) { return combine(a, b, false); }
    friend constexpr Fraction operator-(const Fraction& a, const Fraction& b) { return combine(a, b, true); }
    friend constexpr Fraction operator*(const Fraction& a, const Fraction& b) {
        if (a.num_ == 0 || b.num_ == 0) return {};
        const auto na = magnitude(a.num_), nb = magnitude(b.num_);
        const auto da = static_cast<std::uint64_t>(a.den_), db = static_cast<std::uint64_t>(b.den_);
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(na, db), h = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(nb, da);
        return result(Wide::product(na / g, nb / h), Wide::product(da / h, db / g), (a.num_ < 0) != (b.num_ < 0));
    }
    friend constexpr Fraction operator/(const Fraction& a, const Fraction& b) {
        if (b.num_ == 0) throw std::domain_error("Fraction: division by zero");
        if (a.num_ == 0) return {};
        const auto na = magnitude(a.num_), nb = magnitude(b.num_);
        const auto da = static_cast<std::uint64_t>(a.den_), db = static_cast<std::uint64_t>(b.den_);
        const auto g = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(na, nb), h = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::fraction_gcd(da, db);
        return result(Wide::product(na / g, db / h), Wide::product(da / h, nb / g), (a.num_ < 0) != (b.num_ < 0));
    }
    constexpr Fraction& operator+=(const Fraction& b) { return *this = *this + b; }
    constexpr Fraction& operator-=(const Fraction& b) { return *this = *this - b; }
    constexpr Fraction& operator*=(const Fraction& b) { return *this = *this * b; }
    constexpr Fraction& operator/=(const Fraction& b) { return *this = *this / b; }
    constexpr bool operator==(const Fraction&) const = default;
    constexpr std::strong_ordering operator<=>(const Fraction& b) const {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FRACTION_DISABLE_INT128)
        if (den_ == b.den_) return num_ <=> b.num_;
        __extension__ using SignedWide = __int128;
        const auto left = static_cast<SignedWide>(num_) * b.den_;
        const auto right = static_cast<SignedWide>(b.num_) * den_;
        return left <=> right;
#else
        if (den_ == b.den_) return num_ <=> b.num_;
        if ((num_ < 0) != (b.num_ < 0)) return num_ <=> b.num_;
        const auto left = Wide::product(magnitude(num_), static_cast<std::uint64_t>(b.den_));
        const auto right = Wide::product(magnitude(b.num_), static_cast<std::uint64_t>(den_));
        return num_ < 0 ? right <=> left : left <=> right;
#endif
    }
    friend std::ostream& operator<<(std::ostream& out, const Fraction& x) {
        return out << x.num_ << '/' << x.den_;
    }
    friend std::istream& operator>>(std::istream& in, Fraction& x) {
        std::int64_t n = 0, d = 1;
        if (!(in >> n)) return in;
        if (!in.eof() && in.peek() == '/') {
            in.get();
            if (!(in >> d)) return in;
        }
        try { x = Fraction(n, d); }
        catch (const std::domain_error&) { in.setstate(std::ios::failbit); }
        catch (const std::overflow_error&) { in.setstate(std::ios::failbit); }
        return in;
    }
};
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_WEIGHTED_L1_STATISTICS_DEFINED
#define SIRKIT_WEIGHTED_L1_STATISTICS_DEFINED

namespace detail
{
struct L1Moment
{
    FractionWide value;

    L1Moment &operator+=(const L1Moment &other) noexcept
    {
        value = value + other.value;
        return *this;
    }

    friend L1Moment operator-(const L1Moment &a, const L1Moment &b) noexcept
    {
        return {a.value - b.value};
    }
};
}

class WeightedL1Statistics
{
    using Coordinate = std::int64_t;
    using Weight = std::uint64_t;
    using Moment = detail::FractionWide;
    static constexpr Weight kMax = std::numeric_limits<Weight>::max();

public:
    WeightedL1Statistics() = default;

    explicit WeightedL1Statistics(std::span<const Coordinate> coordinates)
        : coordinates_(coordinates.begin(), coordinates.end())
    {
        std::sort(coordinates_.begin(), coordinates_.end());
        coordinates_.erase(std::unique(coordinates_.begin(), coordinates_.end()), coordinates_.end());
        (void)detail::CheckedFenwickStorageSize(coordinates_.size(), coordinates_.max_size());
        const auto n = static_cast<int>(coordinates_.size());
        counts_ = fenwick_tree<Weight>(n);
        moments_ = fenwick_tree<detail::L1Moment>(n);
    }

    WeightedL1Statistics(const WeightedL1Statistics &) = default;

    WeightedL1Statistics(WeightedL1Statistics &&other) noexcept
    {
        swap(other);
    }

    WeightedL1Statistics &operator=(WeightedL1Statistics other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(WeightedL1Statistics &other) noexcept
    {
        coordinates_.swap(other.coordinates_);
        counts_.swap(other.counts_);
        moments_.swap(other.moments_);
        std::swap(total_, other.total_);
        std::swap(moment_, other.moment_);
    }

    void insert(Coordinate x, Weight weight = 1)
    {
        const int index = Index(x);
        if (weight > kMax - total_)
        {
            throw std::overflow_error("WeightedL1Statistics: total weight out of range");
        }
        Apply(index, weight, Moment::product(Encode(x), weight));
    }

    bool erase(Coordinate x, Weight weight = 1)
    {
        const auto index = Index(x);
        if (weight > counts_.get(index))
        {
            return false;
        }
        Apply(index, Weight{} - weight, Moment{} - Moment::product(Encode(x), weight));
        return true;
    }

    [[nodiscard]] Weight weight_at(Coordinate x) const
    {
        return counts_.get(Index(x));
    }

    [[nodiscard]] Weight total_weight() const noexcept
    {
        return total_;
    }

    [[nodiscard]] std::optional<Coordinate> kth(Weight rank) const
    {
        if (rank >= total_)
        {
            return std::nullopt;
        }
        return Select(rank);
    }

    [[nodiscard]] std::optional<std::pair<Coordinate, Coordinate>> minimizer_interval() const
    {
        if (total_ == 0)
        {
            return std::nullopt;
        }
        return std::pair{Select((total_ - 1) / 2), Select(total_ / 2)};
    }

    [[nodiscard]] Weight absolute_deviation(Coordinate x)
    {
        const int end = static_cast<int>(std::upper_bound(coordinates_.begin(), coordinates_.end(), x) - coordinates_.begin());
        const auto left_count = counts_.sum(0, end);
        const auto left_moment = moments_.sum(0, end).value;
        const auto encoded = Encode(x);
        const auto left = Moment::product(encoded, left_count) - left_moment;
        const auto right = (moment_ - left_moment) - Moment::product(encoded, total_ - left_count);
        const auto sum = left + right;
        if (!sum.fits(kMax))
        {
            throw std::overflow_error("WeightedL1Statistics: absolute deviation out of range");
        }
        return sum.low();
    }

private:
    std::vector<Coordinate> coordinates_;
    fenwick_tree<Weight> counts_;
    fenwick_tree<detail::L1Moment> moments_;
    Weight total_ = 0;
    Moment moment_;

    static Weight Encode(Coordinate x) noexcept
    {
        // 最小のint64_tを原点として、符号付き座標の距離を非負で保持する。
        return static_cast<Weight>(x) ^ (Weight{1} << 63);
    }

    [[nodiscard]] int Index(Coordinate x) const
    {
        const auto it = std::lower_bound(coordinates_.begin(), coordinates_.end(), x);
#ifndef SKIP_RUNTIME_CHECKS
        if (it == coordinates_.end() || *it != x)
        {
            throw std::out_of_range("WeightedL1Statistics: coordinate is not registered");
        }
#endif
        return static_cast<int>(it - coordinates_.begin());
    }

    [[nodiscard]] Coordinate Select(Weight rank) const
    {
        return coordinates_[counts_.lower_bound(rank + 1)];
    }

    void Apply(int index, Weight mass, Moment moment)
    {
        // 削除はそれぞれ2^64、2^128を法とする負の差分。更新後の実値は各型に収まる。
        counts_.add(index, mass);
        moments_.add(index, {moment});
        total_ += mass;
        moment_ = moment_ + moment;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <vector>
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


int main()
{
    std::int64_t x = 0;
    int q = 0;
    verify_in >> x >> q;
    std::vector<std::int64_t> values(2 * q + 1);
    values[0] = x;
    for (int i = 1; i <= 2 * q; ++i)
    {
        verify_in >> values[i];
    }
    WeightedL1Statistics statistics(values);
    statistics.insert(x);
    for (int i = 1; i <= q; ++i)
    {
        statistics.insert(values[2 * i - 1]);
        statistics.insert(values[2 * i]);
        verify_out << *statistics.kth(i) << '\n';
    }
}
