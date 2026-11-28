#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 833e9c3160c79e00d3086d06fd20ce75826fceb6c23375d736816f258fc7bc98
// Verify: https://atcoder.jp/contests/agc041/tasks/agc041_f

// Library: Math/Histogram_Rook_Cover.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（ヒストグラム盤の行を連続区間に分ける見方とVerify問題）: https://img.atcoder.jp/agc041/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/histogram_rook_cover.md
//
// API一覧
// count_histogram_rook_covers(heights): 全マスがルークに支配される置き方の数を法998244353で返す。
// 列iの下からheights[i]個が盤のマス。支配は同じ列または同じ高さの切れ目のない行区間にルークがあること。
// 隣り合うルークも置ける。heightsは非負のuint64_t列。空盤は1通り。
// n列・最大高さHでO(1+n^2 log(H+2))時間、O(1+n)領域。盤のマスを展開しない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HISTOGRAM_ROOK_COVER
#define SIRKIT_HISTOGRAM_ROOK_COVER

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
constexpr std::uint32_t kHistogramRookMod = 998244353;

[[nodiscard]] constexpr std::uint32_t histogram_rook_add(std::uint32_t a, std::uint32_t b)
{
    const auto sum = a + b;
    return sum < kHistogramRookMod ? sum : sum - kHistogramRookMod;
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_subtract(std::uint32_t a, std::uint32_t b)
{
    return histogram_rook_add(a, kHistogramRookMod - b);
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_multiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kHistogramRookMod);
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_power(std::uint32_t base, std::uint64_t exponent)
{
    std::uint32_t result = 1;
    while (exponent != 0)
    {
        if ((exponent & 1) != 0)
        {
            result = histogram_rook_multiply(result, base);
        }
        base = histogram_rook_multiply(base, base);
        exponent >>= 1;
    }
    return result;
}

[[nodiscard]] inline std::vector<std::uint32_t> histogram_rook_merge(
    const std::vector<std::uint32_t> &left, const std::vector<std::uint32_t> &right)
{
    std::vector<std::uint32_t> result(left.size() + right.size() - 1);
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        for (std::size_t j = 0; j < right.size(); ++j)
        {
            result[i + j] = histogram_rook_add(result[i + j], histogram_rook_multiply(left[i], right[j]));
        }
    }
    return result;
}

struct HistogramRookState
{
    // 添字はルークを置いてよい列数。空列と仮定した列を含むかどうかで重みを分ける。
    std::vector<std::uint32_t> without_empty;
    std::vector<std::uint32_t> total;
};

[[nodiscard]] inline HistogramRookState histogram_rook_extend(const HistogramRookState &state, std::uint64_t rows)
{
    const auto size = state.total.size();
    HistogramRookState result{std::vector<std::uint32_t>(size + 1), std::vector<std::uint32_t>(size + 1)};
    for (std::size_t i = 0; i < size; ++i)
    {
        // 新しい列を使う、空列とみなす、包除で除く、の3通り。後二者はtotalで相殺する。
        result.without_empty[i] = histogram_rook_subtract(result.without_empty[i], state.without_empty[i]);
        result.without_empty[i + 1] = state.without_empty[i];
        result.total[i + 1] = state.total[i];
    }
    if (rows == 0)
    {
        return result;
    }
    const auto power_of_two = histogram_rook_power(2, rows);
    std::uint32_t subset_count = 1, all_choices = 1;
    for (std::size_t i = 0; i <= size; ++i)
    {
        // 指定した空列が含まれる行では、行が空になる選び方を除く。
        const auto nonempty_choices = histogram_rook_power(histogram_rook_subtract(subset_count, 1), rows);
        const auto with_empty_column = histogram_rook_subtract(result.total[i], result.without_empty[i]);
        result.without_empty[i] = histogram_rook_multiply(result.without_empty[i], all_choices);
        result.total[i] = histogram_rook_add(result.without_empty[i], histogram_rook_multiply(with_empty_column, nonempty_choices));
        all_choices = histogram_rook_multiply(all_choices, power_of_two);
        subset_count = histogram_rook_add(subset_count, subset_count);
    }
    return result;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint32_t count_histogram_rook_covers(std::span<const std::uint64_t> heights)
{
    const auto n = heights.size();
    if (n == 0)
    {
        return 1;
    }
    std::vector<std::array<std::size_t, 2>> children(n, {n, n});
    std::vector<std::size_t> stack;
    stack.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        auto previous = n;
        while (!stack.empty() && heights[stack.back()] > heights[i])
        {
            previous = stack.back();
            stack.pop_back();
        }
        children[i][0] = previous;
        if (!stack.empty())
        {
            children[stack.back()][1] = i;
        }
        stack.push_back(i);
    }
    const auto root = stack.front();
    stack.clear();
    stack.push_back(root);
    std::vector<std::uint64_t> level(n);
    for (std::size_t i = 0; i < stack.size(); ++i)
    {
        const auto at = stack[i];
        for (const auto next : children[at])
        {
            if (next != n)
            {
                level[next] = heights[at];
                stack.push_back(next);
            }
        }
    }
    // 計算済みで、親にまだ統合していない部分木だけを保持する。
    std::vector<detail::HistogramRookState> dp(n);
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    {
        const auto at = *it;
        detail::HistogramRookState state{{1}, {1}};
        for (const auto child : children[at])
        {
            if (child != n)
            {
                state.without_empty = detail::histogram_rook_merge(state.without_empty, dp[child].without_empty);
                state.total = detail::histogram_rook_merge(state.total, dp[child].total);
                dp[child] = {};
            }
        }
        dp[at] = detail::histogram_rook_extend(state, heights[at] - level[at]);
    }
    return std::accumulate(dp[root].total.begin(), dp[root].total.end(), std::uint32_t{0}, detail::histogram_rook_add);
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
    std::vector<std::uint64_t> heights(n);
    for (auto &height : heights)
    {
        verify_in >> height;
    }
    verify_out << sirkit::count_histogram_rook_covers(heights) << '\n';
}
