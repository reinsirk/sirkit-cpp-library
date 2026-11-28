#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2149

// Library: Math/Young_Tableau_Count.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Young_Tableau_Count.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/young_tableau_count.md
// 参考（フック長の公式）: https://info.atcoder.jp/entry/algorithm_lectures/hook_length_formula
//
// API一覧
// standard_young_tableau_count<Mod=998244353>(shape): 各行・列が増加する標準ヤング盤の個数を法Modで返す。
// domino_tableau_count<Mod=998244353>(shape): 図形を保って右端の横・縦ドミノを順に除く操作列の個数を法Modで返す。
// shapeは行長の広義単調減少列。末尾0を許す。空図形は1通り。Modは正のuint32_tで合成数も可。
// 行数H、幅W、面積N、Modの異なる素因数数kに対しO(H+N(k+1)+(k+1)log(Mod+1))時間、O(H+W+k+1)領域。
// 面積がuint32_tを超えればlength_error、行長の順序が不正ならinvalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_YOUNG_TABLEAU_COUNT_DEFINED
#define SIRKIT_YOUNG_TABLEAU_COUNT_DEFINED

namespace detail
{
inline std::uint32_t YoungTableauCells(std::span<const std::uint32_t> shape)
{
    auto previous = std::numeric_limits<std::uint32_t>::max();
    std::uint64_t cells = 0;
    for (const auto length : shape)
    {
        if (length > previous)
        {
            throw std::invalid_argument("Young tableau: nonincreasing row lengths required");
        }
        cells += length;
        if (cells > std::numeric_limits<std::uint32_t>::max())
        {
            throw std::length_error("Young tableau: too many cells");
        }
        previous = length;
    }
    return static_cast<std::uint32_t>(cells);
}

template <std::uint32_t Mod>
struct YoungTableauFactors
{
    static_assert(Mod > 0, "Young tableau: Mod must be positive");
    std::array<std::uint32_t, std::numeric_limits<std::uint32_t>::digits> primes{};
    std::size_t size{};
    std::uint32_t totient = Mod;

    consteval YoungTableauFactors()
    {
        auto rest = Mod;
        for (std::uint32_t divisor = 2; divisor <= rest / divisor; ++divisor)
        {
            if (rest % divisor == 0)
            {
                primes[size++] = divisor;
                totient -= totient / divisor;
                do
                {
                    rest /= divisor;
                } while (rest % divisor == 0);
            }
        }
        if (rest > 1)
        {
            primes[size++] = rest;
            totient -= totient / rest;
        }
    }
};

template <std::uint32_t Mod>
std::uint32_t YoungTableauPower(std::uint32_t value, std::uint64_t exponent)
{
    std::uint32_t result = 1 % Mod;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = static_cast<std::uint32_t>(std::uint64_t{result} * value % Mod);
        }
        value = static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }
    return result;
}

template <class Consume>
void ForEachYoungTableauHook(std::span<const std::uint32_t> shape, Consume consume)
{
    if (shape.empty())
    {
        return;
    }
    std::vector<std::uint32_t> below(shape.front());
    for (auto row = shape.size(); row != 0; --row)
    {
        const auto length = shape[row - 1];
        for (std::uint32_t column = 0; column < length; ++column)
        {
            consume(length - column + below[column]);
            ++below[column];
        }
    }
}

template <std::uint32_t Mod>
std::uint32_t YoungTableauHookRatio(std::uint32_t cells, std::span<const std::uint32_t> first,
                                    std::span<const std::uint32_t> second)
{
    static_assert(Mod > 0, "Young tableau: Mod must be positive");
    if constexpr (Mod == 1)
    {
        return 0;
    }
    else
    {
        constexpr YoungTableauFactors<Mod> factors;
        std::size_t active = 0;
        while (active < factors.size && factors.primes[active] <= cells)
        {
            ++active;
        }
        const auto primes = std::span(factors.primes).first(active);
        std::array<std::int64_t, std::numeric_limits<std::uint32_t>::digits> powers{};
        const auto strip = [&](std::uint32_t value, int direction)
        {
            for (std::size_t i = 0; i < primes.size(); ++i)
            {
                const auto prime = primes[i];
                while (value % prime == 0)
                {
                    value /= prime;
                    powers[i] += direction;
                }
            }
            return value;
        };
        std::uint64_t numerator = 1, denominator = 1;
        for (std::uint64_t i = 2; i <= cells; ++i)
        {
            numerator = numerator * strip(static_cast<std::uint32_t>(i), 1) % Mod;
        }
        const auto divide_hook = [&](std::uint32_t hook)
        {
            denominator = denominator * strip(hook, -1) % Mod;
        };
        ForEachYoungTableauHook(first, divide_hook);
        ForEachYoungTableauHook(second, divide_hook);
        // 法と共通の素因数を除いた分母だけを逆元にし、差し引いた素因数を掛け直す。
        auto result = numerator * YoungTableauPower<Mod>(static_cast<std::uint32_t>(denominator), factors.totient - 1) % Mod;
        for (std::size_t i = 0; i < active; ++i)
        {
            result = result * YoungTableauPower<Mod>(primes[i], static_cast<std::uint64_t>(powers[i])) % Mod;
        }
        return static_cast<std::uint32_t>(result);
    }
}
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::uint32_t standard_young_tableau_count(std::span<const std::uint32_t> shape)
{
    const auto cells = detail::YoungTableauCells(shape);
    return detail::YoungTableauHookRatio<Mod>(cells, shape, {});
}

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::uint32_t domino_tableau_count(std::span<const std::uint32_t> shape)
{
    const auto cells = detail::YoungTableauCells(shape);
    if ((cells & 1u) != 0)
    {
        return 0;
    }
    while (!shape.empty() && shape.back() == 0)
    {
        shape = shape.first(shape.size() - 1);
    }
    // ベータ数を偶奇に分けると、ドミノの除去は各列の1マス除去になる。
    std::array<std::vector<std::uint32_t>, 2> quotient;
    for (std::size_t i = 0; i < shape.size(); ++i)
    {
        const auto beta = std::uint64_t{shape[i]} + (shape.size() - 1 - i);
        quotient[beta & 1u].push_back(static_cast<std::uint32_t>(beta / 2));
    }
    if (quotient[0].size() != (shape.size() + 1) / 2)
    {
        return 0;
    }
    for (auto &part : quotient)
    {
        for (std::size_t i = 0; i < part.size(); ++i)
        {
            part[i] -= static_cast<std::uint32_t>(part.size() - 1 - i);
        }
    }
    return detail::YoungTableauHookRatio<Mod>(cells / 2, quotient[0], quotient[1]);
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
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint32_t> shape(n);
    for (auto it = shape.rbegin(); it != shape.rend(); ++it)
    {
        verify_in >> *it;
    }
    verify_out << sirkit::domino_tableau_count(shape) << '\n';
}
