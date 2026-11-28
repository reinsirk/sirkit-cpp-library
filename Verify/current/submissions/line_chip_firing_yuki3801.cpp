#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/3801

// Library: Algorithm/Line_Chip_Firing.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（整数直線上のchip firing）: https://www.dam.brown.edu/people/cklivans/Confluence.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/line_chip_firing.md
//
// API一覧
// line_chip_firing(positions): 同じ位置xの2個をx-1とx+1へ移し、重複がなくなるまで続ける。
// LineChipFiringResult{positions, operations}: 最終位置の昇順列と操作の総回数。操作順によらない。
// 入力はint64_tの位置列。個数n>2^20ならlength_error、最終位置がint64_tを超えるとoverflow_error。
// O(1+n log(n+1))時間、O(1+n)領域。浮動小数点や拡張整数型は不要。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LINE_CHIP_FIRING
#define SIRKIT_LINE_CHIP_FIRING

struct LineChipFiringResult
{
    std::vector<std::int64_t> positions;
    std::uint64_t operations = 0;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
struct LineChipCluster
{
    std::int64_t first, count, hole_offset;

    [[nodiscard]] constexpr std::int64_t width() const
    {
        return count + (hole_offset >= 0);
    }

    [[nodiscard]] constexpr std::int64_t last() const
    {
        return first + (width() - 1);
    }

    [[nodiscard]] constexpr std::int64_t local_sum(std::int64_t offset) const
    {
        const auto length = width();
        const auto full = offset * length + length * (length - 1) / 2;
        return hole_offset < 0 ? full : full - (offset + hole_offset);
    }

    [[nodiscard]] constexpr std::int64_t local_square_sum(std::int64_t offset) const
    {
        const auto length = width();
        const auto last_index = length - 1;
        const auto full = length * offset * offset + offset * length * last_index +
                          length * last_index * (2 * length - 1) / 6;
        const auto missing = offset + hole_offset;
        return hole_offset < 0 ? full : full - missing * missing;
    }
};

[[nodiscard]] inline std::int64_t checked_line_chip_coordinate(std::int64_t origin, std::int64_t offset)
{
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    if ((offset < 0 && origin < kMin - offset) || (offset > 0 && origin > kMax - offset))
    {
        throw std::overflow_error("line_chip_firing: final coordinate outside int64_t");
    }
    return origin + offset;
}

[[nodiscard]] inline LineChipCluster merge_line_chip_clusters(const LineChipCluster &a, const LineChipCluster &b,
                                                              std::uint64_t &operations)
{
    // 重なる2区間の相対座標だけを使う。原点の値にかかわらず、距離は駒の個数以下。
    const auto origin = std::min(a.first, b.first);
    const auto a_offset = a.first - origin;
    const auto b_offset = b.first - origin;
    const auto count = a.count + b.count;
    const auto sum = a.local_sum(a_offset) + b.local_sum(b_offset);
    const auto excess = sum - count * (count - 1) / 2;
    auto left = excess / count;
    auto remainder = excess % count;
    if (remainder < 0)
    {
        --left;
        remainder += count;
    }
    // 連続区間の座標和の差を、右端を1つ伸ばして内部の1点を抜くことで合わせる。
    const auto hole = remainder == 0 ? -1 : count - remainder;
    const LineChipCluster merged{checked_line_chip_coordinate(origin, left), count, hole};
    static_cast<void>(checked_line_chip_coordinate(origin, left + merged.width() - 1));
    // 1回の操作で座標の二乗和は2増える。同じ原点で各群を比較する。
    const auto delta = merged.local_square_sum(left) - a.local_square_sum(a_offset) - b.local_square_sum(b_offset);
    operations += static_cast<std::uint64_t>(delta / 2);
    return merged;
}
} // namespace reinsirk::detail

[[nodiscard]] inline LineChipFiringResult line_chip_firing(std::span<const std::int64_t> positions)
{
    if (positions.size() > (std::size_t{1} << 20))
    {
        throw std::length_error("line_chip_firing: too many chips");
    }
    std::vector<std::int64_t> sorted(positions.begin(), positions.end());
    std::ranges::sort(sorted);
    std::vector<detail::LineChipCluster> groups;
    groups.reserve(sorted.size());
    LineChipFiringResult result;
    for (const auto position : sorted)
    {
        detail::LineChipCluster current{position, 1, -1};
        while (!groups.empty() && groups.back().last() >= current.first)
        {
            current = detail::merge_line_chip_clusters(groups.back(), current, result.operations);
            groups.pop_back();
        }
        groups.push_back(current);
    }
    result.positions.reserve(sorted.size());
    for (const auto &group : groups)
    {
        for (std::int64_t i = 0; i < group.width(); ++i)
        {
            if (i != group.hole_offset)
            {
                result.positions.push_back(group.first + i);
            }
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


#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::int64_t> positions(n);
    for (auto &position : positions)
    {
        verify_in >> position;
    }
    verify_out << sirkit::line_chip_firing(positions).operations << '\n';
}
