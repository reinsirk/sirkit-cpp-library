#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/prefix_sum_of_binomial_coefficient

// Library: Math/Combination/Binomial_Prefix_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binomial_prefix_sum.md
//
// API一覧
// binomial_prefix_sums<Mod>(queries): (n,k)ごとにC(n,0)+…+C(n,k)をModで割った余りを返す。
// Modは奇素数で既定値は998244353。n<Mod。k>=nなら2^n。答えは入力順。
// 最大nをN、問い合わせ数をQとしてO(N+Q log(Q+1)+N sqrt(Q))時間、O(N+Q)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BINOMIAL_PREFIX_SUMS_DEFINED
#define SIRKIT_BINOMIAL_PREFIX_SUMS_DEFINED
namespace detail
{
template <std::uint32_t Mod>
struct BinomialPrefixArithmetic
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    static consteval bool is_odd_prime()
    {
        if (Mod < 3 || Mod % 2 == 0)
        {
            return false;
        }
        for (u32 divisor = 3; static_cast<u64>(divisor) * divisor <= Mod; divisor += 2)
        {
            if (Mod % divisor == 0)
            {
                return false;
            }
        }
        return true;
    }

    static u32 add(u32 left, u32 right) noexcept
    {
        const u64 sum = static_cast<u64>(left) + right;
        return static_cast<u32>(sum >= Mod ? sum - Mod : sum);
    }

    static u32 subtract(u32 left, u32 right) noexcept
    {
        return static_cast<u32>(left >= right ? left - right : static_cast<u64>(left) + Mod - right);
    }

    static u32 multiply(u32 left, u32 right) noexcept
    {
        return static_cast<u32>(static_cast<u64>(left) * right % Mod);
    }

    static u32 invert(u32 base) noexcept
    {
        u32 result = 1;
        for (u32 exponent = Mod - 2; exponent > 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                result = multiply(result, base);
            }
            base = multiply(base, base);
        }
        return result;
    }

    static u32 halve(u32 value) noexcept
    {
        const auto adjustment = (value & 1u) != 0 ? Mod : 0u;
        return static_cast<u32>((static_cast<u64>(value) + adjustment) / 2);
    }
};
}

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::uint32_t> binomial_prefix_sums(
    std::span<const std::pair<std::uint32_t, std::uint32_t>> queries)
{
    using u32 = std::uint32_t;
    using Arithmetic = detail::BinomialPrefixArithmetic<Mod>;
    static_assert(Arithmetic::is_odd_prime(), "binomial_prefix_sums: odd prime modulus required");
    if (queries.empty())
    {
        return {};
    }
    u32 max_n = 0;
    for (const auto &[n, k] : queries)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n >= Mod)
        {
            throw std::out_of_range("binomial_prefix_sums: n must be below Mod");
        }
#endif
        max_n = std::max(max_n, n);
    }
    const auto table_size = static_cast<std::size_t>(max_n) + 1;
    std::vector<u32> factorial(table_size, 1), inverse(table_size), powers_of_two(table_size, 1);
    for (std::size_t i = 1; i < table_size; ++i)
    {
        factorial[i] = Arithmetic::multiply(factorial[i - 1], static_cast<u32>(i));
        powers_of_two[i] = Arithmetic::add(powers_of_two[i - 1], powers_of_two[i - 1]);
    }
    inverse[max_n] = Arithmetic::invert(factorial[max_n]);
    for (u32 i = max_n; i > 0; --i)
    {
        inverse[i - 1] = Arithmetic::multiply(inverse[i], i);
    }
    const auto choose = [&](u32 n, u32 k)
    {
        return Arithmetic::multiply(Arithmetic::multiply(factorial[n], inverse[k]), inverse[n - k]);
    };

    struct Query
    {
        u32 n;
        u32 k;
        bool reflected;
        std::size_t input_index;
        std::uint64_t sort_key;
    };
    const auto block_size = std::max(u32{1}, static_cast<u32>(max_n / std::sqrt(static_cast<double>(queries.size()))));
    std::vector<u32> result(queries.size());
    std::vector<Query> ordered;
    ordered.reserve(queries.size());
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [n, k] = queries[i];
        if (k >= n)
        {
            result[i] = powers_of_two[n];
            continue;
        }
        const bool reflected = k > n - 1 - k;
        const auto bound = reflected ? n - 1 - k : k;
        const auto block = n / block_size;
        const auto order_k = (block & 1u) == 0 ? bound : max_n - bound;
        const auto key = (static_cast<std::uint64_t>(block) << 32) | order_k;
        ordered.push_back({n, bound, reflected, i, key});
    }
    std::sort(ordered.begin(), ordered.end(), [](const Query &left, const Query &right)
              {
                  return left.sort_key < right.sort_key;
              });

    u32 n = 0, k = 0, sum = 1;
    for (const auto &query : ordered)
    {
        // nの増加とkの減少を先に行い、途中でもk<=nを維持する。
        while (n < query.n)
        {
            sum = Arithmetic::subtract(Arithmetic::add(sum, sum), choose(n, k));
            ++n;
        }
        while (k > query.k)
        {
            sum = Arithmetic::subtract(sum, choose(n, k));
            --k;
        }
        while (k < query.k)
        {
            ++k;
            sum = Arithmetic::add(sum, choose(n, k));
        }
        while (n > query.n)
        {
            --n;
            sum = Arithmetic::halve(Arithmetic::add(sum, choose(n, k)));
        }
        result[query.input_index] = query.reflected ? Arithmetic::subtract(powers_of_two[n], sum) : sum;
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

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::uint32_t count{};
    verify_in >> count;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(count);
    for (auto &[n, k] : queries)
    {
        verify_in >> n >> k;
    }
    for (const auto value : sirkit::binomial_prefix_sums(queries))
    {
        verify_out << value << '\n';
    }
}
