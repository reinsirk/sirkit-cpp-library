#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/log_of_set_power_series
// Submission: https://judge.yosupo.jp/submission/419860

// Library: Math/Set_Power_Series.cpp
// SPDX-License-Identifier: CC0-1.0
// 部分集合畳み込みの参考: Björklund et al., Fourier Meets Möbius: Fast Subset Convolution (2007).
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/set_power_series.md
//
// API一覧
// subset_convolution(a,b): maskごとに、要素を重複なく分ける組の a[sub]*b[mask^sub] の和を返す。
// set_series_exp(f): 定数項（添字0）が0の集合冪級数を指数化する。
// set_series_log(f): 定数項（添字0）が1の集合冪級数の対数を取る。
// いずれも法998244353。同じ長さ2^nのuint32_t列を渡し、nは0..20。uint32_t全域を法で正規化する。
// 要素は集合のビットマスクで添字を付ける。nを全集合の要素数とし、各O((n+1)^2*2^n)時間・O((n+1)*2^n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SET_POWER_SERIES_DEFINED
#define SIRKIT_SET_POWER_SERIES_DEFINED
namespace detail
{
inline constexpr std::uint32_t kSetSeriesMod = 998244353;
inline constexpr std::size_t kSetSeriesMaxSize = std::size_t{1} << 20;

inline void CheckSetSeriesSize(std::size_t size)
{
    if (!std::has_single_bit(size) || size > kSetSeriesMaxSize)
    {
        throw std::length_error("set power series: expected a power of two up to 2^20");
    }
}

inline std::vector<std::uint32_t> SetSubsetProduct(std::span<const std::uint32_t> a,
                                                   std::span<const std::uint32_t> b)
{
    const std::size_t n = static_cast<std::size_t>(std::countr_zero(a.size()));
    const std::size_t width = n + 1;
    std::vector<std::uint8_t> ranks(a.size());
    std::vector<std::uint32_t> left(a.size() * width), right(left.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        ranks[i] = static_cast<std::uint8_t>(std::popcount(i));
        left[i * width + ranks[i]] = a[i] % kSetSeriesMod;
        right[i * width + ranks[i]] = b[i] % kSetSeriesMod;
    }

    // 集合の大きさ別に両列のゼータ変換を計算する。
    for (std::size_t step = 1; step < a.size(); step *= 2)
    {
        for (std::size_t block = 0; block < a.size(); block += step * 2)
        {
            for (std::size_t i = block; i < block + step; ++i)
            {
                const auto src = i * width;
                const auto dest = src + step * width;
                for (std::size_t d = 0; d <= ranks[i]; ++d)
                {
                    const auto x = left[dest + d] + left[src + d];
                    const auto y = right[dest + d] + right[src + d];
                    left[dest + d] = x >= kSetSeriesMod ? x - kSetSeriesMod : x;
                    right[dest + d] = y >= kSetSeriesMod ? y - kSetSeriesMod : y;
                }
            }
        }
    }

    // 逆変換で使う次数のみ掛ける。大きい次数から上書きすると、入力を別に保存せずに済む。
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        const std::size_t rank = ranks[i];
        auto *x = left.data() + i * width;
        const auto *y = right.data() + i * width;
        for (std::size_t d = std::min(n, rank * 2) + 1; d > rank;)
        {
            --d;
            const auto first = d - rank;
            std::uint64_t sum = 0;
            // 法未満の積は18項までuint64_tに収まる。長い区間だけ途中で剰余を取る。
            auto j = first;
            if (rank - first >= 17)
            {
                for (; j < first + 16; ++j)
                {
                    sum += std::uint64_t{x[j]} * y[d - j];
                }
                sum %= kSetSeriesMod;
            }
            for (; j <= rank; ++j)
            {
                sum += std::uint64_t{x[j]} * y[d - j];
            }
            x[d] = static_cast<std::uint32_t>(sum % kSetSeriesMod);
        }
    }

    for (std::size_t step = 1; step < a.size(); step *= 2)
    {
        for (std::size_t block = 0; block < a.size(); block += step * 2)
        {
            for (std::size_t i = block; i < block + step; ++i)
            {
                const auto src = i * width;
                const auto dest = src + step * width;
                for (std::size_t d = static_cast<std::size_t>(ranks[i]) + 1; d <= n; ++d)
                {
                    const auto x = left[dest + d];
                    const auto y = left[src + d];
                    left[dest + d] = x >= y ? x - y : x + kSetSeriesMod - y;
                }
            }
        }
    }
    std::vector<std::uint32_t> result(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        result[i] = left[i * width + ranks[i]];
    }
    return result;
}
}

inline std::vector<std::uint32_t> subset_convolution(std::span<const std::uint32_t> a,
                                                     std::span<const std::uint32_t> b)
{
    detail::CheckSetSeriesSize(a.size());
#ifndef SKIP_RUNTIME_CHECKS
    if (a.size() != b.size())
    {
        throw std::invalid_argument("subset_convolution: size mismatch");
    }
#endif
    return detail::SetSubsetProduct(a, b);
}

inline std::vector<std::uint32_t> set_series_exp(std::span<const std::uint32_t> f)
{
    detail::CheckSetSeriesSize(f.size());
    if (f[0] % detail::kSetSeriesMod != 0)
    {
        throw std::domain_error("set_series_exp: constant coefficient must be zero modulo 998244353");
    }
    std::vector<std::uint32_t> result(f.size());
    result[0] = 1;
    // x^2=0ならexp(f0 + x f1)=exp(f0)(1 + x f1)。変数を1個ずつ追加する。
    for (std::size_t length = 1; length < f.size(); length *= 2)
    {
        const auto high = detail::SetSubsetProduct(std::span<const std::uint32_t>(result).first(length),
                                                   f.subspan(length, length));
        std::copy(high.begin(), high.end(), result.begin() + length);
    }
    return result;
}

inline std::vector<std::uint32_t> set_series_log(std::span<const std::uint32_t> f)
{
    detail::CheckSetSeriesSize(f.size());
    if (f[0] % detail::kSetSeriesMod != 1)
    {
        throw std::domain_error("set_series_log: constant coefficient must be one modulo 998244353");
    }
    std::vector<std::uint32_t> result(f.size());
    std::vector<std::uint32_t> inverse{1};
    inverse.reserve(f.size() / 2);
    // log(f0 + x f1)=log(f0) + x f1/f0。逆元の追加分は -x f1/f0^2。
    for (std::size_t length = 1; length < f.size(); length *= 2)
    {
        const auto high = detail::SetSubsetProduct(inverse, f.subspan(length, length));
        std::copy(high.begin(), high.end(), result.begin() + length);
        if (length * 2 < f.size())
        {
            const auto next = detail::SetSubsetProduct(inverse, high);
            for (const auto x : next)
            {
                inverse.push_back(x == 0 ? 0 : detail::kSetSeriesMod - x);
            }
        }
    }
    return result;
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
#include <vector>

int main()
{
    unsigned n;
    verify_in >> n;
    std::vector<std::uint32_t> a(std::size_t{1} << n);
    for (auto &x : a)
    {
        verify_in >> x;
    }
    const auto result = set_series_log(a);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
