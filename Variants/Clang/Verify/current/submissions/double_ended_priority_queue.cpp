#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/double_ended_priority_queue

// Library: DataStructure/Double_Ended_Priority_Queue.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Double_Ended_Priority_Queue.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/double_ended_priority_queue.md
//
// API一覧
// DoubleEndedPriorityQueue<T,Compare>(): 空。DoubleEndedPriorityQueue(values,compare={}): O(n)構築。
// size()/empty(): 個数/空か。O(1)。push(value): 要素を追加。償却O(log(n+1))。
// min()/max(): 比較順の両端へのconst参照。O(1)。pop_min()/pop_max(): 両端を取り出す。O(log(n+1))。
// 両端の取得・削除には要素が必要。重複可。追加・削除で取得済みの参照を無効にする。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DOUBLE_ENDED_PRIORITY_QUEUE_DEFINED
#define SIRKIT_DOUBLE_ENDED_PRIORITY_QUEUE_DEFINED

template <class T, class Compare = std::less<T>>
class DoubleEndedPriorityQueue
{
public:
    DoubleEndedPriorityQueue() = default;

    explicit DoubleEndedPriorityQueue(std::vector<T> values, Compare compare = {})
        : values_(std::move(values)), compare_(std::move(compare))
    {
        for (auto i = values_.size() / 2; i != 0;)
        {
            --i;
            sift_down(i, is_min_level(i));
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return values_.empty();
    }

    [[nodiscard]] const T &min() const
    {
        require_nonempty();
        return values_.front();
    }

    [[nodiscard]] const T &max() const
    {
        require_nonempty();
        return values_[max_index()];
    }

    void push(T value)
    {
        auto i = values_.size();
        values_.push_back(std::move(value));
        if (i == 0)
        {
            return;
        }

        auto min_level = is_min_level(i);
        const auto parent = (i - 1) / 2;
        if (earlier(parent, i, min_level))
        {
            // 親レベルとの順序が逆なら、親を起点に更新する。
            std::swap(values_[i], values_[parent]);
            i = parent;
            min_level = !min_level;
        }
        while (i >= 3)
        {
            const auto grandparent = (i - 3) / 4;
            if (!earlier(i, grandparent, min_level))
            {
                break;
            }
            std::swap(values_[i], values_[grandparent]);
            i = grandparent;
        }
    }

    T pop_min()
    {
        require_nonempty();
        return take(0, true);
    }

    T pop_max()
    {
        require_nonempty();
        const auto index = max_index();
        return take(index, index == 0);
    }

private:
    std::vector<T> values_;
    [[no_unique_address]] Compare compare_{};

    static bool is_min_level(std::size_t index) noexcept
    {
        return (std::bit_width(index + 1) & 1U) != 0;
    }

    void require_nonempty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("DoubleEndedPriorityQueue: empty");
        }
#endif
    }

    std::size_t max_index() const
    {
        if (size() < 3)
        {
            return size() - 1;
        }
        return compare_(values_[1], values_[2]) ? 2 : 1;
    }

    bool earlier(std::size_t a, std::size_t b, bool min_level) const
    {
        return min_level ? compare_(values_[a], values_[b]) : compare_(values_[b], values_[a]);
    }

    void sift_down(std::size_t index, bool min_level)
    {
        const auto count = size();
        while (index < count / 2)
        {
            const auto left = index * 2 + 1;
            auto best = left;
            if (left + 1 < count && earlier(left + 1, best, min_level))
            {
                ++best;
            }
            if (left < count / 2)
            {
                const auto first = left * 2 + 1;
                const auto last = first + std::min<std::size_t>(4, count - first);
                for (auto child = first; child < last; ++child)
                {
                    if (earlier(child, best, min_level))
                    {
                        best = child;
                    }
                }
            }
            if (!earlier(best, index, min_level))
            {
                break;
            }
            std::swap(values_[best], values_[index]);
            if (best <= left + 1)
            {
                break;
            }
            const auto parent = (best - 1) / 2;
            if (earlier(parent, best, min_level))
            {
                std::swap(values_[parent], values_[best]);
            }
            index = best;
        }
    }

    T take(std::size_t index, bool min_level)
    {
        auto result = std::move(values_[index]);
        if (index + 1 == size())
        {
            values_.pop_back();
        }
        else
        {
            values_[index] = std::move(values_.back());
            values_.pop_back();
            sift_down(index, min_level);
        }
        return result;
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


#include <utility>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    DoubleEndedPriorityQueue<int> queue(std::move(values));
    while (q-- != 0)
    {
        int kind;
        verify_in >> kind;
        if (kind == 0)
        {
            int value;
            verify_in >> value;
            queue.push(value);
        }
        else
        {
            verify_out << (kind == 1 ? queue.pop_min() : queue.pop_max()) << '\n';
        }
    }
}
