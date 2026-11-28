#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/arc120/tasks/arc120_c

// Library: Algorithm/Adjacent_Swap_Distance.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Adjacent_Swap_Distance.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjacent_swap_distance.md
//
// API一覧
// adjacent_swap_permutation(from,to,compare={}): from[i]に対応するtoの位置を添字iの順に返す。
// 同じ値の出現は先頭から順に対応させる。添字は0始まり。
// adjacent_swap_distance(from,to,compare={}): 隣接する2要素の交換でfromをtoへ変える最小回数。
// 変換できない場合はいずれもnullopt。compareで互いに小さくない値を同値と扱う。
// std::size・添字アクセスに対応する列で使用可。長さnはUINT32_MAX以下。O((n+1)log(n+2))時間・O(n+1)領域。
//
// [同梱の転倒数]
// inversion_count(values): i<jかつvalues[j]<values[i]の組数。O(n log(n+1))時間・O(n)領域。
// range_inversion_counts(values,ranges): 各半開区間[l,r)の転倒数を入力順で返す。
// ブロック幅をbとし、O((n log(n+1)+q log(q+1))+(n²/b+qb)log(n+1))時間・O(n+q)領域。
// n≤UINT32_MAX。比較は狭義弱順序。空区間・重複値を扱う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_INVERSIONS_DEFINED
#define SIRKIT_RANGE_INVERSIONS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_range_inversion_size(std::uint64_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("range_inversion_counts: too many elements");
    }
}
} // namespace detail

template <class T>
std::vector<std::uint64_t> range_inversion_counts(
    const std::vector<T> &values, std::span<const std::pair<std::size_t, std::size_t>> ranges)
{
    const auto n = values.size();
    detail::check_range_inversion_size(n);
    std::vector<std::uint64_t> result(ranges.size());
    if (ranges.empty())
    {
        return result;
    }

    struct Query
    {
        std::size_t left, right, index, block;
    };
    const auto block_size = std::max<std::size_t>(1, static_cast<std::size_t>(n / std::sqrt(static_cast<long double>(ranges.size()))));
    std::vector<Query> queries;
    queries.reserve(ranges.size());
    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
        const auto [left, right] = ranges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > n)
        {
            throw std::out_of_range("range_inversion_counts: invalid interval");
        }
#endif
        if (left != right)
        {
            queries.push_back({left, right, i, left / block_size});
        }
    }
    if (queries.empty())
    {
        return result;
    }
    std::ranges::sort(queries, [](const Query &a, const Query &b)
                      {
                          if (a.block != b.block)
                          {
                              return a.block < b.block;
                          }
                          return a.block % 2 == 0 ? a.right < b.right : b.right < a.right;
                      });

    auto sorted = values;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end(), [](const T &a, const T &b)
                             {
                                 return (a < b) == (b < a);
                             }),
                 sorted.end());
    std::vector<std::uint32_t> rank;
    rank.reserve(n);
    for (const auto &value : values)
    {
        rank.push_back(static_cast<std::uint32_t>(std::lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin()));
    }

    std::vector<std::uint32_t> tree(sorted.size());
    const auto add = [&](std::size_t index, std::uint32_t delta)
    {
        for (; index < tree.size(); index |= index + 1)
        {
            tree[index] += delta;
        }
    };
    const auto count = [&](std::size_t end)
    {
        std::uint32_t total = 0;
        for (; end != 0; end &= end - 1)
        {
            total += tree[end - 1];
        }
        return total;
    };
    std::uint64_t inversions = 0;
    std::size_t left = 0, right = 0;
    for (const auto &[wanted_left, wanted_right, index, block] : queries)
    {
        while (left > wanted_left)
        {
            const auto r = rank[--left];
            inversions += count(r);
            add(r, 1);
        }
        while (right < wanted_right)
        {
            const auto r = rank[right];
            inversions += right - left - count(static_cast<std::size_t>(r) + 1);
            add(r, 1);
            ++right;
        }
        while (left < wanted_left)
        {
            const auto r = rank[left++];
            inversions -= count(r);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        while (right > wanted_right)
        {
            const auto r = rank[--right];
            inversions -= right - left + 1 - count(static_cast<std::size_t>(r) + 1);
            add(r, std::numeric_limits<std::uint32_t>::max());
        }
        result[index] = inversions;
    }
    return result;
}

template <class T>
std::uint64_t inversion_count(const std::vector<T> &values)
{
    const std::pair<std::size_t, std::size_t> range{0, values.size()};
    return range_inversion_counts(values, {&range, 1}).front();
}

#endif

#ifndef SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED
#define SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED

template<class From, class To, class Compare = std::less<>>
std::optional<std::vector<std::size_t>> adjacent_swap_permutation(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return std::nullopt;
    }
    detail::check_range_inversion_size(n);
    const auto sorted_indices = [&](const auto &values) {
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
            if (std::invoke(compare, values[a], values[b]))
            {
                return true;
            }
            if (std::invoke(compare, values[b], values[a]))
            {
                return false;
            }
            return a < b;
        });
        return order;
    };
    const auto sources = sorted_indices(from);
    const auto targets = sorted_indices(to);
    std::vector<std::size_t> permutation(n);
    for (std::size_t rank = 0; rank < n; ++rank)
    {
        const auto first = sources[rank];
        const auto second = targets[rank];
        if (std::invoke(compare, from[first], to[second]) || std::invoke(compare, to[second], from[first]))
        {
            return std::nullopt;
        }
        permutation[first] = second;
    }
    return permutation;
}

template<class From, class To, class Compare = std::less<>>
std::optional<std::uint64_t> adjacent_swap_distance(const From &from, const To &to, Compare compare = {})
{
    const auto permutation = adjacent_swap_permutation(from, to, std::move(compare));
    if (!permutation)
    {
        return std::nullopt;
    }
    // 同値な要素どうしの順序を保った対応なら、余分な交差がない。
    return inversion_count(*permutation);
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
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::int64_t> from(n), to(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> from[i];
        from[i] += static_cast<std::int64_t>(i);
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> to[i];
        to[i] += static_cast<std::int64_t>(i);
    }
    const auto swaps = sirkit::adjacent_swap_distance(from, to);
    if (swaps)
    {
        verify_out << *swaps << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
