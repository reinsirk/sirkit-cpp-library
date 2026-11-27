#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_set_range_composite

// Library: DataStructure/Range_Assign_Range_Product.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Range_Assign_Range_Product.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_assign_range_product.md
//
// API一覧
// RangeAssignRangeProduct<S,op,e>(n=0): 長さnを単位元で初期化。O(1)。
// RangeAssignRangeProduct(n,initial): 一様な初期値でO(1+log(n+1))時間・O(1)領域。
// RangeAssignRangeProduct(values): 初期列からO(n+1)時間・領域。
// assign(l,r,value): 0始まり半開区間[l,r)に同じ値を代入。償却O(1+log(n+1))。
// prod(l,r): 現在の区間積を左から右へ計算。空区間は単位元。O(1+log(n+1))。
// get(p): 位置pの値。O(1+log(n+1))。
// all_prod()/size()/stored_ranges(): 全体の積・長さ・保存区間数。各O(1)。
// 領域は、同時に保存した最大区間数Pに対してO(P+1)。添字・長さはuint64_t。
// 値の等値比較は不要。opとSのコピー・ムーブの構築・代入はnoexcept。演算の途中の値もSで表現できること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ASSIGN_RANGE_PRODUCT_DEFINED
#define SIRKIT_RANGE_ASSIGN_RANGE_PRODUCT_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t NextRangeAssignProductCapacity(std::size_t used, std::size_t capacity,
                                                  std::size_t extra, std::size_t limit)
{
    if (extra > limit - used)
    {
        throw std::length_error("RangeAssignRangeProduct: too many ranges");
    }
    if (used + extra <= capacity)
    {
        return capacity;
    }
    return std::max(used + extra, capacity + std::min(capacity, limit - capacity));
}
}

template <class S, auto op, auto e>
class RangeAssignRangeProduct
{
    static_assert(std::is_nothrow_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S> &&
                  std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>);

public:
    explicit RangeAssignRangeProduct(std::uint64_t n = 0)
        : length_(n), identity_(std::invoke(e))
    {
        BuildUniform(identity_, identity_);
    }

    RangeAssignRangeProduct(std::uint64_t n, const S &initial)
        : length_(n), identity_(std::invoke(e))
    {
        BuildUniform(initial, Power(initial, n));
    }

    explicit RangeAssignRangeProduct(std::span<const S> values)
        : length_(values.size()), identity_(std::invoke(e))
    {
        Reserve(values.size());
        root_ = Build(values);
    }

    RangeAssignRangeProduct(const RangeAssignRangeProduct &) = default;

    RangeAssignRangeProduct(RangeAssignRangeProduct &&other) noexcept
        : nodes_(std::move(other.nodes_)), length_(std::exchange(other.length_, 0)), identity_(other.identity_),
          root_(std::exchange(other.root_, 0)), free_(std::exchange(other.free_, 0)),
          free_count_(std::exchange(other.free_count_, 0))
    {
        other.nodes_.clear();
    }

    RangeAssignRangeProduct &operator=(RangeAssignRangeProduct other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(length_, other.length_);
        std::swap(identity_, other.identity_);
        std::swap(root_, other.root_);
        std::swap(free_, other.free_);
        std::swap(free_count_, other.free_count_);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t stored_ranges() const noexcept
    {
        return nodes_.size() - free_count_;
    }

    [[nodiscard]] S all_prod() const noexcept
    {
        return Aggregate(root_);
    }

    void assign(std::uint64_t left, std::uint64_t right, const S &value)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }
        const auto length = right - left;
        const S copy = value;
        const S aggregate = length == 1 ? copy : Power(copy, length);
        if (length == length_)
        {
            // 根を再利用し、毎回全区間を代入する場合に確保しない。
            ReleaseSubtree(At(root_).left);
            ReleaseSubtree(At(root_).right);
            At(root_) = MakeRun(length, copy, aggregate);
            return;
        }

