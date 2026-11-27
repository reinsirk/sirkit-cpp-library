#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/point_set_range_sort_range_composite

// Library: DataStructure/Range_Sort_Aggregation.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Sort_Aggregation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_sort_aggregation.md
//
// API一覧
// RangeSortAggregation<S,op,e>(items): {int32_tキー,S値}の列。O(n+1)時間・領域。
// sort(l,r,descending=false): [l,r)をキーで昇順／降順に整列。
// set(i,key,value): 位置iのキーと値を置換。get(i): そのキーと値。各O(W+log(n+1))。
// prod(l,r): 現在の順序で非可換の区間積を返す。空区間はe()。O(W+log(n+1))。
// size()/all_prod(): 要素数・列全体の積。O(1)。添字は0始まり。
// opは結合的、e()は単位元。opとSのコピー・ムーブ・代入は非送出。
// 重複キーは要素固有IDで整列し、方向に合わせてIDも昇順／降順。IDはsetでも保持。
// 構築とq操作の全体時間はO(1+(n+q)(W+log(n+1)))、W=64。追加領域はO(n+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_SORT_AGGREGATION_DEFINED
#define SIRKIT_RANGE_SORT_AGGREGATION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedRangeSortAggregationBase(std::size_t n, std::size_t limit)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("RangeSortAggregation: too many items");
    }
    const auto base = std::bit_ceil(std::max(n, std::size_t{1}));
    if (base > limit)
    {
        throw std::length_error("RangeSortAggregation: too many segment tree leaves");
    }
    return base;
}
}

template <class S, auto op, auto e>
class RangeSortAggregation
{
    static_assert(std::is_nothrow_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S> &&
                  std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>);

public:
    struct Item
    {
        std::int32_t key;
        S value;
    };

    explicit RangeSortAggregation(std::span<const Item> items = {}) : identity_(std::invoke(e))
    {
        base_ = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedRangeSortAggregationBase(items.size(), tree_.max_size() / 2);
        n_ = static_cast<int>(items.size());
        nodes_.reserve(2 * items.size() + 1);
        nodes_.push_back({0, 0, 0, 0, 0, identity_, identity_});
        recycled_.reserve(items.size());
        tree_.assign(2 * base_, identity_);
        for (int i = 0; i < n_; ++i)
        {
            const auto &item = items[i];
            const auto node = static_cast<Index>(nodes_.size());
            nodes_.push_back({Pack(item.key, i), 0, 0, 1, 0, item.value, item.value});
            runs_.emplace_hint(runs_.end(), i, Run{node, false});
            tree_[base_ + i] = item.value;
        }
        for (auto i = base_; --i != 0;)
        {
            tree_[i] = Combine(tree_[2 * i], tree_[2 * i + 1]);
        }
    }

    RangeSortAggregation(const RangeSortAggregation &other)
        : identity_(other.identity_), nodes_(other.nodes_), recycled_(other.recycled_), tree_(other.tree_),
          runs_(other.runs_), n_(other.n_), base_(other.base_)
    {
        nodes_.reserve(2 * static_cast<std::size_t>(n_) + 1);
        recycled_.reserve(n_);
    }

    RangeSortAggregation &operator=(const RangeSortAggregation &other)
    {
        if (this != &other)
        {
            RangeSortAggregation next(other);
            Swap(next);
        }
        return *this;
    }

    RangeSortAggregation(RangeSortAggregation &&other) noexcept
        : identity_(other.identity_), nodes_(std::move(other.nodes_)), recycled_(std::move(other.recycled_)),
          tree_(std::move(other.tree_)), runs_(std::move(other.runs_)), n_(std::exchange(other.n_, 0)),
          base_(std::exchange(other.base_, 1))
    {
    }

