#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/runenumerate

// Library: String/Run_Enumerate.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Run_Enumerate.cpp
// 参考: H. Bannai et al., The "Runs" Theorem (https://arxiv.org/abs/1406.0263)。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_enumerate.md
//
// API一覧
// run_enumerate(s): 文字列sの極大な周期区間を重複なく列挙する。O(N log(N+1))時間・領域。
// StringRun{period, left, right}: 最小周期と0始まりの半開区間。長さは周期の2倍以上。
// 同じ周期で左右どちらにも延長できない区間を、(period, left, right)の昇順で返す。
// 文字は符号なしバイトとして扱う。空文字列を受け取れる。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <compare>
#include <cstddef>
#include <limits>
#include <numeric>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STRING_RUN_DEFINED
#define SIRKIT_STRING_RUN_DEFINED

struct StringRun
{
    std::size_t period;
    std::size_t left;
    std::size_t right;

    auto operator<=>(const StringRun &) const = default;
};

namespace detail
{
class RunBlockRanks
{
    using Index = std::size_t;

public:
    explicit RunBlockRanks(std::string_view text) : size_(text.size())
    {
        constexpr auto kByteAlphabet = static_cast<Index>(std::numeric_limits<unsigned char>::max()) + 1;
        ranks_.reserve(std::bit_width(size_));
        auto &first = ranks_.emplace_back(size_);
        for (Index i = 0; i < size_; ++i)
        {
            first[i] = static_cast<unsigned char>(text[i]) + Index{1};
        }

        // 長さ2^kごとに等しいブロックへ同じ順位を付ける。末尾の外は順位0で埋める。
        std::vector<Index> offsets(std::max(kByteAlphabet, size_) + 1);
        std::vector<Index> indices(size_);
        std::vector<Index> order(size_);
        std::iota(indices.begin(), indices.end(), Index{0});
        const auto stable_sort = [&](const std::vector<Index> &rank, Index kinds)
        {
            auto slots = std::span(offsets).first(kinds + 1);
            std::ranges::fill(slots, 0);
            for (const auto index : indices)
            {
                ++slots[rank[index]];
            }
            Index start = 0;
            for (auto &slot : slots)
            {
                const auto count = slot;
                slot = start;
                start += count;
            }
            for (const auto index : indices)
            {
                order[slots[rank[index]]++] = index;
            }
        };

        Index kinds = kByteAlphabet;
        stable_sort(first, kinds);
        for (Index width = 1; width <= size_ / 2; width *= 2)
        {
            const auto &previous = ranks_.back();
            const auto second = [&](Index i)
            {
                return i < size_ - width ? previous[i + width] : 0;
            };
            // 後半の順位順は直前の並びをwidthだけずらして得る。前半の順位で安定ソートする。
            Index count = 0;
            for (Index i = size_ - width; i < size_; ++i)
            {
                indices[count++] = i;
            }
            for (const auto i : order)
            {
                if (i >= width)
                {
                    indices[count++] = i - width;
                }
            }
            stable_sort(previous, kinds);

            std::vector<Index> rank(size_);
            kinds = 0;
            std::pair<Index, Index> last{0, 0};
            for (const auto i : order)
            {
                const std::pair key{previous[i], second(i)};
                kinds += key != last;
                rank[i] = kinds;
                last = key;
            }
            ranks_.push_back(std::move(rank));
        }
    }

    Index CommonPrefix(Index a, Index b) const
    {
        return CommonLength(size_ - b, [a, b](Index matched, Index)
                            {
                                return std::pair(a + matched, b + matched);
                            });
    }

    Index CommonSuffix(Index a, Index b) const
    {
        return CommonLength(a, [a, b](Index matched, Index width)
                            {
                                return std::pair(a - matched - width, b - matched - width);
                            });
    }

private:
    Index size_;
    std::vector<std::vector<Index>> ranks_;

    template <class Positions>
    Index CommonLength(Index limit, Positions positions) const
    {
        Index matched = 0;
        for (auto level = std::bit_width(limit); level != 0; --level)
        {
            const auto width = Index{1} << (level - 1);
            if (width <= limit - matched)
            {
                const auto [a, b] = positions(matched, width);
                if (ranks_[level - 1][a] == ranks_[level - 1][b])
                {
                    matched += width;
                }
            }
        }
        return matched;
    }
};
} // namespace detail

inline std::vector<StringRun> run_enumerate(std::string_view text)
{
    const auto n = text.size();
    if (n < 2)
    {
        return {};
    }
    const detail::RunBlockRanks ranks(text);
    std::vector<StringRun> result;
    std::vector<std::size_t> suffixes;
    suffixes.reserve(n);
    for (const bool reversed_alphabet : {false, true})
    {
        suffixes.clear();
        for (auto i = n; i != 0;)
        {
            --i;
            std::size_t right = 0;
            while (!suffixes.empty())
            {
                const auto j = suffixes.back();
                right = ranks.CommonPrefix(i, j);
                if (j + right == n ||
                    (static_cast<unsigned char>(text[j + right]) < static_cast<unsigned char>(text[i + right])) !=
                        reversed_alphabet)
                {
                    break;
                }
                suffixes.pop_back();
            }
            if (!suffixes.empty())
            {
                // 次に小さい接尾辞との間隔がLyndon根の長さ。これを周期として両側へ延長する。
                const auto j = suffixes.back();
                const auto period = j - i;
                const auto left = ranks.CommonSuffix(i, j);
                if (left + right >= period)
                {
                    result.push_back({period, i - left, j + right});
                }
            }
            suffixes.push_back(i);
        }
    }
    std::ranges::sort(result);
    const auto duplicates = std::ranges::unique(result);
    result.erase(duplicates.begin(), duplicates.end());
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

#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    const auto runs = sirkit::run_enumerate(text);
    verify_out << runs.size() << '\n';
    for (const auto &[period, left, right] : runs)
    {
        verify_out << period << ' ' << left << ' ' << right << '\n';
    }
}
