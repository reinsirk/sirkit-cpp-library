#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc228/tasks/abc228_f

// Library: DataStructure/Sparse_Table_2D.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Sparse_Table_2D.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_table_2d.md
//
// API一覧
// SparseTable2D<T,Op>(grid,op): 長方形の行列から構築する。
// SparseTable2D<T,Op>(height,width,row_major,op): 行を順に連結したheight*width要素から構築する。
// Opはconstで呼べる結合的・可換・冪等な二項演算。演算とTのコピーはO(1)とする。
// H行W列の構築は空行を含めO(H+1+HW log(H+1) log(W+1))時間・O(1+HW log(H+1) log(W+1))領域。
// prod(top,left,bottom,right): [top,bottom) × [left,right) の集約。非空で範囲内。O(1)。
// height()/width()/empty(): 行数・列数・要素がないか。O(1)。
// 検査有効時は形状不一致でinvalid_argument、範囲外・空矩形の照会でout_of_range。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_TABLE_2D_DEFINED
#define SIRKIT_SPARSE_TABLE_2D_DEFINED

template <class T, class Op>
class SparseTable2D
{
public:
    explicit SparseTable2D(const std::vector<std::vector<T>> &grid, Op op)
        : op_(std::move(op)), height_(grid.size()), width_(grid.empty() ? 0 : grid.front().size())
    {
        std::vector<T> base;
        base.reserve(Area(height_, width_));
        for (const auto &row : grid)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (row.size() != width_)
            {
                throw std::invalid_argument("SparseTable2D: ragged grid");
            }
#endif
            base.insert(base.end(), row.begin(), row.end());
        }
        Build(std::move(base));
    }

    SparseTable2D(std::size_t height, std::size_t width, std::span<const T> row_major, Op op)
        : op_(std::move(op)), height_(height), width_(width)
    {
        const auto area = Area(height_, width_);
#ifndef SKIP_RUNTIME_CHECKS
        if (row_major.size() != area)
        {
            throw std::invalid_argument("SparseTable2D: grid size mismatch");
        }
#endif
        Build(std::vector<T>(row_major.begin(), row_major.first(area).end()));
    }

    [[nodiscard]] T prod(std::size_t top, std::size_t left, std::size_t bottom, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (top >= bottom || left >= right || bottom > height_ || right > width_ || levels_.empty())
        {
            throw std::out_of_range("SparseTable2D: invalid rectangle");
        }
#endif
        const auto r_level = static_cast<std::size_t>(std::bit_width(bottom - top) - 1);
        const auto c_level = static_cast<std::size_t>(std::bit_width(right - left) - 1);
        const auto last_row = bottom - (std::size_t{1} << r_level);
        const auto last_column = right - (std::size_t{1} << c_level);
        const auto &level = levels_[Index(r_level, c_level)];
        return std::invoke(op_, std::invoke(op_, level.at(top, left), level.at(top, last_column)),
                           std::invoke(op_, level.at(last_row, left), level.at(last_row, last_column)));
    }

    [[nodiscard]] std::size_t height() const noexcept
    {
        return height_;
    }

    [[nodiscard]] std::size_t width() const noexcept
    {
        return width_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return levels_.empty();
    }

private:
    struct Level
    {
        std::size_t columns = 0;
        std::vector<T> values;

        [[nodiscard]] typename std::vector<T>::const_reference at(std::size_t row, std::size_t column) const
        {
            return values[row * columns + column];
        }
    };

    [[nodiscard]] static std::size_t Area(std::size_t height, std::size_t width)
    {
        if (width != 0 && height > std::vector<T>().max_size() / width)
        {
            throw std::length_error("SparseTable2D: grid too large");
        }
        return height * width;
    }

    [[nodiscard]] std::size_t Index(std::size_t r_level, std::size_t c_level) const noexcept
    {
        return r_level * static_cast<std::size_t>(std::bit_width(width_)) + c_level;
    }

    void Build(std::vector<T> base)
    {
        if (height_ == 0 || width_ == 0)
        {
            return;
        }
        const auto r_levels = static_cast<std::size_t>(std::bit_width(height_));
        const auto c_levels = static_cast<std::size_t>(std::bit_width(width_));
        levels_.resize(r_levels * c_levels);
        levels_.front() = {width_, std::move(base)};
        for (std::size_t i = 0; i < r_levels; ++i)
        {
            for (std::size_t j = 0; j < c_levels; ++j)
            {
                if (i == 0 && j == 0)
                {
                    continue;
                }
                const auto rows = height_ - (std::size_t{1} << i) + 1;
                auto &level = levels_[Index(i, j)];
                level.columns = width_ - (std::size_t{1} << j) + 1;
                level.values.reserve(rows * level.columns);
                const std::size_t dr = j == 0 ? std::size_t{1} << (i - 1) : 0;
                const std::size_t dc = j == 0 ? 0 : std::size_t{1} << (j - 1);
                const auto &previous = levels_[j == 0 ? Index(i - 1, j) : Index(i, j - 1)];
                for (std::size_t row = 0; row < rows; ++row)
                {
                    for (std::size_t column = 0; column < level.columns; ++column)
                    {
                        level.values.push_back(std::invoke(op_, previous.at(row, column),
                                                           previous.at(row + dr, column + dc)));
                    }
                }
            }
        }
    }

    Op op_;
    std::size_t height_, width_;
    std::vector<Level> levels_;
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


#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

int main()
{
    std::size_t h, w, black_height, black_width, white_height, white_width;
    verify_in >> h >> w >> black_height >> black_width >> white_height >> white_width;
    white_height = std::min(white_height, black_height);
    white_width = std::min(white_width, black_width);
    const std::size_t stride = w + 1;
    std::vector<std::int64_t> prefix((h + 1) * stride);
    for (std::size_t row = 0; row < h; ++row)
    {
        std::int64_t partial = 0;
        for (std::size_t col = 0; col < w; ++col)
        {
            std::int64_t a;
            verify_in >> a;
            partial += a;
            prefix[(row + 1) * stride + col + 1] = prefix[row * stride + col + 1] + partial;
        }
    }
    const auto sum = [&](std::size_t i, std::size_t j, std::size_t height, std::size_t width)
    {
        const auto end = (i + height) * stride;
        const auto start = i * stride;
        return prefix[end + j + width] - prefix[end + j] - prefix[start + j + width] + prefix[start + j];
    };
    const std::size_t candidate_rows = h - white_height + 1, candidate_cols = w - white_width + 1;
    std::vector<std::int64_t> candidates;
    candidates.reserve(candidate_rows * candidate_cols);
    for (std::size_t row = 0; row < candidate_rows; ++row)
    {
        for (std::size_t col = 0; col < candidate_cols; ++col)
        {
            candidates.push_back(sum(row, col, white_height, white_width));
        }
    }
    const auto maximum = [](std::int64_t lhs, std::int64_t rhs)
    {
        return std::max(lhs, rhs);
    };
    const sirkit::SparseTable2D table(candidate_rows, candidate_cols, std::span<const std::int64_t>(candidates), maximum);
    const std::size_t query_height = black_height - white_height + 1;
    const std::size_t query_width = black_width - white_width + 1;
    std::int64_t answer = 0;
    for (std::size_t row = 0; row + black_height <= h; ++row)
    {
        for (std::size_t col = 0; col + black_width <= w; ++col)
        {
            const auto taken = table.prod(row, col, row + query_height, col + query_width);
            answer = std::max(answer, sum(row, col, black_height, black_width) - taken);
        }
    }
    verify_out << answer << '\n';
}
