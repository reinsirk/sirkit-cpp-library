#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/sum_of_multiplicative_function

// Library: Math/Multiplicative_Prefix_Sums.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Multiplicative_Prefix_Sums.cpp
// 参考（Lucy DPと簡略化したMin_25篩）: https://atcoder.jp/contests/abc370/editorial/10906
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multiplicative_prefix_sums.md
//
// API一覧
// [MultiplicativePrefixSums]
// MultiplicativePrefixSums(n): n>=1を固定し、sqrt(n)以下の素数を準備する。O(sqrt(n) log log(n+2))時間。
// size(), quotient(index): 相異なるfloor(n/i)の個数、昇順0始まりの座標。各O(1)時間。
// prime_prefix_sums(weight, prefix): 完全乗法的weightの素数に限った累積和。prefix(x)は1..xのweightの総和。
// prefix_sums(prime_sums, prime_power): 素数での累積和とf(p^e)を受け取り、f(1)=1の乗法的関数の累積和を返す。
// prefix_sum(prime_sums, prime_power): n以下の総和1個だけを、最小素因数の列挙で求める。
// 各コールバックとTの演算をO(1)とする。prime_prefix_sumsとprefix_sumsは各O(n^(3/4)/log(n+2))時間。
// prefix_sumは探索する素因数列の個数に比例し、最悪O(n)時間。すべてO(sqrt(n))作業領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MULTIPLICATIVE_PREFIX_SUMS_DEFINED
#define SIRKIT_MULTIPLICATIVE_PREFIX_SUMS_DEFINED
class MultiplicativePrefixSums
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    u64 n_;
    u64 small_;
    u64 large_;
    std::vector<u32> primes_;

    static u64 square_root(u64 value) noexcept
    {
        u64 upper = u64{1} << ((std::bit_width(value) + 1) / 2);
        for (u64 next = (upper + value / upper) / 2; next < upper; next = (upper + value / upper) / 2)
        {
            upper = next;
        }
        return upper;
    }

    [[nodiscard]] u64 coordinate(u64 index) const noexcept
    {
        return index < small_ ? index + 1 : n_ / (size() - index);
    }

    [[nodiscard]] u64 rank(u64 value) const noexcept
    {
        return value <= small_ ? value - 1 : size() - n_ / value;
    }

    // v>=p^2を降順に処理する。v/pも必ず同じ商の座標に含まれる。
    template <class Update>
    void process(u64 p, const Update &update) const
    {
        const auto square = p * p;
        const auto end = std::min(large_, n_ / square);
        const auto large_end = std::min(end, large_ / p);
        for (u64 d = 1; d <= large_end; ++d)
        {
            const auto product = d * p;
            update(size() - d, size() - product, n_ / product);
        }
        for (u64 d = large_end + 1; d <= end; ++d)
        {
            const auto divided = n_ / (d * p);
            update(size() - d, divided - 1, divided);
        }
        for (u64 value = small_; value >= square; --value)
        {
            const auto divided = static_cast<u32>(value) / static_cast<u32>(p);
            update(value - 1, divided - 1, divided);
        }
    }

    static u64 compact_quotient(u64 dividend, u64 divisor) noexcept
    {
        // 呼び出し時は0<divisor<=dividend。被除数が32ビットなら除数も32ビットに収まる。
        if (dividend <= std::numeric_limits<u32>::max())
        {
            return static_cast<u32>(dividend) / static_cast<u32>(divisor);
        }
        return dividend / divisor;
    }

    void check_prime_table_size(std::size_t length) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (length != size())
        {
            throw std::invalid_argument("MultiplicativePrefixSums: prime prefix table size");
        }
#else
        (void)length;
