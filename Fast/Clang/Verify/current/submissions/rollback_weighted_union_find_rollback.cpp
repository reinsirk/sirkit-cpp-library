#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/persistent_unionfind

// Library: DataStructure/rollback-union-findw.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/rollback-union-findw.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rollback_weighted_union_find.md
//
// API一覧
// [RollbackWeightedUnionFind<T>]
// TはT(0), +, -を備えるコピー・ムーブ可能な可換加法群。consistent/unite_checkedには==も必要。
// 標準の符号付き整数Tは最終結果の範囲外でoverflow_error。結合時は保存する根間の差もT範囲内が必要。
// 符号なし整数は剰余演算。整数の容量・演算保護はSKIP_RUNTIME_CHECKSでも維持する。
// RollbackWeightedUnionFind(): 空。RollbackWeightedUnionFind(n): n>=0のn頂点を構築、O(n+1)。
// コピーはO(n+h+1)で独立（hは履歴長）。移動元は空、移動構築O(1)。
// find(v)/same(x,y)/size(v): 代表/同一成分か/成分サイズ、O(log(n+1))。get_weight(v)は根からvへの差。
// root_with_weight(v): {根, 根からvへの差}。diff(x,y): y-xの差、同一成分が前提。各O(log(n+1))。
// try_diff(x,y): 未連結ならnullopt、consistent(x,y,w): 未連結または差がwならtrue。各O(log(n+1))。
// unite(x,y,w): y-x=wを追加、新規結合ならtrue。unite_checkedはMerged/AlreadyConsistent/Contradictionを返す。
// 各追加は冗長・矛盾時も履歴1件、O(log(n+1))償却時間。経路圧縮なし。undo(): 1追加を戻す、O(1)、履歴必須。
// snapshot()/get_state(): 保存位置設定/現在の追加回数、O(1)。rollback(state=-1): 指定/保存位置へ戻す、O(1+戻す回数)。
// 保存位置より前へ戻ると保存位置も縮める。clear_history(): 連結状態と容量を保ち履歴/保存位置を消去、O(h+1)上界、noexcept。
// size()/empty()/component_count(): 全頂点数/空判定/成分数、O(1)。groups(): 成分ごとの頂点昇順一覧、O(1+n log(n+1))。
// reserve_history(k): k追加分の容量を確保、O(h+1)上界。swap(other): 履歴・保存位置ごと交換、O(1)。
// clear(): 空へ戻して容量保持、O(n+h+1)上界。reset(n)/reset(): 指定/現在の頂点数で再構築、O(旧n+旧h+n+1)上界。
// resetはコピー構築・破棄が非送出で頂点/重み容量が足りる場合に領域再利用。代入も非送出なら既存重みを0へ戻す。
// 増加分はresizeで初期化し、保持部分だけ再代入する。その他は一時構築後に交換。reset失敗時は論理状態を保持。
// swap(a,b): 履歴・保存位置ごと定数時間で交換、noexcept。
// SIRKIT_DISABLE_SIMD: groupsの追加AVX2実装とCPU判定を除外。標準C++の経路を使用。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#if (!defined(SIRKIT_DISABLE_SIMD)) && (!defined(__AVX2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
#include <immintrin.h>
#endif
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#if (!defined(SIRKIT_DISABLE_SIMD)) && (!defined(__AVX2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
#ifndef SIRKIT_UNION_GROUP_ROOTS_AVX2_DEFINED
#define SIRKIT_UNION_GROUP_ROOTS_AVX2_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<bool Compress>
[[gnu::target("avx2")]]
inline void GatherGroupRoots(const int* parents, int* leaders, std::size_t n, int* writable = nullptr) {
    std::size_t i = 0;
    auto indices = _mm256_setr_epi32(0,1,2,3,4,5,6,7);
    const auto negative_one = _mm256_set1_epi32(-1);
    for (; i + 8 <= n; i += 8) {
        const auto original = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(parents+i));
        const auto nonroot = _mm256_cmpgt_epi32(original, negative_one);
        auto root = _mm256_blendv_epi8(indices, original, nonroot);
        // 根のlaneは再読込せず、未解決の親だけを追う。
        auto active = nonroot;
        while (!_mm256_testz_si256(active, active)) {
            const auto next = _mm256_mask_i32gather_epi32(negative_one, parents, root, active, 4);
            active = _mm256_cmpgt_epi32(next, negative_one);
            root = _mm256_blendv_epi8(root, next, active);
        }
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(leaders+i), root);
        if constexpr (Compress)
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(writable+i), _mm256_blendv_epi8(original,root,nonroot));
        indices = _mm256_add_epi32(indices, _mm256_set1_epi32(8));
    }
    for (; i < n; ++i) {
        int root = static_cast<int>(i);
        while (parents[root] >= 0) root = parents[root];
        leaders[i] = root;
        if constexpr (Compress) if (parents[i] >= 0) writable[i] = root;
    }
}
} // namespace reinsirk::detail
#endif
#endif