        // 各境界の分割で1節点、代入で1節点まで増える。非空の区間数は長さを超えない。
        Reserve(static_cast<std::size_t>(std::min<std::uint64_t>(3, length_ - stored_ranges())));
        const auto [before, tail] = Split(root_, left);
        const auto [inside, after] = Split(tail, length);
        ReleaseSubtree(inside);
        root_ = Join(before, Allocate(MakeRun(length, copy, aggregate)), after);
    }

    [[nodiscard]] S get(std::uint64_t position) const
    {
        CheckPosition(position);
        auto index = root_;
        for (;;)
        {
            const auto &node = At(index);
            const auto begin = Length(node.left);
            const auto end = begin + node.run_length;
            if (position < begin)
            {
                index = node.left;
            }
            else if (position < end)
            {
                return node.value;
            }
            else
            {
                index = node.right;
                position -= end;
            }
        }
    }

    [[nodiscard]] S prod(std::uint64_t left, std::uint64_t right) const
    {
        CheckRange(left, right);
        return left == right ? identity_ : Fold(root_, left, right);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        std::uint64_t length, run_length;
        Index left, right;
        unsigned height;
        S value, run_product, aggregate;
    };

    std::vector<Node> nodes_;
    std::uint64_t length_;
    S identity_;
    Index root_ = 0, free_ = 0;
    std::size_t free_count_ = 0;

    void CheckPosition([[maybe_unused]] std::uint64_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= length_)
        {
            throw std::out_of_range("RangeAssignRangeProduct: invalid position");
        }
#endif
    }

    void CheckRange([[maybe_unused]] std::uint64_t left, [[maybe_unused]] std::uint64_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > length_)
        {
            throw std::out_of_range("RangeAssignRangeProduct: invalid range");
        }
