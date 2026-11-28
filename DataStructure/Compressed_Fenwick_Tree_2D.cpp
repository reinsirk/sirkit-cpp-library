// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/compressed_fenwick_tree_2d.md
//
// API一覧
// CompressedFenwickTree2D<T=int64_t>(coordinates): 加算する点を登録して零で初期化。座標はint64_tのpair。
// size()/empty(): 登録した異なる座標の数・空判定、O(1)。
// point_id(point): 座標からIDを得る。未登録ならnullopt。IDは座標辞書順の0始まり。O(log(n+1))。
// add(id,delta): 登録点を加算。add(point,delta): 座標で加算し、未登録ならfalse。
// prefix_sum(x,y): X<xかつY<yの重み和。sum(l,b,r,t): [l,r)×[b,t)の重み和。
// 構築O(n log(n+1)+1)時間・領域。更新と照会はO(log²(n+1)+1)。nは構築前の座標数。
// 整数の和は重み型Tのビット幅を法とする。Tはbool以外の64ビット以下の整数型。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPRESSED_FENWICK_TREE_TWO_D_DEFINED
#define SIRKIT_COMPRESSED_FENWICK_TREE_TWO_D_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class CompressedFenwickTree2D
{
    using Storage = std::uint64_t;

public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    explicit CompressedFenwickTree2D(std::span<const Point> coordinates = {})
        : points_(coordinates.begin(), coordinates.end())
    {
        std::sort(points_.begin(), points_.end());
        points_.erase(std::unique(points_.begin(), points_.end()), points_.end());
        // points_ is ordered by (x,y), so x ranks can be assigned in one pass.
        x_ranks_.reserve(points_.size());
        for (const auto &[x, y] : points_)
        {
            if (xs_.empty() || xs_.back() != x)
            {
                xs_.push_back(x);
            }
            x_ranks_.push_back(xs_.size());
        }

        std::vector<std::size_t> y_order(points_.size());
        std::iota(y_order.begin(), y_order.end(), std::size_t{0});
        std::sort(y_order.begin(), y_order.end(),
                  [&](std::size_t lhs, std::size_t rhs)
                  {
                      return points_[lhs].second < points_[rhs].second;
                  });

        std::vector<std::vector<std::int64_t>> rows(xs_.size() + 1);
        for (std::size_t id : y_order)
        {
            const auto y = points_[id].second;
            for (std::size_t index = x_ranks_[id]; index <= xs_.size(); index += Lowbit(index))
            {
                if (rows[index].empty() || rows[index].back() != y)
                {
                    rows[index].push_back(y);
                }
            }
        }

        offsets_.reserve(xs_.size() + 2);
        std::size_t total = 0;
        for (const auto &row : rows)
        {
            offsets_.push_back(total);
            total += row.size();
        }
        offsets_.push_back(total);
        ys_.reserve(total);
        for (const auto &row : rows)
        {
            ys_.insert(ys_.end(), row.begin(), row.end());
        }
        values_.resize(total);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return points_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return points_.empty();
    }

    [[nodiscard]] std::optional<std::size_t> point_id(Point point) const
    {
        auto it = std::lower_bound(points_.begin(), points_.end(), point);
        if (it == points_.end() || *it != point)
        {
            return std::nullopt;
        }
        return static_cast<std::size_t>(it - points_.begin());
    }

    void add(std::size_t id, T delta)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= size())
        {
            throw std::out_of_range("CompressedFenwickTree2D: point ID");
        }
#endif
        const auto y = points_[id].second;
        const auto increment = static_cast<Storage>(delta);
        for (std::size_t index = x_ranks_[id]; index <= xs_.size(); index += Lowbit(index))
        {
            const auto offset = offsets_[index];
            const auto width = offsets_[index + 1] - offset;
            for (std::size_t j = RowRank(index, y) + 1; j <= width; j += Lowbit(j))
            {
                values_[offset + j - 1] += increment;
            }
        }
    }

    bool add(Point point, T delta)
    {
        if (const auto id = point_id(point))
        {
            add(*id, delta);
            return true;
        }
        return false;
    }

    [[nodiscard]] T prefix_sum(std::int64_t x, std::int64_t y) const
    {
        Storage answer = 0;
        for (std::size_t index = XRank(x); index != 0; index -= Lowbit(index))
        {
            answer += RowPrefix(offsets_[index], RowRank(index, y));
        }
        return static_cast<T>(answer);
    }

    [[nodiscard]] T sum(std::int64_t left, std::int64_t bottom, std::int64_t right, std::int64_t top) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || bottom > top)
        {
            throw std::invalid_argument("CompressedFenwickTree2D: reversed rectangle");
        }
#endif
        if (bottom == top)
        {
            return T{};
        }
        Storage answer = 0;
        std::size_t lo = XRank(left), hi = XRank(right);
        for (; lo != hi;)
        {
            if (lo < hi)
            {
                answer += RowSum(hi, bottom, top);
                hi -= Lowbit(hi);
            }
            else
            {
                answer -= RowSum(lo, bottom, top);
                lo -= Lowbit(lo);
            }
        }
        return static_cast<T>(answer);
    }

private:
    static std::size_t Lowbit(std::size_t index) noexcept
    {
        return index & (~index + 1);
    }

    [[nodiscard]] std::size_t XRank(std::int64_t x) const
    {
        return static_cast<std::size_t>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
    }

    [[nodiscard]] std::size_t RowRank(std::size_t index, std::int64_t y) const
    {
        const auto first = ys_.begin() + offsets_[index];
        const auto last = ys_.begin() + offsets_[index + 1];
        return static_cast<std::size_t>(std::lower_bound(first, last, y) - first);
    }

    [[nodiscard]] Storage RowPrefix(std::size_t offset, std::size_t count) const
    {
        Storage sum = 0;
        for (; count != 0; count -= Lowbit(count))
        {
            sum += values_[offset + count - 1];
        }
        return sum;
    }

    [[nodiscard]] Storage RowSum(std::size_t index, std::int64_t bottom, std::int64_t top) const
    {
        const auto offset = offsets_[index];
        return RowPrefix(offset, RowRank(index, top)) - RowPrefix(offset, RowRank(index, bottom));
    }

    std::vector<Point> points_;
    std::vector<std::size_t> x_ranks_, offsets_;
    std::vector<std::int64_t> xs_, ys_;
    std::vector<Storage> values_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
