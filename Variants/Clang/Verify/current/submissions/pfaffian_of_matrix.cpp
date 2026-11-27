#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/pfaffian_of_matrix

// Library: Math/Pfaffian.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Pfaffian.cpp
// 参考: M. Wimmer, Efficient numerical computation of the Pfaffian for dense and banded skew-symmetric matrices
// (https://arxiv.org/abs/1102.3440) の合同変換。有限体上のコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pfaffian.md
//
// API一覧
// pfaffian_mod<Prime=998244353>(matrix): 素数Primeを法とする交代行列のパフィアンを求める。
// D×Dのvector<vector<uint32_t>>を渡す。各要素は[0,Prime)、対角は0、A[i][j]=-A[j][i] mod Prime。
// D=0なら1、奇数なら0。通常版で形・値・交代条件が不正ならinvalid_argument。
// O(D³)時間、行列を含めO(D²)領域。元の行列を使わない場合はstd::moveで渡せる。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PFAFFIAN_DEFINED
#define SIRKIT_PFAFFIAN_DEFINED

namespace detail
{
consteval bool PfaffianPrime(std::uint32_t value)
{
    if (value % 2 == 0)
    {
        return value == 2;
    }
    for (std::uint32_t divisor = 3; divisor <= value / divisor; divisor += 2)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return value > 1;
}
} // namespace detail

template <std::uint32_t Prime = 998244353>
std::uint32_t pfaffian_mod(std::vector<std::vector<std::uint32_t>> matrix)
{
    static_assert(detail::PfaffianPrime(Prime), "pfaffian_mod: modulus must be prime");
    const auto n = matrix.size();
    const auto negate = [](std::uint32_t value) -> std::uint32_t
    {
        return value == 0 ? 0 : Prime - value;
    };
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &row : matrix)
    {
        if (row.size() != n)
        {
            throw std::invalid_argument("pfaffian_mod: expected a square matrix");
        }
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        if (matrix[i][i] != 0)
        {
            throw std::invalid_argument("pfaffian_mod: diagonal must be zero");
        }
        for (std::size_t j = i + 1; j < n; ++j)
        {
            if (matrix[i][j] >= Prime || matrix[j][i] != negate(matrix[i][j]))
            {
                throw std::invalid_argument("pfaffian_mod: expected alternating residues");
            }
        }
    }
#endif
    if (n % 2 != 0)
    {
        return 0;
    }

    const auto multiply = [](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>((std::uint64_t{a} * b) % Prime);
    };
    std::uint32_t result = 1;
    for (std::size_t first = 0; first < n; first += 2)
    {
        const std::size_t second = first + 1;
        std::size_t pivot_column = second;
        while (pivot_column < n && matrix[first][pivot_column] == 0)
        {
            ++pivot_column;
        }
        if (pivot_column == n)
        {
            return 0;
        }
        if (pivot_column != second)
        {
            std::swap(matrix[second], matrix[pivot_column]);
            for (auto &row : matrix)
            {
                std::swap(row[second], row[pivot_column]);
            }
            result = negate(result);
        }

        const auto &top = matrix[first];
        const auto &bottom = matrix[second];
        const auto pivot = top[second];
        result = multiply(result, pivot);
        std::uint32_t inverse = 1;
        for (std::uint32_t base = pivot, power = Prime - 2; power != 0; power >>= 1)
        {
            if ((power & 1u) != 0)
            {
                inverse = multiply(inverse, base);
            }
            base = multiply(base, base);
        }

        // 先頭の2×2交代ブロックを消去し、残る主小行列をSchur補で置き換える。
        for (std::size_t i = second + 1; i < n; ++i)
        {
            const auto factor_top = multiply(top[i], inverse);
            const auto factor_bottom = multiply(bottom[i], inverse);
            auto &row = matrix[i];
            for (std::size_t j = i + 1; j < n; ++j)
            {
                const auto add = multiply(factor_bottom, top[j]);
                const auto sub = multiply(factor_top, bottom[j]);
                const auto sum = row[j] < Prime - add ? row[j] + add : row[j] - (Prime - add);
                row[j] = sum >= sub ? sum - sub : Prime - (sub - sum);
                matrix[j][i] = negate(row[j]);
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
#include <utility>
#include <vector>

int main()
{
    std::size_t pairs{};
    verify_in >> pairs;
    const auto n = 2 * pairs;
    std::vector<std::vector<std::uint32_t>> matrix(n, std::vector<std::uint32_t>(n));
    for (auto &row : matrix)
    {
        for (auto &value : row)
        {
            verify_in >> value;
        }
    }
    verify_out << sirkit::pfaffian_mod(std::move(matrix)) << '\n';
}
