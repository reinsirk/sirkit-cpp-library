#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/sum_of_exponential_times_polynomial_limit
// Submission: https://judge.yosupo.jp/submission/419941

// Library: Math/Exponential_Polynomial_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exponential_polynomial_sum.md
//
// API一覧
// sum_exponential_polynomial(samples,r,n): f(0)..f(d)を渡し、i=0..n-1のr^i*f(i)の和を法998244353で返す。
// sum_exponential_polynomial_limit(samples,r): 同じ和の有理関数としての無限和。r≡1ならnullopt。
// sum_exponential_powers(r,d,n)/sum_exponential_powers_limit(r,d): f(i)=i^d。0^0=1。
// 空のsamplesはf=0、非空ならd+1<998244353。r,nはuint64_t。次数dについてO(d+1+log(n+1))時間・O(d+1)領域。
// べき乗版はi^dを線形篩で作る。整数剰余演算をO(1)として同じ計算量。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXPONENTIAL_POLYNOMIAL_SUM_DEFINED
#define SIRKIT_EXPONENTIAL_POLYNOMIAL_SUM_DEFINED
namespace detail
{
inline constexpr std::uint32_t kExponentialSumMod = 998244353;

inline std::uint32_t exponential_sum_add(std::uint32_t a, std::uint32_t b) noexcept
{
    const auto sum = a + b;
    return sum < kExponentialSumMod ? sum : sum - kExponentialSumMod;
}

inline std::uint32_t exponential_sum_subtract(std::uint32_t a, std::uint32_t b) noexcept
{
    return a >= b ? a - b : a + kExponentialSumMod - b;
}

inline std::uint32_t exponential_sum_multiply(std::uint32_t a, std::uint32_t b) noexcept
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kExponentialSumMod);
}

inline std::uint32_t exponential_sum_power(std::uint32_t a, std::uint64_t n) noexcept
{
    std::uint32_t result = 1;
    for (; n != 0; n >>= 1)
    {
        if (n & 1)
        {
            result = exponential_sum_multiply(result, a);
        }
        a = exponential_sum_multiply(a, a);
    }
    return result;
}

inline void check_exponential_sample_count(std::size_t count)
{
    if (count >= kExponentialSumMod)
    {
        throw std::length_error("exponential polynomial sum: expected fewer samples than the modulus");
    }
}

struct ExponentialFactorials
{
    std::vector<std::uint32_t> inverse;
    std::uint32_t factorial;

    explicit ExponentialFactorials(std::size_t degree) : inverse(degree + 1), factorial(1)
    {
        for (std::uint32_t i = 1; i <= degree; ++i)
        {
            factorial = exponential_sum_multiply(factorial, i);
        }
        inverse[degree] = exponential_sum_power(factorial, kExponentialSumMod - 2);
        for (auto i = degree; i != 0; --i)
        {
            inverse[i - 1] = exponential_sum_multiply(inverse[i], static_cast<std::uint32_t>(i));
        }
    }
};

inline void check_exponential_power_degree(std::size_t degree)
{
    if (degree >= kExponentialSumMod - 1)
    {
        throw std::length_error("exponential power sum: degree too large");
    }
}

inline std::vector<std::uint32_t> exponential_power_samples(std::size_t degree, std::size_t count)
{
    if (degree == 0)
    {
        return std::vector<std::uint32_t>(count, 1);
    }
    std::vector<std::uint32_t> result(count);
    if (count < 2)
    {
        return result;
    }
    std::vector<std::uint32_t> primes;
    result[1] = 1;
    for (std::uint32_t i = 2; i < count; ++i)
    {
        if (result[i] == 0)
        {
            result[i] = exponential_sum_power(i, degree);
            primes.push_back(i);
        }
        // primesにはiの最小素因数がある。そこまで、または積が上限に達するまで取る。
        for (std::size_t j = 0;; ++j)
        {
            const auto prime = primes[j];
            const auto next = std::uint64_t{i} * prime;
            if (next >= count)
            {
                break;
            }
            result[next] = exponential_sum_multiply(result[i], result[prime]);
            if (i % prime == 0)
            {
                break;
            }
        }
    }
    return result;
}

