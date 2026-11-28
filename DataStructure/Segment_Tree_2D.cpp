// SPDX-License-Identifier: CC0-1.0
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
