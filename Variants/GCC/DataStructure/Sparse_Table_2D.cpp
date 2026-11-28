// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
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