#endif
    }

    void Reserve(std::size_t extra)
    {
        const auto slots = extra - std::min(extra, free_count_);
        const auto slot_limit = std::min<std::size_t>(std::numeric_limits<Index>::max(), nodes_.max_size());
        const auto limit = static_cast<std::size_t>(std::min<std::uint64_t>(length_, slot_limit));
        nodes_.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NextRangeAssignProductCapacity(
            nodes_.size(), nodes_.capacity(), slots, limit));
    }

    [[nodiscard]] const Node &At(Index index) const noexcept
    {
        return nodes_[index - 1];
    }

    Node &At(Index index) noexcept
    {
        return nodes_[index - 1];
    }

    [[nodiscard]] unsigned Height(Index index) const noexcept
    {
        return index == 0 ? 0 : At(index).height;
    }

    [[nodiscard]] std::uint64_t Length(Index index) const noexcept
    {
        return index == 0 ? 0 : At(index).length;
    }

    [[nodiscard]] const S &Aggregate(Index index) const noexcept
    {
        return index == 0 ? identity_ : At(index).aggregate;
    }

    static S Product(const S &left, const S &right) noexcept
    {
        return std::invoke(op, left, right);
    }

    S Power(const S &value, std::uint64_t length) const noexcept
    {
        S result = identity_;
        S current = value;
        while (length != 0)
        {
            if ((length & 1) != 0)
            {
                result = Product(result, current);
            }
            length >>= 1;
            if (length != 0)
            {
                current = Product(current, current);
            }
        }
        return result;
    }

    static Node MakeRun(std::uint64_t length, const S &value, const S &aggregate) noexcept
    {
        return {length, length, 0, 0, 1, value, aggregate, aggregate};
    }

    Index Allocate(Node node) noexcept
    {
        if (free_ == 0)
        {
            nodes_.push_back(std::move(node));
            return static_cast<Index>(nodes_.size());
        }
        const auto index = free_;
        free_ = At(index).left;
        --free_count_;
        At(index) = std::move(node);
        return index;
    }

    void ReleaseSubtree(Index index) noexcept
    {
        if (index == 0)
        {
            return;
        }
        auto &node = At(index);
        ReleaseSubtree(node.left);
        ReleaseSubtree(node.right);
        node.value = node.run_product = node.aggregate = identity_;
        node.left = free_;
        free_ = index;
        ++free_count_;
    }

    void BuildUniform(const S &value, const S &aggregate)
    {
        if (length_ != 0)
        {
            Reserve(1);
            root_ = Allocate(MakeRun(length_, value, aggregate));
        }
    }

    Index Build(std::span<const S> values) noexcept
    {
        if (values.empty())
        {
            return 0;
        }
        const auto middle = values.size() / 2;
        const auto index = Allocate(MakeRun(1, values[middle], values[middle]));
        At(index).left = Build(values.first(middle));
        At(index).right = Build(values.subspan(middle + 1));
        Pull(index);
        return index;
    }

    void Pull(Index index) noexcept
    {
        auto &node = At(index);
        node.height = std::max(Height(node.left), Height(node.right)) + 1;
        node.length = Length(node.left) + node.run_length + Length(node.right);
        auto aggregate = node.run_product;
        if (node.left != 0)
        {
            aggregate = Product(At(node.left).aggregate, aggregate);
        }
        if (node.right != 0)
        {
            aggregate = Product(aggregate, At(node.right).aggregate);
        }
        node.aggregate = aggregate;
    }

    Index RotateLeft(Index index) noexcept
    {
        auto &node = At(index);
        const auto pivot = node.right;
        node.right = At(pivot).left;
        At(pivot).left = index;
        Pull(index);
        Pull(pivot);
        return pivot;
    }

    Index RotateRight(Index index) noexcept
    {
        auto &node = At(index);
        const auto pivot = node.left;
        node.left = At(pivot).right;
        At(pivot).right = index;
        Pull(index);
        Pull(pivot);
        return pivot;
    }

    Index Balance(Index index) noexcept
    {
        auto &node = At(index);
        if (Height(node.left) > Height(node.right) + 1)
        {
            const auto &left = At(node.left);
            if (Height(left.left) < Height(left.right))
            {
                node.left = RotateLeft(node.left);
            }
            return RotateRight(index);
        }
        if (Height(node.right) > Height(node.left) + 1)
        {
            const auto &right = At(node.right);
            if (Height(right.right) < Height(right.left))
            {
                node.right = RotateRight(node.right);
            }
            return RotateLeft(index);
        }
        Pull(index);
        return index;
    }

    // left、middle、rightは互いに素で、middleは1区間の根として使う。
    Index Join(Index left, Index middle, Index right) noexcept
    {
        if (Height(left) > Height(right) + 1)
        {
            At(left).right = Join(At(left).right, middle, right);
            return Balance(left);
        }
        if (Height(right) > Height(left) + 1)
        {
            At(right).left = Join(left, middle, At(right).left);
            return Balance(right);
        }
        auto &node = At(middle);
        node.left = left;
        node.right = right;
        Pull(middle);
        return middle;
    }

    std::pair<Index, Index> Split(Index index, std::uint64_t before) noexcept
    {
        if (before == 0)
        {
            return {0, index};
        }
        if (before == Length(index))
        {
            return {index, 0};
        }
        auto &node = At(index);
        const auto left = node.left;
        const auto right = node.right;
        const auto begin = Length(left);
        const auto end = begin + node.run_length;
        if (before <= begin)
        {
            const auto [a, b] = Split(left, before);
            return {a, Join(b, index, right)};
        }
        if (end <= before)
        {
            const auto [a, b] = Split(right, before - end);
            return {Join(left, index, a), b};
        }

        const auto remaining = end - before;
        const auto other = Allocate(MakeRun(remaining, node.value, Power(node.value, remaining)));
        node.run_length = before - begin;
        node.run_product = Power(node.value, node.run_length);
        return {Join(left, index, 0), Join(0, other, right)};
    }

    [[nodiscard]] S Fold(Index index, std::uint64_t left, std::uint64_t right) const noexcept
    {
        const auto &node = At(index);
        if (left == 0 && right == node.length)
        {
            return node.aggregate;
        }
        const auto begin = Length(node.left);
        const auto end = begin + node.run_length;
        if (right <= begin)
        {
            return Fold(node.left, left, right);
        }
        if (end <= left)
        {
            return Fold(node.right, left - end, right - end);
        }

        auto aggregate = left <= begin && end <= right
                             ? node.run_product
                             : Power(node.value, std::min(right, end) - std::max(left, begin));
        if (left < begin)
        {
            aggregate = Product(Fold(node.left, left, begin), aggregate);
        }
        if (end < right)
        {
            aggregate = Product(aggregate, Fold(node.right, 0, right - end));
        }
        return aggregate;
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
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Affine> values(n);
    for (auto &value : values)
    {
        verify_in >> value.a >> value.b;
    }
    RangeAssignRangeProduct<Affine, Compose, Identity> data(values);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            Affine value{};
            verify_in >> value.a >> value.b;
            data.assign(left, right, value);
        }
        else
        {
            std::uint32_t x = 0;
            verify_in >> x;
            const auto fold = data.prod(left, right);
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
        }
    }
}
