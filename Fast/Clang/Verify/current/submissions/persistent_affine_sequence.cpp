#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
#define NDEBUG
// Verify: https://judge.yosupo.jp/problem/persistent_range_affine_range_sum

// Library: DataStructure/Persistent_Affine_Sequence.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Affine_Sequence.cpp
// 参考: https://img.atcoder.jp/arc030/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_affine_sequence.md
//
// API一覧
// PersistentAffineSequence<T>(values): 数列を構築。O(n)時間・領域。空列はPersistentAffineSequence<T>()。
// size()/empty()/total(): 要素数、空判定、全体和。O(1)。sum(l,r): [l,r)の和。O(log(n+1))、ノード追加なし。
// affine(l,r,a,b): [l,r)の各xをa*x+bにする。reverse(l,r): [l,r)を反転する。
// slice(l,r): [l,r)だけを取り出す。split(k): [0,k)と[k,n)の組に分ける。
// concat(other): 後ろに別の列を連結。replace(l,r,other): [l,r)を別の列で置き換える。長さの異なる列も可。
// copy_from(l,r,other): 別の列の同じ添字の範囲で[l,r)を置き換える。
// 更新・切り出しは元の列を保存し、新しい列を返す。時間・新規ノード数O(log(n+m+1))。mは連結・置換する列の長さ。
// コピーはノードを共有しO(1)。最後の参照を失ったノードは解放。移動元は空。
// 0始まりの半開区間。要素数はINT_MAX以下。T{0}, T{1}, 加算、乗算が使え、Tで要素数と演算の中間値を表現できること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_AFFINE_SEQUENCE_DEFINED
#define SIRKIT_PERSISTENT_AFFINE_SEQUENCE_DEFINED

template <class T>
class PersistentAffineSequence
{
    struct Affine
    {
        T multiplier = T(1);
        T increment = T(0);

        T Sum(const T &value, std::size_t count) const
        {
            return multiplier * value + increment * T(count);
        }
    };

    struct Node;
    using Pointer = std::shared_ptr<const Node>;

    struct Node
    {
        Pointer left;
        Pointer right;
        T sum;
        std::size_t length = 1;
        int height = 1;
        Affine affine;
        bool has_affine = false;
        bool reverse = false;
    };

public:
    PersistentAffineSequence() = default;

    explicit PersistentAffineSequence(std::span<const T> values)
        : root_(BuildChecked(values))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return root_ ? root_->length : 0;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return !root_;
    }

    [[nodiscard]] T total() const
    {
        return root_ ? root_->sum : T(0);
    }

    [[nodiscard]] T sum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? T(0) : RangeSum(root_, left, right, false);
    }

    [[nodiscard]] PersistentAffineSequence affine(std::size_t left, std::size_t right, const T &a, const T &b) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        return PersistentAffineSequence(TransformRange(root_, left, right, {a, b}));
    }

    [[nodiscard]] PersistentAffineSequence reverse(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        auto [prefix, suffix] = Split(root_, left);
        auto [middle, tail] = Split(suffix, right - left);
        return PersistentAffineSequence(Join(Join(prefix, Reverse(middle)), tail));
    }

    [[nodiscard]] PersistentAffineSequence slice(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return {};
        }
        return PersistentAffineSequence(Slice(root_, left, right));
    }

    [[nodiscard]] std::pair<PersistentAffineSequence, PersistentAffineSequence> split(std::size_t position) const
    {
        CheckRange(0, position);
        auto [prefix, suffix] = Split(root_, position);
        return {PersistentAffineSequence(std::move(prefix)), PersistentAffineSequence(std::move(suffix))};
    }

    [[nodiscard]] PersistentAffineSequence concat(const PersistentAffineSequence &other) const
    {
        CheckSize(size() + other.size());
        return PersistentAffineSequence(Join(root_, other.root_));
    }

    [[nodiscard]] PersistentAffineSequence replace(std::size_t left, std::size_t right, const PersistentAffineSequence &other) const
    {
        CheckRange(left, right);
        CheckSize(size() - (right - left) + other.size());
        return PersistentAffineSequence(Replace(root_, left, right, other.root_));
    }

    [[nodiscard]] PersistentAffineSequence copy_from(std::size_t left, std::size_t right, const PersistentAffineSequence &other) const
    {
        CheckRange(left, right);
        other.CheckRange(left, right);
        if (left == right)
        {
            return *this;
        }
        if (size() != other.size())
        {
            return PersistentAffineSequence(Replace(root_, left, right, Slice(other.root_, left, right)));
        }
        return PersistentAffineSequence(CopyAligned(root_, other.root_, left, right));
    }

