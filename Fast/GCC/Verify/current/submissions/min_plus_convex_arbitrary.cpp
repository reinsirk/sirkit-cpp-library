#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary

// Library: Algorithm/Monge_Optimization.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Monge_Optimization.cpp
// 参考: https://ei1333.github.io/library/dp/monotone-minima.hpp
// 参考: https://ei1333.github.io/library/dp/smawk.hpp
// 参考: https://info.atcoder.jp/entry/algorithm_lectures/min_plus_convolution
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monge_optimization.md
//
// API一覧
// monotone_minima(h,w,better): 各行の最左最適列が非減少な行列。O(h+w log(h+1))時間・O(h)領域。
// smawk(h,w,better): 部分行列でも最左最適列が非減少な行列。O(h+w)時間・領域。
// better(i,j,k)は行iでkがjより真に良いときtrue。0始まり。列が0なら各行-1。h,w>=0。
// min_plus_convolution_convex_arbitrary(a,b): aが凸。O(|a|+|b|)時間・領域。
// min_plus_convolution_convex_convex(a,b): a,bが凸。O(|a|+|b|)時間、結果以外O(1)領域。
// 凸は隣接差分が非減少。min-plusの和と大小比較はTで丸めず正確に扱えること。片方が空なら空列。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONGE_OPTIMIZATION_DEFINED
#define SIRKIT_MONGE_OPTIMIZATION_DEFINED

namespace detail
{
inline void CheckMongeDimensions([[maybe_unused]] int height, [[maybe_unused]] int width)
{
}

template <class Compare>
void SmawkRows(const std::vector<int> &rows, const std::vector<int> &columns, Compare &better, std::vector<int> &answer)
{
    if (rows.empty())
    {
        return;
    }
    std::vector<int> candidates;
    candidates.reserve(std::min(rows.size(), columns.size()));
    for (int column : columns)
    {
        // 現在の列数に対応する行で、右の列に負けた列を除く。
        while (!candidates.empty() && better(rows[candidates.size() - 1], candidates.back(), column))
        {
            candidates.pop_back();
        }
        if (candidates.size() < rows.size())
        {
            candidates.push_back(column);
        }
    }

    std::vector<int> odd;
    odd.reserve(rows.size() / 2);
    for (std::size_t i = 1; i < rows.size(); i += 2)
    {
        odd.push_back(rows[i]);
    }
    SmawkRows(odd, candidates, better, answer);

    std::size_t left = 0, right = 0;
    for (std::size_t i = 0; i < rows.size(); i += 2)
    {
        const int row = rows[i];
        if (i + 1 < rows.size())
        {
            while (candidates[right] != answer[rows[i + 1]])
            {
                ++right;
            }
        }
        else
        {
            right = candidates.size() - 1;
        }
        int best = candidates[left];
        for (std::size_t j = left + 1; j <= right; ++j)
        {
            if (better(row, best, candidates[j]))
            {
                best = candidates[j];
            }
        }
        answer[row] = best;
        left = right;
    }
}

inline int CheckedMinPlusLength(std::size_t n, std::size_t m)
{
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (n > kLimit || m > kLimit - n + 1)
    {
        throw std::length_error("min-plus convolution: result length");
    }
    return static_cast<int>(n + m - 1);
}
} // namespace detail

template <class Compare>
std::vector<int> monotone_minima(int height, int width, Compare better)
{
    detail::CheckMongeDimensions(height, width);
    std::vector<int> answer(height, -1);
    if (width == 0)
    {
        return answer;
    }
    const auto solve = [&](const auto &self, int top, int bottom, int left, int right) -> void
    {
        if (top == bottom)
        {
            return;
        }
        const int row = std::midpoint(top, bottom);
        int best = left;
        for (int column = left + 1; column <= right; ++column)
        {
            if (better(row, best, column))
            {
                best = column;
            }
        }
        answer[row] = best;
        self(self, top, row, left, best);
        self(self, row + 1, bottom, best, right);
    };
    solve(solve, 0, height, 0, width - 1);
    return answer;
}

template <class Compare>
std::vector<int> smawk(int height, int width, Compare better)
{
    detail::CheckMongeDimensions(height, width);
    std::vector<int> answer(height, -1);
    if (height == 0 || width == 0)
    {
        return answer;
    }
    std::vector<int> rows(height), columns(width);
    std::iota(rows.begin(), rows.end(), 0);
    std::iota(columns.begin(), columns.end(), 0);
    detail::SmawkRows(rows, columns, better, answer);
    return answer;
}

template <class T>
std::vector<T> min_plus_convolution_convex_arbitrary(const std::vector<T> &a, const std::vector<T> &b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    const int length = detail::CheckedMinPlusLength(a.size(), b.size());
    const int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    const auto category = [n](int index)
    {
        // 有効列、右側の範囲外、左側の範囲外の順。同じ側では有効範囲に近い列が良い。
        if (index < 0)
        {
            return 1;
        }
        return index < n ? 0 : 2;
    };
    const auto indices = smawk(length, m, [&](int row, int current, int candidate)
                               {
                                   const int left_index = row - current, right_index = row - candidate;
                                   const int left_class = category(left_index), right_class = category(right_index);
                                   if (left_class != right_class)
                                   {
                                       return right_class < left_class;
                                   }
                                   if (left_class != 0)
                                   {
                                       return left_class == 2; // SMAWKの比較はcurrent < candidate。
                                   }
                                   return a[right_index] + b[candidate] < a[left_index] + b[current];
                               });
    std::vector<T> result;
    result.reserve(length);
    for (int i = 0; i < length; ++i)
    {
        result.push_back(a[i - indices[i]] + b[indices[i]]);
    }
    return result;
}

template <class T>
std::vector<T> min_plus_convolution_convex_convex(const std::vector<T> &a, const std::vector<T> &b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    std::vector<T> result;
    result.reserve(detail::CheckedMinPlusLength(a.size(), b.size()));
    std::size_t i = 0, j = 0;
    result.push_back(a[i] + b[j]);
    while (i + 1 < a.size() && j + 1 < b.size())
    {
        // 差の値を計算せず、候補となる和で次の傾きを比較する。
        if (a[i + 1] + b[j] < a[i] + b[j + 1])
        {
            ++i;
        }
        else
        {
            ++j;
        }
        result.push_back(a[i] + b[j]);
    }
    while (++i < a.size())
    {
        result.push_back(a[i] + b[j]);
    }
    while (++j < b.size())
    {
        result.push_back(a.back() + b[j]);
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
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<int> a(n), b(m);
    for (int &x : a)
    {
        verify_in >> x;
    }
    for (int &x : b)
    {
        verify_in >> x;
    }
    const auto sums = min_plus_convolution_convex_arbitrary(a, b);
    for (std::size_t i = 0; i < sums.size(); ++i)
    {
        verify_out << sums[i] << (i + 1 == sums.size() ? '\n' : ' ');
    }
}