#endif
    }

    template <class T>
    struct CachedPrime
    {
        u64 prime;
        u64 square;
        T prefix;
        T first_power;
        T second_power;
    };

    [[nodiscard]] u64 divisor_rank(u64 value, u64 denominator) const noexcept
    {
        return denominator <= large_ ? size() - denominator : value - 1;
    }

    template <class T, class PrimePower>
    [[nodiscard]] T sum_without_one(u64 value, u64 denominator, std::size_t first,
                                    const std::vector<CachedPrime<T>> &rows, const std::vector<T> &prime_sums,
                                    const PrimePower &prime_power) const
    {
        const auto below = first == 0 ? T{} : rows[first - 1].prefix;
        T answer = prime_sums[divisor_rank(value, denominator)] - below;
        for (auto i = first; i < rows.size(); ++i)
        {
            const auto &row = rows[i];
            const auto prime = row.prime;
            if (row.square > value)
            {
                break;
            }
            const auto divided = compact_quotient(value, prime);
            const auto next_denominator = denominator * prime;
            if (divided < row.square)
            {
                answer += row.first_power * (prime_sums[divisor_rank(divided, next_denominator)] - row.prefix) + row.second_power;
                continue;
            }

            answer += row.first_power * sum_without_one(divided, next_denominator, i + 1, rows, prime_sums, prime_power) + row.second_power;
            auto current = compact_quotient(divided, prime);
            auto divisor = next_denominator * prime;
            auto power = row.second_power;
            for (unsigned exponent = 3; current >= prime; ++exponent)
            {
                const auto next_power = prime_power(prime, exponent);
                if (current < row.square)
                {
                    answer += power * (prime_sums[divisor_rank(current, divisor)] - row.prefix) + next_power;
                }
                else
                {
                    answer += power * sum_without_one(current, divisor, i + 1, rows, prime_sums, prime_power) + next_power;
                }
                current = compact_quotient(current, prime);
                divisor *= prime;
                power = next_power;
            }
        }
        return answer;
    }

public:
    explicit MultiplicativePrefixSums(u64 n) : n_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n == 0)
        {
            throw std::invalid_argument("MultiplicativePrefixSums: positive n required");
        }
#endif
        small_ = square_root(n);
        large_ = n / (small_ + 1);
        if constexpr (sizeof(std::size_t) < sizeof(u64))
        {
            if (size() >= std::numeric_limits<std::size_t>::max())
            {
                throw std::length_error("MultiplicativePrefixSums: table size");
            }
        }
        std::vector<bool> composite(static_cast<std::size_t>(small_) + 1);
        for (u64 p = 2; p <= small_; ++p)
        {
            if (composite[p])
            {
                continue;
            }
            primes_.push_back(static_cast<u32>(p));
            for (u64 multiple = p * p; multiple <= small_; multiple += p)
            {
                composite[multiple] = true;
            }
        }
    }

    [[nodiscard]] u64 size() const noexcept
    {
        return small_ + large_;
    }

    [[nodiscard]] u64 quotient(u64 index) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= size())
        {
            throw std::out_of_range("MultiplicativePrefixSums: quotient index");
        }