inline std::uint32_t exponential_direct(std::span<const std::uint32_t> samples, std::uint32_t ratio)
{
    std::uint32_t answer = 0, weight = 1;
    for (const auto value : samples)
    {
        answer = exponential_sum_add(answer, exponential_sum_multiply(value % kExponentialSumMod, weight));
        weight = exponential_sum_multiply(weight, ratio);
    }
    return answer;
}

inline void exponential_prefix_in_place(std::vector<std::uint32_t> &samples, std::uint32_t ratio)
{
    std::uint32_t answer = 0, weight = 1;
    for (auto &value : samples)
    {
        const auto current = exponential_sum_multiply(value % kExponentialSumMod, weight);
        value = answer;
        answer = exponential_sum_add(answer, current);
        weight = exponential_sum_multiply(weight, ratio);
    }
    samples.push_back(answer);
}

inline std::uint32_t exponential_interpolate(std::span<const std::uint32_t> samples,
                                             std::span<const std::uint32_t> inverse_factorial,
                                             std::uint32_t x)
{
    if (x < samples.size())
    {
        return samples[x];
    }
    const auto degree = samples.size() - 1;
    std::vector<std::uint32_t> prefix(samples.size(), 1);
    for (std::size_t i = 0; i < degree; ++i)
    {
        prefix[i + 1] = exponential_sum_multiply(prefix[i], x - static_cast<std::uint32_t>(i));
    }
    std::uint32_t suffix = 1, result = 0;
    for (auto j = samples.size(); j != 0; --j)
    {
        const auto i = j - 1;
        auto term = exponential_sum_multiply(samples[i], exponential_sum_multiply(prefix[i], suffix));
        term = exponential_sum_multiply(term, exponential_sum_multiply(inverse_factorial[i], inverse_factorial[degree - i]));
        result = (degree - i) % 2 == 0 ? exponential_sum_add(result, term) : exponential_sum_subtract(result, term);
        suffix = exponential_sum_multiply(suffix, x - static_cast<std::uint32_t>(i));
    }
    return result;
}

inline std::uint32_t exponential_constant(std::span<const std::uint32_t> prefix,
                                          const ExponentialFactorials &factorials,
                                          std::uint32_t inverse_ratio)
{
    const auto degree = prefix.size() - 1;
    const auto negate_inverse = kExponentialSumMod - inverse_ratio;
    std::uint32_t weight = 1, numerator = 0;
    for (std::size_t i = 0; i <= degree; ++i)
    {
        auto term = exponential_sum_multiply(prefix[i], weight);
        term = exponential_sum_multiply(term, exponential_sum_multiply(factorials.inverse[i], factorials.inverse[degree - i]));
        numerator = exponential_sum_add(numerator, term);
        weight = exponential_sum_multiply(weight, negate_inverse);
    }
    const auto denominator = exponential_sum_subtract(1, inverse_ratio);
    const auto reciprocal = exponential_sum_power(denominator, kExponentialSumMod - 1 - degree);
    return exponential_sum_multiply(exponential_sum_multiply(numerator, factorials.factorial), reciprocal);
}

