#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/3638

// Library: String/Dynamic_Substring_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Dynamic_Substring_Matching.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_substring_matching.md
//
// API一覧
// DynamicSubstringMatching(text): 固定長バイト列を所有し、各バイトの出現位置を64ビットずつ記録する。
// size(): 元のバイト列の長さ。O(1)。
// set(position, byte): 0始まりの位置を1バイト置換する。O(1)。
// contains(pattern): patternが連続部分文字列として現れるかを返す。
// count(pattern): 重なりも含めた出現回数を返す。
// occurrences(pattern): 出現する0始まりの位置を昇順で返す。
// 空patternはn+1箇所に現れ、長すぎるpatternは0箇所。nは元の長さ、mはパターン長。
// 構築・保管O(n+1)。空でないm<=nの検索はO(m*(1+ceil((n-m+1)/64)))時間・O(m)追加領域。出力にはさらに結果の領域と時間が必要。
// コピーは独立。移動元は空として使える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_SUBSTRING_MATCHING_DEFINED
#define SIRKIT_DYNAMIC_SUBSTRING_MATCHING_DEFINED

class DynamicSubstringMatching
{
public:
    explicit DynamicSubstringMatching(std::string_view text) : text_(text)
    {
        const auto blocks = text.size() / kWordBits + (text.size() % kWordBits != 0);
        // 右にずらした窓を取り出すとき、末尾からも次の語を読めるようにする。
        for (auto &row : positions_)
        {
            row.resize(blocks + 1);
        }
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const auto byte = static_cast<unsigned char>(text[i]);
            ++frequency_[byte];
            positions_[byte][i / kWordBits] |= bit_at(i);
        }
    }

    DynamicSubstringMatching(const DynamicSubstringMatching &) = default;

    DynamicSubstringMatching(DynamicSubstringMatching &&other) noexcept
    {
        swap_state(other);
    }

    DynamicSubstringMatching &operator=(DynamicSubstringMatching other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return text_.size();
    }

    void set(std::size_t position, char replacement)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size())
        {
            throw std::out_of_range("DynamicSubstringMatching: position is out of range");
        }
#endif
        const auto old = static_cast<unsigned char>(text_[position]);
        const auto next = static_cast<unsigned char>(replacement);
        if (old == next)
        {
            return;
        }
        const auto index = position / kWordBits;
        const auto bit = bit_at(position);
        positions_[old][index] &= ~bit;
        positions_[next][index] |= bit;
        --frequency_[old];
        ++frequency_[next];
        text_[position] = replacement;
    }

    [[nodiscard]] bool contains(std::string_view pattern) const
    {
        return pattern.empty() || scan(pattern, [](std::size_t, std::uint64_t) { return true; });
    }

    [[nodiscard]] std::size_t count(std::string_view pattern) const
    {
        if (pattern.empty())
        {
            return size() + 1;
        }
        std::size_t result = 0;
        scan(pattern, [&](std::size_t, std::uint64_t candidates) {
            result += static_cast<std::size_t>(std::popcount(candidates));
            return false;
        });
        return result;
    }

    [[nodiscard]] std::vector<std::size_t> occurrences(std::string_view pattern) const
    {
        if (pattern.empty())
        {
            std::vector<std::size_t> all(size() + 1);
            std::iota(all.begin(), all.end(), std::size_t{0});
            return all;
        }
        std::vector<std::size_t> result;
        scan(pattern, [&](std::size_t base, std::uint64_t candidates) {
            do
            {
                result.push_back(base + static_cast<std::size_t>(std::countr_zero(candidates)));
                candidates &= candidates - 1;
            } while (candidates != 0);
            return false;
        });
        return result;
    }

private:
    static constexpr unsigned kWordBits = 64;
    static constexpr unsigned kAlphabetSize = 256;

    void swap_state(DynamicSubstringMatching &other) noexcept
    {
        text_.swap(other.text_);
        positions_.swap(other.positions_);
        frequency_.swap(other.frequency_);
    }

    static constexpr std::uint64_t bit_at(std::size_t position) noexcept
    {
        return std::uint64_t{1} << (position % kWordBits);
    }

    struct Constraint
    {
        const std::uint64_t *words;
        unsigned shift;

        [[nodiscard]] std::uint64_t at(std::size_t block) const noexcept
        {
            const auto word = words[block];
            if (shift == 0)
            {
                return word;
            }
            return (word >> shift) | (words[block + 1] << (kWordBits - shift));
        }
    };

    template<class Consume>
    bool scan(std::string_view pattern, Consume consume) const
    {
        if (pattern.size() > size())
        {
            return false;
        }
        std::vector<Constraint> constraints;
        constraints.reserve(pattern.size());
        std::size_t rarest = 0;
        auto smallest_frequency = size();
        for (std::size_t i = 0; i < pattern.size(); ++i)
        {
            const auto byte = static_cast<unsigned char>(pattern[i]);
            if (frequency_[byte] == 0)
            {
                return false;
            }
            if (frequency_[byte] < smallest_frequency)
            {
                rarest = i;
                smallest_frequency = frequency_[byte];
            }
            constraints.push_back({positions_[byte].data() + i / kWordBits, static_cast<unsigned>(i % kWordBits)});
        }
        std::swap(constraints.front(), constraints[rarest]);

        const auto last_start = size() - pattern.size();
        const auto last_block = last_start / kWordBits;
        const auto tail_mask = ~std::uint64_t{0} >> ((kWordBits - 1) - last_start % kWordBits);
        for (std::size_t block = 0; block <= last_block; ++block)
        {
            auto candidates = block == last_block ? tail_mask : ~std::uint64_t{0};
            for (const auto &constraint : constraints)
            {
                candidates &= constraint.at(block);
                if (candidates == 0)
                {
                    break;
                }
            }
            if (candidates != 0 && consume(block * kWordBits, candidates))
            {
                return true;
            }
        }
        return false;
    }

    std::string text_;
    std::array<std::vector<std::uint64_t>, kAlphabetSize> positions_;
    std::array<std::size_t, kAlphabetSize> frequency_{};
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
    std::size_t length = 0, query_count = 0;
    verify_in >> length >> query_count;
    std::string word;
    verify_in >> word;
    sirkit::DynamicSubstringMatching text(word);
    for (std::size_t qi = 0; qi < query_count; ++qi)
    {
        int type = 0;
        verify_in >> type;
        if (type == 1)
        {
            std::size_t position = 0;
            char symbol = '\0';
            verify_in >> position >> symbol;
            text.set(position - 1, symbol);
        }
        else
        {
            verify_in >> word;
            verify_out << (text.contains(word) ? "Yes\n" : "No\n");
        }
    }
}
