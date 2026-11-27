#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/counting_squarefrees

// Library: Math/Counting_Squarefrees.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Counting_Squarefrees.cpp
// 数学的参考（Pawlewicz, Algorithm 2）: https://arxiv.org/abs/1107.4890
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/counting_squarefrees.md
//
// API一覧
// count_squarefrees(n): n以下の無平方な正整数の個数。n=0は0。nはuint64_t。
// メビウス関数の線形篩と商をまとめたMertens関数の漸化式を使う。
// 固定幅整数演算をO(1)として、O(n^(2/5))時間・O(n^(2/5))領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNTING_SQUAREFREES_DEFINED
#define SIRKIT_COUNTING_SQUAREFREES_DEFINED

namespace detail
{
inline std::uint64_t squarefree_isqrt(std::uint64_t value)
{
    std::uint64_t low = 0, high = std::uint64_t{1} << 32;
    while (high - low > 1)
    {
        const auto middle = (low + high) / 2;
        if (middle * middle <= value)
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

inline std::uint64_t squarefree_fifth_root(std::uint64_t value)
{
    std::uint64_t low = 0, high = std::uint64_t{1} << 13;
    while (high - low > 1)
    {
        const auto middle = (low + high) / 2;
        const auto square = middle * middle;
        if (middle <= value / (square * square))
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

inline std::vector<std::int32_t> squarefree_mobius_sieve(std::uint32_t limit)
{
    constexpr std::int32_t kUnvisited = 2;
    std::vector<std::int32_t> mobius(static_cast<std::size_t>(limit) + 1, kUnvisited);
    mobius[0] = 0;
    mobius[1] = 1;
    std::vector<std::uint32_t> primes;
    for (std::uint32_t i = 2; i <= limit; ++i)
    {
        if (mobius[i] == kUnvisited)
        {
            mobius[i] = -1;
            primes.push_back(i);
        }
        // 最小素因数は登録済み。最小素因数を処理するか、積が上限を超えると止まる。
        for (std::size_t j = 0;; ++j)
        {
            const auto prime = primes[j];
            const auto next = std::uint64_t{i} * prime;
            if (next > limit)
            {
                break;
            }
            if (i % prime == 0)
            {
                mobius[next] = 0;
                break;
            }
            mobius[next] = -mobius[i];
        }
    }
    return mobius;
}
} // namespace detail

[[nodiscard]] inline std::uint64_t count_squarefrees(std::uint64_t n)
{
    if (n == 0)
    {
        return 0;
    }

    const auto count_large = detail::squarefree_fifth_root(n);
    const auto limit = static_cast<std::uint32_t>(detail::squarefree_isqrt(n / count_large));
    auto prefix = detail::squarefree_mobius_sieve(limit);
    std::uint64_t result = 0;
    for (std::uint64_t d = 1; d <= limit; ++d)
    {
        if (prefix[d] != 0)
        {
            // 最終結果は[0,n]。途中の加減算は符号なし64ビットの剰余でよい。
            const auto term = n / (d * d);
            result += prefix[d] > 0 ? term : std::uint64_t{0} - term;
        }
        prefix[d] += prefix[d - 1];
    }

    std::vector<std::int64_t> large(count_large);
    std::int64_t correction = -static_cast<std::int64_t>(count_large - 1) * prefix.back();
    for (auto i = count_large - 1; i != 0; --i)
    {
        const auto x = detail::squarefree_isqrt(n / i);
        std::int64_t mertens = 1;
        for (std::uint64_t left = 2; left <= x;)
        {
            const auto quotient = x / left;
            const auto right = x / quotient;
            // floor(sqrt(n/i))/left の下限を取ると floor(sqrt(n/(i*left^2)))。
            const auto value = quotient <= limit ? prefix[quotient] : large[i * left * left];
            mertens -= static_cast<std::int64_t>(right - left + 1) * value;
            left = right + 1;
        }
        large[i] = mertens;
        correction += mertens;
    }
    return result + static_cast<std::uint64_t>(correction);
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
    std::uint64_t n;
    verify_in >> n;
    verify_out << count_squarefrees(n) << '\n';
}
