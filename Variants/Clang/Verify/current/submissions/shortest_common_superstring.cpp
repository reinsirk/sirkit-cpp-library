#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc343/tasks/abc343_g

// Library: String/Shortest_Common_Superstring.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Shortest_Common_Superstring.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_common_superstring.md
//
// API一覧
// shortest_common_superstring(words): 各要素を連続部分文字列として含む最短の文字列を一つ返す。
// 空の要素、重複、他の要素に含まれる要素を除く。入力20個以下、全長の合計はUINT32_MAX以下。
// Nは入力数、Sは合計長、Kは除外後の要素数。O(N*S+K^2*2^K+1)時間・O(S+K*2^K+N^2+1)領域。
//
// [同梱のPrefix Function]
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <span>
#include <string>


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

#ifndef SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED
#define SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::uint32_t superstring_add_length(std::uint32_t total, std::size_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max() - total)
    {
        throw std::length_error("shortest_common_superstring: total length exceeds UINT32_MAX");
    }
    return total + static_cast<std::uint32_t>(length);
}

inline std::size_t superstring_overlap(std::string_view text, std::string_view pattern,
                                       const std::vector<std::size_t> &border)
{
    std::size_t matched = 0;
    for (const char byte : text)
    {
        while (matched != 0 && byte != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += byte == pattern[matched];
        if (matched == pattern.size())
        {
            // patternを含む場合の値。そのpatternを残すときはこの辺を使わない。
            return matched;
        }
    }
    return matched;
}
} // namespace detail

inline std::string shortest_common_superstring(std::span<const std::string_view> words)
{
    constexpr std::size_t kMaxWords = 20;
    if (words.size() > kMaxWords)
    {
        throw std::length_error("shortest_common_superstring: too many words");
    }
    std::uint32_t total_length = 0;
    std::vector<std::string_view> candidates;
    for (const auto word : words)
    {
        total_length = detail::superstring_add_length(total_length, word.size());
        if (!word.empty())
        {
            candidates.push_back(word);
        }
    }
    if (candidates.empty())
    {
        return {};
    }

    const auto count = candidates.size();
    std::vector<std::uint32_t> overlaps(count * count);
    std::vector<std::size_t> retained;
    std::size_t retained_length = 0;
    for (std::size_t second = 0; second < count; ++second)
    {
        const auto word = candidates[second];
        const auto border = prefix_function(word);
        bool contained = false;
        for (std::size_t first = 0; first < count; ++first)
        {
            if (first == second)
            {
                continue;
            }
            const auto text = candidates[first];
            const auto common = detail::superstring_overlap(text, word, border);
            overlaps[first * count + second] = static_cast<std::uint32_t>(common);
            if (common == word.size() && (text.size() > word.size() || first < second))
            {
                contained = true;
                break;
            }
        }
        if (!contained)
        {
            retained.push_back(second);
            retained_length += word.size();
        }
    }

    const auto k = retained.size();
    const auto state_count = std::size_t{1} << k;
    const auto per_last = state_count / 2;
    std::vector<std::uint32_t> best(k * per_last);
    const auto score = [&](std::size_t mask, std::size_t last) -> std::uint32_t & {
        // lastのビットは必ず1なので、省いた添字に詰めて保存する。
        const auto low_bits = (std::size_t{1} << last) - 1;
        return best[last * per_last + (mask & low_bits) + ((mask >> 1) & ~low_bits)];
    };
    const auto link = [&](std::size_t first, std::size_t second) {
        return overlaps[retained[first] * count + retained[second]];
    };
    for (std::size_t mask = 1; mask < state_count; ++mask)
    {
        for (auto bits = mask; bits != 0; bits &= bits - 1)
        {
            const auto last = static_cast<std::size_t>(std::countr_zero(bits));
            const auto smaller = mask ^ (std::size_t{1} << last);
            auto &result = score(mask, last);
            for (auto predecessors = smaller; predecessors != 0; predecessors &= predecessors - 1)
            {
                const auto previous = static_cast<std::size_t>(std::countr_zero(predecessors));
                result = std::max(result, score(smaller, previous) + link(previous, last));
            }
        }
    }

    const auto full = state_count - 1;
    std::size_t last = 0;
    for (std::size_t i = 1; i < k; ++i)
    {
        if (score(full, i) > score(full, last))
        {
            last = i;
        }
    }
    const auto answer_length = retained_length - score(full, last);
    std::vector<std::size_t> order;
    for (auto mask = full; mask != 0;)
    {
        order.push_back(last);
        const auto value = score(mask, last);
        mask ^= std::size_t{1} << last;
        for (auto bits = mask; bits != 0; bits &= bits - 1)
        {
            const auto previous = static_cast<std::size_t>(std::countr_zero(bits));
            if (score(mask, previous) + link(previous, last) == value)
            {
                last = previous;
                break;
            }
        }
    }

    std::string result;
    result.reserve(answer_length);
    result += candidates[retained[order.back()]];
    for (std::size_t index = order.size() - 1; index != 0; --index)
    {
        const auto next = order[index - 1];
        result += candidates[retained[next]].substr(link(order[index], next));
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
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::string> words(n);
    std::vector<std::string_view> views;
    views.reserve(n);
    for (auto &word : words)
    {
        verify_in >> word;
        views.push_back(word);
    }
    verify_out << sirkit::shortest_common_superstring(views).size() << '\n';
}
