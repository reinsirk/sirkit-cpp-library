#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/palindromes_in_deque

// Library: String/Palindromes_In_Deque.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Palindromes_In_Deque.cpp
// 参考: Q. Wang, M. Yang, X. Zhu, Double-Ended Palindromic Trees in Linear Time
// (https://arxiv.org/abs/2210.02292) の surface recording。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromes_in_deque.md
//
// API一覧
// PalindromesInDeque(expected_pushes=0): 空の英小文字列を構築。追加回数の見込みを渡すと回文頂点を予約。
// push_front(c)/push_back(c): 英小文字を指定の端へ追加。償却O(1)。
// pop_front()/pop_back(): 指定の端から1文字を除く。O(1)。
// size()/empty(): 現在の長さ/空かどうか。O(1)。
// distinct_palindrome_count(): 空でない相異なる回文部分文字列数。O(1)。
// longest_prefix_length()/longest_suffix_length(): 最長回文接頭辞/接尾辞の長さ。空なら0。O(1)。
// 通常版では範囲外の文字をinvalid_argument、空からの削除をout_of_rangeで通知。
// 過去の回文頂点を再利用する。予約なしで追加回数Uに対しO(26U+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMES_IN_DEQUE_DEFINED
#define SIRKIT_PALINDROMES_IN_DEQUE_DEFINED

class PalindromesInDeque
{
    using Index = std::size_t;
    static constexpr Index kAlphabet = 26;
    static constexpr Index kOdd = 0;
    static constexpr Index kEmpty = 1;

    struct Node
    {
        // quick[c]は、この回文内で直前の文字がcとなる最長の真の回文接尾辞。
        std::array<Index, kAlphabet> quick{};
        Index length = 0;
        Index suffix_link = kEmpty;
        Index terminal_count = 0; // 各接頭辞の最長回文接尾辞として現れる回数。
        Index first_child = kOdd;
        Index next_sibling = kOdd;
        Index letter = 0;
    };

    struct Cell
    {
        Index letter;
        Index prefix = kEmpty;
        Index suffix = kEmpty;
    };

public:
    explicit PalindromesInDeque(std::size_t expected_pushes = 0) : nodes_(2)
    {
        if (expected_pushes > nodes_.max_size() - 2)
        {
            throw std::length_error("PalindromesInDeque: node reservation too large");
        }
        nodes_.reserve(expected_pushes + 2);
    }

    PalindromesInDeque(const PalindromesInDeque &) = default;

    PalindromesInDeque(PalindromesInDeque &&other) : PalindromesInDeque()
    {
        Swap(other);
    }

    PalindromesInDeque &operator=(PalindromesInDeque other)
    {
        Swap(other);
        return *this;
    }

    std::size_t size() const noexcept
    {
        return text_.size();
    }

    bool empty() const noexcept
    {
        return text_.empty();
    }

    std::size_t distinct_palindrome_count() const noexcept
    {
        return distinct_;
    }

    std::size_t longest_prefix_length() const noexcept
    {
        return empty() ? 0 : nodes_[text_.front().prefix].length;
    }

    std::size_t longest_suffix_length() const noexcept
    {
        return empty() ? 0 : nodes_[text_.back().suffix].length;
    }

    void push_front(char character)
    {
        Push(character, true);
    }

    void push_back(char character)
    {
        Push(character, false);
    }

    void pop_front()
    {
        Pop(true);
    }

    void pop_back()
    {
        Pop(false);
    }

private:
    std::vector<Node> nodes_;
    std::deque<Cell> text_;
    Index distinct_ = 0;

    void Swap(PalindromesInDeque &other) noexcept
    {
        nodes_.swap(other.nodes_);
        text_.swap(other.text_);
        std::swap(distinct_, other.distinct_);
    }

    Cell &At(Index offset, bool front)
    {
        return text_[front ? offset : size() - 1 - offset];
    }

    // 更新する端を先頭と見なし、その向きで回文区間の開始と終了を読む。
    Index &Starts(Index offset, bool front)
    {
        auto &cell = At(offset, front);
        return front ? cell.prefix : cell.suffix;
    }

