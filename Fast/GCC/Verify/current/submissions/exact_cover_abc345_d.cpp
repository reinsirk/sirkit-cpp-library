#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc345/tasks/abc345_d

// Library: Algorithm/Exact_Cover.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Exact_Cover.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exact_cover.md
// 参考（Algorithm X / Dancing Links）: https://arxiv.org/abs/cs/0011047
//
// API一覧
// ExactCover(required, optional=0): 必須列[0,required)はちょうど1回、任意列は高々1回覆う。
// add_row(columns): 必須列を1つ以上含む行を追加して、0始まりの行番号を返す。O(k log k)時間。
// solve(): 解を構成する行番号を1組返す。解がなければnullopt。
// for_each_solution(visit): 解ごとに行番号のspanを渡す。visitがfalseを返すと打ち切る。
// 列数をC、全行に含まれる列の延べ数をMとする。探索準備はO(C+M)、使用領域もO(C+M)。探索は最悪指数時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXACT_COVER_DEFINED
#define SIRKIT_EXACT_COVER_DEFINED

namespace detail
{
class ExactCoverSearch
{
    struct Node
    {
        std::size_t left = 0;
        std::size_t right = 0;
        std::size_t up = 0;
        std::size_t down = 0;
        std::size_t column = 0;
        std::size_t row = 0;
    };

public:
    ExactCoverSearch(std::size_t required, std::size_t columns, const std::vector<std::vector<std::size_t>> &rows)
        : nodes_(columns + 1), column_sizes_(columns + 1)
    {
        nodes_[0].left = required;
        nodes_[0].right = required == 0 ? 0 : 1;
        chosen_.reserve(required);
        for (std::size_t column = 1; column <= columns; ++column)
        {
            auto &node = nodes_[column];
            node.up = node.down = column;
            if (column <= required)
            {
                node.left = column - 1;
                node.right = column == required ? 0 : column + 1;
            }
            else
            {
                node.left = node.right = column;
            }
        }
        for (std::size_t row = 0; row < rows.size(); ++row)
        {
            const auto first = nodes_.size();
            const auto last = first + rows[row].size() - 1;
            for (const auto index : rows[row])
            {
                const auto column = index + 1;
                const auto id = nodes_.size();
                const auto up = nodes_[column].up;
                nodes_.push_back({id == first ? last : id - 1, id == last ? first : id + 1, up, column, column, row});
                nodes_[up].down = nodes_[column].up = id;
                ++column_sizes_[column];
            }
        }
    }

    template <class Visitor>
    bool Run(Visitor &visit)
    {
        const auto first = nodes_[0].right;
        if (first == 0)
        {
            return visit(std::span<const std::size_t>(chosen_));
        }
        auto column = first;
        for (auto next = nodes_[first].right; next != 0; next = nodes_[next].right)
        {
            if (column_sizes_[next] < column_sizes_[column])
            {
                column = next;
            }
        }
        Cover(column);
        for (auto row = nodes_[column].down; row != column; row = nodes_[row].down)
        {
            chosen_.push_back(nodes_[row].row);
            for (auto next = nodes_[row].right; next != row; next = nodes_[next].right)
            {
                Cover(nodes_[next].column);
            }
            if (!Run(visit))
            {
                return false;
            }
            for (auto next = nodes_[row].left; next != row; next = nodes_[next].left)
            {
                Uncover(nodes_[next].column);
            }
            chosen_.pop_back();
        }
        Uncover(column);
        return true;
    }

private:
    void Cover(std::size_t column)
    {
        const auto &header = nodes_[column];
        nodes_[header.left].right = header.right;
        nodes_[header.right].left = header.left;
        for (auto row = header.down; row != column; row = nodes_[row].down)
        {
            for (auto next = nodes_[row].right; next != row; next = nodes_[next].right)
            {
                const auto &node = nodes_[next];
                nodes_[node.up].down = node.down;
                nodes_[node.down].up = node.up;
                --column_sizes_[node.column];
            }
        }
    }

    void Uncover(std::size_t column)
    {
        const auto &header = nodes_[column];
        for (auto row = header.up; row != column; row = nodes_[row].up)
        {
            for (auto next = nodes_[row].left; next != row; next = nodes_[next].left)
            {
                const auto &node = nodes_[next];
                nodes_[node.up].down = next;
                nodes_[node.down].up = next;
                ++column_sizes_[node.column];
            }
        }
        nodes_[header.left].right = column;
        nodes_[header.right].left = column;
    }

    std::vector<Node> nodes_;
    std::vector<std::size_t> column_sizes_;
    std::vector<std::size_t> chosen_;
};
} // namespace detail

class ExactCover
{
public:
    explicit ExactCover(std::size_t required, std::size_t optional = 0)
        : required_(required)
    {
        constexpr auto kLimit = std::numeric_limits<std::size_t>::max();
        if (required >= kLimit || optional > kLimit - required - 1)
        {
            throw std::length_error("ExactCover: column count overflows");
        }
        columns_ = required + optional;
    }

    std::size_t add_row(std::span<const std::size_t> columns)
    {
        std::vector<std::size_t> ordered(columns.begin(), columns.end());
        std::ranges::sort(ordered);
        if (ordered.empty() || ordered.front() >= required_ || ordered.back() >= columns_ ||
            std::ranges::adjacent_find(ordered) != ordered.end())
        {
            throw std::invalid_argument("ExactCover: invalid row columns");
        }
        rows_.push_back(std::move(ordered));
        return rows_.size() - 1;
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> solve() const
    {
        std::optional<std::vector<std::size_t>> answer;
        for_each_solution([&](std::span<const std::size_t> rows)
                          {
                              answer.emplace(rows.begin(), rows.end());
                              return false;
                          });
        return answer;
    }

    template <class Visitor>
    void for_each_solution(Visitor &&visit) const
    {
        detail::ExactCoverSearch(required_, columns_, rows_).Run(visit);
    }

private:
    std::size_t required_;
    std::size_t columns_ = 0;
    std::vector<std::vector<std::size_t>> rows_;
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
#include <utility>
#include <vector>

int main()
{
    std::size_t tile_count{}, height{}, width{};
    verify_in >> tile_count >> height >> width;
    sirkit::ExactCover cover(height * width, tile_count);
    for (std::size_t tile = 0; tile < tile_count; ++tile)
    {
        std::size_t h{}, w{};
        verify_in >> h >> w;
        for (int orientation = 0; orientation < (h == w ? 1 : 2); ++orientation)
        {
            for (std::size_t top = 0; top + h <= height; ++top)
            {
                for (std::size_t left = 0; left + w <= width; ++left)
                {
                    std::vector<std::size_t> columns;
                    columns.reserve(h * w + 1);
                    for (auto row = top; row < top + h; ++row)
                    {
                        for (auto column = left; column < left + w; ++column)
                        {
                            columns.push_back(row * width + column);
                        }
                    }
                    columns.push_back(height * width + tile);
                    cover.add_row(columns);
                }
            }
            std::swap(h, w);
        }
    }
    verify_out << (cover.solve() ? "Yes\n" : "No\n");
}
