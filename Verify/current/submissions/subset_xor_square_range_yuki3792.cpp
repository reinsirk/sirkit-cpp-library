#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/3792

// Library: DataStructure/Subset_Xor_Square_Range.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（Verify問題）: https://yukicoder.me/problems/no/3792
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subset_xor_square_range.md
//
// API一覧
// SubsetXorSquareRange<Bits,Mod>(values): 非負32ビット整数列の部分集合XORを集計。構築O(Bits*n)。
//   Bitsは1以上32以下（既定30）、すべての値は2^Bits未満。Modは正のuint32_t（既定998244353）。
// set(index,value): 要素を置き換える。O(Bits log(n+1))。
// sum_squares(left,right): 半開区間内の添字を選ぶ全ての部分集合についてXORの2乗和をmod Modで返す。
//   同じ値でも添字を区別する。空区間の答えは0。O(Bits log(n+1))。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSET_XOR_SQUARE_RANGE
#define SIRKIT_SUBSET_XOR_SQUARE_RANGE

template <int Bits = 30, std::uint32_t Mod = 998244353>
class SubsetXorSquareRange
{
    static_assert(1 <= Bits && Bits <= 32);
    static_assert(Mod != 0);
    static constexpr auto kAll = std::numeric_limits<std::uint32_t>::max() >> (32 - Bits);
    struct Columns
    {
        std::array<std::uint32_t, Bits> equivalent;
        std::uint32_t active;
    };

    // 異なる2本の非零列はF2で一次独立。同じ列のビットは、常に同時に立つ。
    // 等値関係の共通部分と非零列の和集合だけで、結合後の2次モーメントが定まる。
    [[nodiscard]] static Columns join(const Columns &left, const Columns &right)
    {
        Columns result;
        for (int j = 0; j < Bits; ++j)
        {
            result.equivalent[j] = left.equivalent[j] & right.equivalent[j];
        }
        result.active = left.active | right.active;
        return result;
    }

    [[nodiscard]] static Columns singleton(std::uint32_t value)
    {
        if ((value & ~kAll) != 0)
        {
            throw std::invalid_argument("SubsetXorSquareRange: value outside configured bit width");
        }
        Columns result;
        for (int j = 0; j < Bits; ++j)
        {
            result.equivalent[j] = ((value >> j) & 1U) != 0 ? value : kAll ^ value;
        }
        result.active = value;
        return result;
    }

    [[nodiscard]] static constexpr std::uint32_t square(std::uint32_t value)
    {
        return static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }

    std::size_t size_;
    std::vector<Columns> tree_;
    std::vector<std::uint32_t> two_powers_;

public:
    explicit SubsetXorSquareRange(std::span<const std::uint32_t> values)
        : size_(values.size()), tree_(2 * size_), two_powers_(size_ + 1)
    {
        two_powers_[0] = 1 % Mod;
        for (std::size_t index = 0; index < size_; ++index)
        {
            tree_[size_ + index] = singleton(values[index]);
            two_powers_[index + 1] = static_cast<std::uint32_t>(std::uint64_t{two_powers_[index]} * 2 % Mod);
        }
        for (std::size_t node = size_; node > 1;)
        {
            --node;
            tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
        }
    }

    void set(std::size_t index, std::uint32_t value)
    {
        if (index >= size_)
        {
            throw std::out_of_range("SubsetXorSquareRange::set");
        }
        auto node = index + size_;
        tree_[node] = singleton(value);
        while (node > 1)
        {
            node /= 2;
            tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
        }
    }

    [[nodiscard]] std::uint32_t sum_squares(std::size_t left, std::size_t right) const
    {
        if (left > right || right > size_)
        {
            throw std::out_of_range("SubsetXorSquareRange::sum_squares");
        }
        const auto length = right - left;
        if (length == 0)
        {
            return 0;
        }
        if (length == 1)
        {
            return square(tree_[size_ + left].active);
        }
        Columns columns;
        columns.equivalent.fill(kAll);
        columns.active = 0;
        for (left += size_, right += size_; left < right; left /= 2, right /= 2)
        {
            if ((left & 1U) != 0)
            {
                columns = join(columns, tree_[left++]);
            }
            if ((right & 1U) != 0)
            {
                columns = join(columns, tree_[--right]);
            }
        }
        std::uint64_t sum = square(columns.active);
        for (auto remaining = columns.active; remaining != 0;)
        {
            const auto group = columns.equivalent[std::countr_zero(remaining)];
            sum += square(group);
            remaining ^= group;
        }
        return static_cast<std::uint32_t>((sum % Mod) * two_powers_[length - 2] % Mod);
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


#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t size{}, queries{};
    verify_in >> size >> queries;
    std::vector<std::uint32_t> values(size);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::SubsetXorSquareRange<> squares(values);
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::uint32_t type{}, value_or_right{};
        std::size_t index_or_left{};
        verify_in >> type >> index_or_left >> value_or_right;
        if (type == 1)
        {
            squares.set(index_or_left - 1, value_or_right);
        }
        else
        {
            verify_out << squares.sum_squares(index_or_left - 1, value_or_right) << '\n';
        }
    }
}