    RangeSortAggregation &operator=(RangeSortAggregation &&other) noexcept
    {
        if (this != &other)
        {
            RangeSortAggregation next(std::move(other));
            Swap(next);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] S all_prod() const noexcept
    {
        return n_ == 0 ? identity_ : tree_[1];
    }

    void sort(int left, int right, bool descending = false)
    {
        CheckRange(left, right);
        if (right - left <= 1)
        {
            return;
        }
        const auto first = SplitAt(left);
        const auto last = SplitAt(right);
        auto root = first->second.root;
        for (auto it = std::next(first); it != last;)
        {
            root = Join(root, it->second.root);
            UpdateAggregate(it->first, identity_);
            it = runs_.erase(it);
        }
        first->second = {root, descending};
        UpdateAggregate(left, Whole(root, descending));
    }

    void set(int index, std::int32_t key, S value)
    {
        CheckIndex(index);
        const auto position = SplitAt(index);
        SplitAt(index + 1);
        auto &leaf = nodes_[position->second.root];
        leaf.key = Pack(key, static_cast<Index>(leaf.key));
        leaf.forward = leaf.backward = value;
        UpdateAggregate(index, value);
    }

    [[nodiscard]] Item get(int index) const
    {
        CheckIndex(index);
        const auto run = std::prev(runs_.upper_bound(index));
        auto node = run->second.root;
        int rank = index - run->first;
        if (run->second.descending)
        {
            rank = nodes_[node].count - rank - 1;
        }
        while (nodes_[node].height != 0)
        {
            const auto &parent = nodes_[node];
            const int left_size = nodes_[parent.left].count;
            if (rank < left_size)
            {
                node = parent.left;
            }
            else
            {
                rank -= left_size;
                node = parent.right;
            }
        }
        const auto &leaf = nodes_[node];
        return {Unpack(leaf.key), leaf.forward};
    }

    [[nodiscard]] S prod(int left, int right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return identity_;
        }
        const auto first = std::prev(runs_.upper_bound(left));
        const auto last = std::prev(runs_.upper_bound(right - 1));
        if (first == last)
        {
            return Fold(first->second.root, left - first->first, right - first->first, first->second.descending);
        }
        const auto start = Fold(first->second.root, left - first->first, nodes_[first->second.root].count,
                                first->second.descending);
        const auto middle = Aggregate(std::next(first)->first, last->first);
        const auto end = Fold(last->second.root, 0, right - last->first, last->second.descending);
        return Combine(Combine(start, middle), end);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        std::uint64_t key;
        Index left, right;
        int count, height;
        S forward, backward;
    };

    struct Run
    {
        Index root;
        bool descending;
    };

    S identity_;
    std::vector<Node> nodes_;
    std::vector<Index> recycled_;
    std::vector<S> tree_;
    std::map<int, Run> runs_;
    int n_ = 0;
    std::size_t base_ = 1;

    void Swap(RangeSortAggregation &other) noexcept
    {
        using std::swap;
        swap(identity_, other.identity_);
        nodes_.swap(other.nodes_);
        recycled_.swap(other.recycled_);
        tree_.swap(other.tree_);
        runs_.swap(other.runs_);
        swap(n_, other.n_);
        swap(base_, other.base_);
    }

    void CheckIndex(int index) const
    {
        (void)index;
    }

    void CheckRange(int left, int right) const
    {
        (void)left;
        (void)right;
    }

    static std::uint64_t Pack(std::int32_t key, Index id) noexcept
    {
        return (std::uint64_t{static_cast<Index>(key) ^ 0x80000000U} << 32) | id;
    }

    static std::int32_t Unpack(std::uint64_t key) noexcept
    {
        return std::bit_cast<std::int32_t>(static_cast<Index>(key >> 32) ^ 0x80000000U);
    }

    static S Combine(const S &left, const S &right) noexcept
    {
        return std::invoke(op, left, right);
    }

    [[nodiscard]] const S &Whole(Index node, bool descending) const noexcept
    {
        return descending ? nodes_[node].backward : nodes_[node].forward;
    }

    void Pull(Index index) noexcept
    {
        auto &node = nodes_[index];
        const auto &left = nodes_[node.left];
        const auto &right = nodes_[node.right];
        node.key = left.key;
        node.count = left.count + right.count;
        node.forward = Combine(left.forward, right.forward);
        node.backward = Combine(right.backward, left.backward);
    }

    Index Branch(Index left, Index right, int height) noexcept
    {
        const Node new_node{0, left, right, 0, height, identity_, identity_};
        Index result = 0;
        if (recycled_.empty())
        {
            result = static_cast<Index>(nodes_.size());
            nodes_.push_back(new_node);
        }
        else
        {
            result = recycled_.back();
            recycled_.pop_back();
            nodes_[result] = new_node;
        }
        Pull(result);
        return result;
    }

