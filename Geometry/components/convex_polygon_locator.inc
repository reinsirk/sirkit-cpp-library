#ifndef SIRKIT_GEO_CONVEX_POLYGON_LOCATOR_DEFINED
#define SIRKIT_GEO_CONVEX_POLYGON_LOCATOR_DEFINED
namespace geo {
// SIRKIT_API_BEGIN
// [geo::ConvexPolygonLocator<T,C>]
// ConvexPolygonLocator(vertices = {}): 周回順の凸多角形を受け取る。時計回り・反時計回り、辺上の中間点、連続した重複点を扱う。O(n+1)時間・領域。
// locate(p): Outside/Boundary/Insideで外部・境界・内部を返す。空・点・線分も扱う。O(log(n+2))。
// 座標型Tと計算型Cは符号付き整数。判定に使う座標差とその積がCに収まること。
// SIRKIT_API_END

template <std::signed_integral T = std::int64_t, class C = DefaultCalc<T>>
    requires(std::numeric_limits<C>::is_integer && std::numeric_limits<C>::is_signed)
class ConvexPolygonLocator
{
    using P = Point<T, C>;

public:
    explicit ConvexPolygonLocator(std::vector<P> vertices = {})
        : vertices_(std::move(vertices))
    {
        vertices_.erase(std::unique(vertices_.begin(), vertices_.end()), vertices_.end());
        if (vertices_.size() > 1 && vertices_.front() == vertices_.back())
        {
            vertices_.pop_back();
        }
        if (vertices_.size() < 2)
        {
            return;
        }

        auto pivot = std::min_element(vertices_.begin(), vertices_.end());
        std::rotate(vertices_.begin(), pivot, vertices_.end());
        const int direction = orientation(vertices_.front(), vertices_[1], vertices_.back());
        if (direction == 0)
        {
            const auto last = *std::max_element(vertices_.begin(), vertices_.end());
            vertices_.resize(2);
            vertices_[1] = last;
            return;
        }
        if (direction < 0)
        {
            std::reverse(vertices_.begin() + 1, vertices_.end());
        }
        std::size_t kept = 1;
        for (std::size_t i = 1; i < vertices_.size(); ++i)
        {
            const auto vertex = vertices_[i];
            while (kept >= 2 && orientation(vertices_[kept - 2], vertices_[kept - 1], vertex) == 0)
            {
                --kept;
            }
            vertices_[kept++] = vertex;
        }
        if (orientation(vertices_[kept - 2], vertices_[kept - 1], vertices_[0]) == 0)
        {
            --kept;
        }
        vertices_.resize(kept);
    }

    [[nodiscard]] Location locate(const P &point) const
    {
        const auto n = vertices_.size();
        if (n == 0)
        {
            return Location::Outside;
        }
        if (n == 1)
        {
            return point == vertices_[0] ? Location::Boundary : Location::Outside;
        }
        const auto &origin = vertices_[0];
        const int low = orientation(origin, vertices_[1], point);
        if (n == 2)
        {
            return low == 0 ? OnRaySegment(vertices_[1], point) : Location::Outside;
        }
        const int high = orientation(origin, vertices_.back(), point);
        if (low < 0 || high > 0)
        {
            return Location::Outside;
        }
        if (low == 0)
        {
            return OnRaySegment(vertices_[1], point);
        }
        if (high == 0)
        {
            return OnRaySegment(vertices_.back(), point);
        }

        std::size_t left = 1, right = n - 1;
        while (right - left > 1)
        {
            const std::size_t middle = left + (right - left) / 2;
            if (orientation(origin, vertices_[middle], point) >= 0)
            {
                left = middle;
            }
            else
            {
                right = middle;
            }
        }
        const int side = orientation(vertices_[left], vertices_[right], point);
        if (side < 0)
        {
            return Location::Outside;
        }
        return side == 0 ? Location::Boundary : Location::Inside;
    }

private:
    [[nodiscard]] Location OnRaySegment(const P &end, const P &point) const
    {
        return vertices_[0] <= point && point <= end ? Location::Boundary : Location::Outside;
    }

    std::vector<P> vertices_;
};
}
#endif
