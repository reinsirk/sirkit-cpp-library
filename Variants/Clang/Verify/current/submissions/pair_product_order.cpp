#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc155/tasks/abc155_d

// Library: Algorithm/Pair_Product_Order.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Pair_Product_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_product_order.md
//
// API一覧
// PairProductOrder(values): i<jの積values[i]*values[j]を対象にする。
// PairProductOrder(left,right): 二つの列からそれぞれ一つ選ぶ組の積を対象にする。重複する値も添字で区別する。
// size(): 総組数をO(1)で返す。count_at_most(bound): 数学的な積がbound以下の組数をO(n+m+1)で返す。
// kth_smallest(rank): 1始まりのrank番目の積を返す。範囲外の順位ならnullopt。答えがint64_tで表せなければoverflow_error。
// 入力は符号付き64ビット整数、各列長はUINT32_MAX以下。初期化O((n+m+1)log(n+m+2))時間・O(n+m+1)領域。
// 順位検索は最大64回の二分探索。各クエリの追加領域はO(1)。
// コピーは独立。移動元は空として使える。


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
#ifndef SIRKIT_PAIR_PRODUCT_ORDER_DEFINED
#define SIRKIT_PAIR_PRODUCT_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_pair_product_length(std::uint64_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairProductOrder: input is too long");
    }
}

inline constexpr std::uint64_t product_magnitude(std::int64_t negative) noexcept
{
    return std::uint64_t{0} - static_cast<std::uint64_t>(negative);
}

struct ProductMagnitudes
{
    std::vector<std::uint64_t> positive, negative;

    ProductMagnitudes() = default;

    explicit ProductMagnitudes(std::span<const std::int64_t> values)
    {
        check_pair_product_length(values.size());
        for (const std::int64_t value : values)
        {
            if (value > 0)
            {
                positive.push_back(static_cast<std::uint64_t>(value));
            }
            else if (value < 0)
            {
                negative.push_back(product_magnitude(value));
            }
        }
        std::sort(positive.begin(), positive.end());
        std::sort(negative.begin(), negative.end());
    }

    [[nodiscard]] std::uint64_t nonzero_count() const noexcept
    {
        return positive.size() + negative.size();
    }
};

inline constexpr std::uint64_t product_choose_two(std::uint64_t n) noexcept
{
    return n < 2 ? 0 : n * (n - 1) / 2;
}

inline std::uint64_t count_magnitude_products(std::span<const std::uint64_t> rows,
                                              std::span<const std::uint64_t> columns,
                                              std::uint64_t bound) noexcept
{
    if (rows.size() > columns.size())
    {
        std::swap(rows, columns);
    }
    std::uint64_t result = 0;
    auto end = columns.size();
    for (std::size_t index = 0; index < rows.size() && end != 0; ++index)
    {
        const auto limit = bound / rows[index];
        while (end != 0 && columns[end - 1] > limit)
        {
            --end;
        }
        result += end;
    }
    return result;
}

inline std::uint64_t count_distinct_magnitude_products(std::span<const std::uint64_t> magnitudes,
                                                       std::uint64_t bound) noexcept
{
    std::uint64_t result = 0;
    auto end = magnitudes.size();
    for (std::size_t next = 1; next < end; ++next)
    {
        const auto limit = bound / magnitudes[next - 1];
        while (next < end && magnitudes[end - 1] > limit)
        {
            --end;
        }
        result += end - next;
    }
    return result;
}
} // namespace reinsirk::detail

class PairProductOrder
{
public:
    explicit PairProductOrder(std::span<const std::int64_t> values)
        : left_(values), distinct_(true), pair_count_(detail::product_choose_two(values.size())),
          negative_pairs_(static_cast<std::uint64_t>(left_.negative.size()) * left_.positive.size()),
          zero_pairs_(pair_count_ - detail::product_choose_two(left_.nonzero_count()))
    {
    }

    PairProductOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right)
        : left_(left), right_(right), distinct_(false), pair_count_(static_cast<std::uint64_t>(left.size()) * right.size()),
          negative_pairs_(static_cast<std::uint64_t>(left_.negative.size()) * right_.positive.size() +
                          static_cast<std::uint64_t>(left_.positive.size()) * right_.negative.size()),
          zero_pairs_(pair_count_ - left_.nonzero_count() * right_.nonzero_count())
    {
    }

    PairProductOrder(const PairProductOrder &) = default;

    PairProductOrder(PairProductOrder &&other) noexcept
    {
        swap_state(other);
    }

    PairProductOrder &operator=(PairProductOrder other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return pair_count_;
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t bound) const noexcept
    {
        if (bound < 0)
        {
            return negative_pairs_ - negative_magnitudes_at_most(detail::product_magnitude(bound) - 1);
        }
        const auto limit = static_cast<std::uint64_t>(bound);
        const auto zero_or_negative = zero_pairs_ + negative_pairs_;
        if (distinct_)
        {
            return zero_or_negative + detail::count_distinct_magnitude_products(left_.positive, limit) +
                   detail::count_distinct_magnitude_products(left_.negative, limit);
        }
        return zero_or_negative + detail::count_magnitude_products(left_.positive, right_.positive, limit) +
               detail::count_magnitude_products(left_.negative, right_.negative, limit);
    }

    [[nodiscard]] std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
        constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
        auto low = rank <= negative_pairs_ ? minimum : std::int64_t{0};
        auto high = rank <= negative_pairs_ ? std::int64_t{-1} : maximum;
        while (low < high)
        {
            const auto middle = std::midpoint(low, high);
            if (count_at_most(middle) < rank)
            {
                low = middle + 1;
            }
            else
            {
                high = middle;
            }
        }
        if (low == minimum && rank <= negative_pairs_ - negative_magnitudes_at_most(detail::product_magnitude(minimum)))
        {
            throw std::overflow_error("PairProductOrder: product below int64_t range");
        }
        if (low == maximum && rank > count_at_most(maximum))
        {
            throw std::overflow_error("PairProductOrder: product above int64_t range");
        }
        return low;
    }

private:
    void swap_state(PairProductOrder &other) noexcept
    {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(distinct_, other.distinct_);
        std::swap(pair_count_, other.pair_count_);
        std::swap(negative_pairs_, other.negative_pairs_);
        std::swap(zero_pairs_, other.zero_pairs_);
    }

    [[nodiscard]] std::uint64_t negative_magnitudes_at_most(std::uint64_t limit) const noexcept
    {
        if (distinct_)
        {
            return detail::count_magnitude_products(left_.negative, left_.positive, limit);
        }
        return detail::count_magnitude_products(left_.negative, right_.positive, limit) +
               detail::count_magnitude_products(left_.positive, right_.negative, limit);
    }

    detail::ProductMagnitudes left_, right_;
    bool distinct_ = false;
    std::uint64_t pair_count_ = 0, negative_pairs_ = 0, zero_pairs_ = 0;
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
    std::size_t size = 0;
    std::uint64_t rank = 0;
    verify_in >> size >> rank;
    std::vector<std::int64_t> values(size);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairProductOrder products(values);
    verify_out << *products.kth_smallest(rank) << '\n';
}
