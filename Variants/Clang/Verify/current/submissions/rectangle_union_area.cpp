#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/area_of_union_of_rectangles

// Library: Geometry/Rectangle_Union_Area.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Geometry/Rectangle_Union_Area.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rectangle_union_area.md
//
// API一覧
// AreaRectangle{left,bottom,right,top}: 整数座標で軸に平行な長方形。
// rectangle_union_area(rectangles): 和集合の面積をuint64_tで返す。面積が型を超えるとoverflow_error。
// rectangle_union_perimeter(rectangles): 穴の境界も含む和集合の周長をuint64_tで返す。型を超えるとoverflow_error。
// 幅または高さ0は無視する。座標の上下左右は整列済みとする。O(n log(n+1))時間・O(n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECTANGLE_UNION_AREA_DEFINED
#define SIRKIT_RECTANGLE_UNION_AREA_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t rectangle_area_workspace(std::size_t count, std::size_t scale, std::size_t limit)
{
    if (count > limit / scale)
    {
        throw std::length_error("rectangle_union_area: workspace too large");
    }
    return count * scale;
}
} // namespace detail

struct AreaRectangle
{
    std::int64_t left, bottom, right, top;
};

namespace detail
{
template <bool Boundary, bool Transpose = false>
std::uint64_t rectangle_union_sweep(std::span<const AreaRectangle> rectangles)
{
    struct Event
    {
        std::int64_t x, bottom, top;
        bool enter;
    };
    std::vector<Event> events;
    std::vector<std::int64_t> y;
    const auto capacity = detail::rectangle_area_workspace(rectangles.size(), 2, std::min(events.max_size(), y.max_size()));
    events.reserve(capacity);
    y.reserve(capacity);
    for (auto [left, bottom, right, top] : rectangles)
    {
        if constexpr (Transpose)
        {
            std::swap(left, bottom);
            std::swap(right, top);
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || bottom > top)
        {
            throw std::invalid_argument("rectangle_union_area: reversed interval");
        }
#endif
        if (left == right || bottom == top)
        {
            continue;
        }
        events.push_back({left, bottom, top, true});
        events.push_back({right, bottom, top, false});
        y.push_back(bottom);
        y.push_back(top);
    }
    if (events.empty())
    {
        return 0;
    }

    if constexpr (Boundary)
    {
        // 同じ座標では追加を先に処理し、接している辺を二重に数えない。
        std::ranges::sort(events, [](const Event &left, const Event &right)
                          {
                              if (left.x != right.x)
                              {
                                  return left.x < right.x;
                              }
                              return left.enter > right.enter;
                          });
    }
    else
    {
        std::ranges::sort(events, {}, &Event::x);
    }
    std::ranges::sort(y);
    y.erase(std::unique(y.begin(), y.end()), y.end());
    const auto segments = y.size() - 1;
    struct Node
    {
        std::size_t cover = 0;
        std::uint64_t length = 0;
    };
    std::vector<Node> tree;
    tree.resize(detail::rectangle_area_workspace(segments, 4, tree.max_size()));
    const auto length = [](std::int64_t low, std::int64_t high)
    {
        return static_cast<std::uint64_t>(high) - static_cast<std::uint64_t>(low);
    };

    const auto update = [&](auto &&self, std::size_t index, std::size_t left, std::size_t right,
                            std::size_t begin, std::size_t end, bool enter) -> void
    {
        auto &node = tree[index];
        if (begin <= left && right <= end)
        {
            if (enter)
            {
                ++node.cover;
            }
            else
            {
                --node.cover;
            }
        }
        else
        {
            const auto mid = left + (right - left) / 2;
            if (begin < mid)
            {
                self(self, 2 * index, left, mid, begin, end, enter);
            }
            if (mid < end)
            {
                self(self, 2 * index + 1, mid, right, begin, end, enter);
            }
        }
        if (node.cover != 0)
        {
            node.length = length(y[left], y[right]);
        }
        else if (right - left == 1)
        {
            node.length = 0;
        }
        else
        {
            node.length = tree[2 * index].length + tree[2 * index + 1].length;
        }
    };

    const auto update_event = [&](const Event &event)
    {
        const auto begin = static_cast<std::size_t>(std::ranges::lower_bound(y, event.bottom) - y.begin());
        const auto end = static_cast<std::size_t>(std::ranges::lower_bound(y, event.top) - y.begin());
        update(update, 1, 0, segments, begin, end, event.enter);
    };
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    if constexpr (Boundary)
    {
        std::uint64_t measure = 0;
        for (const auto &event : events)
        {
            const auto before = tree[1].length;
            update_event(event);
            const auto after = tree[1].length;
            const auto change = event.enter ? after - before : before - after;
            if (change > kMax - measure)
            {
                throw std::overflow_error("rectangle_union_perimeter: perimeter overflow");
            }
            measure += change;
        }
        return measure;
    }
    else
    {
        auto previous_x = events.front().x;
        std::uint64_t area = 0;
        for (const auto &event : events)
        {
            const auto width = length(previous_x, event.x);
            const auto covered = tree[1].length;
            if (covered != 0 && width > (kMax - area) / covered)
            {
                throw std::overflow_error("rectangle_union_area: area overflow");
            }
            area += width * covered;
            previous_x = event.x;
            update_event(event);
        }
        return area;
    }
}
} // namespace detail

inline std::uint64_t rectangle_union_area(std::span<const AreaRectangle> rectangles)
{
    return detail::rectangle_union_sweep<false>(rectangles);
}

inline std::uint64_t rectangle_union_perimeter(std::span<const AreaRectangle> rectangles)
{
    const auto vertical = detail::rectangle_union_sweep<true>(rectangles);
    const auto horizontal = detail::rectangle_union_sweep<true, true>(rectangles);
    if (horizontal > std::numeric_limits<std::uint64_t>::max() - vertical)
    {
        throw std::overflow_error("rectangle_union_perimeter: perimeter overflow");
    }
    return horizontal + vertical;
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


#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<AreaRectangle> rectangles(n);
    for (auto &[left, bottom, right, top] : rectangles)
    {
        verify_in >> left >> bottom >> right >> top;
    }
    verify_out << rectangle_union_area(rectangles) << '\n';
}
