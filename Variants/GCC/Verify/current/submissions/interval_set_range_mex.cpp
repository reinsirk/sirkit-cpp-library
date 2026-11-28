#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2220

// Library: DataStructure/Interval_Set.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Interval_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_set.md
//
// API一覧
// IntervalSet<T>(): 空の整数集合を作る。Tはbool以外の整数型。O(1)。
// add(left,right)/erase(left,right): 半開区間の整数を追加・削除する。
// left<=right。空区間は何もしない。時間はO(log(m+1)+k)で、mは区間数、kは変更する区間数。
// contains(value): valueが含まれるか。O(log(m+1))。
// covers(left,right): 半開区間をすべて含むか。left<=right。空区間はtrue。O(log(m+1))。
// mex(first=0): first以上で含まれない最小の整数。O(log(m+1))。
// interval_at(value): valueを含む最大の半開区間。含まれなければnullopt。O(log(m+1))。
// size()/interval_count()/empty(): 整数の総数・区間数・空判定。O(1)。
// clear(): 全削除。O(m)。swap(other): 集合を交換する。O(1)。
// Tの最大値は右端として使える。要素としては格納しない。領域O(m+1)。
// 入力検査有効時はleft>rightでinvalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTERVAL_SET_DEFINED
#define SIRKIT_INTERVAL_SET_DEFINED

template <std::integral T = int>
    requires(!std::same_as<T, bool>)
class IntervalSet
{
public:
    using size_type = std::make_unsigned_t<T>;

    IntervalSet() = default;
    IntervalSet(const IntervalSet &) = default;

    IntervalSet(IntervalSet &&other)
    {
        swap(other);
    }

    IntervalSet &operator=(IntervalSet other) noexcept
    {
        swap(other);
        return *this;
    }

    void add(T left, T right)
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return;
        }

        auto first = intervals_.lower_bound(left);
        if (first != intervals_.begin())
        {
            const auto previous = std::prev(first);
            if (left <= previous->second)
            {
                first = previous;
            }
        }
        if (first == intervals_.end() || left < first->first)
        {
            first = intervals_.emplace_hint(first, left, right);
        }
        else
        {
            size_ -= Length(first->first, first->second);
            first->second = std::max(first->second, right);
        }
        auto next = std::next(first);
        while (next != intervals_.end() && next->first <= first->second)
        {
            size_ -= Length(next->first, next->second);
            first->second = std::max(first->second, next->second);
            next = intervals_.erase(next);
        }
        size_ += Length(first->first, first->second);
    }

    void erase(T left, T right)
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return;
        }

        auto current = intervals_.lower_bound(left);
        if (current != intervals_.begin())
        {
            const auto previous = std::prev(current);
            const T old_end = previous->second;
            if (left < old_end)
            {
                if (right < old_end)
                {
                    intervals_.emplace_hint(current, right, old_end);
                    previous->second = left;
                    size_ -= Length(left, right);
                    return;
                }
                previous->second = left;
                size_ -= Length(left, old_end);
            }
        }

        while (current != intervals_.end() && current->first < right)
        {
            if (right < current->second)
            {
                size_ -= Length(current->first, right);
                const auto next = std::next(current);
                auto node = intervals_.extract(current);
                node.key() = right;
                intervals_.insert(next, std::move(node));
                return;
            }
            size_ -= Length(current->first, current->second);
            current = intervals_.erase(current);
        }
    }

    [[nodiscard]] bool contains(T value) const
    {
        return Containing(value) != intervals_.end();
    }

    [[nodiscard]] bool covers(T left, T right) const
    {
        CheckInterval(left, right);
        if (left == right)
        {
            return true;
        }
        const auto current = Containing(left);
        return current != intervals_.end() && right <= current->second;
    }

    [[nodiscard]] T mex(T first = 0) const
    {
        const auto current = Containing(first);
        return current == intervals_.end() ? first : current->second;
    }

    [[nodiscard]] std::optional<std::pair<T, T>> interval_at(T value) const
    {
        const auto current = Containing(value);
        if (current == intervals_.end())
        {
            return std::nullopt;
        }
        return *current;
    }

    [[nodiscard]] size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::size_t interval_count() const noexcept
    {
        return intervals_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return intervals_.empty();
    }

    void clear() noexcept
    {
        intervals_.clear();
        size_ = 0;
    }

    void swap(IntervalSet &other) noexcept
    {
        intervals_.swap(other.intervals_);
        std::swap(size_, other.size_);
    }

private:
    static void CheckInterval([[maybe_unused]] T left, [[maybe_unused]] T right)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (right < left)
        {
            throw std::invalid_argument("IntervalSet: reversed interval");
        }
#endif
    }

    [[nodiscard]] static size_type Length(T left, T right) noexcept
    {
        return static_cast<size_type>(static_cast<size_type>(right) - static_cast<size_type>(left));
    }

    [[nodiscard]] typename std::map<T, T>::const_iterator Containing(T value) const
    {
        auto current = intervals_.upper_bound(value);
        if (current == intervals_.begin())
        {
            return intervals_.end();
        }
        --current;
        return value < current->second ? current : intervals_.end();
    }

    std::map<T, T> intervals_;
    size_type size_ = 0;
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
#include <tuple>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<std::tuple<int, int, int>> events;
    events.reserve(static_cast<std::size_t>(2 * n));
    for (int i = 0; i < n; ++i)
    {
        int left, right, value;
        verify_in >> left >> right >> value;
        if (value <= n)
        {
            events.emplace_back(left, value, 1);
            events.emplace_back(right + 1, value, -1);
        }
    }
    std::sort(events.begin(), events.end());

    sirkit::IntervalSet<int> active;
    std::vector<int> frequency(static_cast<std::size_t>(n + 1));
    std::size_t next = 0;
    int q;
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int position;
        verify_in >> position;
        while (next < events.size() && std::get<0>(events[next]) <= position)
        {
            const auto [coordinate, value, delta] = events[next++];
            auto &count = frequency[static_cast<std::size_t>(value)];
            if (count == 0)
            {
                active.add(value, value + 1);
            }
            count += delta;
            if (count == 0)
            {
                active.erase(value, value + 1);
            }
        }
        verify_out << active.mex() << '\n';
    }
}