#endif
        return coordinate(index);
    }

    template <class Weight, class Prefix>
    [[nodiscard]] auto prime_prefix_sums(const Weight &weight, const Prefix &prefix) const
    {
        using T = std::remove_cvref_t<decltype(prefix(u64{1}))>;
        const auto total = size();
        std::vector<T> values(static_cast<std::size_t>(total));
        for (u64 index = 0; index < total; ++index)
        {
            values[index] = prefix(coordinate(index)) - T{1};
        }
        for (const u64 prime : primes_)
        {
            const auto lower_primes = values[prime - 2];
            const auto prime_weight = weight(prime);
            const auto end = std::min(large_, compact_quotient(n_, prime * prime));
            const auto direct = std::min(end, large_ / prime);
            for (u64 d = 1, source = prime; d <= direct; ++d, source += prime)
            {
                values[total - d] -= prime_weight * (values[total - source] - lower_primes);
            }

            const auto numerator = compact_quotient(n_, prime);
            const auto split = std::min(end, 2 * square_root(numerator));
            auto d = direct + 1;
            for (; d <= split; ++d)
            {
                const auto source = compact_quotient(numerator, d);
                values[total - d] -= prime_weight * (values[source - 1] - lower_primes);
            }
            while (d <= end)
            {
                const auto source = compact_quotient(numerator, d);
                const auto last = std::min(end, compact_quotient(numerator, source)); // 同じ商になる範囲の右端。
                const auto correction = prime_weight * (values[source - 1] - lower_primes);
                for (; d <= last; ++d)
                {
                    values[total - d] -= correction;
                }
            }

            auto remaining = small_;
            for (u64 source = small_ / prime; source >= prime; --source)
            {
                const auto first = source * prime;
                const auto correction = prime_weight * (values[source - 1] - lower_primes);
                for (; remaining >= first; --remaining)
                {
                    values[remaining - 1] -= correction;
                }
            }
        }
        return values;
    }

    template <class T, class PrimePower>
    [[nodiscard]] T prefix_sum(const std::vector<T> &prime_sums, const PrimePower &prime_power) const
    {
        check_prime_table_size(prime_sums.size());
        std::vector<CachedPrime<T>> rows;
        rows.reserve(primes_.size());
        for (const u64 prime : primes_)
        {
            rows.push_back({prime, prime * prime, prime_sums[prime - 1], prime_power(prime, 1), prime_power(prime, 2)});
        }
        return T{1} + sum_without_one(n_, 1, 0, rows, prime_sums, prime_power);
    }

    template <class T, class PrimePower>
    [[nodiscard]] std::vector<T> prefix_sums(std::vector<T> prime_sums, const PrimePower &prime_power) const
    {
        check_prime_table_size(prime_sums.size());
        for (auto it = primes_.rbegin(); it != primes_.rend(); ++it)
        {
            const u64 prime = *it;
            const auto prime_square = prime * prime;
            const auto lower_primes = prime_sums[prime - 1];
            std::array<T, 64> powers{};
            for (u64 left = n_, exponent = 1; left >= prime; left /= prime, ++exponent)
            {
                powers[exponent] = prime_power(prime, static_cast<unsigned>(exponent));
            }
            process(prime, [&](u64 target, u64 divided_rank, u64 divided)
                    {
                        // 最小素因数がprimeの合成数を、primeの指数で分けて戻す。
                        T value = powers[1] * (prime_sums[divided_rank] - lower_primes) + powers[2];
                        for (unsigned e = 2; divided >= prime_square; ++e)
                        {
                            divided /= prime;
                            value += powers[e] * (prime_sums[rank(divided)] - lower_primes) + powers[e + 1];
                        }
                        prime_sums[target] += value;
                    });
        }
        for (auto &value : prime_sums)
        {
            value += T{1};
        }
        return prime_sums;
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

namespace
{
class Residue
{
    static constexpr std::uint32_t kModulus = 469762049;
    std::uint32_t value_;

public:
    Residue(std::uint64_t value = 0) : value_(static_cast<std::uint32_t>(value % kModulus))
    {
    }

    Residue &operator+=(Residue other)
    {
        value_ += other.value_;
        value_ -= value_ >= kModulus ? kModulus : 0;
        return *this;
    }

    Residue &operator-=(Residue other)
    {
        value_ += value_ < other.value_ ? kModulus : 0;
        value_ -= other.value_;
        return *this;
    }

    friend Residue operator+(Residue left, Residue right)
    {
        return left += right;
    }

    friend Residue operator-(Residue left, Residue right)
    {
        return left -= right;
    }

    friend Residue operator*(Residue left, Residue right)
    {
        return Residue{static_cast<std::uint64_t>(left.value_) * right.value_};
    }

    [[nodiscard]] std::uint32_t value() const
    {
        return value_;
    }
};

struct PrimeSums
{
    Residue count;
    Residue sum;

    explicit PrimeSums(std::uint64_t value = 0) : count(value), sum(value)
    {
    }

    PrimeSums(Residue count_value, Residue sum_value) : count(count_value), sum(sum_value)
    {
    }

    PrimeSums &operator-=(PrimeSums other)
    {
        count -= other.count;
        sum -= other.sum;
        return *this;
    }

    friend PrimeSums operator-(PrimeSums left, PrimeSums right)
    {
        return left -= right;
    }

    friend PrimeSums operator*(PrimeSums left, PrimeSums right)
    {
        return {left.count * right.count, left.sum * right.sum};
    }
};
}

int main()
{
    int count{};
    verify_in >> count;
    while (count-- > 0)
    {
        std::uint64_t n{};
        std::uint32_t a_raw{}, b_raw{};
        verify_in >> n >> a_raw >> b_raw;
        const Residue a = a_raw, b = b_raw;
        const sirkit::MultiplicativePrefixSums sieve(n);
        std::vector<Residue> values;
        {
            const auto prime_sums = sieve.prime_prefix_sums([](std::uint64_t p)
                                                            {
                                                                return PrimeSums{Residue{1}, Residue{p}};
                                                            },
                                                            [](std::uint64_t x)
                                                            {
                                                                const auto sum = x % 2 != 0 ? Residue{x} * Residue{x / 2 + 1} : Residue{x / 2} * Residue{x + 1};
                                                                return PrimeSums{Residue{x}, sum};
                                                            });
            values.reserve(prime_sums.size());
            for (const auto &entry : prime_sums)
            {
                values.emplace_back(a * entry.count + b * entry.sum);
            }
        }
        const auto answer = sieve.prefix_sum(values, [&](std::uint64_t prime, unsigned exponent)
                                             {
                                                 return a * Residue{exponent} + b * Residue{prime};
                                             });
        verify_out << answer.value() << '\n';
    }
}
