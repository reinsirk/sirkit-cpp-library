#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/count_points_in_triangle

// Library: Geometry/Count_Points_In_Triangle.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Count_Points_In_Triangle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/count_points_in_triangle.md
//
// API一覧
// CountPointsInTriangle(vertices, points): 頂点候補N点と、数える対象M点から前処理する。
// count(a, b, c): 指定した0始まり添字3つの三角形の内部にある対象点数を返す。境界を除く。
// 頂点の向きは任意。面積0なら0。同じ座標の対象点は重複も数える。
// 2集合を合わせた各軸の座標幅は2×10^9以下。範囲外はoverflow_error。
// N>0で前処理O(N²(M+1))時間、O(N²)追加領域。countはO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_POINTS_IN_TRIANGLE_DEFINED
#define SIRKIT_COUNT_POINTS_IN_TRIANGLE_DEFINED

class CountPointsInTriangle
{
public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    CountPointsInTriangle(std::span<const Point> vertices, std::span<const Point> points)
        : vertices_(vertices.begin(), vertices.end()), multiplicity_(vertices.size())
    {
        if (vertices.empty())
        {
            return;
        }
        CheckBounds(vertices, points);
        scores_.assign(vertices.size(), std::vector<std::int64_t>(vertices.size()));
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            multiplicity_[i] = std::ranges::count(points, vertices_[i]);
            for (std::size_t j = 0; j < i; ++j)
            {
                BuildEdge(i, j, points);
            }
        }
    }

    std::size_t count(std::size_t a, std::size_t b, std::size_t c) const
    {
        const auto area = Side(vertices_[a], vertices_[b], vertices_[c]);
        if (area == 0)
        {
            return 0;
        }
        if (area < 0)
        {
            std::swap(b, c);
        }
        const bool ab = ShiftInward(vertices_[a], vertices_[b]);
        const bool bc = ShiftInward(vertices_[b], vertices_[c]);
        const bool ca = ShiftInward(vertices_[c], vertices_[a]);

        auto result = scores_[a][b] + scores_[b][c] + scores_[c][a];
        // 頂点上の点が内側へ移るのは、接する2辺がどちらも内向きの場合だけ。
        result -= multiplicity_[a] * (ca && ab);
        result -= multiplicity_[b] * (ab && bc);
        result -= multiplicity_[c] * (bc && ca);
        return static_cast<std::size_t>(result);
    }

private:
    std::vector<Point> vertices_;
    std::vector<std::int64_t> multiplicity_;
    std::vector<std::vector<std::int64_t>> scores_;

    static void CheckBounds(std::span<const Point> vertices, std::span<const Point> points)
    {
        Point low = vertices.front(), high = low;
        for (const auto set : {vertices, points})
        {
            for (const auto &[x, y] : set)
            {
                low.first = std::min(low.first, x);
                low.second = std::min(low.second, y);
                high.first = std::max(high.first, x);
                high.second = std::max(high.second, y);
            }
        }
        constexpr std::uint64_t kMaxSpan = 2'000'000'000;
        const auto width = [](std::int64_t min, std::int64_t max)
        {
            return static_cast<std::uint64_t>(max) - static_cast<std::uint64_t>(min);
        };
        if (width(low.first, high.first) > kMaxSpan || width(low.second, high.second) > kMaxSpan)
        {
            throw std::overflow_error("CountPointsInTriangle: coordinate range is too wide");
        }
    }

    static std::int64_t Side(const Point &a, const Point &b, const Point &p)
    {
        return (b.first - a.first) * (p.second - a.second) - (b.second - a.second) * (p.first - a.first);
    }

    static bool ShiftInward(const Point &a, const Point &b)
    {
        // 評価点を(ε, ε²)だけ動かすとき、有向辺の左側へ入るか。
        return b.second < a.second || (a.second == b.second && a.first < b.first);
    }

    void BuildEdge(std::size_t a, std::size_t b, std::span<const Point> points)
    {
        const auto order = [](const Point &point)
        {
            return std::pair(point.second, point.first);
        };
        if (order(vertices_[b]) < order(vertices_[a]))
        {
            std::swap(a, b);
        }
        const auto &lower = vertices_[a], &upper = vertices_[b];
        if (lower.second == upper.second)
        {
            // 水平な辺は半直線と交差しない。右向きの辺からはずらして数えた辺上の点を引く。
            for (const auto &[x, y] : points)
            {
                scores_[a][b] -= y == lower.second && lower.first < x && x < upper.first;
            }
            return;
        }
        std::int64_t crossings = 0, border = 0;
        for (const auto &point : points)
        {
            if (lower.second <= point.second && point.second < upper.second)
            {
                const auto side = Side(lower, upper, point);
                crossings += side > 0;
                border += side == 0 && lower.second < point.second;
            }
        }
        // (ε, ε²)のずれを入れた巻き数。下向きの辺では端点以外の辺上の点を補正する。
        scores_[a][b] = crossings;
        scores_[b][a] = -crossings - border;
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
#include <vector>

int main()
{
    const auto read_points = []
    {
        std::size_t n{};
        verify_in >> n;
        std::vector<sirkit::CountPointsInTriangle::Point> points(n);
        for (auto &[x, y] : points)
        {
            verify_in >> x >> y;
        }
        return points;
    };
    const auto vertices = read_points();
    const auto points = read_points();
    const sirkit::CountPointsInTriangle triangles(vertices, points);

    std::size_t q{};
    verify_in >> q;
    while (q-- != 0)
    {
        std::size_t a{}, b{}, c{};
        verify_in >> a >> b >> c;
        verify_out << triangles.count(a, b, c) << '\n';
    }
}
