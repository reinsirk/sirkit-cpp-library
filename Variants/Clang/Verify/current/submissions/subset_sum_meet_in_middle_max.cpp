#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc184/tasks/abc184_f

// Library: Algorithm/Subset_Sum_Meet_In_Middle.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Subset_Sum_Meet_In_Middle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subset_sum_meet_in_middle.md
//
// API一覧
// MeetInMiddleSubsetSum(weights): 44個以下の符号付き重みを前処理する。空集合も数える。
// 正の重みの総和と負の重みの総和は、それぞれint64_tで表現できること。
// select_max_at_most(limit): limit以下で最大の和と、選んだ元の添字を表す64ビットマスクを返す。該当なしならnullopt。
// count_at_most(limit), count_exact(target): 対応する部分集合の個数を返す。同じ重みでも添字が異なれば別に数える。
// nを重みの個数として、前処理・各照会はO(2^ceil(n/2))時間。保存領域はO(2^ceil(n/2))、照会の追加領域はO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSET_SUM_MEET_IN_MIDDLE_DEFINED
#define SIRKIT_SUBSET_SUM_MEET_IN_MIDDLE_DEFINED

class MeetInMiddleSubsetSum
{
public:
    struct Selection
    {
        std::int64_t sum;
        std::uint64_t mask;
    };

    explicit MeetInMiddleSubsetSum(std::span<const std::int64_t> weights)
        : data_(std::make_shared<const Data>(prepare(weights)))
    {
    }

    MeetInMiddleSubsetSum(const MeetInMiddleSubsetSum &) noexcept = default;
    MeetInMiddleSubsetSum &operator=(const MeetInMiddleSubsetSum &) noexcept = default;

    [[nodiscard]] std::optional<Selection> select_max_at_most(std::int64_t limit) const
    {
        auto last = data_->right.size();
        std::optional<Selection> best;
        for (const auto &left : data_->left)
        {
            last = right_prefix_end<false>(left.sum, limit, last);
            if (last == 0)
            {
                break;
            }
            const auto &right = data_->right[last - 1];
            const auto sum = left.sum + right.sum;
            const auto mask = left.mask | (std::uint64_t{right.mask} << data_->left_size);
            if (sum == limit)
            {
                return Selection{sum, mask};
            }
            if (!best || sum > best->sum)
            {
                best = Selection{sum, mask};
            }
        }
        return best;
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t limit) const
    {
        return count_bounded<false>(limit);
    }

    [[nodiscard]] std::uint64_t count_exact(std::int64_t target) const
    {
        return count_bounded<false>(target) - count_bounded<true>(target);
    }

private:
    struct Entry
    {
        std::int64_t sum;
        std::uint32_t mask;
    };

    struct Data
    {
        std::vector<Entry> left;
        std::vector<Entry> right;
        std::size_t left_size;
    };

    static constexpr std::size_t kMaxSize = 44;
    std::shared_ptr<const Data> data_;

    static Data prepare(std::span<const std::int64_t> weights)
    {
        if (weights.size() > kMaxSize)
        {
            throw std::length_error("MeetInMiddleSubsetSum: too many weights");
        }
        std::int64_t positive = 0;
        std::int64_t negative = 0;
        for (const auto weight : weights)
        {
            if (weight >= 0)
            {
                if (weight > std::numeric_limits<std::int64_t>::max() - positive)
                {
                    throw std::overflow_error("MeetInMiddleSubsetSum: positive sum overflows");
                }
                positive += weight;
            }
            else
            {
                if (weight < std::numeric_limits<std::int64_t>::min() - negative)
                {
                    throw std::overflow_error("MeetInMiddleSubsetSum: negative sum overflows");
                }
                negative += weight;
            }
        }
        const auto left_size = weights.size() / 2;
        return {make_half(weights.first(left_size)), make_half(weights.subspan(left_size)), left_size};
    }

    static std::vector<Entry> make_half(std::span<const std::int64_t> weights)
    {
        std::vector<Entry> entries;
        entries.reserve(std::size_t{1} << weights.size());
        entries.push_back({0, 0});
        for (std::size_t i = 0; i < weights.size(); ++i)
        {
            const auto weight = weights[i];
            const auto bit = std::uint32_t{1} << i;
            std::size_t unselected = entries.size();
            std::size_t selected = entries.size();
            entries.resize(unselected + selected);
            // 整列済みの列を、重みを加えた同じ列と末尾から併合する。
            while (unselected != 0 && selected != 0)
            {
                const auto selected_sum = entries[selected - 1].sum + weight;
                const auto dest = unselected + selected - 1;
                if (entries[unselected - 1].sum > selected_sum)
                {
                    --unselected;
                    entries[dest] = entries[unselected];
                }
                else
                {
                    --selected;
                    entries[dest] = {selected_sum, entries[selected].mask | bit};
                }
            }
            while (selected != 0)
            {
                --selected;
                auto &entry = entries[selected];
                entry.sum += weight;
                entry.mask |= bit;
            }
        }
        return entries;
    }

    template<bool Strict>
    std::size_t right_prefix_end(std::int64_t left_sum, std::int64_t limit, std::size_t last) const
    {
        while (last != 0)
        {
            const auto sum = left_sum + data_->right[last - 1].sum;
            if constexpr (Strict)
            {
                if (sum < limit)
                {
                    break;
                }
            }
            else if (sum <= limit)
            {
                break;
            }
            --last;
        }
        return last;
    }

    template<bool Strict>
    std::uint64_t count_bounded(std::int64_t limit) const
    {
        std::uint64_t count = 0;
        auto last = data_->right.size();
        for (const auto &left : data_->left)
        {
            last = right_prefix_end<Strict>(left.sum, limit, last);
            if (last == 0)
            {
                break;
            }
            count += last;
        }
        return count;
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
#include <cstdint>
#include <vector>

int main()
{
    std::size_t count = 0;
    std::int64_t limit = 0;
    verify_in >> count >> limit;
    std::vector<std::int64_t> weights(count);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    const sirkit::MeetInMiddleSubsetSum sums(weights);
    verify_out << sums.select_max_at_most(limit)->sum << '\n';
}
