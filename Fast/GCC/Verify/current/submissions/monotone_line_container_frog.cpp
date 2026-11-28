#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/dp/tasks/dp_z

// Library: DataStructure/Monotone_Line_Container.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Monotone_Line_Container.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monotone_line_container.md
//
// API一覧
// MonotoneMinLineContainer / MonotoneMaxLineContainer: 符号付き64bit整数の直線包絡。
// add(slope, intercept): 直線を追加。傾きは最小化なら非増加、最大化なら非減少。償却O(1)。
// query(x): 最適な値と元の係数{value,slope,intercept}を返す。任意のxをO(log(h+1))で照会。
// query_monotone(x): xが非減少の一連の照会。addとの合計で償却O(1)。
// どちらも空の包絡ならnullopt。値がint64_tに収まらなければoverflow_error。同点の係数は任意。
// size()/empty(): 有効な直線の本数/空か。各O(1)。clear(): 空に戻し照会順もリセットする。
// reserve(n): 有効な直線n本分を確保する。hは有効な直線数。使用領域は最大保持数・予約容量に比例。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONOTONE_LINE_CONTAINER_DEFINED
#define SIRKIT_MONOTONE_LINE_CONTAINER_DEFINED

namespace detail
{
inline constexpr std::int64_t kMonotoneLineMinimum = std::numeric_limits<std::int64_t>::min();
inline constexpr std::uint64_t kMonotoneLinePositiveLimit = std::numeric_limits<std::int64_t>::max();
inline constexpr std::uint64_t kMonotoneLineNegativeLimit = kMonotoneLinePositiveLimit + 1;

inline std::uint64_t monotone_line_magnitude(std::int64_t value) noexcept
{
    const auto converted = static_cast<std::uint64_t>(value);
    return value < 0 ? std::uint64_t{0} - converted : converted;
}

inline std::int64_t monotone_line_signed_value(std::uint64_t magnitude, bool negative)
{
    if (magnitude > (negative ? kMonotoneLineNegativeLimit : kMonotoneLinePositiveLimit))
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    if (negative && magnitude == kMonotoneLineNegativeLimit)
    {
        return kMonotoneLineMinimum;
    }
    const auto value = static_cast<std::int64_t>(magnitude);
    return negative ? -value : value;
}

inline std::int64_t monotone_line_evaluate(std::int64_t slope, std::int64_t intercept, std::int64_t x)
{
    using Wide = std::uint64_t;
    constexpr auto kLimit = std::numeric_limits<Wide>::max();
    const Wide coefficient = monotone_line_magnitude(slope);
    const Wide coordinate = monotone_line_magnitude(x);
    if (coefficient != 0 && coordinate > kLimit / coefficient)
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    const Wide product = coefficient * coordinate;
    const Wide offset = monotone_line_magnitude(intercept);
    const bool product_negative = (slope < 0) != (x < 0);
    const bool offset_negative = intercept < 0;
    if (product_negative != offset_negative)
    {
        return product >= offset ? monotone_line_signed_value(product - offset, product_negative)
                                 : monotone_line_signed_value(offset - product, offset_negative);
    }
    if (product > kLimit - offset)
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    return monotone_line_signed_value(product + offset, product_negative);
}

// ceil((a-b)/d)。d>0。64bit座標の右外は空、左外は最小の座標に丸める。
inline std::optional<std::int64_t> monotone_line_first_x(std::int64_t a, std::int64_t b, std::uint64_t d) noexcept
{
    using Wide = std::uint64_t;
    if (a < b)
    {
        const Wide quotient = (static_cast<Wide>(b) - static_cast<Wide>(a)) / d;
        return quotient >= kMonotoneLineNegativeLimit ? kMonotoneLineMinimum : -static_cast<std::int64_t>(quotient);
    }
    const Wide numerator = static_cast<Wide>(a) - static_cast<Wide>(b);
    const Wide quotient = numerator / d + (numerator % d != 0);
    if (quotient > kMonotoneLinePositiveLimit)
    {
        return std::nullopt;
    }
    return static_cast<std::int64_t>(quotient);
}
}

template <bool Minimize = true>
class MonotoneLineContainer
{
public:
    struct Result
    {
        std::int64_t value;
        std::int64_t slope;
        std::int64_t intercept;
    };

    MonotoneLineContainer() = default;
    MonotoneLineContainer(const MonotoneLineContainer &) = default;
    MonotoneLineContainer(MonotoneLineContainer &&other) noexcept
    {
        swap_state(other);
    }

