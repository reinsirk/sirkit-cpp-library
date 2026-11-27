#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/convex_layers

// Library: Geometry/Convex_Layers.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Convex_Layers.cpp
// 参考: M. H. Overmars, J. van Leeuwen, Maintenance of configurations in the plane
// (https://doi.org/10.1016/0022-0000(81)90012-X)。接線探索を含むコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convex_layers.md
//
// API一覧
// convex_layers(points): 凸包の境界上にあるすべての点を取り除く操作を繰り返し、各点の削除回を返す。
// 戻り値は入力順、最初の層は1。同じ座標の点は同じ層。辺上の点や一直線上の残点も同時に取り除く。
// x、yそれぞれの最大値と最小値の差は10^6以下。超える場合はoverflow_error。
// O(N log²(N+1))時間・O(N)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONVEX_LAYERS_DEFINED
#define SIRKIT_CONVEX_LAYERS_DEFINED

namespace detail
{
using ConvexLayerPoint = std::pair<std::int64_t, std::int64_t>;

class ConvexLayerHull
{
    using Index = std::size_t;

    struct Bridge
    {
        Index branch = 0;
        Index first = 0;
        Index last = 0;
    };

public:
    ConvexLayerHull(std::span<const ConvexLayerPoint> points, std::int64_t direction)
        : points_(points), direction_(direction), leaves_(std::bit_ceil(points.size())), tree_(2 * leaves_)
    {
        for (Index i = 0; i < points_.size(); ++i)
        {
            tree_[leaves_ + i] = {leaves_ + i, i, i};
        }
        for (auto node = leaves_; node > 1;)
        {
            --node;
            Repair(node);
        }
    }

    bool Empty() const
    {
        return tree_[1].branch == 0;
    }

    void Boundary(std::vector<Index> &boundary) const
    {
        Collect(tree_[1], 0, points_.size() - 1, boundary);
    }

    void Erase(Index point)
    {
        auto node = leaves_ + point;
        tree_[node] = {};
        while (node > 1)
        {
            node /= 2;
            const auto bridge = tree_[node];
            if (bridge.branch != node || point == bridge.first || point == bridge.last)
            {
                Repair(node);
            }
        }
    }

private:
    std::span<const ConvexLayerPoint> points_;
    std::int64_t direction_;
    Index leaves_;
    std::vector<Bridge> tree_;

    Bridge Child(Bridge hull, bool right) const
    {
        return tree_[2 * hull.branch + right];
    }

    static std::int64_t Cross(const ConvexLayerPoint &a, const ConvexLayerPoint &b, const ConvexLayerPoint &c)
    {
        return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first);
    }

    void Collect(Bridge bridge, Index first, Index last, std::vector<Index> &boundary) const
    {
        // 範囲はこの凸包上の点を少なくとも1つ含む。葉まで来たらその点を追加する。
        if (bridge.branch >= leaves_)
        {
            boundary.push_back(bridge.first);
            return;
        }
        if (first <= bridge.first)
        {
            Collect(Child(bridge, false), first, std::min(last, bridge.first), boundary);
        }
        if (bridge.last <= last)
        {
            Collect(Child(bridge, true), std::max(first, bridge.last), last, boundary);
        }
    }

    std::int64_t CompareHeight(const ConvexLayerPoint &a, const ConvexLayerPoint &b,
                               const ConvexLayerPoint &c, const ConvexLayerPoint &d,
                               const ConvexLayerPoint &midpoint_twice) const
    {
        const auto height_numerator = [&](const ConvexLayerPoint &from, const ConvexLayerPoint &to)
        {
            return (2 * from.second - midpoint_twice.second) * (to.first - from.first) +
                   (midpoint_twice.first - 2 * from.first) * (to.second - from.second);
        };
        const auto ab = height_numerator(a, b);
        const auto cd = height_numerator(c, d);
        // x+εyで厳密に左右分離する。分母を払った多項式の定数項、εの係数の順に調べる。
        const auto ordinary = ab * (d.first - c.first) - cd * (b.first - a.first);
        const auto comparison = ordinary != 0 ? ordinary : ab * (d.second - c.second) - cd * (b.second - a.second);
        return ((comparison > 0) - (comparison < 0)) * direction_;
    }

