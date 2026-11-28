#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc281/tasks/abc281_e

// Library: DataStructure/Top_K_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Top_K_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/top_k_sum.md
//
// API一覧
// TopKSum<T,Largest,Sum>(k=0): 空の多重集合を作る。Largestがtrueなら大きい方から選ぶ。O(1)。
// insert(value)/erase(value): 1個ずつ追加・削除する。eraseは要素がなければfalse。O(log(n+1))。
// set_k(k): 選ぶ個数を更新する。実際に選ぶ個数の差をdとしてO(1+d log(n+1))。
// sum(): 先頭min(k,n)個の和。結果がSumに収まること。O(1)。
// kth(): 順序上のk番目を返す。k=0またはn<kならnullopt。O(1)。
// k()/size()/empty(): 選ぶ個数・全要素数・空判定。O(1)。
// clear(): kを保ちながら全要素を削除する。O(n)。swap(other): O(1)。
// T・Sumはbool以外の整数型。nは現在の要素数。領域O(n+1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <set>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOP_K_SUM_DEFINED
#define SIRKIT_TOP_K_SUM_DEFINED

template <std::integral T = int, bool Largest = false, std::integral Sum = T>
    requires(!std::same_as<T, bool> && !std::same_as<Sum, bool>)
class TopKSum
{
public:
    explicit TopKSum(std::size_t k = 0) : k_(k)
    {
    }

    TopKSum(const TopKSum &) = default;

    TopKSum(TopKSum &&other)
    {
        swap(other);
    }

    TopKSum &operator=(TopKSum other) noexcept
    {
        swap(other);
        return *this;
    }

    void insert(T value)
    {
        if (selected_.size() < k_)
        {
            selected_.insert(value);
            sum_bits_ += static_cast<SumBits>(value);
        }
        else if (!selected_.empty() && Order{}(value, *selected_.rbegin()))
        {
            selected_.insert(value);
            sum_bits_ += static_cast<SumBits>(value);
            ExcludeLast();
        }
        else
        {
            remaining_.insert(value);
        }
    }

    bool erase(T value)
    {
        const auto remaining = remaining_.find(value);
        if (remaining != remaining_.end())
        {
            remaining_.erase(remaining);
            return true;
        }
        const auto selected = selected_.find(value);
        if (selected == selected_.end())
        {
            return false;
        }
        sum_bits_ -= static_cast<SumBits>(*selected);
        selected_.erase(selected);
        if (!remaining_.empty())
        {
            IncludeFirst();
        }
        return true;
    }

    void set_k(std::size_t k)
    {
        k_ = k;
        while (selected_.size() > k_)
        {
            ExcludeLast();
        }
        while (selected_.size() < k_ && !remaining_.empty())
        {
            IncludeFirst();
        }
    }

    [[nodiscard]] Sum sum() const noexcept
    {
        return static_cast<Sum>(sum_bits_);
    }

    [[nodiscard]] std::optional<T> kth() const noexcept
    {
        if (k_ == 0 || selected_.size() < k_)
        {
            return std::nullopt;
        }
        return *selected_.rbegin();
    }

    [[nodiscard]] std::size_t k() const noexcept
    {
        return k_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return selected_.size() + remaining_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    void clear() noexcept
    {
        selected_.clear();
        remaining_.clear();
        sum_bits_ = 0;
    }

    void swap(TopKSum &other) noexcept
    {
        selected_.swap(other.selected_);
        remaining_.swap(other.remaining_);
        std::swap(k_, other.k_);
        std::swap(sum_bits_, other.sum_bits_);
    }

private:
    using Order = std::conditional_t<Largest, std::greater<T>, std::less<T>>;
    using SumBits = std::make_unsigned_t<Sum>;

    void ExcludeLast()
    {
        const auto position = std::prev(selected_.end());
        sum_bits_ -= static_cast<SumBits>(*position);
        remaining_.insert(remaining_.begin(), selected_.extract(position));
    }

    void IncludeFirst()
    {
        const auto position = remaining_.begin();
        sum_bits_ += static_cast<SumBits>(*position);
        selected_.insert(selected_.end(), remaining_.extract(position));
    }

    std::size_t k_ = 0;
    // 符号なしで保持し、境界入替えの途中でも符号付きのオーバーフローを起こさない。
    SumBits sum_bits_ = 0;
    std::multiset<T, Order> selected_, remaining_;
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
#include <vector>

int main()
{
    std::size_t n, m, k;
    verify_in >> n >> m >> k;
    std::vector<int> values(n);
    sirkit::TopKSum<int, false, std::int64_t> least(k);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> values[i];
        least.insert(values[i]);
        if (i >= m)
        {
            least.erase(values[i - m]);
            verify_out << ' ';
        }
        if (i + 1 >= m)
        {
            verify_out << least.sum();
        }
    }
    verify_out << '\n';
}
