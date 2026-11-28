#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc413/tasks/abc413_c

// Library: DataStructure/Run_Length_Queue.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Run_Length_Queue.cpp
// 参考（運用例）: https://atcoder.jp/contests/abc413/tasks/abc413_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_length_queue.md
//
// API一覧
// RunLengthQueue<T>(): 空で構築。同じ値の連続を1組にまとめる。Tのconst比較==が必要。
// size()/empty()/run_count(): 要素数(uint64_t)/空か/連続グループ数。O(1)。
// front()/front_count(): 先頭の値/その連続個数。空のfront()はout_of_range、front_count()は0。O(1)。
// push_back(value,count=1): valueをcount個追加。O(1)。検査有効時は要素数上限を超えるとlength_error。
// pop_front(count=1): 先頭count個を削除。consume_front(count,visitor): 削除する各組にvisitor(const T&,個数)を呼ぶ。
// 削除は触る連続グループ数+1に比例。空からの追加・削除全体では操作回数に比例。count<=size()。
// 保存する値は現在の連続グループ数。内部領域は同時保持グループ数の最大+1に比例。コールバックは同じキューを変更しないこと。例外時は成功済みの呼出し分だけ削除。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RUN_LENGTH_QUEUE_DEFINED
#define SIRKIT_RUN_LENGTH_QUEUE_DEFINED

template <typename T>
class RunLengthQueue
{
    using Count = std::uint64_t;
    struct Run
    {
        T value;
        Count end;
    };

    std::deque<Run> runs_;
    Count head_ = 0;

public:
    RunLengthQueue() = default;
    RunLengthQueue(const RunLengthQueue &) requires std::copy_constructible<T> = default;

    RunLengthQueue(RunLengthQueue &&other)
        : runs_(std::move(other.runs_)), head_(std::exchange(other.head_, 0))
    {
        other.runs_.clear();
    }

    RunLengthQueue &operator=(const RunLengthQueue &other) requires std::copy_constructible<T>
    {
        if (this != &other)
        {
            auto copied = other.runs_;
            runs_.swap(copied);
            head_ = other.head_;
        }
        return *this;
    }

    RunLengthQueue &operator=(RunLengthQueue &&other)
    {
        if (this != &other)
        {
            runs_ = std::move(other.runs_);
            head_ = std::exchange(other.head_, 0);
            other.runs_.clear();
        }
        return *this;
    }

    [[nodiscard]] Count size() const noexcept
    {
        return runs_.empty() ? 0 : runs_.back().end - head_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return runs_.empty();
    }

    [[nodiscard]] std::size_t run_count() const noexcept
    {
        return runs_.size();
    }

    [[nodiscard]] const T &front() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (runs_.empty())
        {
            throw std::out_of_range("RunLengthQueue: empty");
        }
#endif
        return runs_.front().value;
    }

    [[nodiscard]] Count front_count() const noexcept
    {
        return runs_.empty() ? 0 : runs_.front().end - head_;
    }

    void push_back(T value, Count count = 1)
    {
        if (count == 0)
        {
            return;
        }
        const Count previous = size();
#ifndef SKIP_RUNTIME_CHECKS
        if (count > std::numeric_limits<Count>::max() - previous)
        {
            throw std::length_error("RunLengthQueue: too many elements");
        }
#endif
        // 通算位置は周回してよい。差を取ると現在の個数に戻る。
        const Count end = head_ + previous + count;
        if (!runs_.empty() && std::as_const(runs_.back().value) == std::as_const(value))
        {
            runs_.back().end = end;
        }
        else
        {
            runs_.push_back(Run{std::move(value), end});
        }
    }

    template <typename Visitor>
    void consume_front(Count count, Visitor &&visitor)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (count > size())
        {
            throw std::out_of_range("RunLengthQueue: not enough elements");
        }
#endif
        while (count != 0)
        {
            const auto &run = runs_.front();
            const Count removed = std::min(count, run.end - head_);
            std::invoke(visitor, run.value, removed);
            head_ += removed;
            count -= removed;
            if (head_ == run.end)
            {
                runs_.pop_front();
            }
        }
    }

    void pop_front(Count count = 1)
    {
        consume_front(count, [](const T &, Count) noexcept
        {
        });
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


#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    sirkit::RunLengthQueue<std::uint64_t> queue;
    for (int i = 0; i < queries; ++i)
    {
        int type{};
        std::uint64_t count{};
        verify_in >> type >> count;
        if (type == 1)
        {
            std::uint64_t value{};
            verify_in >> value;
            queue.push_back(value, count);
        }
        else
        {
            std::uint64_t sum = 0;
            queue.consume_front(count, [&](std::uint64_t value, std::uint64_t copies)
            {
                sum += value * copies;
            });
            verify_out << sum << '\n';
        }
    }
}
