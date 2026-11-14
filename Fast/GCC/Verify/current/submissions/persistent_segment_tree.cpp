#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/point_add_range_sum
// Submission: https://judge.yosupo.jp/submission/414658

// Library: DataStructure/Persistent_Segment_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Persistent_Segment_Tree.cpp
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/persistent_segment_tree
// 参考: https://nyaannyaan.github.io/library/segment-tree/persistent-segment-tree.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_segment_tree.md
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// 速度優先版: 状態本体をnewで確保し、破棄・reset・clear時も解放しない。
// ノード列はvectorの通常の成長・再確保を使う。格納型の破棄の副作用を必要としないこと。
// 入力・容量・件数の前提と確保成功を利用側が保証する。例外時の状態復元は保証しない。
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
// reset(n)/clear(): 全根を無効化して単位元列/長さ0へ戻す。O(1)、旧領域を解放しない。
// コピーは独立。remap_from(original,root): 直接コピー元のコピー時に存在した根を移す、O(1)。
// 元のRootはコピー先へ直接渡せない。latest_root()/empty_root()はコピー先の根を返す。
// moveは根を移動先へ引き継ぐ。move元には破棄・代入だけ行う。swapは根も交換する。
// 更新・構築の失敗時は追加ノードを取り消し、旧根とlatest_rootを維持する。
// SKIP_RUNTIME_CHECKSで根の所属・添字・長さ・述語の単位元検査と所属用状態を除く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
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



// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
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
        explicit Root(std::size_t index) : index_(index) {}
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
        State(std::size_t count, Op operation, T unit)
            : n(count), op(std::move(operation)), identity(std::move(unit)) {}
        State(const State& other)
            : n(other.n), latest(other.latest), op(other.op), identity(other.identity), nodes(other.nodes) {
        }
    };
    State* state_;

    Root handle(std::size_t index) const {
        return Root(index);
    }
    std::size_t checked_root(const Root& root) const {
        return root.index_;
    }
    void check_index(std::size_t index) const {
        static_cast<void>(index);
    }
    void check_range(std::size_t left, std::size_t right) const {
        static_cast<void>(left); static_cast<void>(right);
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
        : state_(new State(n, std::move(op), std::move(identity))) {}
    PersistentSegmentTree(std::span<const T> values, Op op, T identity)
        : PersistentSegmentTree(values.size(), std::move(op), std::move(identity)) { build(values); }
    PersistentSegmentTree(const std::vector<T>& values, Op op, T identity)
        : PersistentSegmentTree(std::span<const T>(values), std::move(op), std::move(identity)) {}
    PersistentSegmentTree(const PersistentSegmentTree& other)
        : state_(new State(*other.state_)) {}
    PersistentSegmentTree& operator=(const PersistentSegmentTree& other) {
        if (this != &other) { auto next = new State(*other.state_); std::swap(state_, next); }
        return *this;
    }
    PersistentSegmentTree(PersistentSegmentTree&& other) noexcept : state_(std::exchange(other.state_, nullptr)) {}
    PersistentSegmentTree& operator=(PersistentSegmentTree&& other) noexcept {
        if (this != &other) state_ = std::exchange(other.state_, nullptr);
        return *this;
    }
    void swap(PersistentSegmentTree& other) noexcept { std::swap(state_, other.state_); }
    friend void swap(PersistentSegmentTree& a, PersistentSegmentTree& b) noexcept { a.swap(b); }
    std::size_t size() const noexcept { return state_->n; }
    bool empty() const noexcept { return size() == 0; }
    std::size_t allocated_nodes() const noexcept { return state_->nodes.size(); }
    Root empty_root() const { return handle(0); }
    Root latest_root() const { return handle(state_->latest); }
    Root remap_from(const PersistentSegmentTree& original, const Root& root) const {
        static_cast<void>(original);
        return handle(root.index_);
    }
    void reserve(std::size_t nodes) { state_->nodes.reserve(nodes); }
    void reset(std::size_t n) {
        auto next = new State(n, state_->op, state_->identity);
        std::swap(state_, next);
    }
    void clear() { reset(0); }
    Root build(std::span<const T> values) {
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
        if (left == size()) return size();
        T aggregate = state_->identity;
        return right_impl(index, left, pred, aggregate, 0, size());
    }
    template<class Pred>
    std::size_t min_left(const Root& root, std::size_t right, Pred pred) const {
        const auto index = checked_root(root); check_range(0, right);
        if (right == 0) return 0;
        T aggregate = state_->identity;
        return left_impl(index, right, pred, aggregate, 0, size());
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
#include <functional>
#include <vector>
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0, q = 0; input >> n >> q;
    std::vector<std::int64_t> values(static_cast<std::size_t>(n));
    for (auto& value : values) input >> value;
    PersistentSegmentTree<std::int64_t, std::plus<std::int64_t>> tree(values, {}, 0);
    auto root = tree.latest_root();
    for (int query = 0; query < q; ++query) {
        int type = 0; input >> type;
        if (type == 0) {
            int position = 0; std::int64_t delta = 0; input >> position >> delta;
            root = tree.transform(root, static_cast<std::size_t>(position), [delta](auto x) { return x + delta; });
        } else {
            int left = 0, right = 0; input >> left >> right;
            output << tree.prod(root, static_cast<std::size_t>(left), static_cast<std::size_t>(right)) << '\n';
        }
    }
}