#ifndef SIRKIT_ROLLBACK_WEIGHTED_UNION_FIND_DEFINED
#define SIRKIT_ROLLBACK_WEIGHTED_UNION_FIND_DEFINED
template <typename T>
struct RollbackWeightedUnionFind {
    enum class MergeResult { Merged, AlreadyConsistent, Contradiction };

private:
    static constexpr bool checked_integer =
        std::is_integral_v<T> && std::is_signed_v<T>;
    // 標準の2語累積では、従来の照会本体を保つ。
    static constexpr bool direct_difference =
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        true;
#else
        !checked_integer;
#endif

    // 根までの高々O(log n)個の整数を、途中の相殺を保って加算する。
    struct SignedSum {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        __extension__ using WideInteger = __int128;
        WideInteger value = 0;
        SignedSum() = default;
        explicit SignedSum(T x) : value(x) {}
        void add(T x) { value += x; }
        void add_sum(const SignedSum& x) { value += x.value; }
        void subtract(const SignedSum& x) { value -= x.value; }
        void negate() { value = -value; }
        bool equals(T x) const { return value == x; }
        T result() const {
            if (value < std::numeric_limits<T>::min() ||
                value > std::numeric_limits<T>::max()) {
                throw std::overflow_error("weighted union find potential overflow");
            }
            return static_cast<T>(value);
        }
#else
        // 標準C++の符号と2桁の大きさ。頂点数がint以内なので2桁で足りる。
        std::uintmax_t low = 0;
        std::uintmax_t high = 0;
        bool negative = false;
        SignedSum() = default;
        explicit SignedSum(T x) {
            negative = x < 0;
            const auto u = static_cast<std::uintmax_t>(x);
            low = negative ? std::uintmax_t{0} - u : u;
        }
        void add_sum(const SignedSum& x) {
            if (negative == x.negative) {
                const auto old = low;
                low += x.low;
                high += x.high + (low < old);
            } else if (high > x.high || (high == x.high && low >= x.low)) {
                const auto old = low;
                low -= x.low;
                high -= x.high + (old < x.low);
            } else {
                const auto old = low;
                low = x.low - low;
                high = x.high - high - (x.low < old);
                negative = x.negative;
            }
            if (high == 0 && low == 0) negative = false;
        }
        void add(T x) { add_sum(SignedSum(x)); }
        void negate() { if (high != 0 || low != 0) negative = !negative; }
        void subtract(SignedSum x) { x.negate(); add_sum(x); }
        bool equals(T x) const {
            const SignedSum other(x);
            return high == 0 && low == other.low && negative == other.negative;
        }
        T result() const {
            const auto positive_limit = static_cast<std::uintmax_t>(std::numeric_limits<T>::max());
            const auto negative_limit = std::uintmax_t{0} -
                static_cast<std::uintmax_t>(std::numeric_limits<T>::min());
            if (high != 0 || low > (negative ? negative_limit : positive_limit)) {
                throw std::overflow_error("weighted union find potential overflow");
            }
            if (!negative) return static_cast<T>(low);
            if (low == negative_limit) return std::numeric_limits<T>::min();
            return static_cast<T>(-static_cast<T>(low));
        }
#endif
    };
    struct WideRootWeight { int root; SignedSum weight; };
    struct NarrowRootWeight { int root; T weight; };
    struct Change { int child; int previous_parent_size; };
    std::vector<int> parent_;
    std::vector<T> weight_;
    std::vector<Change> history_;
    int components_ = 0;
    int snapshot_ = 0;
    // falseはundo後も保持し、全走査なしで保守的な上限を維持する。
    bool bounded_weights_ = true;

