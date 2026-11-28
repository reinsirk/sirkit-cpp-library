#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc477/tasks/abc477_c

// Library: String/Pattern_Range_Occurrences.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Pattern_Range_Occurrences.cpp
// 参考（区間内の出現）: https://atcoder.jp/contests/abc477/tasks/abc477_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_range_occurrences.md
//
// API一覧
// PatternRangeOccurrences(text,pattern): 固定パターンの出現を前計算。長さn,mに対しO(1+n+m)時間・領域。
// size()/pattern_size() const: 文字列とパターンの長さ。O(1)。
// count(l,r) const: 半開区間[l,r)全体に収まる出現数。O(1)。
// positions(l,r) const: 同じ出現の開始位置を昇順の借用spanで返す。O(1)。重なりも数える。
// 0始まり・バイト比較。空パターンは各境界l..rに出現。コピーは内部の読み取り専用の表を共有する。
// 既定構築・move元は両方が空の状態。借用spanは表を所有する実体が残る間有効。
//
// [同梱の接頭辞関数]
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <array>
#include <memory>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_FUNCTION_DEFINED
#define SIRKIT_PREFIX_FUNCTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t occurrence_boundary_count(std::size_t size, std::size_t capacity)
{
    if (size >= capacity)
    {
        throw std::length_error("find_occurrences: too many occurrences");
    }
    return size + 1;
}
} // namespace detail

inline std::vector<std::size_t> prefix_function(std::string_view text)
{
    std::vector<std::size_t> length(text.size());
    for (std::size_t i = 1; i < text.size(); ++i)
    {
        auto previous = length[i - 1];
        while (previous != 0 && text[i] != text[previous])
        {
            previous = length[previous - 1];
        }
        length[i] = previous + (text[i] == text[previous]);
    }
    return length;
}

inline std::vector<std::size_t> find_occurrences(std::string_view text, std::string_view pattern)
{
    std::vector<std::size_t> result;
    if (pattern.empty())
    {
        result.resize(detail::occurrence_boundary_count(text.size(), result.max_size()));
        std::iota(result.begin(), result.end(), std::size_t{0});
        return result;
    }

    const auto border = prefix_function(pattern);
    std::size_t matched = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        while (matched != 0 && text[i] != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += text[i] == pattern[matched];
        if (matched == pattern.size())
        {
            result.push_back(i - (matched - 1));
            matched = border[matched - 1];
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED
#define SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t PatternRangeOffsetCount(std::size_t length, std::size_t capacity)
{
    if (capacity < 2 || length > capacity - 2)
    {
        throw std::length_error("PatternRangeOccurrences: text too long");
    }
    return length + 2;
}
} // namespace reinsirk::detail

class PatternRangeOccurrences
{
    struct Data
    {
        std::size_t length, pattern_length;
        std::vector<std::size_t> offsets, starts;

        Data(std::string_view text, std::string_view pattern)
            : length(text.size()), pattern_length(pattern.size())
        {
            offsets.resize(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PatternRangeOffsetCount(length, offsets.max_size()));
            starts = find_occurrences(text, pattern);
            for (const auto position : starts)
            {
                ++offsets[position + 1];
            }
            std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        }
    };

    static constexpr std::array<std::size_t, 1> kEmptyBoundary{0};
    std::shared_ptr<const Data> data_;

public:
    PatternRangeOccurrences() = default;

    PatternRangeOccurrences(std::string_view text, std::string_view pattern)
        : data_(std::make_shared<const Data>(text, pattern))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_ ? data_->length : 0;
    }

    [[nodiscard]] std::size_t pattern_size() const noexcept
    {
        return data_ ? data_->pattern_length : 0;
    }

    [[nodiscard]] std::span<const std::size_t> positions(std::size_t left, std::size_t right) const
    {
        if (!data_)
        {
            return kEmptyBoundary;
        }
        const auto &data = *data_;
        if (data.pattern_length > right - left)
        {
            return {};
        }

        // offsets[k]はkより前に始まる出現数。最右の開始位置はright-pattern_length。
        const auto first = data.offsets[left];
        const auto stop = data.offsets[right - data.pattern_length + 1];
        return std::span<const std::size_t>(data.starts).subspan(first, stop - first);
    }

    [[nodiscard]] std::size_t count(std::size_t left, std::size_t right) const
    {
        return positions(left, right).size();
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
#include <string>

int main()
{
    std::size_t queries{};
    std::string text, pattern;
    verify_in >> queries >> text >> pattern;
    const sirkit::PatternRangeOccurrences index(text, pattern);
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << (index.count(left - 1, right) != 0 ? "Yes" : "No") << '\n';
    }
}
