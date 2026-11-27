#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/furthest_pair

// Library: Geometry/Furthest_Pair.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Furthest_Pair.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/furthest_pair.md
//
// API一覧
// furthest_pair(points): int32_t座標の最遠点対の距離二乗と元の添字を返す。2点未満ならnullopt。
// FurthestPairResult{distance_squared,first_index,second_index}: 距離二乗はuint64_t、添字は0始まりで昇順。
// 同じ座標の点も扱う。同距離の対は任意。距離二乗がuint64_tを超える場合はoverflow_error。
// 入力を変更せず、N点でO(N log(N+1))時間・O(N)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FURTHEST_PAIR_DEFINED
#define SIRKIT_FURTHEST_PAIR_DEFINED

struct FurthestPairResult
{
    std::uint64_t distance_squared;
    std::size_t first_index;
    std::size_t second_index;
};

namespace detail
{
struct FurthestPairPoint
{
    std::pair<std::int32_t, std::int32_t> coordinates;
    std::size_t index;
};

inline std::uint64_t FurthestPairMagnitude(std::int64_t value)
{
    // 呼出側はint32_tの座標差だけを渡す。
    return static_cast<std::uint64_t>(value < 0 ? -value : value);
}

inline int FurthestPairCrossSign(const FurthestPairPoint &a, const FurthestPairPoint &b,
                                 const FurthestPairPoint &c, const FurthestPairPoint &d)
{
    const auto ab_x = static_cast<std::int64_t>(b.coordinates.first) - a.coordinates.first;
    const auto ab_y = static_cast<std::int64_t>(b.coordinates.second) - a.coordinates.second;
    const auto cd_x = static_cast<std::int64_t>(d.coordinates.first) - c.coordinates.first;
    const auto cd_y = static_cast<std::int64_t>(d.coordinates.second) - c.coordinates.second;

    // 各積の大きさはuint64_tに収まる。差は求めず、積の順序だけ比較する。
    const auto left = FurthestPairMagnitude(ab_x) * FurthestPairMagnitude(cd_y);
    const auto right = FurthestPairMagnitude(ab_y) * FurthestPairMagnitude(cd_x);
    const bool left_negative = left != 0 && ((ab_x < 0) != (cd_y < 0));
    const bool right_negative = right != 0 && ((ab_y < 0) != (cd_x < 0));
    if (left_negative != right_negative)
    {
        return left_negative ? -1 : 1;
    }
    const int order = (left > right) - (left < right);
    return left_negative ? -order : order;
}

inline void ImproveFurthestPair(FurthestPairResult &best, const FurthestPairPoint &a, const FurthestPairPoint &b)
{
    const auto dx = FurthestPairMagnitude(static_cast<std::int64_t>(a.coordinates.first) - b.coordinates.first);
    const auto dy = FurthestPairMagnitude(static_cast<std::int64_t>(a.coordinates.second) - b.coordinates.second);
    const auto square_x = dx * dx;
    const auto squared = square_x + dy * dy;
    if (squared < square_x)
    {
        throw std::overflow_error("furthest_pair: squared distance exceeds uint64_t");
    }
    if (squared > best.distance_squared)
    {
        best = {squared, std::min(a.index, b.index), std::max(a.index, b.index)};
    }
}

inline std::vector<FurthestPairPoint> FurthestPairHull(std::vector<FurthestPairPoint> &points)
{
    std::ranges::sort(points, {}, &FurthestPairPoint::coordinates);
    const auto duplicate = std::ranges::unique(points, {}, &FurthestPairPoint::coordinates);
    points.erase(duplicate.begin(), duplicate.end());
    if (points.size() <= 2)
    {
        return std::move(points);
    }

    std::vector<FurthestPairPoint> hull;
    hull.reserve(points.size());
    const auto append = [&](const FurthestPairPoint &point, std::size_t first)
    {
        while (hull.size() - first >= 2 &&
               FurthestPairCrossSign(hull[hull.size() - 2], hull.back(), hull.back(), point) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(point);
    };
    for (const auto &point : points)
    {
        append(point, 0);
    }
    const auto first = hull.size() - 1;
    for (auto i = points.size() - 1; i != 0; --i)
    {
        append(points[i - 1], first);
    }
    hull.pop_back();
    return hull;
}
} // namespace detail

inline std::optional<FurthestPairResult> furthest_pair(
    std::span<const std::pair<std::int32_t, std::int32_t>> points)
{
    if (points.size() < 2)
    {
        return std::nullopt;
    }
    FurthestPairResult best{0, 0, 1};
    if (points.size() == 2)
    {
        detail::ImproveFurthestPair(best, {points[0], 0}, {points[1], 1});
        return best;
    }

    std::vector<detail::FurthestPairPoint> indexed;
    indexed.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        indexed.push_back({points[i], i});
    }
    const auto hull = detail::FurthestPairHull(indexed);
    if (hull.size() == 1)
    {
        return best;
    }
    if (hull.size() == 2)
    {
        detail::ImproveFurthestPair(best, hull[0], hull[1]);
        return best;
    }

    const auto next = [&](std::size_t i)
    {
        return i + 1 == hull.size() ? 0 : i + 1;
    };
    std::size_t opposite = 1;
    for (std::size_t i = 0; i < hull.size(); ++i)
    {
        const auto j = next(i);
        auto after = next(opposite);
        int turn = detail::FurthestPairCrossSign(hull[i], hull[j], hull[opposite], hull[after]);
        while (turn > 0)
        {
            opposite = after;
            after = next(opposite);
            turn = detail::FurthestPairCrossSign(hull[i], hull[j], hull[opposite], hull[after]);
        }
        detail::ImproveFurthestPair(best, hull[i], hull[opposite]);
        detail::ImproveFurthestPair(best, hull[j], hull[opposite]);
        if (turn == 0)
        {
            // 対向辺が平行な場合は、その両端を候補に含める。
            detail::ImproveFurthestPair(best, hull[i], hull[after]);
            detail::ImproveFurthestPair(best, hull[j], hull[after]);
        }
    }
    return best;
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

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    std::vector<std::pair<std::int32_t, std::int32_t>> points;
    while (tests-- != 0)
    {
        int n{};
        verify_in >> n;
        points.resize(n);
        for (auto &[x, y] : points)
        {
            verify_in >> x >> y;
        }
        const auto answer = furthest_pair(points);
        verify_out << answer->first_index << ' ' << answer->second_index << '\n';
    }
}
