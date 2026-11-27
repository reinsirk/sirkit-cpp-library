// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Prefix_Sum_ND.cpp
// 参考: https://nyaannyaan.github.io/library/data-structure-2d/2d-cumulative-sum.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_sum_nd.md
//
// API一覧
// CumulativeSum2D<T>(h,w) / CumulativeSum3D<T>(h,w,d): 各軸の長さを指定。
// CumulativeSumND<T,D>(shape): D次元版（D>=1）。Index=std::array<std::size_t,D>。
// add(point,value): 1点に加算。O(D)。
// build(): 加算がすべて済んでから一度だけ実行。各軸で累積和を取る。O(DM)。
// query(left,right): 点加算後の半開領域の和。O(2^D)。
// imos(left,right,value=1): 構築前に半開領域へ一括加算。O(2^D)。
// imos_get(point): 構築後の1点の値。O(D)。
// 2次元・3次元では座標を並べて渡せる。例 query(i1,j1,i2,j2)。
// 0始まり。点は各軸で0以上・長さ未満。領域は0<=left[d]<=right[d]<=shape()[d]。
// shape(): 各軸の長さ。O(1)。Mは各軸の長さ+1の積。初期化O(M+D)、領域O(M+D)。
// 長さ0の軸があれば初期化O(D)、build O(1)、保存領域O(D)。空領域の和はT{}。
// T{}は加法の単位元。加算・減算の中間結果をTで表現できること。
// 検査有効時は容量超過にstd::length_error、範囲外にstd::out_of_range、操作順違反にstd::logic_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_SUM_ND_DEFINED
#define SIRKIT_PREFIX_SUM_ND_DEFINED

template <class T, std::size_t Dimensions>
class CumulativeSumND
{
    static_assert(Dimensions > 0);

public:
    using Index = std::array<std::size_t, Dimensions>;

    explicit CumulativeSumND(const Index &shape) : shape_(shape)
    {
        if (std::ranges::find(shape_, std::size_t{0}) != shape_.end())
        {
            return;
        }
        std::size_t cells = 1;
        for (std::size_t axis = Dimensions; axis > 0; --axis)
        {
            const std::size_t length = shape_[axis - 1];
#ifndef SKIP_RUNTIME_CHECKS
            if (length >= data_.max_size() / cells)
            {
                throw std::length_error("CumulativeSumND: padded capacity");
            }
#endif
            strides_[axis - 1] = cells;
            cells *= length + 1;
        }
        data_.resize(cells);
    }

    explicit CumulativeSumND(std::size_t h, std::size_t w)
        requires(Dimensions == 2)
        : CumulativeSumND(Index{h, w})
    {
    }

    explicit CumulativeSumND(std::size_t h, std::size_t w, std::size_t d)
        requires(Dimensions == 3)
        : CumulativeSumND(Index{h, w, d})
    {
    }

    void add(const Index &point, const T &value)
    {
        check_build(false);
        T &cell = data_[point_index(point)];
        cell = cell + value;
    }

    void add(std::size_t i, std::size_t j, const T &value)
        requires(Dimensions == 2)
    {
        add({i, j}, value);
    }

    void add(std::size_t i, std::size_t j, std::size_t k, const T &value)
        requires(Dimensions == 3)
    {
        add({i, j, k}, value);
    }