private:
    Pointer root_;

    explicit PersistentAffineSequence(Pointer root) : root_(std::move(root))
    {
    }

    static void CheckSize(std::size_t length)
    {
        if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("PersistentAffineSequence: too many elements");
        }
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
    }

    static Pointer Leaf(const T &value)
    {
        return std::make_shared<const Node>(Node{nullptr, nullptr, value, 1, 1, {}, false, false});
    }

    static Pointer Branch(const Pointer &left, const Pointer &right)
    {
        return std::make_shared<const Node>(Node{left, right, left->sum + right->sum, left->length + right->length, 1 + std::max(left->height, right->height), {}, false, false});
    }

    static Pointer Build(std::span<const T> values)
    {
        if (values.size() == 1)
        {
            return Leaf(values[0]);
        }
        const auto half = values.size() / 2;
        return Branch(Build(values.first(half)), Build(values.subspan(half)));
    }

    static Pointer BuildChecked(std::span<const T> values)
    {
        CheckSize(values.size());
        return values.empty() ? nullptr : Build(values);
    }

    static Pointer Transform(const Pointer &node, const Affine &f)
    {
        auto result = std::make_shared<Node>(*node);
        result->sum = f.Sum(result->sum, result->length);
        if (result->height > 1)
        {
            result->affine = {f.multiplier * result->affine.multiplier,
                              f.multiplier * result->affine.increment + f.increment};
            result->has_affine = true;
        }
        return result;
    }

    static Pointer Reverse(const Pointer &node)
    {
        if (node->height == 1)
        {
            return node;
        }
        auto result = std::make_shared<Node>(*node);
        std::swap(result->left, result->right);
        result->reverse = !result->reverse;
        return result;
    }

    // Returns the logical children, propagating only the retained parent's tags.
    static std::pair<Pointer, Pointer> Children(const Pointer &node)
    {
        auto left = node->left;
        auto right = node->right;
        if (node->reverse)
        {
            left = Reverse(left);
            right = Reverse(right);
        }
        if (node->has_affine)
        {
            left = Transform(left, node->affine);
            right = Transform(right, node->affine);
        }
        return {std::move(left), std::move(right)};
    }

    static Pointer Balance(const Pointer &left, const Pointer &right)
    {
        if (left->height > right->height + 1)
        {
            auto [outer, inner] = Children(left);
            if (outer->height >= inner->height)
            {
                return Branch(outer, Branch(inner, right));
            }
            auto [a, b] = Children(inner);
            return Branch(Branch(outer, a), Branch(b, right));
        }
        if (right->height > left->height + 1)
        {
            auto [inner, outer] = Children(right);
            if (outer->height >= inner->height)
            {
                return Branch(Branch(left, inner), outer);
            }
            auto [a, b] = Children(inner);
            return Branch(Branch(left, a), Branch(b, outer));
        }
        return Branch(left, right);
    }

    static Pointer Join(const Pointer &left, const Pointer &right)
    {
        if (!left)
        {
            return right;
        }
        if (!right)
        {
            return left;
        }
        if (left->height > right->height + 1)
        {
            auto [a, b] = Children(left);
            return Balance(a, Join(b, right));
        }
        if (right->height > left->height + 1)
        {
            auto [a, b] = Children(right);
            return Balance(Join(left, a), b);
        }
        return Branch(left, right);
    }

    static std::pair<Pointer, Pointer> Split(const Pointer &node, std::size_t size)
    {
        if (size == 0)
        {
            return {nullptr, node};
        }
        if (size == node->length)
        {
            return {node, nullptr};
        }
        auto [left, right] = Children(node);
        const auto left_size = left->length;
        if (size < left_size)
        {
            auto [a, b] = Split(left, size);
            return {std::move(a), Join(b, right)};
        }
        if (size > left_size)
        {
            auto [a, b] = Split(right, size - left_size);
            return {Join(left, a), std::move(b)};
        }
        return {std::move(left), std::move(right)};
    }

    static Pointer Slice(const Pointer &node, std::size_t left, std::size_t right)
    {
        const auto suffix = Split(node, left).second;
        return Split(suffix, right - left).first;
    }

    static Pointer Replace(const Pointer &node, std::size_t left, std::size_t right, const Pointer &other)
    {
        auto [prefix, suffix] = Split(node, left);
        const auto tail = Split(suffix, right - left).second;
        return Join(Join(prefix, other), tail);
    }

    static Pointer TransformRange(const Pointer &node, std::size_t left, std::size_t right, const Affine &f)
    {
        if (left == 0 && right == node->length)
        {
            return Transform(node, f);
        }
        auto [a, b] = Children(node);
        const auto mid = a->length;
        if (right <= mid)
        {
            return Branch(TransformRange(a, left, right, f), b);
        }
        if (left >= mid)
        {
            return Branch(a, TransformRange(b, left - mid, right - mid, f));
        }
        return Branch(TransformRange(a, left, mid, f), TransformRange(b, 0, right - mid, f));
    }

    static Pointer CopyAligned(const Pointer &target, const Pointer &source, std::size_t left, std::size_t right)
    {
        if (target == source)
        {
            return target;
        }
        if (left == 0 && right == target->length)
        {
            return source;
        }
        const auto mid = target->left->length;
        if (mid != source->left->length)
        {
            return Replace(target, left, right, Slice(source, left, right));
        }
        auto [dest_a, dest_b] = Children(target);
        const auto [src_a, src_b] = Children(source);
        if (right <= mid)
        {
            return Branch(CopyAligned(dest_a, src_a, left, right), dest_b);
        }
        if (left >= mid)
        {
            return Branch(dest_a, CopyAligned(dest_b, src_b, left - mid, right - mid));
        }
        return Branch(CopyAligned(dest_a, src_a, left, mid), CopyAligned(dest_b, src_b, 0, right - mid));
    }

    // Sum directly in the shared tree; querying a slice allocates no nodes.
    static T RangeSum(const Pointer &node, std::size_t left, std::size_t right, bool flip)
    {
        if (left == 0 && right == node->length)
        {
            return node->sum;
        }
        const auto &a = flip ? node->right : node->left;
        const auto &b = flip ? node->left : node->right;
        const bool child_flip = flip != node->reverse;
        const auto mid = a->length;
        const T value = right <= mid
                            ? RangeSum(a, left, right, child_flip)
                        : left >= mid
                            ? RangeSum(b, left - mid, right - mid, child_flip)
                            : RangeSum(a, left, mid, child_flip) + RangeSum(b, 0, right - mid, child_flip);
        return node->has_affine ? node->affine.Sum(value, right - left) : value;
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


// ACL: atcoder/modint
#ifndef ATCODER_MODINT_HPP
#define ATCODER_MODINT_HPP 1

#include <cassert>
#include <numeric>
#include <type_traits>

#ifdef _MSC_VER
#include <intrin.h>
#endif

// ACL: atcoder/internal_math
#ifndef ATCODER_INTERNAL_MATH_HPP
#define ATCODER_INTERNAL_MATH_HPP 1

#include <utility>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace atcoder {

namespace internal {

// @param m `1 <= m`
// @return x mod m
constexpr long long safe_mod(long long x, long long m) {
    x %= m;
    if (x < 0) x += m;
    return x;
}

// Fast modular multiplication by barrett reduction
// Reference: https://en.wikipedia.org/wiki/Barrett_reduction
// NOTE: reconsider after Ice Lake
struct barrett {
    unsigned int _m;
    unsigned long long im;

    // @param m `1 <= m`
    explicit barrett(unsigned int m) : _m(m), im((unsigned long long)(-1) / m + 1) {}

    // @return m
    unsigned int umod() const { return _m; }

    // @param a `0 <= a < m`
    // @param b `0 <= b < m`
    // @return `a * b % m`
    unsigned int mul(unsigned int a, unsigned int b) const {
        // [1] m = 1
        // a = b = im = 0, so okay

        // [2] m >= 2
        // im = ceil(2^64 / m)
        // -> im * m = 2^64 + r (0 <= r < m)
        // let z = a*b = c*m + d (0 <= c, d < m)
        // a*b * im = (c*m + d) * im = c*(im*m) + d*im = c*2^64 + c*r + d*im
        // c*r + d*im < m * m + m * im < m * m + 2^64 + m <= 2^64 + m * (m + 1) < 2^64 * 2
        // ((ab * im) >> 64) == c or c + 1
        unsigned long long z = a;
        z *= b;
#ifdef _MSC_VER
        unsigned long long x;
        _umul128(z, im, &x);
#else
        unsigned long long x =
            (unsigned long long)(((unsigned __int128)(z)*im) >> 64);
#endif
        unsigned long long y = x * _m;
        return (unsigned int)(z - y + (z < y ? _m : 0));
    }
};

// @param n `0 <= n`
// @param m `1 <= m`
// @return `(x ** n) % m`
constexpr long long pow_mod_constexpr(long long x, long long n, int m) {
    if (m == 1) return 0;
    unsigned int _m = (unsigned int)(m);
    unsigned long long r = 1;
    unsigned long long y = safe_mod(x, m);
    while (n) {
        if (n & 1) r = (r * y) % _m;
        y = (y * y) % _m;
        n >>= 1;
    }
    return r;
}

// Reference:
// M. Forisek and J. Jancina,
// Fast Primality Testing for Integers That Fit into a Machine Word
// @param n `0 <= n`
constexpr bool is_prime_constexpr(int n) {
    if (n <= 1) return false;
    if (n == 2 || n == 7 || n == 61) return true;
    if (n % 2 == 0) return false;
    long long d = n - 1;
    while (d % 2 == 0) d /= 2;
    constexpr long long bases[3] = {2, 7, 61};
    for (long long a : bases) {
        long long t = d;
        long long y = pow_mod_constexpr(a, t, n);
        while (t != n - 1 && y != 1 && y != n - 1) {
            y = y * y % n;
            t <<= 1;
        }
        if (y != n - 1 && t % 2 == 0) {
            return false;
        }
    }
    return true;
}
template <int n> constexpr bool is_prime = is_prime_constexpr(n);

// @param b `1 <= b`
// @return pair(g, x) s.t. g = gcd(a, b), xa = g (mod b), 0 <= x < b/g
constexpr std::pair<long long, long long> inv_gcd(long long a, long long b) {
    a = safe_mod(a, b);
    if (a == 0) return {b, 0};

    // Contracts:
    // [1] s - m0 * a = 0 (mod b)
    // [2] t - m1 * a = 0 (mod b)
    // [3] s * |m1| + t * |m0| <= b
    long long s = b, t = a;
    long long m0 = 0, m1 = 1;

    while (t) {
        long long u = s / t;
        s -= t * u;
        m0 -= m1 * u;  // |m1 * u| <= |m1| * s <= b

        // [3]:
        // (s - t * u) * |m1| + t * |m0 - m1 * u|
        // <= s * |m1| - t * u * |m1| + t * (|m0| + |m1| * u)
        // = s * |m1| + t * |m0| <= b

        auto tmp = s;
        s = t;
        t = tmp;
        tmp = m0;
        m0 = m1;
        m1 = tmp;
    }
    // by [3]: |m0| <= b/g
    // by g != b: |m0| < b/g
    if (m0 < 0) m0 += b / s;
    return {s, m0};
}

// Compile time primitive root
// @param m must be prime
// @return primitive root (and minimum in now)
constexpr int primitive_root_constexpr(int m) {
    if (m == 2) return 1;
    if (m == 167772161) return 3;
    if (m == 469762049) return 3;
    if (m == 754974721) return 11;
    if (m == 998244353) return 3;
    int divs[20] = {};
    divs[0] = 2;
    int cnt = 1;
    int x = (m - 1) / 2;
    while (x % 2 == 0) x /= 2;
    for (int i = 3; (long long)(i)*i <= x; i += 2) {
        if (x % i == 0) {
            divs[cnt++] = i;
            while (x % i == 0) {
                x /= i;
            }
        }
    }
    if (x > 1) {
        divs[cnt++] = x;
    }
    for (int g = 2;; g++) {
        bool ok = true;
        for (int i = 0; i < cnt; i++) {
            if (pow_mod_constexpr(g, (m - 1) / divs[i], m) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) return g;
    }
}
template <int m> constexpr int primitive_root = primitive_root_constexpr(m);

// @param n `n < 2^32`
// @param m `1 <= m < 2^32`
// @return sum_{i=0}^{n-1} floor((ai + b) / m) (mod 2^64)
unsigned long long floor_sum_unsigned(unsigned long long n,
                                      unsigned long long m,
                                      unsigned long long a,
                                      unsigned long long b) {
    unsigned long long ans = 0;
    while (true) {
        if (a >= m) {
            ans += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            ans += n * (b / m);
            b %= m;
        }

        unsigned long long y_max = a * n + b;
        if (y_max < m) break;
        // y_max < m * (n + 1)
        // floor(y_max / m) <= n
        n = (unsigned long long)(y_max / m);
        b = (unsigned long long)(y_max % m);
        std::swap(m, a);
    }
    return ans;
}

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_INTERNAL_MATH_HPP

// ACL: atcoder/internal_type_traits
#ifndef ATCODER_INTERNAL_TYPE_TRAITS_HPP
#define ATCODER_INTERNAL_TYPE_TRAITS_HPP 1

#include <cassert>
#include <numeric>
#include <type_traits>

namespace atcoder {

namespace internal {

#ifndef _MSC_VER
template <class T>
using is_signed_int128 =
    typename std::conditional<std::is_same<T, __int128_t>::value ||
                                  std::is_same<T, __int128>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using is_unsigned_int128 =
    typename std::conditional<std::is_same<T, __uint128_t>::value ||
                                  std::is_same<T, unsigned __int128>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using make_unsigned_int128 =
    typename std::conditional<std::is_same<T, __int128_t>::value,
                              __uint128_t,
                              unsigned __int128>;

template <class T>
using is_integral = typename std::conditional<std::is_integral<T>::value ||
                                                  is_signed_int128<T>::value ||
                                                  is_unsigned_int128<T>::value,
                                              std::true_type,
                                              std::false_type>::type;

template <class T>
using is_signed_int = typename std::conditional<(is_integral<T>::value &&
                                                 std::is_signed<T>::value) ||
                                                    is_signed_int128<T>::value,
                                                std::true_type,
                                                std::false_type>::type;

template <class T>
using is_unsigned_int =
    typename std::conditional<(is_integral<T>::value &&
                               std::is_unsigned<T>::value) ||
                                  is_unsigned_int128<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using to_unsigned = typename std::conditional<
    is_signed_int128<T>::value,
    make_unsigned_int128<T>,
    typename std::conditional<std::is_signed<T>::value,
                              std::make_unsigned<T>,
                              std::common_type<T>>::type>::type;

#else

template <class T> using is_integral = typename std::is_integral<T>;

template <class T>
using is_signed_int =
    typename std::conditional<is_integral<T>::value && std::is_signed<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using is_unsigned_int =
    typename std::conditional<is_integral<T>::value &&
                                  std::is_unsigned<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using to_unsigned = typename std::conditional<is_signed_int<T>::value,
                                              std::make_unsigned<T>,
                                              std::common_type<T>>::type;

#endif

template <class T>
using is_signed_int_t = std::enable_if_t<is_signed_int<T>::value>;

template <class T>
using is_unsigned_int_t = std::enable_if_t<is_unsigned_int<T>::value>;

template <class T> using to_unsigned_t = typename to_unsigned<T>::type;

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_INTERNAL_TYPE_TRAITS_HPP

namespace atcoder {

namespace internal {

struct modint_base {};
struct static_modint_base : modint_base {};

template <class T> using is_modint = std::is_base_of<modint_base, T>;
template <class T> using is_modint_t = std::enable_if_t<is_modint<T>::value>;

}  // namespace internal

template <int m, std::enable_if_t<(1 <= m)>* = nullptr>
struct static_modint : internal::static_modint_base {
    using mint = static_modint;

  public:
    static constexpr int mod() { return m; }
    static mint raw(int v) {
        mint x;
        x._v = v;
        return x;
    }

    static_modint() : _v(0) {}
    template <class T, internal::is_signed_int_t<T>* = nullptr>
    static_modint(T v) {
        long long x = (long long)(v % (long long)(umod()));
        if (x < 0) x += umod();
        _v = (unsigned int)(x);
    }
    template <class T, internal::is_unsigned_int_t<T>* = nullptr>
    static_modint(T v) {
        _v = (unsigned int)(v % umod());
    }

    int val() const { return _v; }

    mint& operator++() {
        _v++;
        if (_v == umod()) _v = 0;
        return *this;
    }
    mint& operator--() {
        if (_v == 0) _v = umod();
        _v--;
        return *this;
    }
    mint operator++(int) {
        mint result = *this;
        ++*this;
        return result;
    }
    mint operator--(int) {
        mint result = *this;
        --*this;
        return result;
    }

    mint& operator+=(const mint& rhs) {
        _v += rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator-=(const mint& rhs) {
        _v -= rhs._v;
        if (_v >= umod()) _v += umod();
        return *this;
    }
    mint& operator*=(const mint& rhs) {
        unsigned long long z = _v;
        z *= rhs._v;
        _v = (unsigned int)(z % umod());
        return *this;
    }
    mint& operator/=(const mint& rhs) { return *this = *this * rhs.inv(); }

    mint operator+() const { return *this; }
    mint operator-() const { return mint() - *this; }

    mint pow(long long n) const {
        assert(0 <= n);
        mint x = *this, r = 1;
        while (n) {
            if (n & 1) r *= x;
            x *= x;
            n >>= 1;
        }
        return r;
    }
    mint inv() const {
        if (prime) {
            assert(_v);
            return pow(umod() - 2);
        } else {
            auto eg = internal::inv_gcd(_v, m);
            assert(eg.first == 1);
            return eg.second;
        }
    }

    friend mint operator+(const mint& lhs, const mint& rhs) {
        return mint(lhs) += rhs;
    }
    friend mint operator-(const mint& lhs, const mint& rhs) {
        return mint(lhs) -= rhs;
    }
    friend mint operator*(const mint& lhs, const mint& rhs) {
        return mint(lhs) *= rhs;
    }
    friend mint operator/(const mint& lhs, const mint& rhs) {
        return mint(lhs) /= rhs;
    }
    friend bool operator==(const mint& lhs, const mint& rhs) {
        return lhs._v == rhs._v;
    }
    friend bool operator!=(const mint& lhs, const mint& rhs) {
        return lhs._v != rhs._v;
    }

  private:
    unsigned int _v;
    static constexpr unsigned int umod() { return m; }
    static constexpr bool prime = internal::is_prime<m>;
};

template <int id> struct dynamic_modint : internal::modint_base {
    using mint = dynamic_modint;

  public:
    static int mod() { return (int)(bt.umod()); }
    static void set_mod(int m) {
        assert(1 <= m);
        bt = internal::barrett(m);
    }
    static mint raw(int v) {
        mint x;
        x._v = v;
        return x;
    }

    dynamic_modint() : _v(0) {}
    template <class T, internal::is_signed_int_t<T>* = nullptr>
    dynamic_modint(T v) {
        long long x = (long long)(v % (long long)(mod()));
        if (x < 0) x += mod();
        _v = (unsigned int)(x);
    }
    template <class T, internal::is_unsigned_int_t<T>* = nullptr>
    dynamic_modint(T v) {
        _v = (unsigned int)(v % mod());
    }

    int val() const { return _v; }

    mint& operator++() {
        _v++;
        if (_v == umod()) _v = 0;
        return *this;
    }
    mint& operator--() {
        if (_v == 0) _v = umod();
        _v--;
        return *this;
    }
    mint operator++(int) {
        mint result = *this;
        ++*this;
        return result;
    }
    mint operator--(int) {
        mint result = *this;
        --*this;
        return result;
    }

    mint& operator+=(const mint& rhs) {
        _v += rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator-=(const mint& rhs) {
        _v += mod() - rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator*=(const mint& rhs) {
        _v = bt.mul(_v, rhs._v);
        return *this;
    }
    mint& operator/=(const mint& rhs) { return *this = *this * rhs.inv(); }

    mint operator+() const { return *this; }
    mint operator-() const { return mint() - *this; }

    mint pow(long long n) const {
        assert(0 <= n);
        mint x = *this, r = 1;
        while (n) {
            if (n & 1) r *= x;
            x *= x;
            n >>= 1;
        }
        return r;
    }
    mint inv() const {
        auto eg = internal::inv_gcd(_v, mod());
        assert(eg.first == 1);
        return eg.second;
    }

    friend mint operator+(const mint& lhs, const mint& rhs) {
        return mint(lhs) += rhs;
    }
    friend mint operator-(const mint& lhs, const mint& rhs) {
        return mint(lhs) -= rhs;
    }
    friend mint operator*(const mint& lhs, const mint& rhs) {
        return mint(lhs) *= rhs;
    }
    friend mint operator/(const mint& lhs, const mint& rhs) {
        return mint(lhs) /= rhs;
    }
    friend bool operator==(const mint& lhs, const mint& rhs) {
        return lhs._v == rhs._v;
    }
    friend bool operator!=(const mint& lhs, const mint& rhs) {
        return lhs._v != rhs._v;
    }

  private:
    unsigned int _v;
    static internal::barrett bt;
    static unsigned int umod() { return bt.umod(); }
};
template <int id> internal::barrett dynamic_modint<id>::bt(998244353);

using modint998244353 = static_modint<998244353>;
using modint1000000007 = static_modint<1000000007>;
using modint = dynamic_modint<-1>;

namespace internal {

template <class T>
using is_static_modint = std::is_base_of<internal::static_modint_base, T>;

template <class T>
using is_static_modint_t = std::enable_if_t<is_static_modint<T>::value>;

template <class> struct is_dynamic_modint : public std::false_type {};
template <int id>
struct is_dynamic_modint<dynamic_modint<id>> : public std::true_type {};

template <class T>
using is_dynamic_modint_t = std::enable_if_t<is_dynamic_modint<T>::value>;

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_MODINT_HPP

#include <vector>

int main()
{
    using Mint = atcoder::modint998244353;
    using Sequence = sirkit::PersistentAffineSequence<Mint>;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Mint> values(n);
    for (auto &value : values)
    {
        int x = 0;
        verify_in >> x;
        value = Mint::raw(x);
    }
    std::vector<Sequence> versions(q + 1);
    versions[0] = Sequence(values);
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, base = 0;
        verify_in >> type >> base;
        ++base;
        int source = 0;
        if (type == 1)
        {
            verify_in >> source;
            ++source;
        }
        int left = 0, right = 0;
        verify_in >> left >> right;
        if (type == 0)
        {
            int b = 0, c = 0;
            verify_in >> b >> c;
            versions[i] = versions[base].affine(left, right, Mint::raw(b), Mint::raw(c));
        }
        else if (type == 1)
        {
            versions[i] = versions[base].copy_from(left, right, versions[source]);
        }
        else
        {
            verify_out << versions[base].sum(left, right).val() << '\n';
        }
    }
}
