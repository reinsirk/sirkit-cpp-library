#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc331/tasks/abc331_f

// Library: String/Dynamic_Palindrome.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Dynamic_Palindrome.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_palindrome.md
//
// API一覧
// [DynamicPalindrome] 固定長のバイト列への点代入と、部分文字列の回文照会。添字は0始まり。
// DynamicPalindrome(text[,seed]): 内容をコピーする。seedを指定しないと乱数で基数を選ぶ。O(n+1)時間・領域。
// set(index,byte): 1バイトを代入。O(1+log(n+1))時間。get(index): バイトを取得。O(1)。
// is_palindrome(left,right): 半開区間[left,right)が回文か。空区間もtrue。O(1+log(n+1))時間。
// 2つの素数法のローリングハッシュを使う。衝突時は回文でない区間をtrueと判定する可能性がある。
// size(): バイト数。O(1)。コピーは独立。移動元は空。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_PALINDROME_DEFINED
#define SIRKIT_DYNAMIC_PALINDROME_DEFINED

namespace detail
{
inline std::size_t dynamic_palindrome_nodes(std::size_t length)
{
    if (length > std::numeric_limits<std::size_t>::max() / 2)
    {
        throw std::length_error("DynamicPalindrome: text too long");
    }
    return length + 1;
}

inline std::uint64_t dynamic_palindrome_seed()
{
    std::random_device device;
    const auto high = static_cast<std::uint64_t>(device());
    return (high << 32) ^ device();
}
} // namespace detail

class DynamicPalindrome
{
    static constexpr std::uint32_t kModulus1 = 1'000'000'007;
    static constexpr std::uint32_t kModulus2 = 1'000'000'009;

    struct Hash
    {
        std::uint32_t first = 0;
        std::uint32_t second = 0;

        [[nodiscard]] std::uint64_t packed() const noexcept
        {
            return (static_cast<std::uint64_t>(first) << 32) | second;
        }

        Hash operator+(Hash other) const noexcept
        {
            return {(first + other.first) % kModulus1, (second + other.second) % kModulus2};
        }

        Hash operator-(Hash other) const noexcept
        {
            return {(first + kModulus1 - other.first) % kModulus1,
                    (second + kModulus2 - other.second) % kModulus2};
        }

        Hash operator*(Hash other) const noexcept
        {
            return {static_cast<std::uint32_t>(static_cast<std::uint64_t>(first) * other.first % kModulus1),
                    static_cast<std::uint32_t>(static_cast<std::uint64_t>(second) * other.second % kModulus2)};
        }
    };

    struct Node
    {
        Hash forward;
        Hash reverse;

        void accumulate(Node other) noexcept
        {
            forward = forward + other.forward;
            reverse = reverse + other.reverse;
        }
    };

  public:
    explicit DynamicPalindrome(std::string_view text) : DynamicPalindrome(text, detail::dynamic_palindrome_seed())
    {
    }

    DynamicPalindrome(std::string_view text, std::uint64_t seed)
        : powers_(detail::dynamic_palindrome_nodes(text.size())), tree_(powers_.size()), text_(text)
    {
        std::mt19937_64 random(seed);
        const Hash base{std::uniform_int_distribution<std::uint32_t>(257, kModulus1 - 2)(random),
                        std::uniform_int_distribution<std::uint32_t>(257, kModulus2 - 2)(random)};
        powers_[0] = {1, 1};
        for (std::size_t index = 1; index < powers_.size(); ++index)
        {
            powers_[index] = powers_[index - 1] * base;
        }

        for (std::size_t index = 1; index <= size(); ++index)
        {
            tree_[index].accumulate(weighted(index - 1, static_cast<std::uint8_t>(text_[index - 1])));
            const auto parent = index + lowbit(index);
            if (parent <= size())
            {
                tree_[parent].accumulate(tree_[index]);
            }
        }
    }

    DynamicPalindrome(const DynamicPalindrome &) = default;
    DynamicPalindrome(DynamicPalindrome &&other) noexcept
    {
        exchange(other);
    }

    DynamicPalindrome &operator=(DynamicPalindrome other) noexcept
    {
        exchange(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return text_.size();
    }

    [[nodiscard]] std::uint8_t get(std::size_t index) const
    {
        return static_cast<std::uint8_t>(text_[index]);
    }

    void set(std::size_t index, std::uint8_t byte)
    {
        const auto previous = get(index);
        if (previous == byte)
        {
            return;
        }
        const auto old_value = weighted(index, previous);
        const auto new_value = weighted(index, byte);
        const Node delta{new_value.forward - old_value.forward, new_value.reverse - old_value.reverse};
        for (auto node = index + 1; node <= size(); node += lowbit(node))
        {
            tree_[node].accumulate(delta);
        }
        text_[index] = static_cast<char>(byte);
    }

    [[nodiscard]] bool is_palindrome(std::size_t left, std::size_t right) const
    {
        if (right - left < 2)
        {
            return true;
        }
        const auto from = prefix(left);
        const auto to = prefix(right);
        // 順方向と逆方向で、区間に対応する多項式の最小次数をそろえる。
        const auto forward = (to.forward - from.forward) * powers_[size() - right];
        const auto reverse = (to.reverse - from.reverse) * powers_[left];
        return forward.packed() == reverse.packed();
    }

  private:
    static std::size_t lowbit(std::size_t index) noexcept
    {
        return index & (std::size_t{0} - index);
    }

    [[nodiscard]] Node weighted(std::size_t position, std::uint8_t byte) const noexcept
    {
        const Hash value{byte, byte};
        return {value * powers_[position], value * powers_[size() - 1 - position]};
    }

    [[nodiscard]] Node prefix(std::size_t size) const noexcept
    {
        Node total;
        for (auto node = size; node != 0; node -= lowbit(node))
        {
            total.accumulate(tree_[node]);
        }
        return total;
    }


    void exchange(DynamicPalindrome &other) noexcept
    {
        powers_.swap(other.powers_);
        tree_.swap(other.tree_);
        text_.swap(other.text_);
    }

    std::vector<Hash> powers_;
    std::vector<Node> tree_;
    std::string text_;
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
#include <string>

int main()
{
    std::size_t length = 0, query_count = 0;
    std::string input;
    verify_in >> length >> query_count >> input;
    sirkit::DynamicPalindrome text(input);
    for (std::size_t i = 0; i < query_count; ++i)
    {
        int operation = 0;
        std::size_t left = 0;
        verify_in >> operation >> left;
        if (operation == 1)
        {
            std::string byte;
            verify_in >> byte;
            text.set(left - 1, static_cast<std::uint8_t>(byte[0]));
        }
        else
        {
            std::size_t right = 0;
            verify_in >> right;
            verify_out << (text.is_palindrome(left - 1, right) ? "Yes\n" : "No\n");
        }
    }
}
