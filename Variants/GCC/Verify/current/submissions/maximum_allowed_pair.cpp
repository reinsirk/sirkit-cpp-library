#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc331/tasks/abc331_e

// Library: Algorithm/Maximum_Allowed_Pair.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Maximum_Allowed_Pair.cpp
// 参考（問題設定）: https://atcoder.jp/contests/abc331/tasks/abc331_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_allowed_pair.md
//
// API一覧
// MaximumAllowedPairResult: 最大和を取る元の添字 left,right と、その和 sum。
// maximum_allowed_pair(left,right,forbidden): 禁止組を除いた和の最大値と添字を返す。組がなければnullopt。
// 0始まり。同点では(left,right)の辞書順最小。禁止組は重複可。最大和はint64_tで表せること。
// 2列の長さをn,m、禁止指定数をpとする。短い列を候補側とし、kをその長さと「1+行ごとの禁止指定数の最大」の小さい方とする。
// O(n+m+p+min(n,m)*log(k+1))時間、O(n+m+p)領域。空の列を含む場合の時間はO(p+1)。
// 検査有効時は範囲外の禁止添字・最大和の型範囲超過を例外で通知。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAXIMUM_ALLOWED_PAIR_DEFINED
#define SIRKIT_MAXIMUM_ALLOWED_PAIR_DEFINED

struct MaximumAllowedPairResult
{
    std::size_t left;
    std::size_t right;
    std::int64_t sum;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
// 両項を 2^63 ずらし、和の65ビットを桁上がりと下位64ビットに分ける。
inline std::pair<bool, std::uint64_t> AllowedPairExactSum(std::int64_t left, std::int64_t right) noexcept
{
    constexpr std::uint64_t kSignBit = std::uint64_t{1} << 63;
    const std::uint64_t a = static_cast<std::uint64_t>(left) ^ kSignBit;
    const std::uint64_t b = static_cast<std::uint64_t>(right) ^ kSignBit;
    const std::uint64_t low = a + b;
    return {low < a, low};
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::optional<MaximumAllowedPairResult> maximum_allowed_pair(
    std::span<const std::int64_t> left, std::span<const std::int64_t> right,
    std::span<const std::pair<std::size_t, std::size_t>> forbidden)
{
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[i, j] : forbidden)
    {
        if (i >= left.size() || j >= right.size())
        {
            throw std::out_of_range("maximum_allowed_pair: forbidden index");
        }
    }
#endif
    if (left.empty() || right.empty())
    {
        return std::nullopt;
    }
    // 短い列だけをスコア順に並べる。元の添字の順序へ戻すときも orient を使う。
    const bool transposed = left.size() < right.size();
    const auto rows = transposed ? right : left;
    const auto columns = transposed ? left : right;
    const auto orient = [transposed](std::size_t first, std::size_t second)
    {
        return transposed ? std::pair{second, first} : std::pair{first, second};
    };

    std::vector<std::size_t> offsets(rows.size() + 1);
    for (const auto &[i, j] : forbidden)
    {
        const auto [row, column] = orient(i, j);
        ++offsets[row + 1];
    }
    const auto most_forbidden = *std::ranges::max_element(offsets);
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    std::vector<std::size_t> next(offsets);
    std::vector<std::size_t> forbidden_columns(forbidden.size());
    for (const auto &[i, j] : forbidden)
    {
        const auto [row, column] = orient(i, j);
        forbidden_columns[next[row]++] = column;
    }

    // 各行が先頭から読み飛ばす候補数は、その行の禁止指定数以下。
    const auto candidates = most_forbidden < columns.size() ? most_forbidden + 1 : columns.size();
    std::vector<std::size_t> order(columns.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    using Difference = std::vector<std::size_t>::difference_type;
    std::partial_sort(order.begin(), order.begin() + static_cast<Difference>(candidates), order.end(),
                      [columns](std::size_t i, std::size_t j)
                      {
                          if (columns[i] == columns[j])
                          {
                              return i < j;
                          }
                          return columns[i] > columns[j];
                      });
    order.resize(candidates);

    using Pair = std::pair<std::size_t, std::size_t>;
    using ExactSum = std::pair<bool, std::uint64_t>;
    std::optional<std::pair<ExactSum, Pair>> best;
    std::vector<std::size_t> marked(columns.size(), rows.size());
    for (std::size_t row = 0; row < rows.size(); ++row)
    {
        for (std::size_t i = offsets[row]; i < offsets[row + 1]; ++i)
        {
            marked[forbidden_columns[i]] = row;
        }
        for (const auto column : order)
        {
            if (marked[column] == row)
            {
                continue;
            }
            const auto indices = orient(row, column);
            const auto sum = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AllowedPairExactSum(rows[row], columns[column]);
            if (!best || best->first < sum || (best->first == sum && indices < best->second))
            {
                best.emplace(sum, indices);
            }
            break;
        }
    }

    if (!best)
    {
        return std::nullopt;
    }
    const auto &[sum, indices] = *best;
    const auto [carry, low] = sum;
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::uint64_t kSignBit = std::uint64_t{1} << 63;
    if (carry == ((low & kSignBit) != 0))
    {
        throw std::overflow_error("maximum_allowed_pair: sum out of range");
    }
#endif
    return MaximumAllowedPairResult{indices.first, indices.second, static_cast<std::int64_t>(low)};
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

    std::size_t n{}, m{}, p{};
    verify_in >> n >> m >> p;
    std::vector<std::int64_t> left(n), right(m);
    for (auto &value : left)
    {
        verify_in >> value;
    }
    for (auto &value : right)
    {
        verify_in >> value;
    }
    std::vector<std::pair<std::size_t, std::size_t>> forbidden(p);
    for (auto &[i, j] : forbidden)
    {
        verify_in >> i >> j;
        --i;
        --j;
    }
    verify_out << sirkit::maximum_allowed_pair(left, right, forbidden)->sum << '\n';
}