    void build()
    {
        check_build(false);
        const std::size_t cells = data_.size();
        if (cells != 0)
        {
            for (std::size_t axis = 0; axis < Dimensions; ++axis)
            {
                const std::size_t step = strides_[axis];
                const std::size_t width = step * (shape_[axis] + 1);
                for (std::size_t start = 0; start < cells; start += width)
                {
                    for (std::size_t pos = start + step; pos < start + width; ++pos)
                    {
                        data_[pos] = data_[pos] + data_[pos - step];
                    }
                }
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        built_ = true;
#endif
    }

    T query(const Index &left, const Index &right) const
    {
        check_build(true);
        check_box(left, right);
        return data_.empty() ? T{} : box_sum<0>(left, right, 0);
    }

    T query(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2) const
        requires(Dimensions == 2)
    {
        return query({i1, j1}, {i2, j2});
    }

    T query(std::size_t i1, std::size_t j1, std::size_t k1,
            std::size_t i2, std::size_t j2, std::size_t k2) const
        requires(Dimensions == 3)
    {
        return query({i1, j1, k1}, {i2, j2, k2});
    }

    void imos(const Index &left, const Index &right, const T &value = T{1})
    {
        check_build(false);
        check_box(left, right);
        for (std::size_t axis = 0; axis < Dimensions; ++axis)
        {
            if (left[axis] == right[axis])
            {
                return;
            }
        }
        add_corners<0, false>(left, right, 0, value);
    }

    void imos(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2, const T &value = T{1})
        requires(Dimensions == 2)
    {
        imos({i1, j1}, {i2, j2}, value);
    }

    void imos(std::size_t i1, std::size_t j1, std::size_t k1,
              std::size_t i2, std::size_t j2, std::size_t k2, const T &value = T{1})
        requires(Dimensions == 3)
    {
        imos({i1, j1, k1}, {i2, j2, k2}, value);
    }

    T imos_get(const Index &point) const
    {
        check_build(true);
        return data_[point_index(point)];
    }

    T imos_get(std::size_t i, std::size_t j) const
        requires(Dimensions == 2)
    {
        return imos_get({i, j});
    }

    T imos_get(std::size_t i, std::size_t j, std::size_t k) const
        requires(Dimensions == 3)
    {
        return imos_get({i, j, k});
    }

    const Index &shape() const noexcept
    {
        return shape_;
    }

private:
    void check_build([[maybe_unused]] bool expected) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (built_ != expected)
        {
            throw std::logic_error("CumulativeSumND: operation order");
        }
#endif
    }

    void check_box([[maybe_unused]] const Index &left, [[maybe_unused]] const Index &right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        for (std::size_t axis = 0; axis < Dimensions; ++axis)
        {
            if (left[axis] > right[axis] || right[axis] > shape_[axis])
            {
                throw std::out_of_range("CumulativeSumND: range");
            }
        }
#endif
    }

    std::size_t point_index(const Index &point) const
    {
        std::size_t pos = 0;
        for (std::size_t axis = 0; axis < Dimensions; ++axis)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (point[axis] >= shape_[axis])
            {
                throw std::out_of_range("CumulativeSumND: point");
            }
#endif
            pos += (point[axis] + 1) * strides_[axis];
        }
        return pos;
    }

    template <std::size_t Axis, bool Subtract>
    void add_corners(const Index &left, const Index &right, std::size_t start, const T &value)
    {
        if constexpr (Axis == Dimensions)
        {
            T &cell = data_[start];
            if constexpr (Subtract)
            {
                cell = cell - value;
            }
            else
            {
                cell = cell + value;
            }
        }
        else
        {
            const std::size_t step = strides_[Axis];
            add_corners<Axis + 1, Subtract>(left, right, start + (left[Axis] + 1) * step, value);
            // 軸の末尾の差分は配列内のどの点にも影響しない。
            if (right[Axis] < shape_[Axis])
            {
                add_corners<Axis + 1, !Subtract>(left, right, start + (right[Axis] + 1) * step, value);
            }
        }
    }

    template <std::size_t Axis>
    T box_sum(const Index &left, const Index &right, std::size_t start) const
    {
        if constexpr (Axis == Dimensions)
        {
            return data_[start];
        }
        else
        {
            const std::size_t step = strides_[Axis];
            return box_sum<Axis + 1>(left, right, start + right[Axis] * step) -
                   box_sum<Axis + 1>(left, right, start + left[Axis] * step);
        }
    }

    Index shape_;
    Index strides_{};
    std::vector<T> data_;
#ifndef SKIP_RUNTIME_CHECKS
    bool built_ = false;
#endif
};

template <class T>
using CumulativeSum2D = CumulativeSumND<T, 2>;

template <class T>
using CumulativeSum3D = CumulativeSumND<T, 3>;

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
