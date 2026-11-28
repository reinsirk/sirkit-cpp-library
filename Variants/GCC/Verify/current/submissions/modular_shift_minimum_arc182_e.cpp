#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/arc182/tasks/arc182_e

// Library: Math/Modular_Shift_Minimum.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Modular_Shift_Minimum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_minimum.md
// 参考（円環上の各区間を数える帰着）: https://atcoder.jp/contests/arc182/editorial/10477
//
// API一覧
// sum_minimum_modular_shifts(count,modulus,step,offsets,start=0):
// k=0..count-1に対し、(start+step*k+offset) mod modulusのoffsetに関する最小値を取り、全kで総和。
// 結果はuint64_tで厳密。countとmodulusはuint32_t。step/start/offsetは負のint64_tも可。
// modulus=0または空のoffsetsはinvalid_argument。有効な入力でcount=0なら0。
// offsetsがn個、剰余が異なる値がu個ならO(n log(n+1)+u log(modulus+1))時間・O(n)領域。
// count=1またはstepがmodulusの倍数ならO(n)時間。
//
// [同梱のFloor Sum]
// floor_sum(n,m,a,b): i=0..n-1 の floor((a*i+b)/m) の総和。a,bは負でもよい。
// 0<=n<2^32、1<=m<2^32。ACLと同じく結果は2^64を法とする符号付き64ビット値。
// min_of_mod_of_linear(n,m,a,b): i=0..n-1 の (a*i+b) mod m の最小値。1<=n<2^32。
// 両方ともO(log(m+1))時間・O(1)領域。検査有効時、n,mが範囲外ならstd::invalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_DEFINED
#define SIRKIT_FLOOR_SUM_DEFINED

inline std::int64_t floor_sum(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n < 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("floor_sum: n or m");
    }
#endif
    if (n == 0)
    {
        return 0;
    }
    const auto count = static_cast<std::uint64_t>(n);
    const auto triangular = count * (count - 1) / 2;
    std::uint64_t sum = 0;
    if (a < 0)
    {
        const auto remainder = a % m;
        const auto quotient = a / m;
        sum += triangular * static_cast<std::uint64_t>(quotient - (remainder != 0));
        a = remainder == 0 ? 0 : remainder + m;
    }
    if (b < 0)
    {
        const auto remainder = b % m;
        const auto quotient = b / m;
        sum += count * static_cast<std::uint64_t>(quotient - (remainder != 0));
        b = remainder == 0 ? 0 : remainder + m;
    }
    auto length = count;
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = static_cast<std::uint64_t>(a);
    auto offset = static_cast<std::uint64_t>(b);
    while (true)
    {
        if (slope >= modulus)
        {
            sum += (length * (length - 1) / 2) * (slope / modulus);
            slope %= modulus;
        }
        if (offset >= modulus)
        {
            sum += length * (offset / modulus);
            offset %= modulus;
        }
        const auto limit = slope * length + offset;
        if (limit < modulus)
        {
            break;
        }
        length = limit / modulus; // 格子点を横方向から数え直す。
        offset = limit % modulus;
        std::swap(modulus, slope);
    }
    return std::bit_cast<std::int64_t>(sum);
}

inline std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n <= 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("min_of_mod_of_linear: n or m");
    }
#endif
    const auto normalize = [](std::int64_t x, std::int64_t modulus)
    {
        x %= modulus;
        return static_cast<std::uint64_t>(x < 0 ? x + modulus : x);
    };
    auto length = static_cast<std::uint64_t>(n);
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = normalize(a, m);
    auto offset = normalize(b, m);
    auto minimum = offset;
    while (minimum != 0 && slope != 0)
    {
        if (slope * 2 > modulus) // 逆から走査して次の法を半分以下にする。
        {
            offset = (slope * (length - 1) + offset) % modulus;
            slope = modulus - slope;
        }
        minimum = std::min(minimum, offset);
        const auto wraps = (slope * (length - 1) + offset) / modulus;
        if (wraps == 0)
        {
            break;
        }
        const auto reverse_step = (slope - modulus % slope) % slope; // 次の法は slope。
        length = wraps;
        offset = (offset % slope + reverse_step) % slope; // y回目の折り返しは (offset - modulus*y) mod slope。
        modulus = slope;
        slope = reverse_step;
        minimum = std::min(minimum, offset);
    }
    return static_cast<std::int64_t>(minimum);
}

#endif

#ifndef SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED
#define SIRKIT_MODULAR_SHIFT_MINIMUM_DEFINED

namespace detail
{
inline std::uint32_t ShiftMinimumResidue(std::int64_t value, std::uint32_t modulus)
{
    const auto remainder = value % static_cast<std::int64_t>(modulus);
    return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
}
} // namespace detail

[[nodiscard]] inline std::uint64_t sum_minimum_modular_shifts(
    std::uint32_t count, std::uint32_t modulus, std::int64_t step,
    std::span<const std::int64_t> offsets, std::int64_t start = 0)
{
    if (modulus == 0 || offsets.empty())
    {
        throw std::invalid_argument("sum_minimum_modular_shifts: zero modulus or no offsets");
    }
    if (count == 0)
    {
        return 0;
    }
    const auto phase = detail::ShiftMinimumResidue(start, modulus);
    std::vector<std::uint32_t> residues;
    residues.reserve(offsets.size());
    for (const auto offset : offsets)
    {
        const auto base = detail::ShiftMinimumResidue(offset, modulus);
        residues.push_back(static_cast<std::uint32_t>((std::uint64_t{base} + phase) % modulus));
    }
    const auto slope = detail::ShiftMinimumResidue(step, modulus);
    const std::uint64_t length = count;
    if (slope == 0 || count == 1)
    {
        return length * *std::ranges::min_element(residues);
    }
    std::ranges::sort(residues);
    const auto duplicates = std::ranges::unique(residues);
    residues.erase(duplicates.begin(), duplicates.end());

    // 一周の各区切りで、その次の剰余までの距離だけ最小値が減る。
    // 中間結果は2^64で折り返すが、最終結果はcount*(modulus-1)以下。
    auto total = (length * (length - 1) / 2) * slope + length * residues.front();
    for (std::size_t i = 0; i < residues.size(); ++i)
    {
        const auto prior = residues[i];
        const auto gap = i + 1 == residues.size() ? modulus - prior + residues.front() : residues[i + 1] - prior;
        const auto drops = static_cast<std::uint64_t>(floor_sum(count, modulus, slope, prior));
        total -= std::uint64_t{gap} * drops;
    }
    return total;
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
    std::uint32_t modulus{}, count{};
    std::int64_t step{};
    verify_in >> n >> modulus >> step >> count;
    std::vector<std::int64_t> offsets(n);
    for (auto &offset : offsets)
    {
        verify_in >> offset;
    }
    verify_out << sirkit::sum_minimum_modular_shifts(count, modulus, step, offsets) << '\n';
}
