#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc194/tasks/abc194_e

// Library: Algorithm/Range_Mex.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Range_Mex.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_mex.md
//
// API一覧
// RangeMexQuery{left,right}: 0始まりの半開区間[left,right)。0<=left<=right<=N。
// range_mex(values,queries): 各区間のmex（含まれない最小の非負整数）を照会順で返す。空区間は0。
// 最長の区間長をKとしてO((N+Q)log(K+2))時間・O(N+Q)領域。
// subarray_mex_histogram(values): 全非空区間のmexの出現回数を0からNまで返す。O((N+1)log(N+2))時間・O(N+1)領域。
// 配列の要素は符号付き64ビット整数。負の値も許す。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_MEX_DEFINED
#define SIRKIT_RANGE_MEX_DEFINED

struct RangeMexQuery
{
    std::size_t left;
    std::size_t right;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t range_mex_base(std::size_t max_length)
{
    constexpr auto kMaxBase = std::size_t{1} << (std::numeric_limits<std::size_t>::digits - 2);
    if (max_length >= kMaxBase)
    {
        throw std::length_error("range_mex: interval too long");
    }
    return std::bit_ceil(max_length + 1);
}

inline std::uint64_t mex_subarray_count(std::size_t n)
{
    const auto odd = std::uint64_t{n | std::size_t{1}};
    const auto half_even = std::uint64_t{n / 2 + (n % 2)};
    if (half_even != 0 && odd > std::numeric_limits<std::uint64_t>::max() / half_even)
    {
        throw std::overflow_error("subarray_mex_histogram: interval count overflows");
    }
    return odd * half_even;
}

// 各左端で条件を満たす最小の右端は、単調非減少。同じ右端になる左端を区間ごとに保持する。
class MexRightBounds
{
public:
    MexRightBounds(std::size_t size, std::uint64_t count)
        : count_(count)
    {
        for (std::size_t pos = 0; pos <= size; ++pos)
        {
            bounds_.emplace_hint(bounds_.end(), pos, pos);
        }
    }

    void raise(std::size_t left, std::size_t height)
    {
        const auto head = std::prev(bounds_.upper_bound(left));
        if (head->second >= height)
        {
            return;
        }
        auto end = head;
        auto through = left;
        do
        {
            const auto next = std::next(end);
            count_ -= std::uint64_t{next->first - through} * (height - end->second);
            through = next->first;
            end = next;
        } while (end->second < height); // 左端がheight以降なら元の右端もheight以上になる。
        bounds_.erase(head->first == left ? head : std::next(head), end);
        bounds_.emplace_hint(end, left, height);
    }

    [[nodiscard]] std::uint64_t count() const
    {
        return count_;
    }

private:
    std::map<std::size_t, std::size_t> bounds_;
    std::uint64_t count_;
};
} // namespace detail

inline std::vector<std::size_t> range_mex(std::span<const std::int64_t> values,
                                          std::span<const RangeMexQuery> queries)
{
    std::size_t max_length = 0;
    std::size_t last_right = 0;
    for (const auto &[left, right] : queries)
    {
        if (left < right)
        {
            max_length = std::max(max_length, right - left);
            last_right = std::max(last_right, right);
        }
    }
    const auto no_query = queries.size();
    std::vector<std::size_t> result(no_query);
    if (max_length == 0)
    {
        return result;
    }
    std::vector<std::size_t> head(last_right, no_query), next_query(no_query);
    for (std::size_t id = 0; id < no_query; ++id)
    {
        const auto &[left, right] = queries[id];
        if (left < right)
        {
            next_query[id] = std::exchange(head[right - 1], id);
        }
    }
    const auto base = detail::range_mex_base(max_length);
    std::vector<std::size_t> minimum(base * 2);
    for (std::size_t i = 0; i < last_right; ++i)
    {
        const auto value = values[i];
        if (value >= 0 && static_cast<std::uint64_t>(value) <= max_length)
        {
            auto node = base + static_cast<std::size_t>(value);
            minimum[node] = i + 1; // 1始まりの位置と未出現を表す0を区別する。
            while (node > 1)
            {
                node /= 2;
                minimum[node] = std::min(minimum[2 * node], minimum[2 * node + 1]);
            }
        }
        for (auto id = head[i]; id != no_query; id = next_query[id])
        {
            auto node = std::size_t{1};
            while (node < base)
            {
                node *= 2;
                if (minimum[node] > queries[id].left)
                {
                    ++node;
                }
            }
            result[id] = node - base;
        }
    }
    return result;
}

inline std::vector<std::uint64_t> subarray_mex_histogram(std::span<const std::int64_t> values)
{
    const auto n = values.size();
    auto count = detail::mex_subarray_count(n);
    std::vector<std::uint64_t> result(n + 1);
    std::vector<std::vector<std::size_t>> occurrence(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto value = values[i];
        if (value >= 0 && static_cast<std::uint64_t>(value) < n)
        {
            occurrence[static_cast<std::size_t>(value)].push_back(i);
        }
    }
    detail::MexRightBounds right_bounds(n, count);
    for (std::size_t value = 0; value < n && count != 0; ++value)
    {
        std::size_t start = 0;
        for (const auto pos : occurrence[value])
        {
            right_bounds.raise(start, pos);
            start = pos + 1;
        }
        if (start < n)
        {
            right_bounds.raise(start, n);
        }
        const auto remaining = right_bounds.count();
        result[value] = count - remaining;
        count = remaining;
    }
    result[n] = count;
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n = 0, window = 0;
    verify_in >> n >> window;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<sirkit::RangeMexQuery> queries(n - window + 1);
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        queries[i] = {i, i + window};
    }
    const auto mex = sirkit::range_mex(values, queries);
    verify_out << *std::min_element(mex.begin(), mex.end()) << '\n';
}
