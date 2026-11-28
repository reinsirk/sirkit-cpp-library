#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/3671

// Library: DataStructure/Rollback_Lazy_Segment_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Rollback_Lazy_Segment_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rollback_lazy_segment_tree.md
//
// API一覧
// RollbackLazySegmentTree<S,F,op,e,mapping,composition,can_skip>(n/values): n個のe()または初期配列。O(n)時間・領域。
// snapshot(): 現在の変更履歴の長さを返す。O(1)。rollback(s): 現在の履歴上のsまで取り消す。O(取り消すノード数)。
// apply(l,r,f): [l,r)に作用をかける。set(k,x): 1点を代入。O(log(n+1))時間・追加履歴。
// prod(l,r): [l,r)の集約。get(k): 1点取得。O(log(n+1))時間。all_prod()/size(): 全体集約/要素数、O(1)。
// 照会はconstで変更履歴を増やさない。clear_history(): 現在の値を保って履歴を破棄する。
// mapping(f,s,length): 区間の集約sを作用fで変換。composition(f,g): gの後にfを適用。
// can_skip(f,s,length)は省略可。trueなら区間内の各要素への作用が恒等。添字は0始まりの半開区間。
// S,Fはコピー構築可能でムーブ構築・ムーブ代入が非送出。破棄した履歴からのsnapshotは使用不可。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROLLBACK_LAZY_SEGMENT_TREE_DEFINED
#define SIRKIT_ROLLBACK_LAZY_SEGMENT_TREE_DEFINED

template <class S, class F, auto op, auto identity, auto mapping, auto composition, auto can_skip = nullptr>
class RollbackLazySegmentTree
{
    struct Node
    {
        S value;
        std::optional<F> lazy;
    };
    static_assert(std::is_nothrow_move_constructible_v<Node>);
    static_assert(std::is_nothrow_move_assignable_v<Node>);

    struct Change
    {
        std::size_t index;
        Node previous;
    };

public:
    using Snapshot = std::size_t;

    explicit RollbackLazySegmentTree(std::size_t n = 0)
        : n_(n), nodes_(Allocate(n))
    {
    }

    explicit RollbackLazySegmentTree(std::span<const S> values)
        : RollbackLazySegmentTree(values.size())
    {
        if (!values.empty())
        {
            Build(0, 0, n_, values);
        }
    }

    RollbackLazySegmentTree(const RollbackLazySegmentTree &) = default;
    RollbackLazySegmentTree &operator=(const RollbackLazySegmentTree &other)
    {
        RollbackLazySegmentTree copy(other);
        Swap(copy);
        return *this;
    }

    RollbackLazySegmentTree(RollbackLazySegmentTree &&other) noexcept
        : n_(std::exchange(other.n_, 0)), nodes_(std::move(other.nodes_)), history_(std::move(other.history_))
    {
        other.history_.clear();
    }

    RollbackLazySegmentTree &operator=(RollbackLazySegmentTree &&other) noexcept
    {
        RollbackLazySegmentTree moved(std::move(other));
        Swap(moved);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] Snapshot snapshot() const noexcept
    {
        return history_.size();
    }

    void rollback(Snapshot mark)
    {
        if (mark > history_.size())
        {
            throw std::out_of_range("RollbackLazySegmentTree: snapshot");
        }
        Undo(mark);
    }

    void clear_history() noexcept
    {
        history_.clear();
    }

    void apply(std::size_t left, std::size_t right, const F &action)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }
        const auto mark = snapshot();
        try
        {
            Apply(0, 0, n_, left, right, action, std::nullopt);
        }
        catch (...)
        {
            Undo(mark);
            throw;
        }
    }

    void set(std::size_t index, const S &value)
    {
        CheckIndex(index);
        const auto mark = snapshot();
        try
        {
            Set(0, 0, n_, index, value, std::nullopt);
        }
        catch (...)
        {
            Undo(mark);
            throw;
        }
    }

    [[nodiscard]] S all_prod() const
    {
        return n_ == 0 ? std::invoke(identity) : nodes_[0].value;
    }

    [[nodiscard]] S prod(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? std::invoke(identity) : Product(0, 0, n_, left, right, std::nullopt);
    }

    [[nodiscard]] S get(std::size_t index) const
    {
        CheckIndex(index);
        return Product(0, 0, n_, index, index + 1, std::nullopt);
    }

