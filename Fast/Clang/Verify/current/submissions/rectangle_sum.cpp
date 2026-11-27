#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/rectangle_sum

// Library: Geometry/Rectangle_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Rectangle_Sum.cpp
// 参考: https://ei1333.github.io/library/other/static-point-add-rectangle-sum.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rectangle_sum.md
//
// API一覧
// RectangleSumPoint{x,y,weight}: 重み付きの格子点。座標・重みは符号付き64ビット。同じ座標の点も個別に加算。
// RectangleSumQuery{left,bottom,right,top}: 半開長方形[left,right)×[bottom,top)。空なら和は0。
// rectangle_sum(points,queries): 各長方形内の和をクエリ順に返す。O((N+Q) log(N+Q+1))時間・O(N+Q)領域。
// 和は2^64を法とし、符号付き64ビットに対応させて返す。検査有効時、左右または上下が逆ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECTANGLE_SUM_DEFINED
#define SIRKIT_RECTANGLE_SUM_DEFINED

struct RectangleSumPoint
{
    std::int64_t x;
    std::int64_t y;
    std::int64_t weight;
};

struct RectangleSumQuery
{
    std::int64_t left;
    std::int64_t bottom;
    std::int64_t right;
    std::int64_t top;
};

namespace detail
{
inline std::size_t CheckedRectangleSumEventCount(std::size_t queries, std::size_t capacity)
{
    if (queries > capacity / 2)
    {
        throw std::length_error("rectangle_sum: query count");
    }
    return 2 * queries;
}
} // namespace detail

inline std::vector<std::int64_t> rectangle_sum(std::span<const RectangleSumPoint> points,
                                               std::span<const RectangleSumQuery> queries)
{
    if (queries.empty())
    {
        return {};
    }

    auto ordered = std::vector(points.begin(), points.end());
    std::ranges::sort(ordered, {}, &RectangleSumPoint::y);
    std::vector<std::int64_t> ys;
    ys.reserve(ordered.size());
    for (auto &point : ordered)
    {
        if (ys.empty() || ys.back() != point.y)
        {
            ys.push_back(point.y);
        }
        // 元のyはysに保持し、走査時に使う添字をコピー側へ格納する。
        point.y = static_cast<std::int64_t>(ys.size() - 1);
    }

    struct Event
    {
        std::int64_t x;
        std::size_t begin;
        std::size_t end;
        std::size_t id_and_side;
    };
    std::vector<Event> events;
    events.reserve(detail::CheckedRectangleSumEventCount(queries.size(), events.max_size()));
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto &[left, bottom, right, top] = queries[i];
        if (left == right || bottom == top)
        {
            continue;
        }
        const auto lower = std::lower_bound(ys.begin(), ys.end(), bottom);
        const auto upper = std::lower_bound(lower, ys.end(), top);
        if (lower == upper)
        {
            continue;
        }
        const auto begin = static_cast<std::size_t>(lower - ys.begin());
        const auto end = static_cast<std::size_t>(upper - ys.begin());
        events.push_back({left, begin, end, 2 * i});
        events.push_back({right, begin, end, 2 * i + 1});
    }

    std::vector<std::int64_t> answer(queries.size());
    if (events.empty())
    {
        return answer;
    }
    std::ranges::sort(ordered, {}, &RectangleSumPoint::x);
    std::ranges::sort(events, {}, &Event::x);

    // 0始まりのFenwick Tree。符号なしの加減算で途中の和のオーバーフローを避ける。
    std::vector<std::uint64_t> tree(ys.size());
    const auto prefix = [&](std::size_t end)
    {
        std::uint64_t total = 0;
        for (; end != 0; end &= end - 1)
        {
            total += tree[end - 1];
        }
        return total;
    };
    std::size_t next = 0;
    for (const auto &[x, begin, end, id_and_side] : events)
    {
        // 境界と同じxの点はこの時点ではまだ加算しない。
        while (next < ordered.size() && ordered[next].x < x)
        {
            const auto &[px, rank, weight] = ordered[next++];
            for (auto p = static_cast<std::size_t>(rank); p < tree.size(); p |= p + 1)
            {
                tree[p] += static_cast<std::uint64_t>(weight);
            }
        }
        const auto sum = prefix(end) - prefix(begin);
        auto &value = answer[id_and_side / 2];
        const auto unsigned_value = std::bit_cast<std::uint64_t>(value);
        value = std::bit_cast<std::int64_t>(id_and_side % 2 == 0 ? unsigned_value - sum : unsigned_value + sum);
    }
    return answer;
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
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;

    std::vector<RectangleSumPoint> points(n);
    for (auto &[x, y, weight] : points)
    {
        verify_in >> x >> y >> weight;
    }
    std::vector<RectangleSumQuery> queries(q);
    for (auto &[left, bottom, right, top] : queries)
    {
        verify_in >> left >> bottom >> right >> top;
    }
    for (std::int64_t value : rectangle_sum(points, queries))
    {
        verify_out << value << '\n';
    }
}
