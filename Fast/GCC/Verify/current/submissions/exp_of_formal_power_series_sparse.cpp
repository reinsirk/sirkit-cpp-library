#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse

// Library: Math/Sparse_Formal_Power_Series.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Sparse_Formal_Power_Series.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_formal_power_series.md
//
// API一覧
// SparseFpsTerm{degree, coefficient}: 形式的冪級数の1項。係数は法998244353で正規化する。
// 次数の順は任意。同次数の係数は足し、次数がn以上の項は無視する。結果は次数0から並べた長さnの係数列。
// sparse_fps_inverse(n, terms): 定数項が非零の級数の逆元。
// sparse_fps_exp(n, terms): 定数項が0の級数の形式指数。
// sparse_fps_log(n, terms): 定数項が1の級数の形式対数。
// sparse_fps_pow(n, terms, exponent): uint64_tの非負整数乗。0乗は1。
// sparse_fps_sqrt(n, terms): 平方根の1つ。存在しなければnullopt。
// 逆元以外はn<=998244353。
// n=0なら空。不正な定数項は通常版でinvalid_argument、積分可能な長さを超えるとlength_error。
// 入力項数Kに対しO(K log(K+1)+n(K+1))時間・O(n+K)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_FORMAL_POWER_SERIES_DEFINED
#define SIRKIT_SPARSE_FORMAL_POWER_SERIES_DEFINED

struct SparseFpsTerm
{
    std::size_t degree;
    std::uint32_t coefficient;
};

namespace detail
{
inline constexpr std::uint32_t kSparseFpsMod = 998244353;

inline std::uint32_t SparseFpsAdd(std::uint32_t a, std::uint32_t b)
{
    const auto sum = a + b;
    return sum >= kSparseFpsMod ? sum - kSparseFpsMod : sum;
}

inline std::uint32_t SparseFpsSubtract(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : kSparseFpsMod - (b - a);
}

inline std::uint32_t SparseFpsMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kSparseFpsMod);
}

inline std::uint32_t SparseFpsPower(std::uint32_t factor, std::uint64_t exponent)
{
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1) != 0)
        {
            result = SparseFpsMultiply(result, factor);
        }
        factor = SparseFpsMultiply(factor, factor);
    }
    return result;
}

inline std::vector<SparseFpsTerm> NormalizeSparseFps(std::size_t n, std::span<const SparseFpsTerm> input)
{
    std::vector<SparseFpsTerm> terms;
    terms.reserve(input.size());
    for (const auto &[degree, coefficient] : input)
    {
        if (degree < n)
        {
            terms.push_back({degree, coefficient % kSparseFpsMod});
        }
    }
    std::ranges::sort(terms, {}, &SparseFpsTerm::degree);
    std::size_t unique = 0;
    for (const auto [degree, coefficient] : terms)
    {
        if (unique > 0 && terms[unique - 1].degree == degree)
        {
            auto &existing = terms[unique - 1].coefficient;
            existing = SparseFpsAdd(existing, coefficient);
        }
        else
        {
            terms[unique++] = {degree, coefficient};
        }
    }
    terms.resize(unique);
    std::erase_if(terms, [](const auto &term)
                  {
                      return term.coefficient == 0;
                  });
    return terms;
}

inline std::vector<std::uint32_t> SparseFpsIntegerInverses(std::size_t n)
{
    std::vector<std::uint32_t> inverse(n);
    inverse[1] = 1;
    for (std::uint32_t i = 2; i < n; ++i)
    {
        inverse[i] = kSparseFpsMod - SparseFpsMultiply(kSparseFpsMod / i, inverse[kSparseFpsMod % i]);
    }
    return inverse;
}

