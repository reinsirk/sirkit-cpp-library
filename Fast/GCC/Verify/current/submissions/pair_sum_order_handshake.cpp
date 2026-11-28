#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc149/tasks/abc149_e

// Library: Algorithm/Pair_Sum_Order.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Pair_Sum_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_sum_order.md
//
// API一覧
// PairSumOrder(values): 1列でi<jとなる組の和を対象にする。
// PairSumOrder(left,right): 2列の全ての組の和を対象にする。同値も添字で区別する。
// 各列はint64_t、長さはUINT32_MAX以下。整列してコピーする。O((n+m+1)log(n+m+2))時間、O(n+m+1)領域。
// size(): 組数。sorted_left()/sorted_right(): 昇順の入力列の参照。各O(1)。
// count_below(x), count_at_most(x), count_equal(x): x未満、以下、等しい和の個数。各O(n+m+1)。
// kth_smallest(rank)/kth_largest(rank): 1始まりの順位の和。順位範囲外はnullopt。
// 各順位検索は最大64回の二分探索と範囲検査。和の符号付き64ビット範囲外を選ぶとoverflow_error。
// 個数照会は数学的な和で比較する。コピーは独立、ムーブ元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PAIR_SUM_ORDER_DEFINED
#define SIRKIT_PAIR_SUM_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr auto kMinimumPairSum = std::numeric_limits<std::int64_t>::min();
inline constexpr auto kMaximumPairSum = std::numeric_limits<std::int64_t>::max();

inline void ValidatePairSumLength(std::uint64_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairSumOrder: input is too long");
    }
}

inline auto CopyAndSortPairSumInput(std::span<const std::int64_t> values)
{
    ValidatePairSumLength(values.size());
    std::vector<std::int64_t> result(values.begin(), values.end());
    std::sort(result.begin(), result.end());
    return result;
}

[[nodiscard]] inline std::int64_t ClampPairSum(std::int64_t a, std::int64_t b) noexcept
{
    if (a > 0 && b > kMaximumPairSum - a)
    {
        return kMaximumPairSum;
    }
    if (a < 0 && b < kMinimumPairSum - a)
    {
        return kMinimumPairSum;
    }
    return a + b;
}

template <bool Inclusive>
[[nodiscard]] std::size_t PairSumPrefixEnd(std::span<const std::int64_t> sorted,
                                           std::size_t begin, std::size_t end,
                                           std::int64_t a, std::int64_t bound) noexcept
{
    if (a > 0 && bound < kMinimumPairSum + a)
    {
        return begin;
    }
    if (a < 0 && bound > kMaximumPairSum + a)
    {
        return end;
    }
    const auto limit = bound - a;
    const auto outside = [limit](std::int64_t value) {
        if constexpr (Inclusive)
        {
            return value > limit;
        }
        else
        {
            return value >= limit;
        }
    };
    while (end > begin && outside(sorted[end - 1]))
    {
        --end;
    }
    return end;
}
} // namespace reinsirk::detail

class PairSumOrder
{
    std::vector<std::int64_t> left_, right_;
    bool within_;

    template <bool Inclusive>
    [[nodiscard]] std::uint64_t count(std::int64_t bound) const noexcept
    {
        const auto columns = sorted_right();
        auto end = columns.size();
        std::uint64_t answer = 0;
        for (std::size_t i = 0; i < left_.size(); ++i)
        {
            const auto begin = within_ ? i + 1 : 0;
            if (begin >= end)
            {
                break;
            }
            end = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PairSumPrefixEnd<Inclusive>(columns, begin, end, left_[i], bound);
            answer += end - begin;
        }
        return answer;
    }

public:
    explicit PairSumOrder(std::span<const std::int64_t> values)
        : left_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(values)), within_(true)
    {
    }

    PairSumOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right)
        : left_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(left)),
          right_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(right)), within_(false)
    {
    }

    PairSumOrder(const PairSumOrder &) = default;

    PairSumOrder(PairSumOrder &&other) noexcept
        : left_(std::move(other.left_)), right_(std::move(other.right_)), within_(other.within_)
    {
        other.left_.clear();
        other.right_.clear();
    }

    PairSumOrder &operator=(PairSumOrder other) noexcept
    {
        left_.swap(other.left_);
        right_.swap(other.right_);
        std::swap(within_, other.within_);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        const auto n = static_cast<std::uint64_t>(left_.size());
        if (within_)
        {
            return n < 2 ? 0 : n * (n - 1) / 2;
        }
        return n * right_.size();
    }

    [[nodiscard]] std::span<const std::int64_t> sorted_left() const noexcept
    {
        return left_;
    }

    [[nodiscard]] std::span<const std::int64_t> sorted_right() const noexcept
    {
        if (within_)
        {
            return left_;
        }
        return right_;
    }

    [[nodiscard]] std::uint64_t count_below(std::int64_t bound) const noexcept
    {
        return count<false>(bound);
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t bound) const noexcept
    {
        return count<true>(bound);
    }

    [[nodiscard]] std::uint64_t count_equal(std::int64_t target) const noexcept
    {
        return count_at_most(target) - count_below(target);
    }

    [[nodiscard]] std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        const auto columns = sorted_right();
        const auto first_column = within_ ? 1 : 0;
        const auto last_row = left_.size() - (within_ ? 2 : 1);
        auto left = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ClampPairSum(left_.front(), columns[first_column]);
        auto right = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ClampPairSum(left_[last_row], columns.back());
        if ((left == /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kMinimumPairSum && count_below(left) >= rank) ||
            (right == /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kMaximumPairSum && count_at_most(right) < rank))
        {
            throw std::overflow_error("PairSumOrder: sum does not fit in int64_t");
        }
        while (left < right)
        {
            const auto mid = std::midpoint(left, right);
            if (count_at_most(mid) < rank)
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }
        return left;
    }

    [[nodiscard]] std::optional<std::int64_t> kth_largest(std::uint64_t rank) const
    {
        const auto total = size();
        if (rank == 0 || rank > total)
        {
            return std::nullopt;
        }
        return kth_smallest(total - rank + 1);
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

int main()
{
    std::size_t n{};
    std::uint64_t wanted{};
    verify_in >> n >> wanted;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairSumOrder order(values, values);
    const auto threshold = *order.kth_largest(wanted);
    const auto sorted = order.sorted_left();
    std::vector<std::int64_t> prefix(n + 1);
    for (std::size_t i = 0; i < n; ++i)
    {
        prefix[i + 1] = prefix[i] + sorted[i];
    }
    std::size_t begin = n;
    std::uint64_t greater_count = 0;
    std::int64_t greater_sum = 0;
    for (const auto value : sorted)
    {
        while (begin != 0 && value + sorted[begin - 1] > threshold)
        {
            --begin;
        }
        const auto remaining = n - begin;
        greater_count += remaining;
        greater_sum += static_cast<std::int64_t>(remaining) * value + prefix[n] - prefix[begin];
    }
    const auto answer = greater_sum + static_cast<std::int64_t>(wanted - greater_count) * threshold;
    verify_out << answer << '\n';
}
