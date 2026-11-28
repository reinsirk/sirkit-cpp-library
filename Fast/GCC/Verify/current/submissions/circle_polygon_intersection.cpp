#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_H

// Library: Geometry/Circle_Polygon_Intersection.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Circle_Polygon_Intersection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/circle_polygon_intersection.md
//
// API一覧
// circle_polygon_intersection_area(vertices, radius, center={0,0}): 円板と単純多角形の共通部分の面積。
// 頂点はpair<double,double>の周回順。凸・凹、時計回り・反時計回りに対応。重複頂点可。
// 半径0または頂点3個未満は面積0。半径と中心相対の各座標は有限で絶対値1e140以下、半径は非負。
// 頂点数nに対しO(n)時間、O(1)追加領域。浮動小数点の丸めを含む。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <numbers>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CIRCLE_POLYGON_INTERSECTION_DEFINED
#define SIRKIT_CIRCLE_POLYGON_INTERSECTION_DEFINED

namespace detail
{
using CircleClipPoint = std::pair<double, double>;

inline double circle_clip_cross(const CircleClipPoint &a, const CircleClipPoint &b) noexcept
{
    return a.first * b.second - a.second * b.first;
}

inline double circle_clip_dot(const CircleClipPoint &a, const CircleClipPoint &b) noexcept
{
    return a.first * b.first + a.second * b.second;
}

inline double circle_clip_angle(CircleClipPoint a, CircleClipPoint b)
{
    // 円弧の端点は非零。角度の計算では長さの桁の差を取り除く。
    const auto scale_a = std::max(std::abs(a.first), std::abs(a.second));
    const auto scale_b = std::max(std::abs(b.first), std::abs(b.second));
    a = {a.first / scale_a, a.second / scale_a};
    b = {b.first / scale_b, b.second / scale_b};
    return std::atan2(circle_clip_cross(a, b), circle_clip_dot(a, b));
}

struct CircleClipContribution
{
    double twice_area;
    bool has_segment;
};

inline CircleClipContribution circle_clip_edge(const CircleClipPoint &a, const CircleClipPoint &b,
                                               double radius, double radius_squared)
{
    if (a == b)
    {
        return {0, false};
    }
    if (circle_clip_dot(a, a) <= radius_squared && circle_clip_dot(b, b) <= radius_squared)
    {
        return {circle_clip_cross(a, b), true};
    }

    const double dx = b.first - a.first;
    const double dy = b.second - a.second;
    const double length = std::hypot(dx, dy);
    const CircleClipPoint direction{dx / length, dy / length};
    const double h = circle_clip_cross(direction, a);
    const double distance = std::abs(h);
    if (distance >= radius)
    {
        return {radius_squared * circle_clip_angle(a, b), false};
    }
    const double half = std::sqrt((radius - distance) * (radius + distance));
    const double first = circle_clip_dot(a, direction);
    const double last = circle_clip_dot(b, direction);
    const double enter = std::max(first, -half);
    const double leave = std::min(last, half);
    if (enter >= leave)
    {
        return {radius_squared * circle_clip_angle(a, b), false};
    }

    // 交点は線分の端から補間せず、円の中心に近い点から計算する。
    const auto point_at = [&](double t) -> CircleClipPoint
    {
        return {t * direction.first - h * direction.second,
                t * direction.second + h * direction.first};
    };
    auto start = a;
    auto end = b;
    double arcs = 0;
    if (enter > first)
    {
        start = point_at(enter);
        arcs += circle_clip_angle(a, start);
    }
    if (leave < last)
    {
        end = point_at(leave);
        arcs += circle_clip_angle(end, b);
    }
    return {circle_clip_cross(start, end) + radius_squared * arcs, true};
}
}

[[nodiscard]] inline double circle_polygon_intersection_area(
    std::span<const std::pair<double, double>> vertices, double radius,
    std::pair<double, double> center = {})
{
    constexpr double kLimit = 1e140; // 積・累積和をdoubleの範囲に収める。
    if (!(0 <= radius && radius <= kLimit) ||
        !std::isfinite(center.first) || !std::isfinite(center.second))
    {
        throw std::invalid_argument("circle_polygon_intersection_area: circle");
    }
    const double radius_squared = radius * radius;
    if (vertices.size() < 3 || radius_squared == 0)
    {
        return 0;
    }
    const auto relative = [&](const auto &p) -> detail::CircleClipPoint
    {
        const double x = p.first - center.first;
        const double y = p.second - center.second;
        if (!(std::abs(x) <= kLimit) || !(std::abs(y) <= kLimit))
        {
            throw std::out_of_range("circle_polygon_intersection_area: relative coordinate");
        }
        return {x, y};
    };

    const double disk_area = std::numbers::pi * radius_squared;
    double twice_area = 0;
    double error = 0;
    bool has_segment = false;
    auto from = relative(vertices.back());
    for (const auto &vertex : vertices)
    {
        const auto to = relative(vertex);
        const auto edge = detail::circle_clip_edge(from, to, radius, radius_squared);
        const double summand = edge.twice_area - error;
        const double next = twice_area + summand;
        error = (next - twice_area) - summand;
        twice_area = next;
        has_segment |= edge.has_segment;
        from = to;
    }
    // 円板の内部に入る辺がなければ、境界が囲む角度は0または1周。
    if (!has_segment)
    {
        return std::abs(twice_area) > disk_area ? disk_area : 0;
    }
    return std::min(disk_area, std::abs(twice_area) * 0.5);
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


#include <utility>
#include <vector>

int main()
{
    int n = 0, radius = 0;
    verify_in >> n >> radius;
    std::vector<std::pair<double, double>> vertices(n);
    for (auto &[x, y] : vertices)
    {
        int px = 0, py = 0;
        verify_in >> px >> py;
        x = px;
        y = py;
    }
    verify_out.fixed(12) << sirkit::circle_polygon_intersection_area(vertices, radius) << '\n';
}
