#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc429/tasks/abc429_g

// Library: Math/Floor_Path_Product.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Floor_Path_Product.cpp
// 参考（floor sumのモノイド積）: https://atcoder.jp/contests/arc183/editorial/10768
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/floor_path_product.md
//
// API一覧
// floor_path_product(n,m,a,b,horizontal,vertical,identity,combine):
// y_i=floor((a*i+b)/m)とし、vertical^y_0 と各 horizontal vertical^(y_(i+1)-y_i) (0<=i<n) の積。
// n,mはuint32_t、m>0。a,bは非負uint64_t。combineの結合則とidentityが必要。可換性は不要。
// O(log(n+1)+log(m+1)+log(a+1)+log(b+1))回のcombine、O(log(m+1))領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_PATH_PRODUCT_DEFINED
#define SIRKIT_FLOOR_PATH_PRODUCT_DEFINED
namespace detail
{
template <class T, class Combine>
T FloorWordPower(T base, std::uint64_t exponent, const T &identity, const Combine &combine)
{
    T result = identity;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1U) != 0)
        {
            result = combine(result, base);
        }
        if (exponent > 1)
        {
            base = combine(base, base);
        }
    }
    return result;
}

template <class T, class Combine>
T FloorWordProduct(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b,
                   T horizontal, const T &vertical, const T &identity, const Combine &combine)
{
    const auto initial = FloorWordPower(vertical, b / m, identity, combine);
    if (n == 0)
    {
        return initial;
    }
    horizontal = combine(horizontal, FloorWordPower(vertical, a / m, identity, combine));
    a %= m;
    b %= m;
    const auto height = (a * n + b) / m;
    if (height == 0)
    {
        return combine(initial, FloorWordPower(horizontal, n, identity, combine));
    }

    // j番目の上移動の直前に横移動をceil((m*j-b)/a)回終えている。
    const auto transposed = FloorWordProduct(height - 1, a, m, m + a - 1 - b,
                                             vertical, horizontal, identity, combine);
    const auto last_column = (m * height + a - 1 - b) / a;
    return combine(initial, combine(transposed, combine(vertical, FloorWordPower(horizontal, n - last_column, identity, combine))));
}
}

template <class T, class Combine>
T floor_path_product(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                     const T &horizontal, const T &vertical, const T &identity, const Combine &combine)
{
    if (m == 0)
    {
        throw std::invalid_argument("floor_path_product: positive modulus required");
    }
    return detail::FloorWordProduct(n, m, a, b, horizontal, vertical, identity, combine);
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

#include <cstdint>
#include <numeric>
#include <utility>

namespace
{
std::uint64_t Power(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1U) != 0)
        {
            result = result * base % modulus;
        }
        base = base * base % modulus;
    }
    return result;
}

std::uint64_t Inverse(std::uint64_t value, std::uint64_t modulus)
{
    auto divisor = static_cast<std::int64_t>(modulus);
    auto remainder = static_cast<std::int64_t>(value % modulus);
    std::int64_t coefficient = 0, next_coefficient = 1;
    while (remainder != 0)
    {
        const auto quotient = divisor / remainder;
        const auto next = divisor % remainder;
        divisor = std::exchange(remainder, next);
        coefficient = std::exchange(next_coefficient, coefficient - quotient * next_coefficient);
    }
    const auto signed_modulus = static_cast<std::int64_t>(modulus);
    return static_cast<std::uint64_t>((coefficient % signed_modulus + signed_modulus) % signed_modulus);
}

struct Geometric
{
    std::uint64_t multiplier;
    std::uint64_t sum;
};

std::uint64_t CoprimePart(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                          std::uint64_t base, std::uint64_t modulus)
{
    if (modulus == 1)
    {
        return 0;
    }
    const auto combine = [modulus](Geometric left, Geometric right)
    {
        return Geometric{left.multiplier * right.multiplier % modulus,
                         (left.sum + left.multiplier * right.sum) % modulus};
    };
    const Geometric horizontal{Power(base % modulus, a, modulus), 1};
    const Geometric vertical{Power(Inverse(base, modulus), m, modulus), 0};
    const auto word = sirkit::floor_path_product(n, m, a, b, horizontal, vertical, Geometric{1, 0}, combine);
    return word.sum * Power(base % modulus, b, modulus) % modulus;
}

std::uint64_t NilpotentPart(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                            std::uint64_t base, std::uint64_t modulus)
{
    const std::uint64_t gcd = std::gcd(a, std::uint64_t{m});
    const auto period = m / gcd;
    const auto inverse = Inverse(a / gcd, period);
    std::uint64_t answer = 0;
    // baseの冪が0になるまでに現れる剰余だけ数える。
    for (std::uint64_t r = 0, power = 1; r < m && power != 0; ++r, power = power * base % modulus)
    {
        const auto difference = (m + r - b) % m;
        if (difference % gcd == 0)
        {
            const auto first = (difference / gcd) * inverse % period;
            if (first < n)
            {
                const auto occurrences = 1 + (n - 1 - first) / period;
                answer = (answer + power * occurrences) % modulus;
            }
        }
    }
    return answer;
}
}

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint32_t n{}, m{};
        std::uint64_t a{}, b{}, base{}, modulus{};
        verify_in >> n >> m >> a >> b >> base >> modulus;
        auto unit = modulus;
        for (auto shared = std::gcd(unit, base); shared > 1; shared = std::gcd(unit, base))
        {
            unit /= shared;
        }
        const auto nilpotent = modulus / unit;
        const auto residue = CoprimePart(n, m, a, b, base, unit);
        if (nilpotent == 1)
        {
            verify_out << residue << '\n';
        }
        else
        {
            const auto other = NilpotentPart(n, m, a, b, base, nilpotent);
            const auto difference = (other + nilpotent - residue % nilpotent) % nilpotent;
            const auto offset = difference * Inverse(unit, nilpotent) % nilpotent;
            verify_out << residue + unit * offset << '\n';
        }
    }
}
