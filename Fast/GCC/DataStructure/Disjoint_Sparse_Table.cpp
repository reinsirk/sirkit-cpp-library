// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Disjoint_Sparse_Table.cpp
// 参考（数学的構成）: https://info.atcoder.jp/entry/algorithm_lectures/disjoint_sparse_table
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/disjoint_sparse_table.md
//
// API一覧
// [DisjointSparseTable<T, Op>]
// DisjointSparseTable(vector<T>={}, identity=T{}, op=Op{}): 静的列を構築。O(n log(n+1)+1)時間・領域。
// Op(a,b)はconst呼出可能かつ結合的、identityは左右の単位元。非可換でも可。 演算/コピーをO(1)とする。
// size()/empty(): 要素数/空か。get(i): const T&（T=boolは値）。prod(l,r): 半開区間[l,r)の積。all_prod(): 全積。O(1)。
// prodの空区間・空列の全積はidentity。参照はassign/clear/交換/移動で無効になり得る。添字は0始まり。
// max_right(l,pred)/min_left(r,pred): predが真の最大右端/最小左端。O(log(n+1))。pred(identity)==trueと境界の単調性が前提。
// values(): 元の列をコピー、O(n+1)。assign(vector<T>): 再構築。O(旧n log(旧n+1)+n log(n+1)+1)。点更新は行わない。
// clear()/swap(other): 空へ戻す/内容交換。O(旧n log(旧n+1)+1)/O(1)。コピーは独立、移動元は空。演算等の例外時は内容の強い保証なし。

#include <algorithm>
#include <bit>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_DISJOINT_SPARSE_TABLE_DEFINED
#define SIRKIT_DISJOINT_SPARSE_TABLE_DEFINED
template<class T, class Op>
class DisjointSparseTable {
    [[no_unique_address]] Op op_;
    T identity_;
    // 第0行は、原列と半幅1の行を兼ねる。
    std::vector<std::vector<T>> table_;
    std::size_t length_ = 0;
    static constexpr bool fill_identity_rows_ =
        std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) >= 4;
    void check_range(std::size_t l, std::size_t r) const {
        (void)l; (void)r;
    }
    T range_unchecked(std::size_t l, std::size_t r) const {
        if (l == r) return identity_;
        const auto last = r - 1;
        if (l == last) return table_.front()[l];
        const auto k = std::bit_width(l ^ last) - 1;
        return op_(table_[k][l], table_[k][last]);
    }
public:
    using const_reference = typename std::vector<T>::const_reference;
    explicit DisjointSparseTable(const std::vector<T>& values = {}, const T& identity = T{}, Op op = Op{})
        : op_(std::move(op)), identity_(identity), length_(values.size()) {
        const auto n = values.size();
        if (!n) return;
        table_.reserve(n <= 1 ? 1 : std::bit_width(n - 1));
        table_.push_back(values);
        if (n <= 2) return;
        for (std::size_t half = 2;;) {
            if constexpr (fill_identity_rows_) table_.emplace_back(n, identity_);
            else table_.push_back(values);
            auto& row = table_.back();
            for (std::size_t begin = 0; begin < n;) {
                const auto middle = begin + std::min(half, n - begin);
                const auto end = middle + std::min(half, n - middle);
                // 集約の起点だけを設定し、残りは左右の走査で埋める。
                if constexpr (fill_identity_rows_) row[middle - 1] = values[middle - 1];
                for (auto i = middle; i > begin + 1; --i) row[i - 2] = op_(values[i - 2], row[i - 1]);
                if (middle < end) {
                    if constexpr (fill_identity_rows_) row[middle] = values[middle];
                    for (auto i = middle + 1; i < end; ++i) row[i] = op_(row[i - 1], values[i]);
                }
                begin = end;
            }
            if (half > (n - 1) / 2) break;
            half *= 2;
        }
    }
    DisjointSparseTable(const DisjointSparseTable&) = default;
    DisjointSparseTable& operator=(const DisjointSparseTable& other) {
        if (this != &other) { DisjointSparseTable replacement(other); swap(replacement); }
        return *this;
    }
    DisjointSparseTable(DisjointSparseTable&& other)
        : op_(other.op_), identity_(other.identity_), table_(std::move(other.table_)), length_(other.length_) {
        other.table_.clear(); other.length_ = 0;
    }
    DisjointSparseTable& operator=(DisjointSparseTable&& other) {
        if (this != &other) { DisjointSparseTable replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return length_; }
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
    std::vector<T> values() const { return table_.empty() ? std::vector<T>{} : table_.front(); }
    void assign(const std::vector<T>& values) { DisjointSparseTable replacement(values, identity_, op_); swap(replacement); }
    void clear() noexcept { table_.clear(); length_ = 0; }
    void swap(DisjointSparseTable& other) noexcept(std::is_nothrow_swappable_v<Op> && std::is_nothrow_swappable_v<T>) {
        using std::swap;
        swap(op_, other.op_); swap(identity_, other.identity_);
        table_.swap(other.table_); swap(length_, other.length_);
    }
};
#endif
