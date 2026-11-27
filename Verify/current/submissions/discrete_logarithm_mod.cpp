#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/discrete_logarithm_mod
// Submission: https://judge.yosupo.jp/submission/419642

// Library: Math/Discrete_Logarithm.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（Baby-step Giant-step法）: https://flintlib.org/doc/algorithms/number-theory/discrete-log-tables.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/discrete_logarithm.md
//
// API一覧
// discrete_logarithm_mod(base,target,modulus): base^k≡target (mod modulus) の最小のk>=0。
// 解なしはstd::nullopt。0^0=1、法は1以上のuint32_t、baseとtargetは法で縮約する。
// baseと法が互いに素でなくても可。ハッシュ表が平均O(1)ならO(sqrt(modulus))時間。
// 最悪O(modulus)時間、追加O(sqrt(modulus))領域。
// 検査有効時、法が0ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DISCRETE_LOGARITHM_MOD_DEFINED
#define SIRKIT_DISCRETE_LOGARITHM_MOD_DEFINED

inline std::optional<std::uint32_t> discrete_logarithm_mod(
    std::uint32_t base, std::uint32_t target, std::uint32_t modulus)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (modulus == 0)
    {
        throw std::invalid_argument("discrete_logarithm_mod: zero modulus");
    }
#endif
    target %= modulus;
    auto factor = std::uint32_t{1} % modulus;
    std::uint32_t prefix = 0;
    if (target == factor)
    {
        return prefix;
    }
    base %= modulus;

    const auto multiply = [](std::uint32_t a, std::uint32_t b, std::uint32_t mod)
    {
        return static_cast<std::uint32_t>((std::uint64_t{a} * b) % mod);
    };
    // baseと法の共通因子を除き、factor * base^k ≡ target の形にする。
    for (auto divisor = std::gcd(base, modulus); divisor > 1; divisor = std::gcd(base, modulus))
    {
        if (target % divisor != 0)
        {
            return std::nullopt;
        }
        modulus /= divisor;
        target /= divisor;
        factor = multiply(factor, base / divisor, modulus);
        ++prefix;
        if (target == factor)
        {
            return prefix;
        }
    }
    if (std::gcd(target, modulus) != 1)
    {
        return std::nullopt;
    }

    // 指数が小さい場合や周期が短い場合は表を確保せずに終える。
    auto probe = factor;
    constexpr std::uint32_t kProbeLimit = 8;
    for (std::uint32_t exponent = 1; exponent <= kProbeLimit; ++exponent)
    {
        probe = multiply(probe, base, modulus);
        if (probe == target)
        {
            return prefix + exponent;
        }
        if (probe == factor)
        {
            return std::nullopt;
        }
    }

    std::uint32_t low = 0, block_size = 65'536;
    while (block_size - low > 1)
    {
        const auto mid = low + (block_size - low) / 2;
        if (std::uint64_t{mid} * mid < modulus)
        {
            low = mid;
        }
        else
        {
            block_size = mid;
        }
    }

    const std::size_t capacity = std::bit_ceil(2U * block_size);
    const auto index = [shift = 64 - std::countr_zero(capacity)](std::uint32_t residue)
    {
        constexpr std::uint64_t kMultiplier = 0x9e3779b97f4a7c15ULL;
        return static_cast<std::size_t>((residue * kMultiplier) >> shift);
    };
    // 残余と指数+1を1枠に詰める。下位32ビットが0の枠は空。
    std::vector<std::uint64_t> offsets(capacity);
    const auto find_offset = [&](std::uint32_t residue)
    {
        auto slot = index(residue);
        while (const auto entry = offsets[slot])
        {
            if ((entry >> 32) == residue)
            {
                return static_cast<std::uint32_t>(entry);
            }
            slot = (slot + 1) & (capacity - 1);
        }
        return std::uint32_t{0};
    };
    auto baby = target;
    for (std::uint32_t offset = 0; offset < block_size; ++offset)
    {
        auto slot = index(baby);
        while (offsets[slot] != 0)
        {
            slot = (slot + 1) & (capacity - 1);
        }
        offsets[slot] = (std::uint64_t{baby} << 32) | (offset + 1);
        baby = multiply(baby, base, modulus);
        if (baby == target)
        {
            // 1ブロックで周期を一周できる。残りの指数は同じ値を繰り返す。
            const auto hit = find_offset(factor);
            if (hit == 0)
            {
                return std::nullopt;
            }
            return prefix + (offset + 2 - hit);
        }
    }

    auto multiplier = std::uint32_t{1};
    auto power = base;
    for (auto exponent = block_size; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1) != 0)
        {
            multiplier = multiply(multiplier, power, modulus);
        }
        power = multiply(power, power, modulus);
    }
    // 境界を昇順に調べると、最初に見つかったブロックに最小の指数がある。
    auto giant = factor;
    for (std::uint64_t boundary = block_size; boundary - block_size < modulus; boundary += block_size)
    {
        giant = multiply(giant, multiplier, modulus);
        const auto hit = find_offset(giant);
        if (hit != 0)
        {
            return prefix + static_cast<std::uint32_t>(boundary + 1 - hit);
        }
    }
    return std::nullopt;
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

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint32_t base{}, target{}, modulus{};
        verify_in >> base >> target >> modulus;
        const auto exponent = discrete_logarithm_mod(base, target, modulus);
        verify_out << (exponent ? static_cast<std::int64_t>(*exponent) : -1) << '\n';
    }
}
