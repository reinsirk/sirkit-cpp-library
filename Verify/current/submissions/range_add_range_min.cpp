#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/range_add_range_min

// Library: DataStructure/Range_Add_Range_Min.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_range_min.md
//
// API一覧
// RangeAddRangeMin<T>(n/values): n個の0または初期配列から構築。O(n)時間・領域。
// add(l,r,d): 半開区間[l,r)にdを加える。O(log(n+1))。
// minimum(l,r): 区間の最小値。空区間はnullopt。O(log(n+1))。
// all_minimum(): 全体の最小値、空ならnullopt。size(): 要素数。O(1)。
// 添字は0始まり。Tは符号付き整数、内部の最小値・保留中の加算・計算の中間値もTの範囲内であること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ADD_RANGE_MIN_DEFINED
#define SIRKIT_RANGE_ADD_RANGE_MIN_DEFINED

template <std::signed_integral T = std::int64_t>
class RangeAddRangeMin
{
    struct Node
    {
        T minimum = 0;
        T added = 0;
    };

public:
    explicit RangeAddRangeMin(std::size_t n = 0) : nodes_(Allocate(n))
    {
    }

    explicit RangeAddRangeMin(std::span<const T> values) : RangeAddRangeMin(values.size())
    {
        if (!values.empty())
        {
            Build(0, 0, values.size(), values);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return (nodes_.size() + 1) / 2;
    }

    void add(std::size_t left, std::size_t right, T value)
    {
        CheckRange(left, right);
        if (left != right)
        {
            Add(0, 0, size(), left, right, value);
        }
    }

    [[nodiscard]] std::optional<T> minimum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return std::nullopt;
        }
        return Min(0, 0, size(), left, right);
    }

    [[nodiscard]] std::optional<T> all_minimum() const noexcept
    {
        return nodes_.empty() ? std::optional<T>{} : std::optional<T>{nodes_[0].minimum};
    }

private:
    std::vector<Node> nodes_;

    static std::vector<Node> Allocate(std::size_t size)
    {
        if (size > std::vector<Node>().max_size() / 2)
        {
            throw std::length_error("RangeAddRangeMin: too many elements");
        }
        return std::vector<Node>(size == 0 ? 0 : 2 * size - 1);
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("RangeAddRangeMin: range");
        }
#endif
    }

    void Build(std::size_t p, std::size_t begin, std::size_t end, std::span<const T> values)
    {
        if (end - begin == 1)
        {
            nodes_[p].minimum = values[begin];
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        Build(p + 1, begin, mid, values);
        Build(r, mid, end, values);
        nodes_[p].minimum = std::min(nodes_[p + 1].minimum, nodes_[r].minimum);
    }

    // 子の値は親の保留加算を含まない。区間加算と最小値が可換なので押し下げを省く。
    void Add(std::size_t p, std::size_t begin, std::size_t end,
             std::size_t left, std::size_t right, T value)
    {
        auto &node = nodes_[p];
        if (left <= begin && end <= right)
        {
            node.minimum += value;
            node.added += value;
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        if (left < mid)
        {
            Add(p + 1, begin, mid, left, right, value);
        }
        if (right > mid)
        {
            Add(r, mid, end, left, right, value);
        }
        node.minimum = node.added + std::min(nodes_[p + 1].minimum, nodes_[r].minimum);
    }

    T Min(std::size_t p, std::size_t begin, std::size_t end, std::size_t left, std::size_t right) const
    {
        const auto &node = nodes_[p];
        if (left <= begin && end <= right)
        {
            return node.minimum;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto r = p + 2 * (mid - begin);
        if (right <= mid)
        {
            return node.added + Min(p + 1, begin, mid, left, right);
        }
        if (left >= mid)
        {
            return node.added + Min(r, mid, end, left, right);
        }
        return node.added + std::min(Min(p + 1, begin, mid, left, right),
                                     Min(r, mid, end, left, right));
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

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &x : values)
    {
        verify_in >> x;
    }
    sirkit::RangeAddRangeMin<std::int64_t> minimum(values);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::int64_t delta = 0;
            verify_in >> delta;
            minimum.add(left, right, delta);
        }
        else
        {
            verify_out << *minimum.minimum(left, right) << '\n';
        }
    }
}
