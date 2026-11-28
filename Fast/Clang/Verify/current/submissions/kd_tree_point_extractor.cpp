#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/awc0163/tasks/awc0163_e

// Library: Geometry/Kd_Tree_Point_Extractor.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Kd_Tree_Point_Extractor.cpp
// 数学的参考: https://atcoder.jp/contests/awc0163/editorial/25858
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/kd_tree_point_extractor.md
//
// API一覧
// KdTreePointExtractor(points): 2次元整数点を前処理する。平均O(n log(n+1))時間、O(n)領域。
// Point: std::array<std::int32_t,2>。入力順の0始まりのIDを使う。同じ座標にも別々のIDが付く。
// size(): まだ取り出していない点の数。O(1)。
// take_in_circle(center,radius,output): 閉円内の点のIDをoutputの末尾に追加し、木から取り除く。radiusはuint32_t。
// take_in_rectangle(lower,upper,output): 閉長方形内の点を同様に取り出す。各軸でlower<=upper。
// 出力順は未規定。照会は訪れた節点数と出力数に比例し最悪O(n)。再確保時はoutputの既存要素数も加算。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KD_TREE_POINT_EXTRACTOR_DEFINED
#define SIRKIT_KD_TREE_POINT_EXTRACTOR_DEFINED

class KdTreePointExtractor
{
public:
    using Point = std::array<std::int32_t, 2>;

    explicit KdTreePointExtractor(const std::vector<Point> &points)
    {
        nodes_.reserve(points.size());
        for (std::size_t id = 0; id < points.size(); ++id)
        {
            nodes_.push_back(Node{.point = points[id], .id = id});
        }
        build(0, nodes_.size());
    }

    std::size_t size() const
    {
        return alive(root());
    }

    void take_in_circle(Point center, std::uint32_t radius,
                        std::vector<std::size_t> &output)
    {
        extract(Circle{center, square(radius)}, output);
    }

    void take_in_rectangle(Point lower, Point upper,
                           std::vector<std::size_t> &output)
    {
        extract(Box{lower, upper}, output);
    }

private:
    static constexpr auto kNoNode = std::numeric_limits<std::size_t>::max();

    struct Box
    {
        Point lower;
        Point upper;

        bool contains(Point p) const
        {
            return lower[0] <= p[0] && p[0] <= upper[0] && lower[1] <= p[1] &&
                   p[1] <= upper[1];
        }

        bool excludes(const Box &box) const
        {
            return upper[0] < box.lower[0] || box.upper[0] < lower[0] ||
                   upper[1] < box.lower[1] || box.upper[1] < lower[1];
        }

        bool covers(const Box &box) const
        {
            return contains(box.lower) && contains(box.upper);
        }
    };

    static std::uint64_t square(std::int64_t delta)
    {
        const auto unsigned_delta = static_cast<std::uint64_t>(delta);
        return unsigned_delta * unsigned_delta;
    }

    struct Circle
    {
        Point center;
        std::uint64_t radius_squared;

        bool inside_squared(std::uint64_t dx_squared,
                            std::uint64_t dy_squared) const
        {
            // 一軸の距離の二乗は64ビットに収まる。和を作らずに加算の桁あふれを防ぐ。
            return dx_squared <= radius_squared &&
                   dy_squared <= radius_squared - dx_squared;
        }

        bool contains(Point p) const
        {
            return inside_squared(
                square(static_cast<std::int64_t>(p[0]) - center[0]),
                square(static_cast<std::int64_t>(p[1]) - center[1]));
        }

        bool excludes(const Box &box) const
        {
            const Point closest{
                std::clamp(center[0], box.lower[0], box.upper[0]),
                std::clamp(center[1], box.lower[1], box.upper[1]),
            };
            return !contains(closest);
        }

        bool covers(const Box &box) const
        {
            const auto max_squared = [&](std::size_t axis)
            {
                return std::max(
                    square(static_cast<std::int64_t>(box.lower[axis]) - center[axis]),
                    square(static_cast<std::int64_t>(box.upper[axis]) - center[axis]));
            };
            return inside_squared(max_squared(0), max_squared(1));
        }
    };

    struct Node
    {
        Point point;
        std::size_t id;
        Box bounds{};
        std::size_t left = kNoNode, right = kNoNode, remaining = 1;
        bool present = true;
    };

    std::vector<Node> nodes_;
    std::size_t root() const
    {
        return nodes_.empty() ? kNoNode : nodes_.size() / 2;
    }

