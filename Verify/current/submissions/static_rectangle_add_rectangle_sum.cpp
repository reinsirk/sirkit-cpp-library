#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum
// Submission: https://judge.yosupo.jp/submission/419937

// Library: DataStructure/Static_Rectangle_Add_Rectangle_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_rectangle_add_rectangle_sum.md
//
// API一覧
// StaticRectangleAdd{left,bottom,right,top,weight}: 半開長方形内の各格子点へ重みを加える。
// StaticRectangleSumQuery{left,bottom,right,top}: 半開長方形内の格子点の総和を照会する。
// static_rectangle_add_rectangle_sum<Mod>(additions,queries): 全追加後の和を照会順に返す。
// 座標と重みは符号付き64ビット。幅・高さ0の追加は無視し、幅・高さ0の照会は0。
// 合計はModでの非負剰余。Mod既定値は998244353。素数でなくてよい。
// 追加n件・照会q件についてO((n+q)log(n+q+1))時間、O(n+q)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RECTANGLE_ADD_RECTANGLE_SUM_DEFINED
#define SIRKIT_STATIC_RECTANGLE_ADD_RECTANGLE_SUM_DEFINED

struct StaticRectangleAdd
{
    std::int64_t left, bottom, right, top, weight;
};

struct StaticRectangleSumQuery
{
    std::int64_t left, bottom, right, top;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedStaticRectangleAreaDoubleCount(std::size_t count, std::size_t limit)
{
    if (count > limit / 2)
    {
        throw std::length_error("static_rectangle_add_rectangle_sum: too many events");
    }
    return 2 * count;
}

template <std::uint32_t Mod>
struct StaticRectangleAreaArithmetic
{
    static_assert(Mod > 0);

    static std::uint32_t Normalize(std::int64_t value)
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b)
    {
        const auto sum = static_cast<std::uint64_t>(a) + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Subtract(std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : a + (Mod - b);
    }

    static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(a) * b) % Mod);
    }
};
} // namespace reinsirk::detail

