#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_linear_add_range_min

// Library: DataStructure/Range_Linear_Add_Range_Min.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Range_Linear_Add_Range_Min.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_linear_add_range_min.md
//
// API一覧
// RangeLinearAddRangeMin(values): int64_tの列。O(n+1)時間・領域、n<=INT_MAX。
// range_add(l,r,b,c): 各i in [l,r)にb*i+cを加える。空区間は何もしない。O(√n+1)。
// range_min(l,r): [l,r)の最小値。空区間はnullopt。O((√n+1)log(n+2))。
// size(): 要素数。O(1)。添字は0始まり。
// 各操作後の要素はint64_tに収まること。中間の積や累積係数が64ビットを超えても可。
// 凸包の比較・値の計算は標準C++の整数演算で厳密に行う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_LINEAR_ADD_RANGE_MIN_DEFINED
#define SIRKIT_RANGE_LINEAR_ADD_RANGE_MIN_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckRangeLinearAddRangeMinSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("RangeLinearAddRangeMin: too many values");
    }
}
}

class RangeLinearAddRangeMin
{
public:
    explicit RangeLinearAddRangeMin(std::span<const std::int64_t> input = {})
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckRangeLinearAddRangeMinSize(input.size());
        if (input.empty())
        {
            return;
        }
        values_.assign(input.begin(), input.end());
        const int n = size();
        block_size_ = static_cast<int>(std::sqrt(n));
        blocks_.reserve((n - 1) / block_size_ + 1);
        for (int begin = 0; begin < n;)
        {
            const int end = begin + std::min(block_size_, n - begin);
            auto &block = blocks_.emplace_back(begin, end);
            block.hull.reserve(end - begin);
            Rebuild(block);
            begin = end;
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(values_.size());
    }

    void range_add(int left, int right, std::int64_t b, std::int64_t c)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }

        const auto step = static_cast<std::uint64_t>(b);
        const auto intercept = static_cast<std::uint64_t>(c);
        const int last = (right - 1) / block_size_;
        for (int i = left / block_size_; i <= last; ++i)
        {
            auto &block = blocks_[i];
            const int begin = std::max(left, block.begin);
            const int end = std::min(right, block.end);
            const auto first = step * static_cast<std::uint64_t>(begin) + intercept;
            if (begin == block.begin && end == block.end)
            {
                block.slope.add(b);
                block.offset += first;
            }
            else
            {
                if (block.slope.low != 0 || block.offset != 0)
                {
                    AddValues(block.begin, block.end, block.slope.low, block.offset);
                }
                block.slope = {};
                block.offset = 0;
                AddValues(begin, end, step, first);
                Rebuild(block);
            }
        }
    }

    [[nodiscard]] std::optional<std::int64_t> range_min(int left, int right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return std::nullopt;
        }

        auto result = std::numeric_limits<std::int64_t>::max();
        const int last = (right - 1) / block_size_;
        for (int i = left / block_size_; i <= last; ++i)
        {
            const auto &block = blocks_[i];
            const int begin = std::max(left, block.begin);
            const int end = std::min(right, block.end);
            if (begin == block.begin && end == block.end)
            {
                const auto after = std::upper_bound(
                    block.hull.begin() + 1, block.hull.end(), block.slope,
                    [](const WideSlope &slope, const HullVertex &vertex)
                    {
                        return vertex.cutoff < slope;
                    });
                const auto position = (after - 1)->position;
                result = std::min(result, ValueWithDelta(values_[position], DeltaAt(block, position)));
            }
            else
            {
                auto delta = DeltaAt(block, begin);
                for (int position = begin; position < end; ++position)
                {
                    result = std::min(result, ValueWithDelta(values_[position], delta));
                    delta += block.slope.low;
                }
            }
        }
        return result;
    }

private:
    struct WideSlope
    {
        std::uint64_t high = 0, low = 0;

        void add(std::int64_t value) noexcept
        {
            const auto previous = low;
            low += static_cast<std::uint64_t>(value);
            high += static_cast<std::uint64_t>(low < previous);
            high -= static_cast<std::uint64_t>(value < 0);
        }

        bool operator<(const WideSlope &other) const noexcept
        {
            if (high != other.high)
            {
                return std::bit_cast<std::int64_t>(high) < std::bit_cast<std::int64_t>(other.high);
            }
            return low < other.low;
        }
    };

    struct HullVertex
    {
        int position;
        WideSlope cutoff;
    };

    struct Block
    {
        Block(int begin, int end) : begin(begin), end(end)
        {
        }

        int begin, end;
        std::vector<HullVertex> hull;
        WideSlope slope{};
        std::uint64_t offset = 0;
    };

    std::vector<std::int64_t> values_;
    std::vector<Block> blocks_;
    int block_size_ = 1;

    void CheckRange(int left, int right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || left > right || right > size())
        {
            throw std::out_of_range("RangeLinearAddRangeMin: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    static std::int64_t ValueWithDelta(std::int64_t value, std::uint64_t delta) noexcept
    {
        return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(value) + delta);
    }

    static std::uint64_t DeltaAt(const Block &block, int position) noexcept
    {
        return block.offset + block.slope.low * static_cast<std::uint64_t>(position - block.begin);
    }

    void AddValues(int begin, int end, std::uint64_t step, std::uint64_t delta) noexcept
    {
        for (int i = begin; i < end; ++i)
        {
            values_[i] = ValueWithDelta(values_[i], delta);
            delta += step;
        }
    }

    // moving側がfixed側以下になる最大の整数傾き。差は最大2^64-1。
    static WideSlope Cutoff(std::int64_t fixed, std::int64_t moving, std::uint64_t distance) noexcept
    {
        const auto a = static_cast<std::uint64_t>(fixed);
        const auto b = static_cast<std::uint64_t>(moving);
        if (fixed >= moving)
        {
            return {0, (a - b) / distance};
        }
        const auto difference = b - a;
        const auto ceiling = difference / distance + (difference % distance != 0);
        return {std::numeric_limits<std::uint64_t>::max(), std::uint64_t{0} - ceiling};
    }

    void Rebuild(Block &block)
    {
        auto &hull = block.hull;
        hull.clear();
        hull.push_back({block.begin, {}});
        for (int i = block.begin + 1; i < block.end; ++i)
        {
            auto previous = hull.back().position;
            auto cutoff = Cutoff(values_[previous], values_[i], i - previous);
            // 交点を降順にそろえ、整数傾きの範囲で最小にならない点を除く。
            while (hull.size() > 1 && !(cutoff < hull.back().cutoff))
            {
                hull.pop_back();
                previous = hull.back().position;
                cutoff = Cutoff(values_[previous], values_[i], i - previous);
            }
            hull.push_back({i, cutoff});
        }
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


#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    RangeLinearAddRangeMin data(values);
    while (q-- > 0)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::int64_t b = 0, c = 0;
            verify_in >> b >> c;
            data.range_add(left, right, b, c);
        }
        else
        {
            verify_out << *data.range_min(left, right) << '\n';
        }
    }
}
