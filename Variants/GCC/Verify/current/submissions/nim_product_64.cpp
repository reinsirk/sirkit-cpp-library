#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/nim_product_64

// Library: Math/Nim_Product.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Nim_Product.cpp
// 数学的参考: J. H. Conway, On Numbers and Games, Chapter 6; R. K. Guy, Impartial Games.
// https://www.cambridge.org/core/books/abs/games-of-no-chance/impartial-games/FAC3E206D7C0A86C60615EF8F669EABB
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/nim_product.md
//
// API一覧
// nim_product(a, b): uint64_tのニム積を返す。ニム和はa^b。0は吸収元、1は単位元。
// 初回に8ビット積表など69 KiBを生成し、以後は3段の分割で定数時間・追加確保なし。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NIM_PRODUCT_DEFINED
#define SIRKIT_NIM_PRODUCT_DEFINED

namespace detail
{
template <int Half, class Multiply, class Scale>
constexpr std::uint64_t NimProductSplit(std::uint64_t a, std::uint64_t b, Multiply multiply, Scale scale)
{
    constexpr auto kMask = (std::uint64_t{1} << Half) - 1;
    const auto a_high = a >> Half;
    const auto b_high = b >> Half;
    const auto a_low = a & kMask;
    const auto b_low = b & kMask;
    const auto high = multiply(a_high, b_high);
    const auto low = multiply(a_low, b_low);
    const auto mixed = multiply(a_high ^ a_low, b_high ^ b_low);
    // F=2^Half。F⊗F=F xor (F/2) なので、上位係数は mixed xor low。
    return ((mixed ^ low) << Half) ^ low ^ scale(high);
}

template <int Bits>
constexpr std::uint64_t SmallNimProduct(std::uint64_t a, std::uint64_t b)
{
    if constexpr (Bits == 1)
    {
        return a & b;
    }
    else
    {
        constexpr int kHalf = Bits / 2;
        return NimProductSplit<kHalf>(a, b, [](std::uint64_t x, std::uint64_t y)
                                      {
                                          return SmallNimProduct<kHalf>(x, y);
                                      },
                                      [](std::uint64_t x)
                                      {
                                          return SmallNimProduct<kHalf>(x, std::uint64_t{1} << (kHalf - 1));
                                      });
    }
}

class NimProductTable
{
public:
    NimProductTable()
    {
        for (std::size_t a = 0; a < byte_product_.size(); ++a)
        {
            for (std::size_t b = 0; b < byte_product_[a].size(); ++b)
            {
                byte_product_[a][b] = static_cast<std::uint8_t>(SmallNimProduct<8>(a, b));
            }
        }
        FillScale(scale16_);
        FillScale(scale32_);
    }

    template <int Bits>
    std::uint64_t multiply(std::uint64_t a, std::uint64_t b) const
    {
        if constexpr (Bits == 8)
        {
            return byte_product_[a][b];
        }
        else
        {
            constexpr int kHalf = Bits / 2;
            return NimProductSplit<kHalf>(a, b, [this](std::uint64_t x, std::uint64_t y)
                                          {
                                              return multiply<kHalf>(x, y);
                                          },
                                          [this](std::uint64_t x)
                                          {
                                              return Scale<kHalf>(x);
                                          });
        }
    }

private:
    std::array<std::array<std::uint8_t, 256>, 256> byte_product_;
    std::array<std::array<std::uint16_t, 256>, 2> scale16_;
    std::array<std::array<std::uint32_t, 256>, 4> scale32_;

    template <class T, std::size_t Bytes>
    void FillScale(std::array<std::array<T, 256>, Bytes> &table)
    {
        constexpr std::size_t kBits = Bytes * 8;
        constexpr auto kFactor = std::uint64_t{1} << (kBits - 1);
        for (std::size_t byte = 0; byte < Bytes; ++byte)
        {
            for (std::size_t value = 0; value < table[byte].size(); ++value)
            {
                table[byte][value] = static_cast<T>(multiply<kBits>(std::uint64_t{value} << (8 * byte), kFactor));
            }
        }
    }

    template <int Bits>
    std::uint64_t Scale(std::uint64_t value) const
    {
        // 定数倍はxorに対して線形なので、各バイトの寄与を足す。
        if constexpr (Bits == 8)
        {
            return byte_product_[128][value];
        }
        else if constexpr (Bits == 16)
        {
            return scale16_[0][value & 255] ^ scale16_[1][value >> 8];
        }
        else
        {
            return scale32_[0][value & 255] ^ scale32_[1][(value >> 8) & 255] ^
                   scale32_[2][(value >> 16) & 255] ^ scale32_[3][value >> 24];
        }
    }
};
} // namespace detail

inline std::uint64_t nim_product(std::uint64_t a, std::uint64_t b)
{
    static const detail::NimProductTable table;
    return table.multiply<64>(a, b);
}

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

int main()
{
    std::size_t queries{};
    verify_in >> queries;
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::uint64_t a{}, b{};
        verify_in >> a >> b;
        verify_out << sirkit::nim_product(a, b) << '\n';
    }
}
