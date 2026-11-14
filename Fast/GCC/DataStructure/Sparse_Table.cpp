// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sparse_Table.cpp
// 参考（数学的構成）: https://info.atcoder.jp/entry/algorithm_lectures/sparse_table
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_table.md
//
// API一覧
// [SparseTable<T, Op>]
// SparseTable(vector<T>={}, identity=T{}, op=Op{}): 静的列を構築。O(n log(n+1)+1)時間・領域。
// Op(a,b)はconst呼出可能かつ結合的かつべき等、identityは左右の単位元。可換性は不要。 演算/コピーをO(1)とする。
// size()/empty(): 要素数/空か。get(i): const T&（T=boolは値）。prod(l,r): 半開区間[l,r)の積。all_prod(): 全積。O(1)。
// prodの空区間・空列の全積はidentity。参照はassign/clear/交換/移動で無効になり得る。添字は0始まり。
// max_right(l,pred)/min_left(r,pred): predが真の最大右端/最小左端。O(log(n+1))。pred(identity)==trueと境界の単調性が前提。
// values(): 元の列をコピー、O(n+1)。assign(vector<T>): 再構築。O(旧n log(旧n+1)+n log(n+1)+1)。点更新は行わない。
// clear()/swap(other): 空へ戻す/内容交換。O(旧n log(旧n+1)+1)/O(1)。コピーは独立、移動元は空。演算等の例外時は内容の強い保証なし。
// SIMD: 32bit以下の整数（bool以外）の構築に条件付きAVX2経路。SIRKIT_DISABLE_SIMDで専用経路とCPU判定を除く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_TABLE_DEFINED
#define SIRKIT_SPARSE_TABLE_DEFINED
template<class T, class Op>
class SparseTable {
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<std::vector<T>> table_;
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
    [[gnu::target("avx2"), gnu::optimize("O3")]]
    // 入出力は別々のvectorが所有する領域への非所有ポインタ。
    static void fill_row_avx2(T* target, const T* left, const T* right, std::size_t count, Op& op) {
        for (std::size_t i = 0; i < count; ++i) target[i] = T(op(left[i], right[i]));
    }
    static bool supports_avx2() noexcept {
        static const bool available = [] {
            __builtin_cpu_init();
            return __builtin_cpu_supports("avx2") != 0;
        }();
        return available;
    }
#endif
    void check_range(std::size_t l, std::size_t r) const {
        (void)l; (void)r;
    }
    T range_unchecked(std::size_t l, std::size_t r) const {
        if (l == r) return identity_;
        const auto k = std::bit_width(r - l) - 1;
        const auto width = std::size_t{1} << k;
        return op_(table_[k][l], table_[k][r - width]);
    }
public:
    using const_reference = typename std::vector<T>::const_reference;
    explicit SparseTable(const std::vector<T>& values = {}, const T& identity = T{}, Op op = Op{})
        : op_(std::move(op)), identity_(identity) {
        const auto n = values.size();
        if (!n) return;
        table_.reserve(std::bit_width(n));
        table_.push_back(values);
        for (std::size_t width = 2; width <= n;) {
            const auto half = width / 2;
            const auto count = n - width + 1;
            std::vector<T> row;
            bool filled = false;
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
            using Result = decltype(op_(std::declval<const std::vector<T>&>()[std::size_t{}],
                                        std::declval<const std::vector<T>&>()[std::size_t{}]));
            if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 4 &&
                          std::is_same_v<std::remove_cvref_t<Result>, T>) {
                if (count >= 256 && supports_avx2()) {
                    row = std::vector<T>(count, identity_);
                    const auto& previous = table_.back();
                    fill_row_avx2(row.data(), previous.data(), previous.data() + half, count, op_);
                    filled = true;
                }
            }
#endif
            if (!filled) {
                row.reserve(count);
                const auto& previous = table_.back();
                for (std::size_t i = 0; i < count; ++i) row.emplace_back(op_(previous[i], previous[i + half]));
            }
            table_.push_back(std::move(row));
            if (width > n / 2) break;
            width *= 2;
        }
    }
    SparseTable(const SparseTable&) = default;
    SparseTable& operator=(const SparseTable& other) {
        if (this != &other) { SparseTable replacement(other); swap(replacement); }
        return *this;
    }
    SparseTable(SparseTable&& other)
        : op_(other.op_), identity_(other.identity_), table_(std::move(other.table_)) {
        other.table_.clear();
    }
    SparseTable& operator=(SparseTable&& other) {
        if (this != &other) { SparseTable replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return (table_.empty() ? 0 : table_.front().size()); }
    bool empty() const noexcept { return size() == 0; }
    const_reference get(std::size_t i) const {
        return table_.front()[i];
    }
    T prod(std::size_t l, std::size_t r) const { check_range(l, r); return range_unchecked(l, r); }
    T all_prod() const { return range_unchecked(0, size()); }
    template<class Pred> std::size_t max_right(std::size_t l, Pred pred) const {
        check_range(l, l);
        auto low = l, high = size();
        while (low < high) {
            const auto distance = high - low;
            const auto middle = low + distance / 2 + distance % 2;
            if (pred(range_unchecked(l, middle))) low = middle;
            else high = middle - 1;
        }
        return low;
    }
    template<class Pred> std::size_t min_left(std::size_t r, Pred pred) const {
        check_range(r, r);
        std::size_t low = 0, high = r;
        while (low < high) {
            const auto middle = low + (high - low) / 2;
            if (pred(range_unchecked(middle, r))) high = middle;
            else low = middle + 1;
        }
        return low;
    }
    std::vector<T> values() const { return (table_.empty() ? std::vector<T>{} : table_.front()); }
    void assign(const std::vector<T>& values) { SparseTable replacement(values, identity_, op_); swap(replacement); }
    void clear() noexcept { table_.clear(); }
    void swap(SparseTable& other) noexcept(std::is_nothrow_swappable_v<Op> && std::is_nothrow_swappable_v<T>) {
        using std::swap;
        swap(op_, other.op_); swap(identity_, other.identity_);
        table_.swap(other.table_);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
