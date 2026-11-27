#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/point_add_rectangle_sum
// Submission: https://judge.yosupo.jp/submission/419876

// Library: DataStructure/Offline_Point_Add_Rectangle_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_point_add_rectangle_sum.md
//
// API一覧
// PointAdd{x,y,weight}: 重み付き点を追加する。同じ位置に追加した重みも含まれる。
// RectangleSumRequest{left,bottom,right,top}: 半開長方形内を集計する。空長方形の答えは0。
// PointAddRectangleSumOperation: 上記2種類の操作を表すstd::variant。
// offline_point_add_rectangle_sum(initial,operations): 初期点と各照会までの追加点を集計し、照会順に返す。
// 座標・重みは符号付き64ビット。合計は2^64を法とし、符号付き64ビット値へ対応させる。
// 初期点n・操作qについて決定的O((n+q)log²(n+q+2))時間、O(n+q)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_POINT_ADD_RECTANGLE_SUM_DEFINED
#define SIRKIT_OFFLINE_POINT_ADD_RECTANGLE_SUM_DEFINED

struct PointAdd
{
    std::int64_t x, y, weight;
};

struct RectangleSumRequest
{
    std::int64_t left, bottom, right, top;
};

using PointAddRectangleSumOperation = std::variant<PointAdd, RectangleSumRequest>;

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPointAddRectangleSumEventCount(std::size_t initial, std::size_t updates,
                                                         std::size_t queries, std::size_t limit)
{
    if (queries > limit / 2)
    {
        throw std::length_error("offline_point_add_rectangle_sum: too many queries");
    }
    const auto room = limit - 2 * queries;
    if (initial > room || updates > room - initial)
    {
        throw std::length_error("offline_point_add_rectangle_sum: too many points");
    }
    return initial + updates + 2 * queries;
}
}

