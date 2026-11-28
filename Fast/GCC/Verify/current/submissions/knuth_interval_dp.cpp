#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/awc0100/tasks/awc0100_o

// Library: Algorithm/Knuth_Interval_DP.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Knuth_Interval_DP.cpp
// 参考（円環へ適用する方法）: https://atcoder.jp/contests/awc0100/editorial/21322
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/knuth_interval_dp.md
//
// API一覧
// KnuthIntervalDP(n,cost)/KnuthIntervalDP(n,max_length,cost): 長さ2以上の区間DPをKnuth最適化で構築。
// D(l,r)=cost(l,r)+min_{l<k<r}(D(l,k)+D(k,r))。長さ0,1は0。costの単調性・四辺形不等式は文書参照。
// 添字は0始まり半開、0<=l<=r<=nかつr-l<=max_length。省略時の最大区間長はn。
// size()/max_length(): 列長/最大区間長、O(1)。get(l,r): 最小費用。split(l,r): 最適な分割の最大k、長さ0,1はnullopt。各O(1)。
// 構築O(n*max_length+1)時間・領域（costがO(1)の場合）。費用と途中の和はint64_tに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KNUTH_INTERVAL_DP_DEFINED
#define SIRKIT_KNUTH_INTERVAL_DP_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::int64_t KnuthIntervalCostSum(std::int64_t left, std::int64_t right)
{
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(left) + static_cast<std::uint64_t>(right));
}
} // namespace reinsirk::detail

class KnuthIntervalDP
{
    using Value = std::int64_t;
    struct State
    {
        Value value;
        int split;
    };

    std::vector<State> states_;
    int size_ = 0;
    int limit_ = 0;

    static std::uint64_t total_states(int size, int limit) noexcept
    {
        if (limit < 2)
        {
            return 0;
        }
        // 長さ2からlimitまでの区間数は等差数列。長さ0,1は保存しない。
        const auto n = static_cast<std::uint64_t>(size);
        const auto m = static_cast<std::uint64_t>(limit);
        return (m - 1) * (2 * n - m) / 2;
    }

    static std::size_t storage_size(int size, int limit)
    {
        const auto states = total_states(size, limit);
        if (states > std::vector<State>().max_size())
        {
            throw std::length_error("KnuthIntervalDP: too many states");
        }
        return static_cast<std::size_t>(states);
    }

    [[nodiscard]] std::size_t index(int left, int right) const noexcept
    {
        return static_cast<std::size_t>(total_states(size_, right - left - 1)) + static_cast<std::size_t>(left);
    }

    [[nodiscard]] Value value_unchecked(int left, int right) const noexcept
    {
        return right - left < 2 ? 0 : states_[index(left, right)].value;
    }

    void check_range([[maybe_unused]] int left, [[maybe_unused]] int right) const
    {
    }

public:
    template <typename Cost>
    KnuthIntervalDP(int size, Cost cost)
        : KnuthIntervalDP(size, size, std::move(cost))
    {
    }

    template <typename Cost>
    KnuthIntervalDP(int size, int max_length, Cost cost)
        : states_(storage_size(size, max_length)), size_(size), limit_(max_length)
    {
        for (std::int64_t length = 2; length <= limit_; ++length)
        {
            const int span = static_cast<int>(length);
            for (int left = 0; left <= size_ - span; ++left)
            {
                const int right = left + span;
                int first = left + 1;
                int last = right - 1;
                // 同点では最大の分割点を使うと、前層の隣り合う最適点で探索を挟める。
                if (span > 2)
                {
                    first = states_[index(left, right - 1)].split;
                    last = states_[index(left + 1, right)].split;
                }
                Value best = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(value_unchecked(left, first), value_unchecked(first, right));
                int split = first;
                for (int middle = first + 1; middle <= last; ++middle)
                {
                    const Value candidate = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(value_unchecked(left, middle),
                                                                                   value_unchecked(middle, right));
                    if (candidate <= best)
                    {
                        split = middle;
                        best = candidate;
                    }
                }
                const Value interval_cost = std::invoke(cost, left, right);
                states_[index(left, right)] = {/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(best, interval_cost), split};
            }
        }
    }

    KnuthIntervalDP(const KnuthIntervalDP &) = default;
    KnuthIntervalDP &operator=(const KnuthIntervalDP &) = default;

    KnuthIntervalDP(KnuthIntervalDP &&other) noexcept
        : states_(std::move(other.states_)), size_(std::exchange(other.size_, 0)), limit_(std::exchange(other.limit_, 0))
    {
    }

    KnuthIntervalDP &operator=(KnuthIntervalDP &&other) noexcept
    {
        if (this != &other)
        {
            states_ = std::move(other.states_);
            size_ = std::exchange(other.size_, 0);
            limit_ = std::exchange(other.limit_, 0);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] int max_length() const noexcept
    {
        return limit_;
    }

    [[nodiscard]] Value get(int left, int right) const
    {
        check_range(left, right);
        return value_unchecked(left, right);
    }

    [[nodiscard]] std::optional<int> split(int left, int right) const
    {
        check_range(left, right);
        if (right - left < 2)
        {
            return std::nullopt;
        }
        return states_[index(left, right)].split;
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


#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    const int doubled = 2 * n - 1;
    std::vector<std::int64_t> prefix(static_cast<std::size_t>(doubled + 1));
    for (int i = 0; i < n; ++i)
    {
        std::int64_t value{};
        verify_in >> value;
        prefix[static_cast<std::size_t>(i + 1)] = prefix[static_cast<std::size_t>(i)] + value;
    }
    const auto total = prefix[static_cast<std::size_t>(n)];
    for (int i = n + 1; i <= doubled; ++i)
    {
        prefix[static_cast<std::size_t>(i)] = total + prefix[static_cast<std::size_t>(i - n)];
    }
    const sirkit::KnuthIntervalDP dp(doubled, n, [&](int left, int right)
    {
        return prefix[static_cast<std::size_t>(right)] - prefix[static_cast<std::size_t>(left)];
    });
    auto answer = dp.get(0, n);
    for (int i = 1; i < n; ++i)
    {
        answer = std::min(answer, dp.get(i, i + n));
    }
    verify_out << answer << '\n';
}
