#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 11e0c053e39d502938d938cb73e8d6008bc0b8b3c58cac1cfb5a226f2cc9b489
// Verify: https://atcoder.jp/contests/abc472/tasks/abc472_f

// Library: Geometry/Polygon_Chord_Moments.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考: https://atcoder.jp/contests/abc472/editorial/24789
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polygon_chord_moments.md
//
// API一覧
// PolygonChordMoments(points): 境界順の整数頂点を前処理。O(n)時間・領域。
// slice(from,to): fromからtoまで入力の順に進み、to→fromの辺で閉じる部分の符号付き面積・重心。O(1)。
// total(): 元の多角形の符号付き面積・重心。O(1)。size(): 頂点数。O(1)。
// 結果のsigned_area2は符号付き面積の2倍（正確なint64_t）、centroidは重心座標。面積0はnullopt。
// 頂点はpair<int64_t,int64_t>で|座標|<=10^9。凸多角形を周回順に渡す。時計回りも可。
// slice(i,i)は面積0。最後の頂点に先頭を再掲しない。丸めは重心計算時のみ。
// コピーはO(1)で前処理を共有。移動元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POLYGON_CHORD_MOMENTS_DEFINED
#define SIRKIT_POLYGON_CHORD_MOMENTS_DEFINED

namespace detail
{
struct PolygonMomentInteger
{
    std::uint64_t low = 0;
    std::uint64_t high = 0;

    PolygonMomentInteger operator+(const PolygonMomentInteger &other) const noexcept
    {
        const auto sum = low + other.low;
        return {sum, high + other.high + static_cast<unsigned>(sum < low)};
    }

    PolygonMomentInteger operator-(const PolygonMomentInteger &other) const noexcept
    {
        return {low - other.low, high - other.high - static_cast<unsigned>(low < other.low)};
    }

    static PolygonMomentInteger FromSigned(std::int64_t value) noexcept
    {
        return {static_cast<std::uint64_t>(value),
                value < 0 ? std::numeric_limits<std::uint64_t>::max() : 0};
    }

    static PolygonMomentInteger Product(std::int64_t a, std::int64_t b) noexcept
    {
        const auto x = Magnitude(a);
        const auto y = Magnitude(b); // |b| <= 2e9
        const auto low_product = (x & 0xffffffffULL) * y;
        const auto high_product = (x >> 32) * y;
        const auto product = PolygonMomentInteger{low_product, 0} +
                             PolygonMomentInteger{high_product << 32, high_product >> 32};
        return (a < 0) == (b < 0) ? product : PolygonMomentInteger{} - product;
    }

    std::int64_t ToSigned() const
    {
        const auto value = std::bit_cast<std::int64_t>(low);
        if (high != FromSigned(value).high)
        {
            throw std::overflow_error("PolygonChordMoments: twice area outside int64");
        }
        return value;
    }

    double ToDouble() const noexcept
    {
        const bool negative = (high >> 63) != 0;
        const auto magnitude = negative ? PolygonMomentInteger{} - *this : *this;
        const double result = std::ldexp(static_cast<double>(magnitude.high), 64) +
                              static_cast<double>(magnitude.low);
        return negative ? -result : result;
    }

private:
    static std::uint64_t Magnitude(std::int64_t value) noexcept
    {
        const auto bits = static_cast<std::uint64_t>(value);
        return value < 0 ? std::uint64_t{0} - bits : bits;
    }
};
}

class PolygonChordMoments
{
public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    struct Result
    {
        std::int64_t signed_area2;
        std::optional<std::pair<double, double>> centroid;
    };

    explicit PolygonChordMoments(std::span<const Point> points)
        : state_(std::make_shared<const Storage>(points))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return state_ ? state_->points.size() : 0;
    }

    [[nodiscard]] Result total() const
    {
        return state_ ? Summarize(state_->prefix.back()) : Result{0, std::nullopt};
    }

    [[nodiscard]] Result slice(std::size_t from, std::size_t to) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from >= size() || to >= size())
        {
            throw std::out_of_range("PolygonChordMoments: vertex index");
        }
#endif
        const auto &prefix = state_->prefix;
        auto moments = prefix[to] - prefix[from];
        if (from > to)
        {
            moments = moments + prefix.back();
        }
        return Summarize(moments + EdgeMoments(state_->points[to], state_->points[from]));
    }

private:
    using Integer = detail::PolygonMomentInteger;

    struct Moments
    {
        Integer area;
        Integer x;
        Integer y;

        Moments operator+(const Moments &other) const noexcept
        {
            return {area + other.area, x + other.x, y + other.y};
        }

        Moments operator-(const Moments &other) const noexcept
        {
            return {area - other.area, x - other.x, y - other.y};
        }
    };

    struct Storage
    {
        std::vector<Point> points;
        std::vector<Moments> prefix;

        explicit Storage(std::span<const Point> input) : points(input.begin(), input.end())
        {
            for (const auto &[x, y] : points)
            {
                CheckCoordinate(x);
                CheckCoordinate(y);
            }
            prefix.reserve(points.size() + 1);
            prefix.emplace_back();
            for (std::size_t to = 1; to < points.size(); ++to)
            {
                prefix.push_back(prefix.back() + EdgeMoments(points[to - 1], points[to]));
            }
            if (!points.empty())
            {
                prefix.push_back(prefix.back() + EdgeMoments(points.back(), points.front()));
            }
        }
    };

    std::shared_ptr<const Storage> state_;

    static void CheckCoordinate(std::int64_t value)
    {
        constexpr std::int64_t kLimit = 1000000000;
        if (value < -kLimit || value > kLimit)
        {
            throw std::out_of_range("PolygonChordMoments: coordinate magnitude");
        }
    }

    static Moments EdgeMoments(const Point &from, const Point &to) noexcept
    {
        const auto cross = from.first * to.second - from.second * to.first;
        return {Integer::FromSigned(cross),
                Integer::Product(cross, from.first + to.first),
                Integer::Product(cross, from.second + to.second)};
    }

    static Result Summarize(const Moments &moments)
    {
        const auto area = moments.area.ToSigned();
        if (area == 0)
        {
            return {0, std::nullopt};
        }
        const double divisor = 3 * static_cast<double>(area);
        return {area, std::pair{moments.x.ToDouble() / divisor, moments.y.ToDouble() / divisor}};
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


#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<sirkit::PolygonChordMoments::Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const sirkit::PolygonChordMoments polygon(points);
    verify_out.fixed(12);
    for (int i = 0; i < q; ++i)
    {
        int u = 0, v = 0;
        verify_in >> u >> v;
        const auto [x, y] = *polygon.slice(u - 1, v - 1).centroid;
        verify_out << x << ' ' << y << '\n';
    }
}
