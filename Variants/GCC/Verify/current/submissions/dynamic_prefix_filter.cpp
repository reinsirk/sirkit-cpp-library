#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc403/tasks/abc403_e

// Library: String/Dynamic_Prefix_Filter.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Dynamic_Prefix_Filter.cpp
// 参考（禁止接頭辞の用途）: https://atcoder.jp/contests/abc403/editorial/12832
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_prefix_filter.md
//
// API一覧
// DynamicPrefixFilter(): 単語多重集合と禁止接頭辞集合を空で用意。O(1)。
// insert(word,copies=1): 追加後のwordの個数。erase(word,copies=1): 足りなければfalseで変更しない。
// count(word): wordの総個数。size(): 禁止にかかわらず登録した全個数。O(1)。
// block(prefix)/unblock(prefix): 禁止接頭辞に追加/解除。集合が変わればtrue。禁止中の単語も保持する。
// is_blocked(word): 禁止接頭辞を一つでも持つか。未登録の単語も照会可。
// allowed_count(): 禁止接頭辞を持たない登録単語の総個数。O(1)。
// 各文字列は任意のバイト列。空語も可、空接頭辞を禁止すると全ての単語が対象。
// 文字列操作は長さLに対しO(1+256L)、挿入時は確保を含め償却。領域は一度でも追加した接頭辞の種類数に比例。
// 総個数がSIZE_MAXを超える追加はlength_error。コピーは独立、ムーブ元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_PREFIX_FILTER_DEFINED
#define SIRKIT_DYNAMIC_PREFIX_FILTER_DEFINED

class DynamicPrefixFilter
{
    static constexpr auto kMissing = std::numeric_limits<std::size_t>::max();

    struct Node
    {
        std::size_t first_child = 0;
        std::size_t next_sibling = 0;
        std::size_t parent = 0;
        std::size_t words = 0;
        std::size_t allowed_children = 0;
        unsigned char byte = 0;
        bool blocked = false;

        [[nodiscard]] std::size_t allowed() const noexcept
        {
            return blocked ? 0 : words + allowed_children;
        }
    };

    std::vector<Node> nodes_;
    std::size_t size_ = 0;

    [[nodiscard]] std::size_t child(std::size_t at, unsigned char byte) const noexcept
    {
        for (auto next = nodes_[at].first_child; next != 0; next = nodes_[next].next_sibling)
        {
            if (nodes_[next].byte == byte)
            {
                return next;
            }
        }
        return 0;
    }

    [[nodiscard]] std::size_t find(std::string_view word) const noexcept
    {
        if (nodes_.empty())
        {
            return kMissing;
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            at = child(at, byte);
            if (at == 0)
            {
                return kMissing;
            }
        }
        return at;
    }

    std::size_t ensure(std::string_view word)
    {
        if (nodes_.empty())
        {
            nodes_.emplace_back();
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            auto next = child(at, byte);
            if (next == 0)
            {
                next = nodes_.size();
                nodes_.push_back(Node{.next_sibling = nodes_[at].first_child, .parent = at, .byte = byte});
                nodes_[at].first_child = next;
            }
            at = next;
        }
        return at;
    }

    // 禁止されたノードも直下の集計を保ち、解除時に子孫を走査せず復元する。
    void propagate(std::size_t at, std::size_t previous) noexcept
    {
        auto current = nodes_[at].allowed();
        while (at != 0 && current != previous)
        {
            at = nodes_[at].parent;
            auto &parent = nodes_[at];
            const auto parent_previous = parent.allowed();
            parent.allowed_children = parent.allowed_children - previous + current;
            previous = parent_previous;
            current = parent.allowed();
        }
    }

    bool set_blocked(std::size_t at, bool blocked) noexcept
    {
        auto &node = nodes_[at];
        if (node.blocked == blocked)
        {
            return false;
        }
        const auto previous = node.allowed();
        node.blocked = blocked;
        propagate(at, previous);
        return true;
    }

public:
    DynamicPrefixFilter() = default;
    DynamicPrefixFilter(const DynamicPrefixFilter &) = default;

    DynamicPrefixFilter(DynamicPrefixFilter &&other) noexcept
        : nodes_(std::move(other.nodes_)), size_(std::exchange(other.size_, 0))
    {
        other.nodes_.clear();
    }

    DynamicPrefixFilter &operator=(DynamicPrefixFilter other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(size_, other.size_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::size_t allowed_count() const noexcept
    {
        return nodes_.empty() ? 0 : nodes_[0].allowed();
    }

    [[nodiscard]] std::size_t count(std::string_view word) const noexcept
    {
        const auto at = find(word);
        return at == kMissing ? 0 : nodes_[at].words;
    }

    std::size_t insert(std::string_view word, std::size_t copies = 1)
    {
        if (copies == 0)
        {
            return count(word);
        }
        if (copies > std::numeric_limits<std::size_t>::max() - size_)
        {
            throw std::length_error("DynamicPrefixFilter: too many words");
        }
        const auto at = ensure(word);
        auto &node = nodes_[at];
        const auto previous = node.allowed();
        node.words += copies;
        size_ += copies;
        propagate(at, previous);
        return node.words;
    }

    bool erase(std::string_view word, std::size_t copies = 1) noexcept
    {
        if (copies == 0)
        {
            return true;
        }
        const auto at = find(word);
        if (at == kMissing || nodes_[at].words < copies)
        {
            return false;
        }
        auto &node = nodes_[at];
        const auto previous = node.allowed();
        node.words -= copies;
        size_ -= copies;
        propagate(at, previous);
        return true;
    }

    bool block(std::string_view prefix)
    {
        return set_blocked(ensure(prefix), true);
    }

    bool unblock(std::string_view prefix) noexcept
    {
        const auto at = find(prefix);
        return at != kMissing && set_blocked(at, false);
    }

    [[nodiscard]] bool is_blocked(std::string_view word) const noexcept
    {
        if (nodes_.empty())
        {
            return false;
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            if (nodes_[at].blocked)
            {
                return true;
            }
            at = child(at, byte);
            if (at == 0)
            {
                return false;
            }
        }
        return nodes_[at].blocked;
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
#include <string>

int main()
{
    std::size_t q{};
    verify_in >> q;
    sirkit::DynamicPrefixFilter filter;
    std::string word;
    for (std::size_t i = 0; i < q; ++i)
    {
        int type{};
        verify_in >> type >> word;
        if (type == 1)
        {
            filter.block(word);
        }
        else
        {
            filter.insert(word);
        }
        verify_out << filter.allowed_count() << '\n';
    }
}