    int find_unchecked(int v) const {
        while (parent_[v] >= 0) v = parent_[v];
        return v;
    }
    WideRootWeight wide_root_weight(int v) const requires (checked_integer) {
        SignedSum value;
        while (parent_[v] >= 0) {
            value.add(weight_[v]);
            v = parent_[v];
        }
        return {v, value};
    }
    using ArithmeticArgument = std::conditional_t<
        std::is_trivially_copy_constructible_v<T> ||
        !requires(const T& a, const T& b) {
            static_cast<T>(a + b); static_cast<T>(a - b);
        }, T, const T&>;
    static bool try_add(ArithmeticArgument a, ArithmeticArgument b, T& out) {
        if constexpr (checked_integer) {
#if !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
            return !__builtin_add_overflow(a, b, &out);
#else
            using U = std::make_unsigned_t<T>;
            const U ua = static_cast<U>(a), ub = static_cast<U>(b);
            const U sum = static_cast<U>(ua + ub);
            constexpr U sign = U{1} << std::numeric_limits<T>::digits;
            if (((ua ^ sum) & (ub ^ sum) & sign) != 0) return false;
            out = std::bit_cast<T>(sum);
            return true;
#endif
        } else {
            out = static_cast<T>(a + b);
            return true;
        }
    }
    static bool try_subtract(ArithmeticArgument a, ArithmeticArgument b, T& out) {
        if constexpr (checked_integer) {
#if !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
            return !__builtin_sub_overflow(a, b, &out);
#else
            using U = std::make_unsigned_t<T>;
            const U ua = static_cast<U>(a), ub = static_cast<U>(b);
            const U difference = static_cast<U>(ua - ub);
            constexpr U sign = U{1} << std::numeric_limits<T>::digits;
            if (((ua ^ ub) & (ua ^ difference) & sign) != 0) return false;
            out = std::bit_cast<T>(difference);
            return true;
#endif
        } else {
            out = static_cast<T>(a - b);
            return true;
        }
    }
    // 通常はTのまま返す。途中で範囲を超えた場合だけ広い演算で再走査する。
    NarrowRootWeight narrow_root_weight(int v) const {
        if (parent_[v] < 0) return {v, T(0)};
        T value = weight_[v];
        v = parent_[v];
        if constexpr (checked_integer) {
            // サイズ併合の深さ<=intの桁数。全辺の絶対値<=Tmax/(2*桁数)なら途中和と2経路の差も安全。
            if (bounded_weights_) {
                while (parent_[v] >= 0) {
                    value = static_cast<T>(value + weight_[v]);
                    v = parent_[v];
                }
                return {v, value};
            }
        }
        while (parent_[v] >= 0) {
            if (!try_add(value, weight_[v], value)) return {-1, T(0)};
            v = parent_[v];
        }
        return {v, std::move(value)};
    }
    static void check_history_capacity(std::size_t size, std::size_t storage_limit) {
        if (size >= static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
            size >= storage_limit) {
            throw std::length_error("weighted union find history capacity");
        }
    }
    MergeResult record_merge(int parent, int child, T gap) {
        if constexpr (checked_integer) {
            constexpr T limit = std::numeric_limits<T>::max() /
                (2 * std::numeric_limits<int>::digits);
            if (gap < -limit || gap > limit) bounded_weights_ = false;
        }
        history_.push_back({child, parent_[parent]});
        try {
            weight_[child] = std::move(gap);
        } catch (...) {
            history_.pop_back();
            throw;
        }
        parent_[parent] += parent_[child];
        parent_[child] = parent;
        --components_;
        return MergeResult::Merged;
    }
    template <bool CheckConsistency>
    [[gnu::noinline]]
    MergeResult unite_wide(int x, int y, const T& w) requires (checked_integer) {
        auto a = wide_root_weight(x);
        auto b = wide_root_weight(y);
        if (a.root == b.root) {
            MergeResult status = MergeResult::AlreadyConsistent;
            if constexpr (CheckConsistency) {
                b.weight.subtract(a.weight);
                if (!b.weight.equals(w)) status = MergeResult::Contradiction;
            }
            history_.push_back({-1, 0});
            return status;
        }
        SignedSum gap(w);
        gap.add_sum(a.weight);
        gap.subtract(b.weight);
        if (parent_[a.root] > parent_[b.root]) {
            std::swap(a.root, b.root);
            gap.negate();
        }
        return record_merge(a.root, b.root, gap.result());
    }
    template <bool CheckConsistency>
    [[gnu::always_inline]]
    MergeResult unite_impl(int x, int y, const T& w) {
        check_history_capacity(history_.size(), history_.max_size());
        if (parent_[y] == -1 && x != y) {
            const auto a = narrow_root_weight(x);
            if constexpr (checked_integer)
                if (a.root < 0) return unite_wide<CheckConsistency>(x, y, w);
            T gap(0);
            [[maybe_unused]] const bool fits = try_add(w, a.weight, gap);
            if constexpr (checked_integer) if (!fits) {
                SignedSum wide_gap(w);
                wide_gap.add(a.weight);
                gap = wide_gap.result();
            }
            return record_merge(a.root, y, std::move(gap));
        }
        auto a = narrow_root_weight(x);
        auto b = narrow_root_weight(y);
        if constexpr (checked_integer)
            if (a.root < 0 || b.root < 0) return unite_wide<CheckConsistency>(x, y, w);
        if (a.root == b.root) {
            MergeResult status = MergeResult::AlreadyConsistent;
            if constexpr (CheckConsistency) {
                T difference(0);
                if (!try_subtract(b.weight, a.weight, difference) || !(difference == w)) {
                    status = MergeResult::Contradiction;
                }
            }
            history_.push_back({-1, 0});
            return status;
        }
        T gap(0);
        [[maybe_unused]] bool fits;
        const bool reverse = parent_[a.root] > parent_[b.root];
        if (reverse) {
            fits = try_subtract(b.weight, a.weight, gap) && try_subtract(gap, w, gap);
            std::swap(a.root, b.root);
        } else {
            fits = try_add(w, a.weight, gap) && try_subtract(gap, b.weight, gap);
        }
        if constexpr (checked_integer) if (!fits) {
            SignedSum wide_gap(w);
            wide_gap.add(a.weight);
            wide_gap.subtract(SignedSum(b.weight));
            if (reverse) wide_gap.negate();
            gap = wide_gap.result();
        }
        return record_merge(a.root, b.root, std::move(gap));
    }
    void undo_unchecked() noexcept {
        const auto change = history_.back();
        if (change.child >= 0) {
            const int root = parent_[change.child];
            if constexpr (std::is_integral_v<T>) weight_[change.child] = T(0);
            parent_[change.child] = parent_[root] - change.previous_parent_size;
            parent_[root] = change.previous_parent_size;
            ++components_;
        }
        history_.pop_back();
    }
    [[gnu::noinline]]
    std::pair<int, T> root_with_weight_slow(int v) const {
        const auto narrow = narrow_root_weight(v);
        if constexpr (checked_integer) if (narrow.root < 0) {
            const auto found = wide_root_weight(v);
            return {found.root, found.weight.result()};
        }
        return {narrow.root, narrow.weight};
    }
    template<bool CheckConnected>
    std::optional<SignedSum> wide_difference(int x, int y) const requires (checked_integer) {
        const auto a = wide_root_weight(x);
        auto b = wide_root_weight(y);
        if constexpr (CheckConnected)
            if (a.root != b.root) return std::nullopt;
        b.weight.subtract(a.weight);
        return b.weight;
    }
    T diff_wide(int x, int y) const requires (checked_integer) {
        const auto diff = wide_difference<false>(x, y);
        return diff->result();
    }
    std::optional<T> optional_diff_wide(int x, int y) const requires (checked_integer) {
        const auto diff = wide_difference<true>(x, y);
        return diff ? std::optional<T>(diff->result()) : std::nullopt;
    }
    [[gnu::noinline]]
    T diff_slow(int x, int y) const {
        const auto narrow_a = narrow_root_weight(x);
        const auto narrow_b = narrow_root_weight(y);
        if constexpr (checked_integer)
            if (narrow_a.root < 0 || narrow_b.root < 0) return diff_wide(x, y);
        T difference(0);
        [[maybe_unused]] const bool fits = try_subtract(narrow_b.weight, narrow_a.weight, difference);
        if constexpr (checked_integer)
            if (!fits) return diff_wide(x, y);
        return difference;
    }

public:
    RollbackWeightedUnionFind() = default;
    explicit RollbackWeightedUnionFind(int n) {
        const auto count = static_cast<std::size_t>(n);
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
            count > parent_.max_size() || count > weight_.max_size()) {
            throw std::length_error("weighted union find vertex capacity");
        }
        parent_.assign(count, -1);
        weight_.assign(count, T(0));
        history_.reserve(std::min(count, std::size_t{64}));
        components_ = n;
    }
    RollbackWeightedUnionFind(const RollbackWeightedUnionFind&) = default;
    RollbackWeightedUnionFind(RollbackWeightedUnionFind&& other) noexcept { swap(other); }
    RollbackWeightedUnionFind& operator=(RollbackWeightedUnionFind other) noexcept {
        swap(other);
        return *this;
    }
    void swap(RollbackWeightedUnionFind& other) noexcept {
        parent_.swap(other.parent_);
        weight_.swap(other.weight_);
        history_.swap(other.history_);
        std::swap(components_, other.components_);
        std::swap(snapshot_, other.snapshot_);
        std::swap(bounded_weights_, other.bounded_weights_);
    }
    void reset(int n) {
        if constexpr (std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
            const auto count = static_cast<std::size_t>(n);
            if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
                count > parent_.max_size() || count > weight_.max_size())
                throw std::length_error("weighted union find vertex capacity");
            if (count > parent_.capacity() || count > weight_.capacity()) {
                RollbackWeightedUnionFind replacement(n); swap(replacement); return;
            }
            const T initial(0);
            parent_.reserve(count); weight_.reserve(count);
            history_.reserve(std::min(count, std::size_t{64}));
            history_.clear();
            const auto retained_parents = std::min(count, parent_.size());
            parent_.resize(count, -1);
            std::fill_n(parent_.begin(), retained_parents, -1);
            if constexpr (std::is_nothrow_copy_assignable_v<T>) {
                const auto retained_weights = std::min(count, weight_.size());
                weight_.resize(count, initial);
                std::fill_n(weight_.begin(), retained_weights, initial);
            } else { weight_.clear(); weight_.resize(count, initial); }
            components_ = n; snapshot_ = 0; bounded_weights_ = true;
        } else { RollbackWeightedUnionFind replacement(n); swap(replacement); }
    }
    void reset() { reset(size()); }
    friend void swap(RollbackWeightedUnionFind& a, RollbackWeightedUnionFind& b) noexcept { a.swap(b); }
    void clear() {
        parent_.clear(); weight_.clear(); history_.clear();
        components_ = 0; snapshot_ = 0;
        bounded_weights_ = true;
    }
    int size() const noexcept { return static_cast<int>(parent_.size()); }
    bool empty() const noexcept { return parent_.empty(); }
    int component_count() const noexcept { return components_; }
    int find(int v) const {
        return find_unchecked(v);
    }
    int size(int v) const { return -parent_[find(v)]; }
    bool same(int x, int y) const {
        const int px = parent_[x], py = parent_[y];
        const int a = px < 0 ? x : px, b = py < 0 ? y : py;
        return a == b || find_unchecked(a) == find_unchecked(b);
    }
    std::pair<int, T> root_with_weight(int v) const {
        const int p = parent_[v];
        if (p < 0) return {v, T(0)};
        if (parent_[p] < 0) return {p, weight_[v]};
        return root_with_weight_slow(v);
    }
    T get_weight(int v) const {
        const int p = parent_[v];
        if (p < 0) return T(0);
        if (parent_[p] < 0) return weight_[v];
        return root_with_weight_slow(v).second;
    }
    std::optional<T> try_diff(int x, int y) const {
        // 同じ親を持つ子どうしは、共通の経路を打ち消せる。
        if constexpr (direct_difference) {
            const int px = parent_[x];
            if (px >= 0 && px == parent_[y]) {
                T difference(0);
                if (try_subtract(weight_[y], weight_[x], difference)) return difference;
            }
        }
        const auto narrow_a = narrow_root_weight(x);
        const auto narrow_b = narrow_root_weight(y);
        if constexpr (checked_integer)
            if (narrow_a.root < 0 || narrow_b.root < 0) return optional_diff_wide(x, y);
        if (narrow_a.root != narrow_b.root) return std::nullopt;
        T difference(0);
        [[maybe_unused]] const bool fits = try_subtract(narrow_b.weight, narrow_a.weight, difference);
        if constexpr (checked_integer)
            if (!fits) return optional_diff_wide(x, y);
        return difference;
    }
    T diff(int x, int y) const {
        const int px = parent_[x], py = parent_[y];
        const int a = px < 0 ? x : px, b = py < 0 ? y : py;
        if (a == b) {
            T difference(0);
            if constexpr (std::is_integral_v<T>) {
                if (try_subtract(weight_[y], weight_[x], difference)) return difference;
            } else {
                const T wx = px < 0 ? T(0) : weight_[x];
                const T wy = py < 0 ? T(0) : weight_[y];
                try_subtract(wy, wx, difference);
                return difference;
            }
        }
        return diff_slow(x, y);
    }
    bool consistent(int x, int y, const T& w) const {
        const auto narrow_a = narrow_root_weight(x);
        const auto narrow_b = narrow_root_weight(y);
        if constexpr (checked_integer) if (narrow_a.root < 0 || narrow_b.root < 0) {
            const auto diff = wide_difference<true>(x, y);
            return !diff || diff->equals(w);
        }
        if (narrow_a.root != narrow_b.root) return true;
        T difference(0);
        return try_subtract(narrow_b.weight, narrow_a.weight, difference) && difference == w;
    }
    bool unite(int x, int y, T w) { return unite_impl<false>(x, y, w) == MergeResult::Merged; }
    MergeResult unite_checked(int x, int y, T w) { return unite_impl<true>(x, y, w); }
    void clear_history() noexcept { history_.clear(); snapshot_ = 0; }
    void reserve_history(std::size_t calls) {
        if (calls > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
            calls > history_.max_size()) {
            throw std::length_error("weighted union find history capacity");
        }
        if (calls <= history_.size()) return;
        history_.reserve(calls);
    }
    int get_state() const noexcept { return static_cast<int>(history_.size()); }
    void snapshot() noexcept { snapshot_ = get_state(); }
    void undo() {
        undo_unchecked();
        snapshot_ = std::min(snapshot_, get_state());
    }
    void rollback(int state = -1) {
        if (state == -1) state = snapshot_;
        while (get_state() > state) undo_unchecked();
        snapshot_ = std::min(snapshot_, get_state());
    }
    std::vector<std::vector<int>> groups() const {
        std::vector<std::vector<int>> result;
        result.reserve(components_);
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
            result.emplace_back(parent_.size());
            for (int v = 0; v < size(); ++v) result[0][v] = v;
            return result;
        }
