// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sliding_Window_Aggregation.cpp
// 参考（構成・問題選定）: https://nyaannyaan.github.io/library/data-structure/slide-window-aggregation.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sliding_window_aggregation.md
//
// API一覧
// [SlidingWindowAggregation<T, Op>]
// SlidingWindowAggregation(identity, op=Op{}): 空のキュー。identityは左右の単位元。O(1)。
// Op(a,b)はconst呼出可能な結合演算。非可換でも可。T/Opはコピー可能、演算に外部副作用がないこと。
// push_back(value): 末尾にコピーを追加。pop_front(): 先頭を削除。演算/コピーをO(1)として償却O(1)、最悪O(n)。
// front()/back(): 先頭/末尾のconst T&。all_prod(): 先頭から順に合成した値、空ならidentity。O(1)。
// front/back/pop_frontは非空が前提。既定で空を検査しout_of_range。SKIP_RUNTIME_CHECKSで検査を除く。
// size()/empty(): 要素数/空か。O(1)。clear(): 全削除、O(n)。swap(other): 演算/単位元/列を交換、O(1)。
// 代入/swapではT/Opをswapで交換可能とする。コピーは独立、移動元は空。保持領域O(過去最大n+1)。
// front/backの参照は更新/clear/交換/移動で無効になり得る。
// push/pop中のコピー・演算・確保の例外では列を保つ。代入の強い保証はT/Opのswapがnoexceptの場合。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SLIDING_WINDOW_AGGREGATION_DEFINED
#define SIRKIT_SLIDING_WINDOW_AGGREGATION_DEFINED
template<class T, class Op>
class SlidingWindowAggregation {
    struct Entry { T value, product; };
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<Entry> incoming_, outgoing_;

    void check_nonempty() const {
    }
    void transfer() {
        // 構築が完了するまで元のキューを変更しない。
        std::vector<Entry> replacement;
        replacement.reserve(incoming_.size());
        for (auto i = incoming_.size(); i != 0; --i) {
            const T& value = incoming_[i - 1].value;
            T product = replacement.empty() ? value : std::as_const(op_)(value, replacement.back().product);
            replacement.push_back(Entry{value, std::move_if_noexcept(product)});
        }
        outgoing_.swap(replacement);
        incoming_.clear();
    }
public:
    explicit SlidingWindowAggregation(const T& identity, Op op = Op{})
        : op_(std::move_if_noexcept(op)), identity_(identity) {}
    SlidingWindowAggregation(const SlidingWindowAggregation&) = default;
    SlidingWindowAggregation& operator=(const SlidingWindowAggregation& other) {
        if (this != &other) { SlidingWindowAggregation replacement(other); swap(replacement); }
        return *this;
    }
    SlidingWindowAggregation(SlidingWindowAggregation&& other)
        : op_(other.op_), identity_(other.identity_), incoming_(std::move(other.incoming_)), outgoing_(std::move(other.outgoing_)) {
        other.incoming_.clear(); other.outgoing_.clear();
    }
    SlidingWindowAggregation& operator=(SlidingWindowAggregation&& other) {
        if (this != &other) { SlidingWindowAggregation replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return incoming_.size() + outgoing_.size(); }
    bool empty() const noexcept { return incoming_.empty() && outgoing_.empty(); }
    void push_back(const T& value) {
        if (size() == std::numeric_limits<std::size_t>::max())
            throw std::length_error("SlidingWindowAggregation: size");
        T product = incoming_.empty() ? value : std::as_const(op_)(incoming_.back().product, value);
        incoming_.push_back(Entry{value, std::move_if_noexcept(product)});
    }
    void pop_front() {
        check_nonempty();
        if (outgoing_.empty()) transfer();
        outgoing_.pop_back();
    }
    const T& front() const {
        check_nonempty();
        return outgoing_.empty() ? incoming_.front().value : outgoing_.back().value;
    }
    const T& back() const {
        check_nonempty();
        return incoming_.empty() ? outgoing_.front().value : incoming_.back().value;
    }
    T all_prod() const {
        if (outgoing_.empty()) return incoming_.empty() ? identity_ : incoming_.back().product;
        if (incoming_.empty()) return outgoing_.back().product;
        return op_(outgoing_.back().product, incoming_.back().product);
    }
    void clear() noexcept { incoming_.clear(); outgoing_.clear(); }
    void swap(SlidingWindowAggregation& other) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>) {
        using std::swap;
        swap(op_, other.op_); swap(identity_, other.identity_);
        incoming_.swap(other.incoming_); outgoing_.swap(other.outgoing_);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