private:
    std::size_t n_;
    std::vector<Node> nodes_;
    std::vector<Change> history_;

    static std::vector<Node> Allocate(std::size_t n)
    {
        std::vector<Node> nodes;
        if (n > nodes.max_size() / 2)
        {
            throw std::length_error("RollbackLazySegmentTree: too many elements");
        }
        return n == 0 ? std::vector<Node>{} : std::vector<Node>(2 * n - 1, Node{std::invoke(identity), std::nullopt});
    }

    void Swap(RollbackLazySegmentTree &other) noexcept
    {
        std::swap(n_, other.n_);
        nodes_.swap(other.nodes_);
        history_.swap(other.history_);
    }

    void CheckIndex([[maybe_unused]] std::size_t index) const
    {
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
    }

    void Build(std::size_t node, std::size_t begin, std::size_t end, std::span<const S> values)
    {
        if (end - begin == 1)
        {
            nodes_[node].value = S(values[begin]);
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        Build(node + 1, begin, mid, values);
        Build(b, mid, end, values);
        nodes_[node].value = std::invoke(op, nodes_[node + 1].value, nodes_[b].value);
    }

    void Save(std::size_t index, Node after)
    {
        history_.push_back({index, nodes_[index]});
        nodes_[index] = std::move(after);
    }

    void Undo(Snapshot mark) noexcept
    {
        while (history_.size() > mark)
        {
            auto &last = history_.back();
            nodes_[last.index] = std::move(last.previous);
            history_.pop_back();
        }
    }

    static bool CanSkip(const F &action, const S &value, std::size_t length)
    {
        if constexpr (std::is_same_v<decltype(can_skip), std::nullptr_t>)
        {
            return false;
        }
        else
        {
            return std::invoke(can_skip, action, value, length);
        }
    }

    void ApplyWhole(std::size_t node, std::size_t length, const F &f)
    {
        const auto &previous = nodes_[node];
        auto result = std::invoke(mapping, f, previous.value, length);
        std::optional<F> tag;
        if (length > 1)
        {
            tag.emplace(previous.lazy ? std::invoke(composition, f, *previous.lazy) : f);
        }
        Save(node, Node{std::move(result), std::move(tag)});
    }

    static std::optional<F> Pending(const std::optional<F> &newer, const std::optional<F> &older)
    {
        if (newer && older)
        {
            return std::invoke(composition, *newer, *older);
        }
        return newer ? newer : older;
    }

    void KeepChild(std::size_t node, std::size_t length, const std::optional<F> &tag)
    {
        if (tag && !CanSkip(*tag, nodes_[node].value, length))
        {
            ApplyWhole(node, length, *tag);
        }
    }

    void Pull(std::size_t node, std::size_t right_child)
    {
        auto value = std::invoke(op, nodes_[node + 1].value, nodes_[right_child].value);
        Save(node, Node{std::move(value), std::nullopt});
    }

    // 遅延作用を引き継いだ子に対し、途中の状態を履歴へ追加せず一度だけ書く。
    void Apply(std::size_t node, std::size_t begin, std::size_t end,
               std::size_t left, std::size_t right, const F &f, const std::optional<F> &pending)
    {
        const auto length = end - begin;
        if (left <= begin && end <= right)
        {
            const auto action = pending ? std::invoke(composition, f, *pending) : f;
            if (!CanSkip(action, nodes_[node].value, length))
            {
                ApplyWhole(node, length, action);
            }
            return;
        }
        if (!pending && CanSkip(f, nodes_[node].value, length))
        {
            return;
        }
        const auto mid = begin + length / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, nodes_[node].lazy);
        if (left < mid)
        {
            Apply(node + 1, begin, mid, left, right, f, child_tag);
        }
        else
        {
            KeepChild(node + 1, mid - begin, child_tag);
        }
        if (right > mid)
        {
            Apply(b, mid, end, left, right, f, child_tag);
        }
        else
        {
            KeepChild(b, end - mid, child_tag);
        }
        Pull(node, b);
    }

    void Set(std::size_t node, std::size_t begin, std::size_t end, std::size_t index,
             const S &value, const std::optional<F> &pending)
    {
        if (end - begin == 1)
        {
            Save(node, Node{value, std::nullopt});
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, nodes_[node].lazy);
        if (index < mid)
        {
            Set(node + 1, begin, mid, index, value, child_tag);
            KeepChild(b, end - mid, child_tag);
        }
        else
        {
            KeepChild(node + 1, mid - begin, child_tag);
            Set(b, mid, end, index, value, child_tag);
        }
        Pull(node, b);
    }

    [[nodiscard]] S Product(std::size_t node, std::size_t begin, std::size_t end,
                            std::size_t left, std::size_t right, const std::optional<F> &pending) const
    {
        const auto &current = nodes_[node];
        if (left == begin && right == end)
        {
            return pending ? std::invoke(mapping, *pending, current.value, end - begin) : current.value;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, current.lazy);
        if (right <= mid)
        {
            return Product(node + 1, begin, mid, left, right, child_tag);
        }
        if (left >= mid)
        {
            return Product(b, mid, end, left, right, child_tag);
        }
        return std::invoke(op, Product(node + 1, begin, mid, left, mid, child_tag),
                           Product(b, mid, end, mid, right, child_tag));
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


#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
constexpr unsigned kBits = 30;
constexpr std::uint32_t kMask = (std::uint32_t{1} << kBits) - 1;
struct Summary
{
    std::array<std::uint32_t, kBits> counts{};
    std::uint64_t sum = 0;
    std::uint32_t some = 0;
    std::uint32_t every = kMask;
};
struct BitUpdate
{
    std::uint32_t keep;
    std::uint32_t put;
};

Summary Identity()
{
    return {};
}

Summary Combine(const Summary &a, const Summary &b)
{
    Summary result;
    for (unsigned i = 0; i < kBits; ++i)
    {
        result.counts[i] = a.counts[i] + b.counts[i];
    }
    result.sum = a.sum + b.sum;
    result.some = a.some | b.some;
    result.every = a.every & b.every;
    return result;
}

Summary Map(BitUpdate f, Summary value, std::size_t length)
{
    // In f.put bits are forced to one; other cleared bits are forced to zero.
    std::uint32_t changed = (value.some & ~(f.keep | f.put)) | (f.put & ~value.every);
    while (changed != 0)
    {
        const unsigned bit = std::countr_zero(changed);
        const auto bit_mask = std::uint32_t{1} << bit;
        changed &= changed - 1;
        const auto count = (f.put & bit_mask) != 0 ? static_cast<std::uint32_t>(length) : 0;
        value.sum -= std::uint64_t(value.counts[bit]) << bit;
        value.sum += std::uint64_t(count) << bit;
        value.counts[bit] = count;
    }
    value.some = (value.some & f.keep) | f.put;
    value.every = (value.every & f.keep) | f.put;
    return value;
}

BitUpdate Compose(BitUpdate after, BitUpdate before)
{
    return {after.keep & before.keep, (before.put & after.keep) | after.put};
}

bool NoChange(BitUpdate f, const Summary &value, std::size_t)
{
    return (value.some & ~(f.keep | f.put)) == 0 && (f.put & ~value.every) == 0;
}

Summary Single(std::uint32_t value)
{
    Summary result;
    result.sum = result.some = result.every = value;
    for (unsigned i = 0; i < kBits; ++i)
    {
        result.counts[i] = (value >> i) & 1u;
    }
    return result;
}
}