inline std::vector<std::int64_t> offline_point_add_rectangle_sum(
    std::span<const PointAdd> initial, std::span<const PointAddRectangleSumOperation> operations)
{
    using Index = std::uint32_t;
    constexpr auto kPoint = std::numeric_limits<Index>::max();
    struct Event
    {
        std::int64_t x;
        std::uint64_t weight_or_query;
        Index first, last;
    };

    std::size_t updates = 0;
    for (const auto &operation : operations)
    {
        if (std::holds_alternative<PointAdd>(operation))
        {
            ++updates;
        }
#ifndef SKIP_RUNTIME_CHECKS
        else
        {
            const auto &rectangle = std::get<RectangleSumRequest>(operation);
            if (rectangle.left > rectangle.right || rectangle.bottom > rectangle.top)
            {
                throw std::invalid_argument("offline_point_add_rectangle_sum: reversed rectangle");
            }
        }
#endif
    }
    const auto queries = operations.size() - updates;
    if (queries == 0)
    {
        return {};
    }

    std::vector<Event> events;
    // lastの最大値を点追加の印として予約する。
    const auto limit = std::min<std::size_t>(kPoint - 1, events.max_size());
    const auto capacity = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPointAddRectangleSumEventCount(
        initial.size(), updates, queries, limit);

    std::vector<std::int64_t> ys;
    ys.reserve(initial.size() + updates);
    for (const auto &point : initial)
    {
        ys.push_back(point.y);
    }
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<PointAdd>(&operation))
        {
            ys.push_back(point->y);
        }
    }
    std::vector<std::int64_t> result(queries);
    if (ys.empty())
    {
        return result;
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    const auto rank = [&](std::int64_t y)
    {
        return static_cast<Index>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
    };
    events.reserve(capacity);
    const auto append = [&](const PointAdd &point)
    {
        if (point.weight != 0)
        {
            events.push_back({point.x, static_cast<std::uint64_t>(point.weight), rank(point.y), kPoint});
        }
    };
    for (const auto &point : initial)
    {
        append(point);
    }
    const auto first_dynamic = events.size();
    Index query_id = 0;
    std::size_t query_events = 0;
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<PointAdd>(&operation))
        {
            append(*point);
            continue;
        }
        const auto id = query_id++;
        const auto &rectangle = std::get<RectangleSumRequest>(operation);
        const auto first = rank(rectangle.bottom);
        const auto last = rank(rectangle.top);
        if (rectangle.left == rectangle.right || first == last)
        {
            continue;
        }
        const auto code = std::uint64_t{id} * 2;
        events.push_back({rectangle.left, code + 1, first, last});
        events.push_back({rectangle.right, code, first, last});
        query_events += 2;
    }
    if (query_events == 0 || events.size() == query_events)
    {
        return result;
    }

    std::vector<std::uint64_t> fenwick(ys.size());
    const auto update = [&](std::size_t rank_y, std::uint64_t delta)
    {
        for (; rank_y < fenwick.size(); rank_y |= rank_y + 1)
        {
            fenwick[rank_y] += delta;
        }
    };
    const auto prefix = [&](std::size_t count)
    {
        std::uint64_t value = 0;
        for (; count != 0; count &= count - 1)
        {
            value += fenwick[count - 1];
        }
        return value;
    };
    const auto sweep = [&](std::size_t left, std::size_t middle, std::size_t right)
    {
        auto cursor = left;
        for (auto i = middle; i < right; ++i)
        {
            const auto &query = events[i];
            if (query.last == kPoint)
            {
                continue;
            }
            while (cursor < middle && events[cursor].x < query.x)
            {
                const auto &point = events[cursor++];
                if (point.last == kPoint)
                {
                    update(point.first, point.weight_or_query);
                }
            }
            const auto sum = prefix(query.last) - prefix(query.first);
            const auto code = query.weight_or_query;
            auto &answer = result[code / 2];
            const auto delta = (code & 1) ? std::uint64_t{0} - sum : sum;
            answer = std::bit_cast<std::int64_t>(std::bit_cast<std::uint64_t>(answer) + delta);
        }
        for (auto i = left; i < cursor; ++i)
        {
            const auto &point = events[i];
            if (point.last == kPoint)
            {
                update(point.first, std::uint64_t{0} - point.weight_or_query);
            }
        }
    };

    const auto by_x = [](const Event &a, const Event &b)
    {
        return a.x < b.x;
    };
    if (events.size() - first_dynamic == query_events)
    {
        std::sort(events.begin() + first_dynamic, events.end(), by_x);
    }
    else
    {
        std::vector<Event> scratch(events.size() - first_dynamic);
        const auto divide = [&](auto &&self, std::size_t left, std::size_t right) -> unsigned
        {
            constexpr unsigned kHasPoint = 1, kHasQuery = 2;
            if (right - left == 1)
            {
                return events[left].last == kPoint ? kHasPoint : kHasQuery;
            }
            const auto middle = left + (right - left) / 2;
            const auto before = self(self, left, middle);
            const auto after = self(self, middle, right);
            if ((before & kHasPoint) && (after & kHasQuery))
            {
                sweep(left, middle, right);
            }
            if (events[middle - 1].x > events[middle].x)
            {
                const auto first = events.begin() + left;
                const auto split = events.begin() + middle;
                const auto end = events.begin() + right;
                const auto produced = std::merge(first, split, split, end, scratch.begin(), by_x);
                std::copy(scratch.begin(), produced, first);
            }
            return before | after;
        };
        divide(divide, first_dynamic, events.size());
    }
    if (first_dynamic != 0)
    {
        std::sort(events.begin(), events.begin() + first_dynamic, by_x);
        sweep(0, first_dynamic, events.size());
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


#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<PointAdd> initial(n);
    for (auto &point : initial)
    {
        verify_in >> point.x >> point.y >> point.weight;
    }
    std::vector<PointAddRectangleSumOperation> operations;
    operations.reserve(q);
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            PointAdd point{};
            verify_in >> point.x >> point.y >> point.weight;
            operations.emplace_back(point);
        }
        else
        {
            RectangleSumRequest rectangle{};
            verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top;
            operations.emplace_back(rectangle);
        }
    }
    for (const auto answer : offline_point_add_rectangle_sum(initial, operations))
    {
        verify_out << answer << '\n';
    }
}