template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> static_rectangle_add_rectangle_sum(
    std::span<const StaticRectangleAdd> additions, std::span<const StaticRectangleSumQuery> queries)
{
    using Arithmetic = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::StaticRectangleAreaArithmetic<Mod>;
    using Index = std::uint32_t;
    using Moments = std::array<std::uint32_t, 4>;

    struct AddEvent
    {
        std::int64_t x;
        Index bottom, top;
        std::uint32_t weight;
    };

    struct QueryEvent
    {
        std::int64_t x;
        std::size_t encoded_query;
    };

    struct QueryY
    {
        Index bottom, top;
        std::uint32_t bottom_mod, top_mod;
    };

#ifndef SKIP_RUNTIME_CHECKS
    const auto check = [](const auto &rectangle)
    {
        if (rectangle.left > rectangle.right || rectangle.bottom > rectangle.top)
        {
            throw std::invalid_argument("static_rectangle_add_rectangle_sum: reversed rectangle");
        }
    };
#endif

    std::vector<std::int64_t> ys;
    ys.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(
        additions.size(), std::min<std::size_t>(std::numeric_limits<Index>::max(), ys.max_size())));
    for (const auto &rectangle : additions)
    {
#ifndef SKIP_RUNTIME_CHECKS
        check(rectangle);
#endif
        if (rectangle.left != rectangle.right && rectangle.bottom != rectangle.top &&
            Arithmetic::Normalize(rectangle.weight) != 0)
        {
            ys.push_back(rectangle.bottom);
            ys.push_back(rectangle.top);
        }
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    const auto rank = [&](std::int64_t y)
    {
        return static_cast<Index>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
    };

    std::vector<QueryEvent> query_events;
    query_events.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(queries.size(), query_events.max_size()));
    std::vector<QueryY> query_ys(queries.size());
    for (std::size_t id = 0; id < queries.size(); ++id)
    {
        const auto &rectangle = queries[id];
#ifndef SKIP_RUNTIME_CHECKS
        check(rectangle);
#endif
        if (rectangle.left == rectangle.right || rectangle.bottom == rectangle.top || ys.empty())
        {
            continue;
        }
        query_ys[id] = {rank(rectangle.bottom), rank(rectangle.top),
                        Arithmetic::Normalize(rectangle.bottom), Arithmetic::Normalize(rectangle.top)};
        // 偶数は左辺で減算、奇数は右辺で加算する。
        query_events.push_back({rectangle.left, 2 * id});
        query_events.push_back({rectangle.right, 2 * id + 1});
    }

    std::vector<std::uint32_t> result(queries.size());
    if (query_events.empty())
    {
        return result;
    }

    std::vector<AddEvent> add_events;
    add_events.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(additions.size(), add_events.max_size()));
    for (const auto &rectangle : additions)
    {
        const auto weight = Arithmetic::Normalize(rectangle.weight);
        if (rectangle.left == rectangle.right || rectangle.bottom == rectangle.top || weight == 0)
        {
            continue;
        }
        const auto bottom = rank(rectangle.bottom);
        const auto top = rank(rectangle.top);
        add_events.push_back({rectangle.left, bottom, top, weight});
        add_events.push_back({rectangle.right, bottom, top, Arithmetic::Subtract(0, weight)});
    }
    std::ranges::sort(add_events, {}, &AddEvent::x);
    std::ranges::sort(query_events, {}, &QueryEvent::x);

    std::vector<std::uint32_t> ys_mod(ys.size());
    std::transform(ys.begin(), ys.end(), ys_mod.begin(), Arithmetic::Normalize);
    std::vector<Moments> fenwick(ys.size());
    const auto update = [&](std::size_t index, std::uint32_t weight, std::uint32_t weight_x)
    {
        const Moments delta{weight, weight_x, Arithmetic::Multiply(weight, ys_mod[index]),
                            Arithmetic::Multiply(weight_x, ys_mod[index])};
        for (; index < fenwick.size(); index |= index + 1)
        {
            auto &node = fenwick[index];
            for (std::size_t i = 0; i < node.size(); ++i)
            {
                node[i] = Arithmetic::Add(node[i], delta[i]);
            }
        }
    };
    const auto prefix_area = [&](std::size_t count, std::uint32_t x, std::uint32_t y)
    {
        Moments sum{};
        while (count != 0)
        {
            const auto &node = fenwick[count - 1];
            for (std::size_t i = 0; i < node.size(); ++i)
            {
                sum[i] = Arithmetic::Add(sum[i], node[i]);
            }
            count &= count - 1;
        }
        // 左下の角(x0,y0)からの寄与w(x-x0)(y-y0)を4項に展開する。
        const auto first = Arithmetic::Subtract(Arithmetic::Multiply(sum[0], x), sum[1]);
        const auto second = Arithmetic::Subtract(Arithmetic::Multiply(sum[2], x), sum[3]);
        return Arithmetic::Subtract(Arithmetic::Multiply(first, y), second);
    };

    std::size_t next = 0;
    for (const auto &event : query_events)
    {
        while (next < add_events.size() && add_events[next].x < event.x)
        {
            const auto &addition = add_events[next++];
            const auto weight_x = Arithmetic::Multiply(addition.weight, Arithmetic::Normalize(addition.x));
            update(addition.bottom, addition.weight, weight_x);
            update(addition.top, Arithmetic::Subtract(0, addition.weight), Arithmetic::Subtract(0, weight_x));
        }

        const auto id = event.encoded_query / 2;
        const auto &y = query_ys[id];
        const auto x = Arithmetic::Normalize(event.x);
        const auto slice = Arithmetic::Subtract(prefix_area(y.top, x, y.top_mod),
                                                prefix_area(y.bottom, x, y.bottom_mod));
        if (event.encoded_query % 2 == 0)
        {
            result[id] = Arithmetic::Subtract(result[id], slice);
        }
        else
        {
            result[id] = Arithmetic::Add(result[id], slice);
        }
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
    std::vector<StaticRectangleAdd> additions(n);
    std::vector<StaticRectangleSumQuery> queries(q);
    for (auto &rectangle : additions)
    {
        verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top >> rectangle.weight;
    }
    for (auto &rectangle : queries)
    {
        verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top;
    }
    for (const auto answer : static_rectangle_add_rectangle_sum(additions, queries))
    {
        verify_out << answer << '\n';
    }
}