int main()
{
    std::uint32_t n = 0, m = 0;
    verify_in >> n >> m;
    std::vector<Summary> values(n);
    for (auto &entry : values)
    {
        std::uint32_t value = 0;
        verify_in >> value;
        entry = Single(value);
    }
    std::array<std::vector<std::uint32_t>, 5> parameters;
    for (auto &row : parameters)
    {
        row.resize(m);
        for (auto &x : row)
        {
            verify_in >> x;
        }
    }
    sirkit::RollbackLazySegmentTree<Summary, BitUpdate, Combine, Identity, Map, Compose, NoChange> tree(values);
    const auto initial = tree.snapshot();
    std::uint32_t q = 0;
    verify_in >> q;
    for (std::uint32_t group = 1; group <= q; ++group)
    {
        std::uint32_t start = 0, updates = 0;
        verify_in >> start >> updates;
        std::uint32_t y = group;
        for (std::uint32_t j = 1; j <= updates; ++j)
        {
            const auto z = (start + j) % m;
            const auto index = [&](int row)
            {
                return std::clamp(parameters[row][z] ^ y, std::uint32_t{1}, n);
            };
            const auto a = index(0), b = index(1);
            const auto left = index(3), right = index(4);
            const auto mask = parameters[2][z] ^ y;
            const auto f = (z % 2) != 0 ? BitUpdate{kMask ^ mask, mask} : BitUpdate{mask, 0};
            tree.apply(std::min(a, b) - 1, std::max(a, b), f);
            y = static_cast<std::uint32_t>(tree.prod(std::min(left, right) - 1, std::max(left, right)).sum) & kMask;
        }
        verify_out << y << '\n';
        tree.rollback(initial);
    }
}
