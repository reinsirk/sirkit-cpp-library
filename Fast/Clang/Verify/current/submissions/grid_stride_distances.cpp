#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc170/tasks/abc170_f

// Library: Graph/Grid_Stride_Distances.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Grid_Stride_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_stride_distances.md
//
// API一覧
// grid_stride_distances(grid,start,max_stride,wall='#'): 始点から各マスへ最少何回で移動できるか。
// grid_stride_distance(grid,start,target,max_stride,wall='#'): 指定した1マスへの最少回数。
//   座標は0始まりの(行,列)。上下左右へ1..max_strideマス直進し、壁を通らない。
//   壁または到達不能の距離は-1。全距離の添字はrow*width+column。
//   O(HW α(HW))時間、O(HW)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_STRIDE_LAYOUT_DEFINED
#define SIRKIT_GRID_STRIDE_LAYOUT_DEFINED

namespace detail
{
inline int grid_stride_area(std::size_t height, std::size_t width)
{
    if (height == 0 || width == 0)
    {
        throw std::invalid_argument("grid stride: empty grid");
    }
    constexpr auto kMax = static_cast<std::size_t>(std::numeric_limits<int>::max() - 1);
    if (height > kMax / width)
    {
        throw std::length_error("grid stride: too many cells");
    }
    return static_cast<int>(height * width);
}

struct StrideGrid
{
    std::span<const std::string> rows;
    int height, width, area;
    char wall;

    [[nodiscard]] int id(std::pair<int, int> point) const
    {
        const auto [r, c] = point;
        return r * width + c;
    }
};

inline StrideGrid make_stride_grid(std::span<const std::string> rows, char wall)
{
    const std::size_t width = rows.empty() ? 0 : rows.front().size();
    const int area = grid_stride_area(rows.size(), width);
    if (std::ranges::any_of(rows, [width](const std::string &row)
                            {
                                return row.size() != width;
                            }))
    {
        throw std::invalid_argument("grid stride: nonrectangular grid");
    }
    return {rows, static_cast<int>(rows.size()), static_cast<int>(width), area, wall};
}
} // namespace detail

#endif

#ifndef SIRKIT_GRID_STRIDE_DISTANCES_DEFINED
#define SIRKIT_GRID_STRIDE_DISTANCES_DEFINED

namespace detail
{
class GridStrideSuccessors
{
public:
    explicit GridStrideSuccessors(int size) : parent_(size + 1, -1), largest_(size + 1)
    {
        std::iota(largest_.begin(), largest_.end(), 0);
    }

    int next(int position)
    {
        return largest_[root(position)];
    }

    void erase(int position)
    {
        int a = root(position);
        int b = root(position + 1);
        if (parent_[a] > parent_[b])
        {
            std::swap(a, b);
        }
        parent_[a] += parent_[b];
        parent_[b] = a;
        largest_[a] = std::max(largest_[a], largest_[b]);
    }

private:
    int root(int position)
    {
        int leader = position;
        while (parent_[leader] >= 0)
        {
            leader = parent_[leader];
        }
        while (position != leader)
        {
            const int next = parent_[position];
            parent_[position] = leader;
            position = next;
        }
        return leader;
    }

    std::vector<int> parent_;
    std::vector<int> largest_;
};

template <class IsOpen, class FlatIndex>
std::vector<std::pair<int, int>> grid_stride_intervals(int lines, int length, IsOpen is_open, FlatIndex index)
{
    std::vector<std::pair<int, int>> interval(static_cast<std::size_t>(lines) * length);
    for (int line = 0; line < lines; ++line)
    {
        for (int first = 0; first < length;)
        {
            if (!is_open(line, first))
            {
                ++first;
                continue;
            }
            int last = first + 1;
            while (last < length && is_open(line, last))
            {
                ++last;
            }
            for (int step = first; step < last; ++step)
            {
                interval[index(line, step)] = {first, last - 1};
            }
            first = last;
        }
    }
    return interval;
}

inline std::vector<int> grid_stride_search(const StrideGrid &grid, int source, int target, std::uint64_t max_stride)
{
    const auto [rows, height, width, area, wall] = grid;
    const auto is_open = [&](int r, int c)
    {
        return rows[r][c] != wall;
    };
    const auto blocked = [&](int id)
    {
        return !is_open(id / width, id % width);
    };
    std::vector<int> distance(area, -1);
    if (blocked(source) || (target >= 0 && blocked(target)))
    {
        return distance;
    }
    distance[source] = 0;
    if (source == target || max_stride == 0)
    {
        return distance;
    }

    const auto horizontal = grid_stride_intervals(height, width, is_open,
                                                  [width](int r, int c)
                                                  {
                                                      return r * width + c;
                                                  });
    const auto vertical = grid_stride_intervals(width, height, [&](int c, int r)
                                                {
                                                    return is_open(r, c);
                                                },
                                                [width](int c, int r)
                                                {
                                                    return r * width + c;
                                                });
    const int move_x = static_cast<int>(std::min<std::uint64_t>(max_stride, width - 1));
    const int move_y = static_cast<int>(std::min<std::uint64_t>(max_stride, height - 1));

    GridStrideSuccessors by_row(area), by_column(area);
    std::vector<int> queue;
    queue.reserve(area);
    auto discover = [&](int cell, int d)
    {
        distance[cell] = d;
        queue.push_back(cell);
        by_row.erase(cell);
        by_column.erase((cell % width) * height + cell / width);
    };
    discover(source, 0);
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const int u = queue[i];
        if (u == target)
        {
            break;
        }
        const int r = u / width;
        const int c = u % width;
        const int after = distance[u] + 1;
        const auto [left_wall, right_wall] = horizontal[u];
        const auto [top_wall, bottom_wall] = vertical[u];
        const int left = std::max(left_wall, c - move_x);
        const int right = std::min(right_wall, c + std::min(move_x, width - 1 - c));
        const int top = std::max(top_wall, r - move_y);
        const int bottom = std::min(bottom_wall, r + std::min(move_y, height - 1 - r));
        const int row_start = r * width;
        const int column_start = c * height;
        for (int flat = by_row.next(row_start + left); flat <= row_start + right; flat = by_row.next(flat))
        {
            discover(flat, after);
        }
        for (int flat = by_column.next(column_start + top); flat <= column_start + bottom; flat = by_column.next(flat))
        {
            discover((flat - column_start) * width + c, after);
        }
    }
    return distance;
}
} // namespace detail

inline std::vector<int> grid_stride_distances(std::span<const std::string> grid, std::pair<int, int> start,
                                              std::uint64_t max_stride, char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    return detail::grid_stride_search(view, view.id(start), -1, max_stride);
}

inline int grid_stride_distance(std::span<const std::string> grid, std::pair<int, int> start,
                                std::pair<int, int> target, std::uint64_t max_stride, char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    const int goal = view.id(target);
    return detail::grid_stride_search(view, view.id(start), goal, max_stride)[goal];
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


#include <cstdint>
#include <string>
#include <vector>

int main()
{
    int h{}, w{}, x1{}, y1{}, x2{}, y2{};
    std::uint64_t k{};
    verify_in >> h >> w >> k >> x1 >> y1 >> x2 >> y2;
    static_cast<void>(w);
    std::vector<std::string> grid(h);
    for (auto &row : grid)
    {
        verify_in >> row;
    }
    verify_out << sirkit::grid_stride_distance(grid, {x1 - 1, y1 - 1}, {x2 - 1, y2 - 1}, k, '@') << '\n';
}
