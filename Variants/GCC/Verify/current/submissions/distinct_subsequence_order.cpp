#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/tdpc/tasks/tdpc_lexicographical

// Library: String/Distinct_Subsequence_Order.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Distinct_Subsequence_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/distinct_subsequence_order.md
//
// API一覧
// DistinctSubsequenceOrder(text): 空でない相異なる部分列を、符号なしバイトの辞書順で索引化する。O(n+1)時間・領域。
// size(): 元の列の長さ。count_distinct(): 空でない部分列の種類数。各O(1)。
// kth_smallest(rank): 辞書順で1始まりのrank番目の部分列を返す。範囲外ならnullopt。
// rank_of(pattern): 空でない部分列の辞書順順位。該当しなければnullopt。
// prefix_count(prefix): prefixで始まる空でない部分列の種類数。空のprefixなら全種類数。
// 個数とrank_ofの結果はUINT64_MAXで上限をとる。kth_smallestは表せる順位まで正確。
// byte種類数σ、入力または出力の長さkに対し、kth_smallest/rank_ofはO(σk log(n+2)+1)、prefix_countはO(k log(n+2)+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DISTINCT_SUBSEQUENCE_ORDER_DEFINED
#define SIRKIT_DISTINCT_SUBSEQUENCE_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr std::uint64_t kSubsequenceCountCap = std::numeric_limits<std::uint64_t>::max();

inline constexpr std::uint64_t add_subsequence_counts(std::uint64_t left, std::uint64_t right) noexcept
{
    return left + std::min(kSubsequenceCountCap - left, right);
}

inline std::size_t subsequence_slot_count(std::size_t size)
{
    if (size >= std::vector<std::uint64_t>().max_size())
    {
        throw std::length_error("DistinctSubsequenceOrder: text is too long");
    }
    return size + 1;
}
} // namespace reinsirk::detail

class DistinctSubsequenceOrder
{
private:
    static constexpr unsigned kAlphabetSize = 256;

    struct Index
    {
        explicit Index(std::string_view text) : suffix_count(detail::subsequence_slot_count(text.size()))
        {
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                positions[static_cast<unsigned char>(text[i])].push_back(i);
            }
            for (unsigned code = 0; code < kAlphabetSize; ++code)
            {
                if (!positions[code].empty())
                {
                    alphabet.push_back(static_cast<unsigned char>(code));
                }
            }
            std::array<std::uint64_t, kAlphabetSize> starting_count{};
            for (std::size_t i = text.size(); i != 0;)
            {
                --i;
                const auto following = suffix_count[i + 1];
                if (following == detail::kSubsequenceCountCap)
                {
                    // 接尾辞を延ばしても、作れる部分列は減らない。
                    suffix_count[i] = following;
                    continue;
                }
                auto &branch = starting_count[static_cast<unsigned char>(text[i])];
                suffix_count[i] = detail::add_subsequence_counts(following - branch, following + 1);
                branch = following + 1;
            }
        }

        [[nodiscard]] std::size_t next_position(unsigned char symbol, std::size_t start) const
        {
            const auto &row = positions[symbol];
            const auto next = std::lower_bound(row.begin(), row.end(), start);
            return next == row.end() ? suffix_count.size() - 1 : *next;
        }

        [[nodiscard]] std::uint64_t continuation_count(std::size_t position) const noexcept
        {
            return detail::add_subsequence_counts(1, suffix_count[position + 1]);
        }

        std::array<std::vector<std::size_t>, kAlphabetSize> positions;
        std::vector<unsigned char> alphabet;
        std::vector<std::uint64_t> suffix_count;
    };

public:
    explicit DistinctSubsequenceOrder(std::string_view text) : index_(std::make_shared<Index>(text))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return index_ ? index_->suffix_count.size() - 1 : 0;
    }

    [[nodiscard]] std::uint64_t count_distinct() const noexcept
    {
        return index_ ? index_->suffix_count.front() : 0;
    }

    [[nodiscard]] std::optional<std::string> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > count_distinct())
        {
            return std::nullopt;
        }
        std::string answer;
        std::size_t start = 0;
        while (rank != 0)
        {
            // 残りの順位は、いずれかのバイトで始まる枝に必ず含まれる。
            for (std::size_t i = 0;; ++i)
            {
                const auto symbol = index_->alphabet[i];
                const auto next = index_->next_position(symbol, start);
                if (next == size())
                {
                    continue;
                }
                const auto branch = index_->continuation_count(next);
                if (rank <= branch)
                {
                    answer.push_back(static_cast<char>(symbol));
                    start = next + 1;
                    --rank;
                    break;
                }
                rank -= branch;
            }
        }
        return answer;
    }

    [[nodiscard]] std::optional<std::uint64_t> rank_of(std::string_view pattern) const
    {
        if (pattern.empty() || !index_)
        {
            return std::nullopt;
        }
        std::size_t start = 0;
        std::uint64_t rank = 0;
        for (const char byte : pattern)
        {
            const auto symbol = static_cast<unsigned char>(byte);
            for (const auto smaller : index_->alphabet)
            {
                if (smaller >= symbol)
                {
                    break;
                }
                const auto next = index_->next_position(smaller, start);
                if (next != size())
                {
                    rank = detail::add_subsequence_counts(rank, index_->continuation_count(next));
                }
            }
            const auto next = index_->next_position(symbol, start);
            if (next == size())
            {
                return std::nullopt;
            }
            rank = detail::add_subsequence_counts(rank, 1);
            start = next + 1;
        }
        return rank;
    }

    [[nodiscard]] std::uint64_t prefix_count(std::string_view prefix) const
    {
        if (prefix.empty())
        {
            return count_distinct();
        }
        if (!index_)
        {
            return 0;
        }
        std::size_t start = 0;
        for (const char byte : prefix)
        {
            const auto next = index_->next_position(static_cast<unsigned char>(byte), start);
            if (next == size())
            {
                return 0;
            }
            start = next + 1;
        }
        return index_->continuation_count(start - 1);
    }

private:
    // 索引は読み取り専用。同じ文字列を索引化したコピーが共用する。
    std::shared_ptr<const Index> index_;
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
#include <string>

int main()
{
    std::string text;
    std::uint64_t rank = 0;
    verify_in >> text >> rank;
    const sirkit::DistinctSubsequenceOrder order(text);
    const auto answer = order.kth_smallest(rank);
    if (answer)
    {
        verify_out << *answer << '\n';
    }
    else
    {
        verify_out << "Eel\n";
    }
}