#if (!defined(SIRKIT_DISABLE_SIMD)) && (!defined(__AVX2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if (parent_.size() >= 256 && __builtin_cpu_supports("avx2")) {
            auto roots_storage = std::make_unique_for_overwrite<int[]>(parent_.size());
            int* const roots = roots_storage.get();
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::GatherGroupRoots<false>(parent_.data(), roots, parent_.size());
            std::vector<int> slot(parent_.size());
            for (int v = 0; v < size(); ++v) if (parent_[v] < 0) {
                slot[v] = static_cast<int>(result.size());
                result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-parent_[v]));
            }
            for (int v = 0; v < size(); ++v) result[slot[roots[v]]].push_back(v);
            return result;
        }
#endif
        std::vector<int> roots(parent_.size()), counts(parent_.size(), 0);
        for (int v = 0; v < size(); ++v) ++counts[roots[v] = find_unchecked(v)];
        for (int v = 0; v < size(); ++v) if (counts[v]) {
            const int count = counts[v];
            counts[v] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(count);
        }
        for (int v = 0; v < size(); ++v) result[counts[roots[v]]].push_back(v);
        return result;
    }

};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
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

#include <cstdint>
#include <vector>

int main() {
    int n, q; verify_in >> n >> q;
    // N<=200000、typeは0/1なのでuの上位1bitへ格納できる。
    struct Query { std::uint32_t encoded_u = 0; int v = 0, head = -1, next = 0; };
    std::vector<Query> queries(q + 1);
    for (int i = 1; i <= q; ++i) {
        int k; std::uint32_t type, u; auto& query = queries[i];
        verify_in >> type >> k >> u >> query.v;
        query.encoded_u = u | (type << 31);
        query.next = queries[k + 1].head; queries[k + 1].head = i;
    }
    RollbackWeightedUnionFind<std::int64_t> uf(n);
    struct Frame { int vertex, state; bool exit; };
    std::vector<Frame> stack{{0, 0, false}};
    while (!stack.empty()) {
        const auto frame = stack.back(); stack.pop_back();
        if (frame.exit) {
            uf.rollback(frame.state);
            continue;
        }
        const int first = queries[frame.vertex].head;
        if (frame.vertex != 0) {
            auto& query = queries[frame.vertex];
            if ((query.encoded_u >> 31) == 0) {
                if (first == -1) continue;
                const int state = uf.get_state();
                uf.unite(static_cast<int>(query.encoded_u & 0x7fffffffu), query.v, std::int64_t{0});
                stack.push_back({frame.vertex, state, true});
            } else query.v = uf.same(static_cast<int>(query.encoded_u & 0x7fffffffu), query.v);
        }
        for (int child = first; child != -1; child = queries[child].next) stack.push_back({child, 0, false});
    }
    // type1の版は参照されないため、vは判定後に答えとして再利用する。
    for (int i = 1; i <= q; ++i)
        if (queries[i].encoded_u >> 31) verify_out << queries[i].v << '\n';
}
