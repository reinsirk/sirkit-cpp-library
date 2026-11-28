#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2172

// Library: String/Concatenated_Pattern_Count.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Concatenated_Pattern_Count.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/concatenated_pattern_count.md
//
// API一覧
// [ConcatenatedPatternCounter] 固定パターンの出現回数を、文字列を展開せずに管理する。重なった出現も数える。
// ConcatenatedPatternCounter(pattern,modulus=998244353): 空でないバイト列と正の法を指定。O(m*a)時間・領域。
// summarize(text): 文字列の要約Summaryを構築。空文字列は連結の単位元。O(m*(|text|+1))時間・O(m)領域。
// concatenate(left,right): 連結left+rightの要約。入力は変更しない。O(m)時間・領域。
// count(summary): パターンの出現回数を法modulusで返す。O(1)時間・領域。
// 要約は作成した個体とそのコピーで使える。mはパターン長、aはパターンの相異なるバイト数（1..256）。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONCATENATED_PATTERN_COUNT_DEFINED
#define SIRKIT_CONCATENATED_PATTERN_COUNT_DEFINED

namespace detail
{
inline std::size_t concatenated_pattern_table_size(std::size_t states, std::size_t symbols,
                                                  std::size_t capacity)
{
    const auto limit = std::min<std::size_t>(std::numeric_limits<std::uint32_t>::max(), capacity / symbols);
    if (states > limit)
    {
        throw std::length_error("ConcatenatedPatternCounter: pattern too long");
    }
    return states * symbols;
}
} // namespace detail

class ConcatenatedPatternCounter
{
    struct Transition
    {
        std::uint32_t state = 0;
        std::uint32_t matches = 0;
    };

    struct Profile
    {
        static constexpr auto kMissingSymbol = std::numeric_limits<std::uint16_t>::max();

        Profile(std::string_view pattern, std::uint32_t modulus_value)
            : length(pattern.size()), modulus(modulus_value)
        {
            if (pattern.empty() || modulus == 0)
            {
                throw std::invalid_argument("ConcatenatedPatternCounter: empty pattern or zero modulus");
            }
            column.fill(kMissingSymbol);
            for (const char character : pattern)
            {
                const auto byte = static_cast<unsigned char>(character);
                if (column[byte] == kMissingSymbol)
                {
                    column[byte] = symbol_count++;
                }
            }

            transitions.resize(detail::concatenated_pattern_table_size(length, symbol_count, transitions.max_size()));
            // 行iは長さiの接頭辞が一致した状態。異なるバイトは、直前の最長の真の接頭尾辞と同じように処理する。
            const auto single_match = std::uint32_t{1} % modulus;
            std::uint32_t border = 0;
            for (std::size_t i = 0; i < length; ++i)
            {
                const auto row = i * symbol_count;
                if (i != 0)
                {
                    const auto fallback_row = static_cast<std::size_t>(border) * symbol_count;
                    std::copy_n(transitions.begin() + fallback_row, symbol_count, transitions.begin() + row);
                }
                const auto byte = static_cast<unsigned char>(pattern[i]);
                auto &match = transitions[row + column[byte]];
                border = match.state;
                match = i + 1 == length ? Transition{border, single_match}
                                        : Transition{static_cast<std::uint32_t>(i + 1), 0};
            }
        }

        [[nodiscard]] Transition next(std::uint32_t state, unsigned char byte) const noexcept
        {
            const auto symbol = column[byte];
            if (symbol == kMissingSymbol)
            {
                return {};
            }
            return transitions[static_cast<std::size_t>(state) * symbol_count + symbol];
        }

        [[nodiscard]] std::uint32_t add(std::uint32_t a, std::uint32_t b) const noexcept
        {
            const auto sum = static_cast<std::uint64_t>(a) + b;
            return static_cast<std::uint32_t>(sum < modulus ? sum : sum - modulus);
        }

        std::size_t length;
        std::uint32_t modulus;
        std::uint16_t symbol_count = 0;
        std::array<std::uint16_t, 256> column;
        std::vector<Transition> transitions;
    };

  public:
    class Summary
    {
      public:
        Summary(const Summary &) = default;
        Summary(Summary &&) noexcept = default;

        Summary &operator=(Summary other) noexcept
        {
            transitions_.swap(other.transitions_);
            profile_.swap(other.profile_);
            return *this;
        }

      private:
        friend class ConcatenatedPatternCounter;

        explicit Summary(const std::shared_ptr<const Profile> &profile)
            : transitions_(profile->length), profile_(profile)
        {
        }

        std::vector<Transition> transitions_;
        std::shared_ptr<const Profile> profile_;
    };

    explicit ConcatenatedPatternCounter(std::string_view pattern, std::uint32_t modulus = 998'244'353)
        : profile_(std::make_shared<const Profile>(pattern, modulus))
    {
    }

    ConcatenatedPatternCounter(const ConcatenatedPatternCounter &) noexcept = default;
    ConcatenatedPatternCounter &operator=(const ConcatenatedPatternCounter &) noexcept = default;

    [[nodiscard]] Summary summarize(std::string_view text) const
    {
        Summary result(profile_);
        for (std::size_t i = 0; i < profile_->length; ++i)
        {
            Transition combined{static_cast<std::uint32_t>(i), 0};
            for (const char character : text)
            {
                const auto step = profile_->next(combined.state, static_cast<unsigned char>(character));
                combined = {step.state, profile_->add(combined.matches, step.matches)};
            }
            result.transitions_[i] = combined;
        }
        return result;
    }

    [[nodiscard]] Summary concatenate(const Summary &left, const Summary &right) const
    {
        Summary result(profile_);
        for (std::size_t i = 0; i < profile_->length; ++i)
        {
            const auto a = left.transitions_[i];
            const auto b = right.transitions_[a.state];
            result.transitions_[i] = {b.state, profile_->add(a.matches, b.matches)};
        }
        return result;
    }

    [[nodiscard]] std::uint32_t count(const Summary &summary) const
    {
        return summary.transitions_.front().matches;
    }

  private:

    std::shared_ptr<const Profile> profile_;
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
#include <vector>

int main()
{
    std::size_t count;
    std::string pattern;
    verify_in >> count >> pattern;
    const sirkit::ConcatenatedPatternCounter counter(pattern);
    std::vector<sirkit::ConcatenatedPatternCounter::Summary> summaries;
    summaries.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        std::string text;
        verify_in >> text;
        if (text == "~")
        {
            std::size_t left, right;
            verify_in >> left >> right;
            summaries.push_back(counter.concatenate(summaries[left - 1], summaries[right - 1]));
        }
        else
        {
            summaries.push_back(counter.summarize(text));
        }
    }
    verify_out << counter.count(summaries.back()) << '\n';
}