    Bridge FindBridge(Index node, Bridge left, Bridge right) const
    {
        const auto child_width = leaves_ >> std::bit_width(node);
        const auto split = (2 * node + 1) * child_width - leaves_;
        const ConvexLayerPoint midpoint_twice{points_[split - 1].first + points_[split].first,
                                              points_[split - 1].second + points_[split].second};

        // 分割位置での支持直線の高さは、傾きを増すと左の凸包で増加し右の凸包で減少する。
        // 2つの辺の傾きと高さを比較し、共通接線の接点を含む子へ下りる。
        while (left.first != left.last || right.first != right.last)
        {
            const auto &a = points_[left.first], &b = points_[left.last];
            const auto &c = points_[right.first], &d = points_[right.last];
            if (left.first == left.last)
            {
                right = Child(right, direction_ * Cross(c, d, a) < 0);
                continue;
            }
            if (right.first == right.last)
            {
                left = Child(left, direction_ * Cross(a, b, c) >= 0);
                continue;
            }

            const auto slope_order = direction_ *
                                     ((b.second - a.second) * (d.first - c.first) -
                                      (d.second - c.second) * (b.first - a.first));
            const auto height_order = CompareHeight(a, b, c, d, midpoint_twice);
            if (slope_order == 0 && height_order == 0)
            {
                // 同一直線では、境界上の点を隠さない最も短い橋を選ぶ。
                left = Child(left, true);
                right = Child(right, false);
            }
            else if (slope_order <= 0)
            {
                if (height_order <= 0)
                {
                    left = Child(left, true);
                }
                else
                {
                    right = Child(right, false);
                }
            }
            else if (height_order <= 0)
            {
                right = Child(right, true);
            }
            else
            {
                left = Child(left, false);
            }
        }
        return {node, left.first, right.first};
    }

    void Repair(Index node)
    {
        const auto left = tree_[2 * node];
        const auto right = tree_[2 * node + 1];
        if (left.branch == 0)
        {
            tree_[node] = right;
        }
        else if (right.branch == 0)
        {
            tree_[node] = left;
        }
        else
        {
            tree_[node] = FindBridge(node, left, right);
        }
    }
};
} // namespace detail

inline std::vector<std::size_t> convex_layers(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    if (points.empty())
    {
        return {};
    }

    const auto min_x = std::ranges::min_element(points, {}, &detail::ConvexLayerPoint::first)->first;
    const auto min_y = std::ranges::min_element(points, {}, &detail::ConvexLayerPoint::second)->second;
    constexpr std::uint64_t kMaxSpan = 1'000'000;
    const auto translate = [&](std::int64_t value, std::int64_t minimum)
    {
        const auto difference = static_cast<std::uint64_t>(value) - static_cast<std::uint64_t>(minimum);
        if (difference > kMaxSpan)
        {
            throw std::overflow_error("convex_layers: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(difference);
    };
    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return points[i];
                      });
    std::vector<detail::ConvexLayerPoint> unique;
    unique.reserve(points.size());
    std::vector<std::size_t> rank(points.size());
    for (const auto i : order)
    {
        const auto &[x, y] = points[i];
        const detail::ConvexLayerPoint point{translate(x, min_x), translate(y, min_y)};
        if (unique.empty() || unique.back() != point)
        {
            unique.push_back(point);
        }
        rank[i] = unique.size() - 1;
    }

    detail::ConvexLayerHull lower(unique, 1), upper(unique, -1);
    std::vector<std::size_t> layers(unique.size()), boundary;
    for (std::size_t layer = 1; !lower.Empty(); ++layer)
    {
        boundary.clear();
        lower.Boundary(boundary);
        upper.Boundary(boundary);
        for (const auto point : boundary)
        {
            if (layers[point] == 0)
            {
                layers[point] = layer;
                lower.Erase(point);
                upper.Erase(point);
            }
        }
    }
    for (auto &point : rank)
    {
        point = layers[point];
    }
    return rank;
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
#include <utility>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::pair<std::int64_t, std::int64_t>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    for (const auto layer : sirkit::convex_layers(points))
    {
        verify_out << layer << '\n';
    }
}