inline std::uint32_t SparseFpsDot(std::size_t i, std::span<const SparseFpsTerm> terms,
                                  std::span<const std::uint32_t> values)
{
    // 16(p−1)^2+(p−1) < 2^64。積をまとめてから剰余を取る。
    constexpr unsigned kProductsPerReduction = 16;
    unsigned remaining = kProductsPerReduction;
    std::uint64_t sum = 0;
    for (const auto &[degree, coefficient] : terms)
    {
        if (degree > i)
        {
            break;
        }
        sum += std::uint64_t{coefficient} * values[i - degree];
        if (--remaining == 0)
        {
            sum %= kSparseFpsMod;
            remaining = kProductsPerReduction;
        }
    }
    return static_cast<std::uint32_t>(sum % kSparseFpsMod);
}

inline void CheckSparseFpsLength(std::size_t n)
{
    if (n > kSparseFpsMod)
    {
        throw std::length_error("sparse FPS: degree exceeds characteristic");
    }
}
// g = (f/a_0)^exponent に、指定した非零の定数項を掛けた級数を求める。
// f*g'=exponent*f'*g より、g_i = ((exponent+1)/i) Σ_j(j*f_j*g_{i-j}) - Σ_j(f_j*g_{i-j})。
inline void SparseFpsUnitPower(std::span<std::uint32_t> result, std::span<SparseFpsTerm> tail,
                               SparseFpsTerm leading, std::uint32_t exponent, std::uint32_t constant)
{
    result[0] = constant;
    if (result.size() == 1 || tail.empty())
    {
        return;
    }
    const auto inverse_leading = SparseFpsPower(leading.coefficient, kSparseFpsMod - 2);
    const auto factor = SparseFpsAdd(exponent, 1);
    std::vector<SparseFpsTerm> derivative;
    derivative.reserve(tail.size());
    for (auto &[degree, coefficient] : tail)
    {
        degree -= leading.degree;
        coefficient = SparseFpsMultiply(coefficient, inverse_leading);
        const auto weighted = SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
        derivative.push_back({degree, SparseFpsMultiply(weighted, factor)});
    }
    const auto inverse = SparseFpsIntegerInverses(result.size());
    for (std::size_t i = 1; i < result.size(); ++i)
    {
        const auto derivative_sum = SparseFpsMultiply(SparseFpsDot(i, derivative, result), inverse[i]);
        result[i] = SparseFpsSubtract(derivative_sum, SparseFpsDot(i, tail, result));
    }
}

// 998244353 = 119*2^23+1。非平方剰余3からTonelli–Shanks法で平方根を求める。
inline std::optional<std::uint32_t> SparseFpsScalarSquareRoot(std::uint32_t value)
{
    if (SparseFpsPower(value, (kSparseFpsMod - 1) / 2) != 1)
    {
        return std::nullopt;
    }
    constexpr unsigned kOdd = 119;
    unsigned depth = 23;
    auto root = SparseFpsPower(value, (kOdd + 1) / 2);
    auto residue = SparseFpsPower(value, kOdd);
    auto step = SparseFpsPower(3, kOdd);
    while (residue != 1)
    {
        unsigned order = 0;
        for (auto power = residue; power != 1; ++order)
        {
            power = SparseFpsMultiply(power, power);
        }
        auto adjust = step;
        for (unsigned i = order + 1; i < depth; ++i)
        {
            adjust = SparseFpsMultiply(adjust, adjust);
        }
        root = SparseFpsMultiply(root, adjust);
        step = SparseFpsMultiply(adjust, adjust);
        residue = SparseFpsMultiply(residue, step);
        depth = order;
    }
    return std::min(root, kSparseFpsMod - root);
}
} // namespace detail

