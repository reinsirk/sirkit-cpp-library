#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc228/tasks/abc228_f

// Library: DataStructure/Sliding_Window_Extrema.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sliding_Window_Extrema.cpp
// 参考: https://atcoder.jp/contests/abc228/editorial/2945
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sliding_window_extrema.md
//
// API一覧
// sliding_window_argextrema(values,k,compare=std::less<>{}): 幅kの各窓で選ぶ要素の0始まり添字を返す。
// sliding_window_extrema(values,k,compare=std::less<>{}): 同じ窓の選んだ値を返す。
// 長さnをO(1)で取得しランダムアクセスできる列。O(n)時間・O(k)作業領域。出力は左端の昇順。
// sliding_window_extrema_2d(grid,kh,kw,compare=std::less<>{}): 高さkh、幅kwの各窓から値を選ぶ。
// gridは長方形のvector<vector<T>>。O(HW)時間・O(H(W-kw+1)+max(kh,kw))追加領域。
// compareはconst呼出し可能な狭義弱順序。std::greater<>{}なら最大値。等価な候補では行、次いで列の小さいものを選ぶ。
// 幅または高さ0、または窓が入力より大きければ空配列。検査有効時に不揃いな行は例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SLIDING_WINDOW_EXTREMA_DEFINED
#define SIRKIT_SLIDING_WINDOW_EXTREMA_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class Sequence>
concept WindowExtremaRange =
    std::ranges::random_access_range<const Sequence> && std::ranges::sized_range<const Sequence>;

template <class At, class Compare, class Emit>
void ScanWindowExtrema(std::size_t length, std::size_t width, std::vector<std::size_t> &queue, const At &at,
                       const Compare &compare, const Emit &emit)
{
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t count = 0;
    for (std::size_t i = 0; i < length; ++i)
    {
        if (i >= width && queue[head] <= i - width)
        {
            if (++head == width)
            {
                head = 0;
            }
            --count;
        }

        const auto &current = at(i);
        while (count != 0)
        {
            const auto back = (tail == 0 ? width : tail) - 1;
            if (!std::invoke(compare, current, at(queue[back])))
            {
                break;
            }
            tail = back;
            --count;
        }
        queue[tail] = i;
        if (++tail == width)
        {
            tail = 0;
        }
        ++count;
        if (i >= width - 1)
        {
            emit(i - (width - 1), queue[head]);
        }
    }
}

template <bool Indices, WindowExtremaRange Sequence, class Compare>
auto SlidingWindowExtrema(const Sequence &values, std::size_t width, const Compare &compare)
{
    using Element = std::ranges::range_value_t<const Sequence>;
    using Result = std::conditional_t<Indices, std::size_t, Element>;
    std::vector<Result> output;
    const auto length = static_cast<std::size_t>(std::ranges::size(values));
    if (width == 0 || width > length)
    {
        return output;
    }

    std::vector<std::size_t> queue(width);
    output.reserve(length - width + 1);
    const auto at = [first = std::ranges::begin(values)](std::size_t i) -> decltype(auto) {
        return first[static_cast<std::iter_difference_t<decltype(first)>>(i)];
    };
    ScanWindowExtrema(length, width, queue, at, compare, [&](std::size_t, std::size_t selected) {
        if constexpr (Indices)
        {
            output.push_back(selected);
        }
        else
        {
            output.emplace_back(at(selected));
        }
    });
    return output;
}
} // namespace reinsirk::detail

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WindowExtremaRange Sequence, class Compare = std::less<>>
[[nodiscard]] std::vector<std::size_t> sliding_window_argextrema(const Sequence &values, std::size_t width,
                                                               const Compare &compare = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SlidingWindowExtrema<true>(values, width, compare);
}

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WindowExtremaRange Sequence, class Compare = std::less<>>
[[nodiscard]] std::vector<std::ranges::range_value_t<const Sequence>>
sliding_window_extrema(const Sequence &values, std::size_t width, const Compare &compare = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SlidingWindowExtrema<false>(values, width, compare);
}

template <class T, class Compare = std::less<>>
[[nodiscard]] std::vector<std::vector<T>> sliding_window_extrema_2d(
    const std::vector<std::vector<T>> &grid, std::size_t window_height, std::size_t window_width,
    const Compare &compare = {})
{
    const auto height = grid.size();
    const auto width = grid.empty() ? 0 : grid.front().size();
    if (window_height == 0 || window_width == 0 || window_height > height || window_width > width)
    {
        return {};
    }

    const auto horizontal_width = width - window_width + 1;
    std::vector<std::size_t> queue(std::max(window_height, window_width));
    std::vector<std::vector<T>> output(height);
    for (std::size_t y = 0; y < height; ++y)
    {
        const auto &row = grid[y];
        output[y].reserve(horizontal_width);
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ScanWindowExtrema(
            width, window_width, queue, [&](std::size_t x) -> decltype(auto) { return row[x]; }, compare,
            [&](std::size_t, std::size_t selected) { output[y].push_back(row[selected]); });
    }

    for (std::size_t x = 0; x < horizontal_width; ++x)
    {
        const auto at = [&](std::size_t y) -> decltype(auto) { return std::as_const(output)[y][x]; };
        // 出力を書き込む行は現在の窓の先頭で、次の窓から外れる。
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ScanWindowExtrema(
            height, window_height, queue, at, compare, [&](std::size_t y, std::size_t selected) {
                if (y != selected)
                {
                    output[y][x] = at(selected);
                }
            });
    }
    output.resize(height - window_height + 1);
    return output;
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
#include <functional>
#include <vector>

int main()
{
    std::size_t h{}, w{}, h_black{}, w_black{}, h_white{}, w_white{};
    verify_in >> h >> w >> h_black >> w_black >> h_white >> w_white;
    h_white = std::min(h_white, h_black);
    w_white = std::min(w_white, w_black);

    const std::size_t stride = w + 1;
    std::vector<std::int64_t> prefix((h + 1) * stride);
    for (std::size_t i = 1; i <= h; ++i)
    {
        std::int64_t row_sum = 0;
        for (std::size_t j = 1; j <= w; ++j)
        {
            std::int64_t value{};
            verify_in >> value;
            row_sum += value;
            prefix[i * stride + j] = prefix[(i - 1) * stride + j] + row_sum;
        }
    }
    const auto sum = [&](std::size_t y, std::size_t x, std::size_t dy, std::size_t dx) {
        const auto top = y * stride;
        const auto bottom = (y + dy) * stride;
        return prefix[bottom + x + dx] - prefix[bottom + x] - prefix[top + x + dx] + prefix[top + x];
    };

    std::vector white_sums(h - h_white + 1, std::vector<std::int64_t>(w - w_white + 1));
    for (std::size_t y = 0; y < white_sums.size(); ++y)
    {
        for (std::size_t x = 0; x < white_sums[y].size(); ++x)
        {
            white_sums[y][x] = sum(y, x, h_white, w_white);
        }
    }
    const auto worst = sirkit::sliding_window_extrema_2d(
        white_sums, h_black - h_white + 1, w_black - w_white + 1, std::greater<>{});
    std::int64_t score = 0;
    for (std::size_t y = 0; y < worst.size(); ++y)
    {
        for (std::size_t x = 0; x < worst[y].size(); ++x)
        {
            score = std::max(score, sum(y, x, h_black, w_black) - worst[y][x]);
        }
    }
    verify_out << score << '\n';
}
