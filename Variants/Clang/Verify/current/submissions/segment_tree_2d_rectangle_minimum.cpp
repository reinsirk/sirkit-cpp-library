#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://onlinejudge.u-aizu.ac.jp/problems/1068

// Library: DataStructure/Segment_Tree_2D.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Segment_Tree_2D.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/segment_tree_2d.md
//
// API一覧
// SegmentTree2D<T,Op>(height,width,identity,op): 単位元のH行W列で構築。O(HW+1)。
// SegmentTree2D<T,Op>(height,width,row_major,identity,op): 行を連結したデータから構築。O(HW+1)。
// opは結合的・可換、identityは単位元。演算と要素の処理をO(1)とする。
// set(row,col,value): 1点を代入する。O(log(H+1)log(W+1))。点は範囲内、演算と代入は非送出が前提。
// get(row,col): 指定点の値を返す。点は範囲内。O(1)。
// prod(top,left,bottom,right): [top,bottom)×[left,right)を集約。空ならidentity。O(log(H+1)log(W+1))。
// all_prod()/height()/width()/empty(): 全体の集約・行数・列数・要素がないか。O(1)。領域O(HW+1)。
// 入力検査有効時は形状不一致でinvalid_argument、矩形・点が範囲外でout_of_range。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SEGMENT_TREE_2D_DEFINED
#define SIRKIT_SEGMENT_TREE_2D_DEFINED

template <class T, class Op>
class SegmentTree2D
{
public:
    SegmentTree2D(std::size_t height, std::size_t width, T identity, Op op)
        : op_(std::move(op)), identity_(std::move(identity)), height_(height), width_(width)
    {
        if (height_ == 0 || width_ == 0)
        {
            return;
        }
        const auto capacity = values_.max_size();
        if (height_ > capacity / 2 || width_ > capacity / 2 || 2 * height_ > capacity / (2 * width_))
        {
            throw std::length_error("SegmentTree2D: grid too large");
        }
        stride_ = 2 * width_;
        values_.assign(2 * height_ * stride_, identity_);
    }

    SegmentTree2D(std::size_t height, std::size_t width, std::span<const T> row_major, T identity, Op op)
        : SegmentTree2D(height, width, std::move(identity), std::move(op))
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (row_major.size() != height_ * width_)
        {
            throw std::invalid_argument("SegmentTree2D: grid size mismatch");
        }
#endif
        if (empty())
        {
            return;
        }
        for (std::size_t row = 0; row < height_; ++row)
        {
            for (std::size_t col = 0; col < width_; ++col)
            {
                At(row + height_, col + width_) = row_major[row * width_ + col];
            }
            for (std::size_t col = width_; col-- > 1;)
            {
                MergeColumns(row + height_, col);
            }
        }
        for (std::size_t row = height_; row-- > 1;)
        {
            for (std::size_t col = 1; col < stride_; ++col)
            {
                MergeRows(row, col);
            }
        }
    }

    SegmentTree2D(const SegmentTree2D &) = default;

    SegmentTree2D(SegmentTree2D &&other) noexcept(
        std::is_nothrow_move_constructible_v<Op> && std::is_nothrow_move_constructible_v<T>)
        : op_(std::move(other.op_)), identity_(std::move(other.identity_)), height_(std::exchange(other.height_, 0)),
          width_(std::exchange(other.width_, 0)), stride_(std::exchange(other.stride_, 0)),
          values_(std::move(other.values_))
    {
    }

    SegmentTree2D &operator=(SegmentTree2D other) noexcept
        requires(std::is_nothrow_swappable_v<Op> && std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(op_, other.op_);
        swap(identity_, other.identity_);
        swap(height_, other.height_);
        swap(width_, other.width_);
        swap(stride_, other.stride_);
        values_.swap(other.values_);
        return *this;
    }

    void set(std::size_t row, std::size_t col, T value)
    {
        CheckPoint(row, col);
        row += height_;
        col += width_;
        At(row, col) = std::move(value);
        for (auto ancestor = col / 2; ancestor != 0; ancestor /= 2)
        {
            MergeColumns(row, ancestor);
        }
        for (row /= 2; row != 0; row /= 2)
        {
            for (auto ancestor = col; ancestor != 0; ancestor /= 2)
            {
                MergeRows(row, ancestor);
            }
        }
    }

    [[nodiscard]] T get(std::size_t row, std::size_t col) const
    {
        CheckPoint(row, col);
        return At(row + height_, col + width_);
    }

    [[nodiscard]] T prod(std::size_t top, std::size_t left, std::size_t bottom, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (top > bottom || left > right || bottom > height_ || right > width_)
        {
            throw std::out_of_range("SegmentTree2D: invalid rectangle");
        }
#endif
        T result = identity_;
        if (top == bottom || left == right)
        {
            return result;
        }
        left += width_;
        right += width_;
        for (top += height_, bottom += height_; top < bottom; top /= 2, bottom /= 2)
        {
            if ((top & 1U) != 0)
            {
                AppendRow(result, top++, left, right);
            }
            if ((bottom & 1U) != 0)
            {
                AppendRow(result, --bottom, left, right);
            }
        }
        return result;
    }

    [[nodiscard]] T all_prod() const
    {
        return empty() ? identity_ : At(1, 1);
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
        return values_.empty();
    }

private:
    [[nodiscard]] typename std::vector<T>::reference At(std::size_t row, std::size_t col) noexcept
    {
        return values_[row * stride_ + col];
    }

    [[nodiscard]] typename std::vector<T>::const_reference At(std::size_t row, std::size_t col) const noexcept
    {
        return values_[row * stride_ + col];
    }

    void MergeColumns(std::size_t row, std::size_t col)
    {
        const auto &self = std::as_const(*this);
        At(row, col) = std::invoke(self.op_, self.At(row, 2 * col), self.At(row, 2 * col + 1));
    }

    void MergeRows(std::size_t row, std::size_t col)
    {
        const auto &self = std::as_const(*this);
        At(row, col) = std::invoke(self.op_, self.At(2 * row, col), self.At(2 * row + 1, col));
    }

    void AppendRow(T &value, std::size_t row, std::size_t left, std::size_t right) const
    {
        for (; left < right; left /= 2, right /= 2)
        {
            if ((left & 1U) != 0)
            {
                value = std::invoke(op_, value, At(row, left++));
            }
            if ((right & 1U) != 0)
            {
                value = std::invoke(op_, value, At(row, --right));
            }
        }
    }

    void CheckPoint([[maybe_unused]] std::size_t row, [[maybe_unused]] std::size_t col) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (row >= height_ || col >= width_)
        {
            throw std::out_of_range("SegmentTree2D: point out of bounds");
        }
#endif
    }

    Op op_;
    T identity_;
    std::size_t height_, width_, stride_ = 0;
    std::vector<T> values_;
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
#include <limits>
#include <span>
#include <vector>

int main()
{
    constexpr int kIdentity = std::numeric_limits<int>::max();
    const auto minimum = [](int lhs, int rhs) noexcept
    {
        return std::min(lhs, rhs);
    };
    for (;;)
    {
        std::size_t h, w, q;
        verify_in >> h >> w >> q;
        if (h == 0)
        {
            break;
        }
        std::vector<int> grid(h * w);
        for (auto &value : grid)
        {
            verify_in >> value;
        }
        const sirkit::SegmentTree2D tree(h, w, std::span<const int>(grid), kIdentity, minimum);
        while (q-- != 0)
        {
            std::size_t top, left, bottom, right;
            verify_in >> top >> left >> bottom >> right;
            verify_out << tree.prod(top, left, bottom + 1, right + 1) << '\n';
        }
    }
}
