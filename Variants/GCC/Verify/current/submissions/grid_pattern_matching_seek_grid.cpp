#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc391/tasks/abc391_b

// Library: String/Grid_Pattern_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Grid_Pattern_Matching.cpp
// 参考: R. S. Bird, Two dimensional pattern matching (1977).
// https://ora.ox.ac.uk/objects/uuid:8831b471-0180-41d7-b60e-f590cd428368
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_pattern_matching.md
//
// API一覧
// find_grid_pattern(text, pattern): 長方形の完全一致の左上座標 {行, 列} を0始まりの行優先順で返す。
// 各行はstd::string_viewへ変換できるバイト列。重なりも列挙する。高さまたは幅が0なら空配列を返す。
// 行の長さが不揃いなら検査有効時にstd::invalid_argument。
// 時間O(H+R+σ(HW+RC)+K)、追加領域O(RC+W+K)。盤面H×W、パターンR×C、一致数K、
// σはパターンに含まれる異なるバイト数（非空なら1..256）。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_PATTERN_MATCHING_DEFINED
#define SIRKIT_GRID_PATTERN_MATCHING_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class Grid>
concept GridPatternRows = requires(const Grid &grid, std::size_t index) {
    static_cast<std::size_t>(grid.size());
    std::string_view{grid[index]};
};

template <GridPatternRows Grid>
std::size_t grid_pattern_width(const Grid &grid)
{
    if (grid.size() == 0)
    {
        return 0;
    }
    const auto width = std::string_view{grid[0]}.size();
#ifndef SKIP_RUNTIME_CHECKS
    for (std::size_t row = 1; row < static_cast<std::size_t>(grid.size()); ++row)
    {
        if (std::string_view{grid[row]}.size() != width)
        {
            throw std::invalid_argument("find_grid_pattern: ragged grid");
        }
    }
#endif
    return width;
}

// 行の幅が共通なので、最長一致の状態番号をそのまま行の識別子にできる。
class GridPatternRowAutomaton
{
    struct Node
    {
        std::size_t first_child = 0;
        std::size_t next_sibling = 0;
        std::size_t failure = 0;
        unsigned char byte = 0;
    };

    std::array<std::size_t, 256> root_children_{};
    std::vector<Node> nodes_{1};

    [[nodiscard]] std::size_t child(std::size_t state, unsigned char byte) const
    {
        if (state == 0)
        {
            return root_children_[byte];
        }
        for (auto next = nodes_[state].first_child; next != 0; next = nodes_[next].next_sibling)
        {
            if (nodes_[next].byte == byte)
            {
                return next;
            }
        }
        return 0;
    }

public:
    std::size_t add(std::string_view row)
    {
        std::size_t state = 0;
        for (const unsigned char byte : row)
        {
            auto next = child(state, byte);
            if (next == 0)
            {
                next = nodes_.size();
                nodes_.push_back({0, nodes_[state].first_child, 0, byte});
                if (state == 0)
                {
                    root_children_[byte] = next;
                }
                else
                {
                    nodes_[state].first_child = next;
                }
            }
            state = next;
        }
        return state;
    }

    void build()
    {
        std::vector<std::size_t> order;
        order.reserve(nodes_.size() - 1);
        for (const auto state : root_children_)
        {
            if (state != 0)
            {
                order.push_back(state);
            }
        }
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const auto state = order[i];
            for (auto next = nodes_[state].first_child; next != 0; next = nodes_[next].next_sibling)
            {
                auto &node = nodes_[next];
                node.failure = step(nodes_[state].failure, node.byte);
                order.push_back(next);
            }
        }
    }

    [[nodiscard]] std::size_t step(std::size_t state, unsigned char byte) const
    {
        auto next = child(state, byte);
        while (next == 0 && state != 0)
        {
            state = nodes_[state].failure;
            next = child(state, byte);
        }
        return next;
    }
};
} // namespace detail

template <detail::GridPatternRows Text, detail::GridPatternRows Pattern>
[[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>> find_grid_pattern(const Text &text, const Pattern &pattern)
{
    const auto text_width = detail::grid_pattern_width(text);
    const auto pattern_width = detail::grid_pattern_width(pattern);
    const auto text_height = static_cast<std::size_t>(text.size());
    const auto pattern_height = static_cast<std::size_t>(pattern.size());
    if (pattern_width == 0 || pattern_height > text_height || pattern_width > text_width)
    {
        return {};
    }

    detail::GridPatternRowAutomaton automaton;
    std::vector<std::size_t> row_ids;
    row_ids.reserve(pattern_height);
    for (std::size_t y = 0; y < pattern_height; ++y)
    {
        row_ids.push_back(automaton.add(std::string_view{pattern[y]}));
    }
    automaton.build();

    std::vector<std::size_t> border(pattern_height);
    for (std::size_t i = 1; i < pattern_height; ++i)
    {
        auto matched = border[i - 1];
        while (matched != 0 && row_ids[matched] != row_ids[i])
        {
            matched = border[matched - 1];
        }
        border[i] = matched + (row_ids[matched] == row_ids[i]);
    }

    const auto first_end = pattern_width - 1;
    std::vector<std::size_t> column_matches(text_width - first_end);
    std::vector<std::pair<std::size_t, std::size_t>> result;
    for (std::size_t y = 0; y < text_height; ++y)
    {
        const auto &row = text[y];
        const std::string_view bytes{row};
        std::size_t state = 0;
        for (std::size_t x = 0; x < first_end; ++x)
        {
            state = automaton.step(state, bytes[x]);
        }
        for (std::size_t x = first_end; x < text_width; ++x)
        {
            state = automaton.step(state, bytes[x]);
            auto &matched = column_matches[x - first_end];
            while (matched != 0 && row_ids[matched] != state)
            {
                matched = border[matched - 1];
            }
            matched += (row_ids[matched] == state);
            if (matched == pattern_height)
            {
                result.emplace_back(y - (pattern_height - 1), x - first_end);
                matched = border[matched - 1];
            }
        }
    }
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
#include <vector>

int main()
{
    std::size_t text_size{}, pattern_size{};
    verify_in >> text_size >> pattern_size;
    std::vector<std::string> text(text_size), pattern(pattern_size);
    for (auto &row : text)
    {
        verify_in >> row;
    }
    for (auto &row : pattern)
    {
        verify_in >> row;
    }
    const auto matches = sirkit::find_grid_pattern(text, pattern);
    const auto [row, column] = matches.front();
    verify_out << row + 1 << ' ' << column + 1 << '\n';
}