    std::size_t build(std::size_t first, std::size_t last)
    {
        if (first == last)
        {
            return kNoNode;
        }
        Box bounds{nodes_[first].point, nodes_[first].point};
        for (std::size_t i = first + 1; i < last; ++i)
        {
            for (std::size_t axis = 0; axis < 2; ++axis)
            {
                bounds.lower[axis] =
                    std::min(bounds.lower[axis], nodes_[i].point[axis]);
                bounds.upper[axis] =
                    std::max(bounds.upper[axis], nodes_[i].point[axis]);
            }
        }
        const auto width =
            static_cast<std::int64_t>(bounds.upper[0]) - bounds.lower[0];
        const auto height =
            static_cast<std::int64_t>(bounds.upper[1]) - bounds.lower[1];
        const std::size_t axis = height > width;
        const std::size_t mid = first + (last - first) / 2;
        std::nth_element(nodes_.begin() + first, nodes_.begin() + mid,
                         nodes_.begin() + last,
                         [axis](const Node &a, const Node &b)
                         {
                             return a.point[axis] < b.point[axis];
                         });
        Node &node = nodes_[mid];
        node.bounds = bounds;
        node.left = build(first, mid);
        node.right = build(mid + 1, last);
        node.remaining = last - first;
        return mid;
    }

    std::size_t alive(std::size_t id) const
    {
        return id == kNoNode ? 0 : nodes_[id].remaining;
    }

    template <class Shape>
    std::size_t count(std::size_t id, const Shape &shape) const
    {
        if (alive(id) == 0 || shape.excludes(nodes_[id].bounds))
        {
            return 0;
        }
        const auto &node = nodes_[id];
        if (shape.covers(node.bounds))
        {
            return node.remaining;
        }
        return std::size_t(node.present && shape.contains(node.point)) +
               count(node.left, shape) + count(node.right, shape);
    }

    template <class Shape>
    void extract(const Shape &shape, std::vector<std::size_t> &output)
    {
        const auto id = root();
        const auto capacity = output.capacity();
        if (alive(id) > capacity - output.size())
        {
            const auto required = output.size() + count(id, shape);
            if (required > capacity)
            {
                const auto growth = std::min(capacity / 2, output.max_size() - capacity);
                output.reserve(std::max(required, capacity + growth));
            }
        }
        take(id, shape, output);
    }

    void take_all(std::size_t id, std::vector<std::size_t> &output)
    {
        if (alive(id) == 0)
        {
            return;
        }
        Node &node = nodes_[id];
        if (node.present)
        {
            output.push_back(node.id);
        }
        take_all(node.left, output);
        take_all(node.right, output);
        node.remaining = 0;
    }

    template <class Shape>
    void take(std::size_t id, const Shape &shape,
              std::vector<std::size_t> &output)
    {
        if (alive(id) == 0)
        {
            return;
        }
        Node &node = nodes_[id];
        if (shape.excludes(node.bounds))
        {
            return;
        }
        if (shape.covers(node.bounds))
        {
            take_all(id, output);
            return;
        }
        if (node.present && shape.contains(node.point))
        {
            output.push_back(node.id);
            node.present = false;
        }
        take(node.left, shape, output);
        take(node.right, shape, output);
        node.remaining =
            alive(node.left) + alive(node.right) + std::size_t(node.present);
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
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    std::int32_t height;
    std::uint32_t radius;
    verify_in >> n >> height >> radius;
    using Extractor = sirkit::KdTreePointExtractor;
    std::vector<Extractor::Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto close_to_destination = [&](Extractor::Point point)
    {
        const std::int64_t x = point[0];
        const auto dy = static_cast<std::int64_t>(height) - point[1];
        const std::int64_t radius_wide = radius;
        return x * x + dy * dy <= radius_wide * radius_wide;
    };
    Extractor remaining(points);
    std::vector<std::size_t> queue;
    queue.reserve(n);
    remaining.take_in_circle({0, 0}, radius, queue);
    int jumps = 1;
    for (std::size_t begin = 0; begin < queue.size(); ++jumps)
    {
        const auto end = queue.size();
        for (; begin < end; ++begin)
        {
            const auto point = points[queue[begin]];
            if (close_to_destination(point))
            {
                verify_out << jumps + 1 << '\n';
                return 0;
            }
            remaining.take_in_circle(point, radius, queue);
        }
    }
    verify_out << "-1\n";
}
