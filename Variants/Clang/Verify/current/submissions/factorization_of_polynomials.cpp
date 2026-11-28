#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/factorization_of_polynomials

// Library: Math/Polynomial_Factorization.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Polynomial_Factorization.cpp
// 参考（有限体上の因数分解）: https://shoup.net/ntb/ntb-v2.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_factorization.md
//
// API一覧
// [PolynomialFactor]
// coefficients, multiplicity: モニック既約因子の係数（低次順）と重複度。
// [PolynomialFactorization]
// leading_coefficient, factors: 元の最高次係数と、相異なる既約因子の列。
// factor_polynomial(coefficients,prime,seed): 素数primeを法として非零多項式を因数分解する。
// primeはuint32_t。係数は剰余へ正規化する。零多項式はdomain_error、合成数の法はinvalid_argument。
// Nは次数+1、Lは入力の係数数。期待O(L+N³ log(prime)+sqrt(prime))時間、O(L)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POLYNOMIAL_FACTORIZATION_DEFINED
#define SIRKIT_POLYNOMIAL_FACTORIZATION_DEFINED
struct PolynomialFactor
{
    std::vector<std::uint32_t> coefficients;
    std::size_t multiplicity;
};

struct PolynomialFactorization
{
    std::uint32_t leading_coefficient;
    std::vector<PolynomialFactor> factors;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
class PrimePolynomialFactorizer
{
    using Residue = std::uint32_t;
    using Polynomial = std::vector<Residue>;

    const Residue prime_;
    std::mt19937_64 engine_;
    std::vector<PolynomialFactor> factors_;

    static Residue CheckedPrime(Residue prime)
    {
        if (prime < 2)
        {
            throw std::invalid_argument("factor_polynomial: prime modulus required");
        }
        for (Residue divisor = 2; divisor <= prime / divisor; ++divisor)
        {
            if (prime % divisor == 0)
            {
                throw std::invalid_argument("factor_polynomial: prime modulus required");
            }
        }
        return prime;
    }

    static void Trim(Polynomial &value)
    {
        while (!value.empty() && value.back() == 0)
        {
            value.pop_back();
        }
    }

    [[nodiscard]] Residue add(Residue a, Residue b) const noexcept
    {
        const auto threshold = prime_ - b;
        return a >= threshold ? a - threshold : a + b;
    }

    [[nodiscard]] Residue subtract(Residue a, Residue b) const noexcept
    {
        return a >= b ? a - b : a + (prime_ - b);
    }

    [[nodiscard]] Residue multiply(Residue a, Residue b) const noexcept
    {
        return static_cast<Residue>(std::uint64_t{a} * b % prime_);
    }

    [[nodiscard]] Residue inverse(Residue a) const noexcept
    {
        Residue result = 1;
        for (auto exponent = prime_ - 2; exponent > 0; exponent >>= 1)
        {
            if ((exponent & 1U) != 0)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
        }
        return result;
    }

    void make_monic(Polynomial &value) const
    {
        if (value.back() != 1)
        {
            const auto scale = inverse(value.back());
            for (auto &coefficient : value)
            {
                coefficient = multiply(coefficient, scale);
            }
        }
    }

    void reduce(Polynomial &value, const Polynomial &modulus, Polynomial *quotient = nullptr) const
    {
        if (value.size() < modulus.size())
        {
            return;
        }
        const auto degree = modulus.size() - 1;
        const auto steps = value.size() - degree;
        const auto reciprocal = inverse(modulus.back());
        if (quotient != nullptr)
        {
            quotient->resize(steps);
        }
        for (auto i = steps; i != 0; --i)
        {
            const auto offset = i - 1;
            const auto coefficient = multiply(value[degree + offset], reciprocal);
            if (quotient != nullptr)
            {
                (*quotient)[offset] = coefficient;
            }
            if (coefficient != 0)
            {
                for (std::size_t j = 0; j < degree; ++j)
                {
                    value[offset + j] = subtract(value[offset + j], multiply(coefficient, modulus[j]));
                }
            }
        }
        value.resize(degree);
        Trim(value);
    }

    [[nodiscard]] Polynomial divide(Polynomial value, const Polynomial &divisor) const
    {
        Polynomial quotient;
        reduce(value, divisor, &quotient);
        return quotient;
    }

    [[nodiscard]] Polynomial gcd(Polynomial a, Polynomial b) const
    {
        while (!b.empty())
        {
            reduce(a, b);
            a.swap(b);
        }
        make_monic(a);
        return a;
    }

    [[nodiscard]] Polynomial product_mod(const Polynomial &a, const Polynomial &b, const Polynomial &modulus) const
    {
        if (a.empty() || b.empty())
        {
            return {};
        }
        Polynomial result(a.size() + b.size() - 1);
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            for (std::size_t j = 0; j < b.size(); ++j)
            {
                result[i + j] = add(result[i + j], multiply(a[i], b[j]));
            }
        }
        reduce(result, modulus);
        return result;
    }

    [[nodiscard]] Polynomial power_mod(Polynomial base, Residue exponent, const Polynomial &modulus) const
    {
        Polynomial result{1};
        while (exponent != 0)
        {
            if ((exponent & 1U) != 0)
            {
                result = product_mod(result, base, modulus);
            }
            exponent >>= 1;
            if (exponent != 0)
            {
                base = product_mod(base, base, modulus);
            }
        }
        return result;
    }