    MonotoneLineContainer &operator=(MonotoneLineContainer other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lines_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return lines_.empty();
    }

    void clear() noexcept
    {
        lines_.clear();
        last_slope_.reset();
        last_query_.reset();
        cursor_ = 0;
    }

    void reserve(std::size_t capacity)
    {
        lines_.reserve(capacity);
    }

    void add(std::int64_t slope, std::int64_t intercept)
    {
        Entry incoming{slope, intercept, detail::kMonotoneLineMinimum};
        auto keep = lines_.size();
        while (keep > 0)
        {
            const auto &previous = lines_[keep - 1];
            if (slope == previous.slope)
            {
                if (!better_intercept(intercept, previous.intercept))
                {
                    last_slope_ = slope;
                    return;
                }
            }
            else
            {
                const auto start = first_x(previous, incoming);
                if (!start)
                {
                    last_slope_ = slope;
                    return;
                }
                if (*start > previous.start)
                {
                    incoming.start = *start;
                    break;
                }
            }
            --keep;
        }
        if (keep == lines_.size())
        {
            lines_.push_back(incoming);
        }
        else
        {
            lines_[keep] = incoming;
            lines_.resize(keep + 1);
            cursor_ = std::min(cursor_, keep);
        }
        last_slope_ = slope;
    }

    [[nodiscard]] std::optional<Result> query(std::int64_t x) const
    {
        if (empty())
        {
            return std::nullopt;
        }
        const auto end = std::ranges::upper_bound(lines_, x, {}, &Entry::start);
        return eval(*std::prev(end), x);
    }

    [[nodiscard]] std::optional<Result> query_monotone(std::int64_t x)
    {
        if (empty())
        {
            last_query_ = x;
            return std::nullopt;
        }
        auto best = cursor_;
        while (best + 1 < lines_.size() && lines_[best + 1].start <= x)
        {
            ++best;
        }
        const auto answer = eval(lines_[best], x);
        cursor_ = best;
        last_query_ = x;
        return answer;
    }

private:
    struct Entry
    {
        std::int64_t slope;
        std::int64_t intercept;
        std::int64_t start;
    };

    static bool ordered_before(std::int64_t incoming, std::int64_t previous) noexcept
    {
        if constexpr (Minimize)
        {
            return incoming > previous;
        }
        else
        {
            return incoming < previous;
        }
    }

    static bool better_intercept(std::int64_t incoming, std::int64_t previous) noexcept
    {
        return ordered_before(previous, incoming);
    }

    static std::optional<std::int64_t> first_x(const Entry &previous, const Entry &incoming) noexcept
    {
        using Wide = std::uint64_t;
        if constexpr (Minimize)
        {
            const Wide difference = static_cast<Wide>(previous.slope) - static_cast<Wide>(incoming.slope);
            return detail::monotone_line_first_x(incoming.intercept, previous.intercept, difference);
        }
        else
        {
            const Wide difference = static_cast<Wide>(incoming.slope) - static_cast<Wide>(previous.slope);
            return detail::monotone_line_first_x(previous.intercept, incoming.intercept, difference);
        }
    }

    static Result eval(const Entry &line, std::int64_t x)
    {
        return {detail::monotone_line_evaluate(line.slope, line.intercept, x), line.slope, line.intercept};
    }

    void swap_state(MonotoneLineContainer &other) noexcept
    {
        lines_.swap(other.lines_);
        std::swap(last_slope_, other.last_slope_);
        std::swap(last_query_, other.last_query_);
        std::swap(cursor_, other.cursor_);
    }

    std::vector<Entry> lines_;
    std::optional<std::int64_t> last_slope_;
    std::optional<std::int64_t> last_query_;
    std::size_t cursor_ = 0;
};

using MonotoneMinLineContainer = MonotoneLineContainer<true>;
using MonotoneMaxLineContainer = MonotoneLineContainer<false>;

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

int main()
{
    int n = 0;
    std::int64_t jump = 0, height = 0;
    verify_in >> n >> jump >> height;
    sirkit::MonotoneMinLineContainer hull;
    hull.reserve(n);
    hull.add(-2 * height, height * height);
    std::int64_t cost = 0;
    for (int i = 1; i < n; ++i)
    {
        verify_in >> height;
        cost = hull.query_monotone(height)->value + height * height + jump;
        hull.add(-2 * height, height * height + cost);
    }
    verify_out << cost << '\n';
}