inline std::vector<std::uint32_t> sparse_fps_inverse(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    auto terms = detail::NormalizeSparseFps(n, input);
    const auto inverse = detail::SparseFpsPower(terms.front().coefficient, detail::kSparseFpsMod - 2);
    std::vector<std::uint32_t> result(n);
    result[0] = inverse;
    if (terms.size() > 1)
    {
        auto tail = std::span(terms).subspan(1);
        for (auto &[degree, coefficient] : tail)
        {
            coefficient = detail::kSparseFpsMod - detail::SparseFpsMultiply(coefficient, inverse);
        }
        // f*g=1 の次数iの係数から、g[i]をそれより低い係数だけで求める。
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsDot(i, tail, result);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_exp(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
    std::vector<std::uint32_t> result(n);
    result[0] = 1;
    if (!terms.empty())
    {
        for (auto &[degree, coefficient] : terms)
        {
            coefficient = detail::SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
        }
        // g'=f'*g の次数i-1から、i*g[i]を求める。
        const auto inverse = detail::SparseFpsIntegerInverses(n);
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsMultiply(detail::SparseFpsDot(i, terms, result), inverse[i]);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_log(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
    std::vector<std::uint32_t> result(n);
    if (terms.size() > 1)
    {
        auto tail = std::span(terms).subspan(1);
        for (auto &[degree, coefficient] : tail)
        {
            result[degree] = detail::SparseFpsMultiply(static_cast<std::uint32_t>(degree), coefficient);
            coefficient = detail::kSparseFpsMod - coefficient;
        }
        // f*h=x*f' の解hを先に計算し、h_i/iで対数の係数を得る。
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsAdd(result[i], detail::SparseFpsDot(i, tail, result));
        }
        const auto inverse = detail::SparseFpsIntegerInverses(n);
        for (std::size_t i = 1; i < n; ++i)
        {
            result[i] = detail::SparseFpsMultiply(result[i], inverse[i]);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> sparse_fps_pow(std::size_t n, std::span<const SparseFpsTerm> input,
                                                 std::uint64_t exponent)
{
    if (n == 0)
    {
        return {};
    }
    detail::CheckSparseFpsLength(n);
    std::vector<std::uint32_t> result(n);
    if (exponent == 0)
    {
        result[0] = 1;
        return result;
    }
    auto terms = detail::NormalizeSparseFps(n, input);
    if (terms.empty() || terms.front().degree > (n - 1) / exponent)
    {
        return result;
    }
    const auto leading = terms.front();
    const auto shift = static_cast<std::size_t>(leading.degree * exponent);
    const auto constant = detail::SparseFpsPower(leading.coefficient, exponent);
    const auto field_exponent = static_cast<std::uint32_t>(exponent % detail::kSparseFpsMod);
    detail::SparseFpsUnitPower(std::span(result).subspan(shift), std::span(terms).subspan(1), leading,
                               field_exponent, constant);
    return result;
}

inline std::optional<std::vector<std::uint32_t>> sparse_fps_sqrt(std::size_t n, std::span<const SparseFpsTerm> input)
{
    if (n == 0)
    {
        return std::vector<std::uint32_t>{};
    }
    detail::CheckSparseFpsLength(n);
    auto terms = detail::NormalizeSparseFps(n, input);
    std::vector<std::uint32_t> result(n);
    if (terms.empty())
    {
        return result;
    }
    const auto leading = terms.front();
    if (leading.degree % 2 != 0)
    {
        return std::nullopt;
    }
    const auto constant = detail::SparseFpsScalarSquareRoot(leading.coefficient);
    if (!constant)
    {
        return std::nullopt;
    }
    constexpr auto kHalf = (detail::kSparseFpsMod + 1) / 2;
    detail::SparseFpsUnitPower(std::span(result).subspan(leading.degree / 2, n - leading.degree),
                               std::span(terms).subspan(1), leading, kHalf, *constant);
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
#include <vector>

int main()
{
    std::size_t n{}, k{};
    verify_in >> n >> k;
    std::vector<sirkit::SparseFpsTerm> terms(k);
    for (auto &[degree, coefficient] : terms)
    {
        verify_in >> degree >> coefficient;
    }
    const auto result = sirkit::sparse_fps_exp(n, terms);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