    [[nodiscard]] Polynomial split_equal_degree(const Polynomial &value, std::size_t degree)
    {
        std::uniform_int_distribution<Residue> random(0, prime_ - 1);
        while (true)
        {
            Polynomial sample(value.size() - 1);
            for (auto &coefficient : sample)
            {
                coefficient = random(engine_);
            }
            Trim(sample);
            Polynomial separator;
            if (prime_ == 2)
            {
                // 標数2ではトレースの値0と1で分ける。
                separator = sample;
                for (std::size_t i = 1; i < degree; ++i)
                {
                    sample = product_mod(sample, sample, value);
                    separator.resize(std::max(separator.size(), sample.size()));
                    for (std::size_t j = 0; j < sample.size(); ++j)
                    {
                        separator[j] ^= sample[j];
                    }
                }
                Trim(separator);
            }
            else
            {
                // (p^d-1)/2乗を、(p-1)/2乗とFrobenius写像で計算する。
                sample = power_mod(std::move(sample), (prime_ - 1) / 2, value);
                separator = sample;
                for (std::size_t i = 1; i < degree; ++i)
                {
                    sample = power_mod(std::move(sample), prime_, value);
                    separator = product_mod(separator, sample, value);
                }
                separator.resize(std::max(std::size_t{1}, separator.size()));
                separator[0] = subtract(separator[0], 1);
                Trim(separator);
            }
            auto factor = gcd(value, std::move(separator));
            if (factor.size() > 1 && factor.size() < value.size())
            {
                return factor;
            }
        }
    }

    void factor_same_degree(Polynomial value, std::size_t degree, std::size_t multiplicity)
    {
        std::vector<Polynomial> pending;
        pending.push_back(std::move(value));
        while (!pending.empty())
        {
            auto next = std::move(pending.back());
            pending.pop_back();
            if (next.size() == degree + 1)
            {
                factors_.push_back({std::move(next), multiplicity});
            }
            else
            {
                auto factor = split_equal_degree(next, degree);
                pending.push_back(divide(std::move(next), factor));
                pending.push_back(std::move(factor));
            }
        }
    }

    void factor_square_free(Polynomial value, std::size_t multiplicity)
    {
        Polynomial frobenius{0, 1};
        // x^(p^d)-xの既約因子は次数がdの約数。小さい次数から除く。
        for (std::size_t degree = 1; degree <= (value.size() - 1) / 2; ++degree)
        {
            frobenius = power_mod(std::move(frobenius), prime_, value);
            auto difference = frobenius;
            difference.resize(std::max(std::size_t{2}, difference.size()));
            difference[1] = subtract(difference[1], 1);
            Trim(difference);
            auto selected = gcd(value, std::move(difference));
            if (selected.size() > 1)
            {
                value = divide(std::move(value), selected);
                factor_same_degree(std::move(selected), degree, multiplicity);
                reduce(frobenius, value);
            }
        }
        if (value.size() > 1)
        {
            factors_.push_back({std::move(value), multiplicity});
        }
    }

    void remove_multiplicities(Polynomial value, std::size_t multiplicity_scale)
    {
        Polynomial derivative(value.size() - 1);
        for (std::size_t i = 1; i < value.size(); ++i)
        {
            derivative[i - 1] = multiply(value[i], static_cast<Residue>(i % prime_));
        }
        Trim(derivative);
        auto repeated = gcd(value, std::move(derivative));
        auto remaining = divide(std::move(value), repeated);
        for (std::size_t multiplicity = 1; remaining.size() > 1; ++multiplicity)
        {
            auto overlap = gcd(remaining, repeated);
            auto exact = divide(std::move(remaining), overlap);
            if (exact.size() > 1)
            {
                factor_square_free(std::move(exact), multiplicity * multiplicity_scale);
            }
            repeated = divide(std::move(repeated), overlap);
            remaining = std::move(overlap);
        }
        if (repeated.size() > 1)
        {
            // 微分が0の残りはp乗。指数をpで割った多項式に再帰する。
            Polynomial root((repeated.size() - 1) / prime_ + 1);
            for (std::size_t i = 0; i < root.size(); ++i)
            {
                root[i] = repeated[i * prime_];
            }
            remove_multiplicities(std::move(root), multiplicity_scale * prime_);
        }
    }

public:
    PrimePolynomialFactorizer(Residue prime, std::uint64_t seed) : prime_(CheckedPrime(prime)), engine_(seed)
    {
    }

    [[nodiscard]] PolynomialFactorization run(std::span<const Residue> coefficients)
    {
        Polynomial value(coefficients.begin(), coefficients.end());
        for (auto &coefficient : value)
        {
            coefficient %= prime_;
        }
        Trim(value);
        if (value.empty())
        {
            throw std::domain_error("factor_polynomial: zero polynomial");
        }
        const auto leading = value.back();
        make_monic(value);
        if (value.size() > 1)
        {
            remove_multiplicities(std::move(value), 1);
        }
        std::ranges::sort(factors_, {}, &PolynomialFactor::coefficients);
        return {leading, std::move(factors_)};
    }
};
}

inline PolynomialFactorization factor_polynomial(std::span<const std::uint32_t> coefficients, std::uint32_t prime,
                                                 std::uint64_t seed = std::random_device{}())
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PrimePolynomialFactorizer(prime, seed).run(coefficients);
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
    std::size_t degree{};
    std::uint32_t prime{};
    verify_in >> degree >> prime;
    std::vector<std::uint32_t> coefficients(degree + 1);
    for (auto &coefficient : coefficients)
    {
        verify_in >> coefficient;
    }
    const auto result = sirkit::factor_polynomial(coefficients, prime);
    verify_out << result.factors.size() << '\n';
    for (const auto &factor : result.factors)
    {
        verify_out << factor.multiplicity << ' ' << factor.coefficients.size() - 1;
        for (const auto coefficient : factor.coefficients)
        {
            verify_out << ' ' << coefficient;
        }
        verify_out << '\n';
    }
}