inline std::uint32_t exponential_finite(std::vector<std::uint32_t> samples, std::uint32_t ratio,
                                        std::uint64_t n)
{
    if (samples.empty() || n == 0)
    {
        return 0;
    }
    if (ratio == 0)
    {
        return samples.front() % kExponentialSumMod;
    }
    const auto x = static_cast<std::uint32_t>(n % kExponentialSumMod);
    const auto terms = ratio == 1 ? x : n;
    if (terms <= samples.size())
    {
        return exponential_direct(std::span<const std::uint32_t>(samples).first(static_cast<std::size_t>(terms)), ratio);
    }

    const ExponentialFactorials factorials(samples.size());
    exponential_prefix_in_place(samples, ratio);
    if (ratio == 1)
    {
        return exponential_interpolate(samples, factorials.inverse, x);
    }

    // S(i)=C+r^i*q(i) でqは元のf以下の次数。qの次の有限差分が0となるようにCを求める。
    const auto inverse_ratio = exponential_sum_power(ratio, kExponentialSumMod - 2);
    const auto constant = exponential_constant(samples, factorials, inverse_ratio);
    samples.pop_back();
    std::uint32_t weight = 1;
    for (auto &value : samples)
    {
        value = exponential_sum_multiply(exponential_sum_subtract(value, constant), weight);
        weight = exponential_sum_multiply(weight, inverse_ratio);
    }
    const auto q = exponential_interpolate(samples, factorials.inverse, x);
    return exponential_sum_add(constant, exponential_sum_multiply(exponential_sum_power(ratio, n), q));
}

inline std::optional<std::uint32_t> exponential_limit(std::vector<std::uint32_t> samples, std::uint32_t ratio)
{
    if (ratio == 1)
    {
        return std::nullopt;
    }
    if (samples.empty())
    {
        return 0;
    }
    if (ratio == 0)
    {
        return samples.front() % kExponentialSumMod;
    }
    const ExponentialFactorials factorials(samples.size());
    exponential_prefix_in_place(samples, ratio);
    const auto inverse_ratio = exponential_sum_power(ratio, kExponentialSumMod - 2);
    return exponential_constant(samples, factorials, inverse_ratio);
}
} // namespace detail

[[nodiscard]] inline std::uint32_t sum_exponential_polynomial(std::span<const std::uint32_t> samples,
                                                              std::uint64_t ratio, std::uint64_t n)
{
    detail::check_exponential_sample_count(samples.size());
    return detail::exponential_finite({samples.begin(), samples.end()}, ratio % detail::kExponentialSumMod, n);
}

[[nodiscard]] inline std::optional<std::uint32_t> sum_exponential_polynomial_limit(
    std::span<const std::uint32_t> samples, std::uint64_t ratio)
{
    detail::check_exponential_sample_count(samples.size());
    return detail::exponential_limit({samples.begin(), samples.end()}, ratio % detail::kExponentialSumMod);
}

[[nodiscard]] inline std::uint32_t sum_exponential_powers(std::uint64_t ratio, std::size_t degree,
                                                          std::uint64_t n)
{
    detail::check_exponential_power_degree(degree);
    const auto r = static_cast<std::uint32_t>(ratio % detail::kExponentialSumMod);
    if (r == 0)
    {
        return n != 0 && degree == 0;
    }
    const auto terms = r == 1 ? n % detail::kExponentialSumMod : n;
    const auto count = degree + 1;
    if (terms <= count)
    {
        return detail::exponential_direct(detail::exponential_power_samples(degree, static_cast<std::size_t>(terms)), r);
    }
    return detail::exponential_finite(detail::exponential_power_samples(degree, count), r, n);
}

[[nodiscard]] inline std::optional<std::uint32_t> sum_exponential_powers_limit(std::uint64_t ratio,
                                                                               std::size_t degree)
{
    detail::check_exponential_power_degree(degree);
    const auto r = static_cast<std::uint32_t>(ratio % detail::kExponentialSumMod);
    if (r == 1)
    {
        return std::nullopt;
    }
    const auto count = r == 0 ? std::size_t{1} : degree + 1;
    return detail::exponential_limit(detail::exponential_power_samples(degree, count), r);
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
    std::uint64_t ratio;
    std::size_t degree;
    verify_in >> ratio >> degree;
    verify_out << *sum_exponential_powers_limit(ratio, degree) << '\n';
}