    Index &Ends(Index offset, bool front)
    {
        auto &cell = At(offset, front);
        return front ? cell.suffix : cell.prefix;
    }

    Index Transition(Index parent, Index letter) const
    {
        auto child = nodes_[parent].first_child;
        while (child != kOdd && nodes_[child].letter != letter)
        {
            child = nodes_[child].next_sibling;
        }
        return child;
    }

    Index FindOrCreate(Index inner, Index letter, bool front)
    {
        const auto child = Transition(inner, letter);
        if (child != kOdd)
        {
            return child;
        }

        Node node;
        node.length = inner == kOdd ? 1 : nodes_[inner].length + 2;
        if (inner != kOdd)
        {
            node.suffix_link = Transition(nodes_[inner].quick[letter], letter);
        }
        const Node &suffix = nodes_[node.suffix_link];
        node.quick = suffix.quick;
        const auto following = suffix.length == 0 ? letter : At(suffix.length - 1, front).letter;
        node.quick[following] = node.suffix_link;
        node.letter = letter;
        node.next_sibling = nodes_[inner].first_child;

        const auto id = nodes_.size();
        nodes_.push_back(node);
        nodes_[inner].first_child = id;
        return id;
    }

    void Push(char character, bool front)
    {
        const auto letter = static_cast<Index>(character - 'a');
        auto inner = empty() ? kEmpty : Starts(0, front);
        const auto length = nodes_[inner].length;
        if (length == size() || At(length, front).letter != letter)
        {
            inner = nodes_[inner].quick[letter];
        }
        const auto id = FindOrCreate(inner, letter, front);
        if (front)
        {
            text_.push_front({letter});
        }
        else
        {
            text_.push_back({letter});
        }

        auto &node = nodes_[id];
        const auto &suffix = nodes_[node.suffix_link];
        const auto suffix_start = node.length - suffix.length;
        // 以前の境界回文が新しい回文の接尾辞になった場合、その開始位置の記録を消す。
        if (suffix.length != 0 && Starts(suffix_start, front) == node.suffix_link)
        {
            Starts(suffix_start, front) = kEmpty;
        }
        Starts(0, front) = id;
        Ends(node.length - 1, front) = id;
        // 最初の出現はその位置で最長の回文接尾辞になる。より長い回文があれば、
        // その接頭辞として同じ回文が先に現れてしまうため。
        if (node.terminal_count++ == 0)
        {
            ++distinct_;
        }
    }

    void Pop(bool front)
    {
        const auto id = Starts(0, front);
        auto &node = nodes_[id];
        const auto &suffix = nodes_[node.suffix_link];
        const auto suffix_start = node.length - suffix.length;

        // 消える境界回文の真の接尾辞が別の回文に覆われなければ、そこを新しい記録とする。
        Index restored = kEmpty;
        if (suffix.length != 0 && nodes_[Starts(suffix_start, front)].length < suffix.length)
        {
            restored = node.suffix_link;
            Starts(suffix_start, front) = restored;
        }
        Ends(node.length - 1, front) = restored;
        if (--node.terminal_count == 0)
        {
            --distinct_;
        }
        if (front)
        {
            text_.pop_front();
        }
        else
        {
            text_.pop_back();
        }
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

int main()
{
    std::size_t q{};
    verify_in >> q;
    sirkit::PalindromesInDeque palindromes(q);
    while (q-- != 0)
    {
        int type{};
        verify_in >> type;
        if (type < 2)
        {
            char letter{};
            verify_in >> letter;
            if (type == 0)
            {
                palindromes.push_front(letter);
            }
            else
            {
                palindromes.push_back(letter);
            }
        }
        else if (type == 2)
        {
            palindromes.pop_front();
        }
        else
        {
            palindromes.pop_back();
        }
        verify_out << palindromes.distinct_palindrome_count() << ' '
                   << palindromes.longest_prefix_length() << ' ' << palindromes.longest_suffix_length() << '\n';
    }
}