    // いずれも非空で、要素IDにより共通キーを持たないトライを結合する。
    Index Join(Index a, Index b) noexcept
    {
        const int separating_bit = std::bit_width(nodes_[a].key ^ nodes_[b].key);
        if (separating_bit > std::max(nodes_[a].height, nodes_[b].height))
        {
            if (nodes_[a].key > nodes_[b].key)
            {
                std::swap(a, b);
            }
            return Branch(a, b, separating_bit);
        }

        if (nodes_[a].height < nodes_[b].height)
        {
            std::swap(a, b);
        }
        auto &head = nodes_[a];
        if (head.height == nodes_[b].height)
        {
            const auto left = nodes_[b].left;
            const auto right = nodes_[b].right;
            recycled_.push_back(b);
            head.left = Join(head.left, left);
            head.right = Join(head.right, right);
        }
        else if ((nodes_[b].key >> (head.height - 1)) & 1)
        {
            head.right = Join(head.right, b);
        }
        else
        {
            head.left = Join(head.left, b);
        }
        Pull(a);
        return a;
    }

    // 0<rank<count。境界の枝を1個解放し、残りを再利用する。
    std::pair<Index, Index> Split(Index root, int rank) noexcept
    {
        auto &node = nodes_[root];
        const int left_size = nodes_[node.left].count;
        if (rank == left_size)
        {
            recycled_.push_back(root);
            return {node.left, node.right};
        }
        if (rank < left_size)
        {
            const auto [left, right] = Split(node.left, rank);
            node.left = right;
            Pull(root);
            return {left, root};
        }

        const auto [left, right] = Split(node.right, rank - left_size);
        node.right = left;
        Pull(root);
        return {root, right};
    }

    typename std::map<int, Run>::iterator SplitAt(int index)
    {
        if (index == n_)
        {
            return runs_.end();
        }
        const auto before = std::prev(runs_.upper_bound(index));
        if (before->first == index)
        {
            return before;
        }

        auto &run = before->second;
        // mapの確保後にトライを分割する。確保失敗なら元の区間を維持する。
        const auto after = runs_.emplace_hint(std::next(before), index, Run{0, run.descending});
        const int prefix = index - before->first;
        const int rank = run.descending ? nodes_[run.root].count - prefix : prefix;
        auto [left, right] = Split(run.root, rank);
        if (run.descending)
        {
            std::swap(left, right);
        }
        run.root = left;
        after->second.root = right;
        UpdateAggregate(before->first, Whole(left, run.descending));
        UpdateAggregate(index, Whole(right, run.descending));
        return after;
    }

    S Fold(Index index, int left, int right, bool descending) const noexcept
    {
        const auto &node = nodes_[index];
        if (left == 0 && right == node.count)
        {
            return Whole(index, descending);
        }
        const auto first = descending ? node.right : node.left;
        const auto last = descending ? node.left : node.right;
        const int middle = nodes_[first].count;
        if (right <= middle)
        {
            return Fold(first, left, right, descending);
        }
        if (left >= middle)
        {
            return Fold(last, left - middle, right - middle, descending);
        }
        return Combine(Fold(first, left, middle, descending), Fold(last, 0, right - middle, descending));
    }

    void UpdateAggregate(int index, const S &value) noexcept
    {
        auto position = base_ + static_cast<std::size_t>(index);
        tree_[position] = value;
        for (position >>= 1; position != 0; position >>= 1)
        {
            tree_[position] = Combine(tree_[2 * position], tree_[2 * position + 1]);
        }
    }

    S Aggregate(int left, int right) const noexcept
    {
        S prefix = identity_, suffix = identity_;
        auto first = base_ + static_cast<std::size_t>(left);
        auto last = base_ + static_cast<std::size_t>(right);
        for (; first < last; first >>= 1, last >>= 1)
        {
            if (first & 1)
            {
                prefix = Combine(prefix, tree_[first++]);
            }
            if (last & 1)
            {
                suffix = Combine(tree_[--last], suffix);
            }
        }
        return Combine(prefix, suffix);
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

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;
};

constexpr Affine Compose(Affine left, Affine right) noexcept
{
    return {
        static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
        static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
    };
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}
}

int main()
{
    using Sortable = RangeSortAggregation<Affine, Compose, Identity>;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Sortable::Item> items(n);
    for (auto &item : items)
    {
        verify_in >> item.key >> item.value.a >> item.value.b;
    }
    Sortable data(items);
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            int index = 0;
            std::int32_t key = 0;
            Affine value{};
            verify_in >> index >> key >> value.a >> value.b;
            data.set(index, key, value);
        }
        else
        {
            int left = 0, right = 0;
            verify_in >> left >> right;
            if (type == 1)
            {
                std::uint32_t x = 0;
                verify_in >> x;
                const auto fold = data.prod(left, right);
                verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
            }
            else
            {
                data.sort(left, right, type == 3);
            }
        }
    }
}
