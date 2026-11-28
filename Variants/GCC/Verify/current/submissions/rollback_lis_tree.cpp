#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc165/tasks/abc165_f

// Library: Algorithm/Rollback_LIS.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Rollback_LIS.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rollback_lis.md
//
// API一覧
// [RollbackLIS<T,Strict=true>] 末尾追加・削除する列の最長増加部分列。Strict=falseは非減少。
// push_back(value): 末尾追加。現在のLIS長をLとして償却O(1+log(L+1))。
// pop_back(): 末尾削除。空でないこと。O(1)。
// rollback(prefix_size): 指定した長さまで末尾を削除。prefix_size<=size()。1要素あたりO(1)。
// indices(): 一つの最長増加部分列を、現在の列の0始まりの添字で返す。O(L+1)時間・領域。
// length()/size()/empty(): LIS長/現在の列長/空か。各O(1)。
// reserve(capacity)/clear(): 領域予約/空へ戻す。確保済み領域は末尾削除やclearで解放しない。
// Tのムーブ構築と破棄はnoexcept、operator<は一貫した狭義弱順序。比較とT操作をO(1)として計算。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROLLBACK_LIS_DEFINED
#define SIRKIT_ROLLBACK_LIS_DEFINED

template<class T, bool Strict = true>
    requires std::destructible<T> && std::is_nothrow_move_constructible_v<T>
class RollbackLIS
{
    static constexpr auto kNoIndex = std::numeric_limits<std::size_t>::max();

    struct Entry
    {
        T value;
        std::size_t slot;
        std::size_t previous_tail;
        std::size_t predecessor;
    };

  public:
    RollbackLIS() = default;
    RollbackLIS(const RollbackLIS &) requires std::copy_constructible<T> = default;
    RollbackLIS(RollbackLIS &&other) noexcept
    {
        exchange(other);
    }

    RollbackLIS &operator=(RollbackLIS other) noexcept
    {
        exchange(other);
        return *this;
    }

    void push_back(T value)
    {
        const auto slot = find_slot(value);
        const auto index = size();
        const bool extends = slot == length();
        const auto previous = extends ? kNoIndex : tails_[slot];
        const auto predecessor = slot == 0 ? kNoIndex : tails_[slot - 1];
        entries_.push_back({std::move(value), slot, previous, predecessor});
        if (extends)
        {
            try
            {
                tails_.push_back(index);
            }
            catch (...)
            {
                entries_.pop_back();
                throw;
            }
        }
        else
        {
            tails_[slot] = index;
        }
    }

    void pop_back()
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("RollbackLIS: pop_back on an empty sequence");
        }
#endif
        undo_last();
    }

    void rollback(std::size_t prefix_size)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (prefix_size > size())
        {
            throw std::out_of_range("RollbackLIS: invalid prefix length");
        }
#endif
        while (size() > prefix_size)
        {
            undo_last();
        }
    }

    [[nodiscard]] std::vector<std::size_t> indices() const
    {
        std::vector<std::size_t> result(length());
        if (!result.empty())
        {
            auto index = tails_.back();
            for (std::size_t i = result.size(); i > 0; --i)
            {
                result[i - 1] = index;
                index = entries_[index].predecessor;
            }
        }
        return result;
    }

    [[nodiscard]] std::size_t length() const noexcept
    {
        return tails_.size();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return entries_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return entries_.empty();
    }

    void reserve(std::size_t capacity)
    {
        entries_.reserve(capacity);
        tails_.reserve(capacity);
    }

    void clear() noexcept
    {
        entries_.clear();
        tails_.clear();
    }

  private:
    [[nodiscard]] std::size_t find_slot(const T &value) const
    {
        if constexpr (Strict)
        {
            const auto found = std::lower_bound(tails_.begin(), tails_.end(), value,
                [this](std::size_t index, const T &target) { return entries_[index].value < target; });
            return static_cast<std::size_t>(found - tails_.begin());
        }
        else
        {
            const auto found = std::upper_bound(tails_.begin(), tails_.end(), value,
                [this](const T &target, std::size_t index) { return target < entries_[index].value; });
            return static_cast<std::size_t>(found - tails_.begin());
        }
    }

    void undo_last() noexcept
    {
        const auto &entry = entries_.back();
        if (entry.previous_tail == kNoIndex)
        {
            tails_.pop_back();
        }
        else
        {
            tails_[entry.slot] = entry.previous_tail;
        }
        entries_.pop_back();
    }

    void exchange(RollbackLIS &other) noexcept
    {
        entries_.swap(other.entries_);
        tails_.swap(other.tails_);
    }

    std::vector<Entry> entries_;
    std::vector<std::size_t> tails_;
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
#include <vector>

int main()
{
    int n = 0;
    verify_in >> n;
    std::vector<int> values(n);
    for (int &value : values)
    {
        verify_in >> value;
    }
    std::vector<std::vector<int>> edges(n);
    for (int i = 1; i < n; ++i)
    {
        int first = 0, second = 0;
        verify_in >> first >> second;
        --first;
        --second;
        edges[first].push_back(second);
        edges[second].push_back(first);
    }

    struct Visit
    {
        int vertex;
        int parent;
        bool leave;
    };
    std::vector<Visit> stack;
    stack.reserve(n);
    stack.push_back({0, -1, false});
    sirkit::RollbackLIS<int> lis;
    lis.reserve(n);
    std::vector<std::size_t> answers(n);
    while (!stack.empty())
    {
        const auto [vertex, parent, leave] = stack.back();
        stack.pop_back();
        if (leave)
        {
            lis.pop_back();
            continue;
        }

        lis.push_back(values[vertex]);
        answers[vertex] = lis.length();
        stack.push_back({vertex, parent, true});
        for (int adjacent : edges[vertex])
        {
            if (adjacent != parent)
            {
                stack.push_back({adjacent, vertex, false});
            }
        }
    }
    for (auto answer : answers)
    {
        verify_out << answer << '\n';
    }
}
