// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_affine_range_sum.md
//
// API一覧
// SparseAffineRangeSum<Mod>(n=0,initial=0): 長さnをinitialで初期化。O(1)、nはuint64_t。
// range_affine(l,r,a,b): 半開区間[l,r)の各値をa*x+bに変換。償却O(log(n+1))。
// range_sum(l,r): 半開区間の和の非負剰余。空区間は0。O(log(n+1))。照会時に節点を確保しない。
// size()/all_sum()/stored_nodes(): 長さ・全体和・確保した節点数。各O(1)。
// 重みと係数はint64_tで受け取り縮約。Modは正のuint32_t、既定998244353。
// U回の更新後の領域はO(1+U log(n+1))。未分割区間は一様値だけで保持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_AFFINE_RANGE_SUM_DEFINED
#define SIRKIT_SPARSE_AFFINE_RANGE_SUM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t NextSparseAffineNodeCapacity(std::size_t used, std::size_t capacity, std::size_t growth,
                                                std::size_t limit)
{
    if (used > limit || growth > limit - used)
    {
        throw std::length_error("SparseAffineRangeSum: too many nodes");
    }
    const auto need = used + growth;
    if (need <= capacity)
    {
        return capacity;
    }
    const auto doubled = capacity <= limit / 2 ? 2 * capacity : limit;
    return std::max(need, doubled);
}
}

template <std::uint32_t Mod = 998244353>
class SparseAffineRangeSum
{
    static_assert(Mod > 0);

public:
    explicit SparseAffineRangeSum(std::uint64_t n = 0, std::int64_t initial = 0)
        : length_(n), root_(Normalize(initial))
    {
    }

    SparseAffineRangeSum(const SparseAffineRangeSum &) = default;

    SparseAffineRangeSum &operator=(const SparseAffineRangeSum &other)
    {
        if (this != &other)
        {
            SparseAffineRangeSum copy(other);
            Swap(copy);
        }
        return *this;
    }

    SparseAffineRangeSum(SparseAffineRangeSum &&other) noexcept
        : nodes_(std::move(other.nodes_)), length_(std::exchange(other.length_, 0)),
          root_(std::exchange(other.root_, 0))
    {
        other.nodes_.clear();
    }

    SparseAffineRangeSum &operator=(SparseAffineRangeSum &&other) noexcept
    {
        if (this != &other)
        {
            SparseAffineRangeSum copy(std::move(other));
            Swap(copy);
        }
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t stored_nodes() const noexcept
    {
        return nodes_.size();
    }

    [[nodiscard]] std::uint32_t all_sum() const noexcept
    {
        return CurrentSum(root_, length_);
    }

    void range_affine(std::uint64_t left, std::uint64_t right, std::int64_t a, std::int64_t b)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }
        const Affine change{Normalize(a), Normalize(b)};
        if (change.identity())
        {
            return;
        }
        if (left == 0 && right == length_)
        {
            root_ = Apply(root_, length_, change);
            return;
        }

        // 1層の未確定境界は二つ以下。対数個の新規節点を先に確保する。
        ReserveForUpdate();
        root_ = Update(root_, 0, length_, left, right, change);
    }

    [[nodiscard]] std::uint32_t range_sum(std::uint64_t left, std::uint64_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return 0;
        }
        return Query(root_, 0, length_, left, right, {});
    }

private:
    static constexpr auto kOne = std::uint32_t{1} % Mod;
    using Handle = std::conditional_t<Mod <= (std::uint64_t{1} << 31), std::uint32_t, std::uint64_t>;
    static constexpr Handle kNodeBit = Handle{1} << (std::numeric_limits<Handle>::digits - 1);

    struct Affine
    {
        std::uint32_t a = kOne, b = 0;

        bool identity() const noexcept
        {
            return a == kOne && b == 0;
        }
    };

    struct Node
    {
        Handle left, right;
        std::uint32_t sum;
        Affine pending{};
    };

    std::vector<Node> nodes_;
    std::uint64_t length_;
    Handle root_;

    void Swap(SparseAffineRangeSum &other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(length_, other.length_);
        std::swap(root_, other.root_);
    }

    void CheckRange(std::uint64_t left, std::uint64_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > length_)
        {
            throw std::out_of_range("SparseAffineRangeSum: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    void ReserveForUpdate()
    {
        const auto extra = static_cast<std::size_t>(2 * std::bit_width(length_));
        const auto limit = static_cast<std::size_t>(std::min<std::uint64_t>(kNodeBit, nodes_.max_size()));
        nodes_.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NextSparseAffineNodeCapacity(
            nodes_.size(), nodes_.capacity(), extra, limit));
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto reduced = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(reduced < 0 ? reduced + Mod : reduced);
    }

    static std::uint32_t Add(std::uint32_t left, std::uint32_t right) noexcept
    {
        const auto sum = std::uint64_t{left} + right;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b) noexcept
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }

    static std::uint32_t Scale(std::uint32_t value, std::uint64_t length) noexcept
    {
        return Multiply(value, static_cast<std::uint32_t>(length % Mod));
    }

    static std::uint32_t Transform(Affine f, std::uint32_t sum, std::uint64_t length) noexcept
    {
        const auto product = std::uint64_t{f.a} * sum;
        const auto offset = std::uint64_t{f.b} * (length % Mod);
        if constexpr (Mod <= (std::uint64_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    static Affine Compose(Affine after, Affine before) noexcept
    {
        return {Multiply(after.a, before.a), Transform(after, before.b, 1)};
    }

    static bool IsNode(Handle handle) noexcept
    {
        return (handle & kNodeBit) != 0;
    }

    static std::size_t Index(Handle handle) noexcept
    {
        return static_cast<std::size_t>(handle & (kNodeBit - 1));
    }

    std::uint32_t CurrentSum(Handle handle, std::uint64_t length) const noexcept
    {
        if (IsNode(handle))
        {
            return nodes_[Index(handle)].sum;
        }
        return Scale(static_cast<std::uint32_t>(handle), length);
    }

    Handle Apply(Handle handle, std::uint64_t length, Affine change) noexcept
    {
        if (!IsNode(handle))
        {
            return Transform(change, static_cast<std::uint32_t>(handle), 1);
        }
        auto &node = nodes_[Index(handle)];
        node.sum = Transform(change, node.sum, length);
        node.pending = Compose(change, node.pending);
        return handle;
    }

    Handle Update(Handle handle, std::uint64_t start, std::uint64_t end,
                  std::uint64_t left, std::uint64_t right, Affine change) noexcept
    {
        if (left <= start && end <= right)
        {
            return Apply(handle, end - start, change);
        }

        if (!IsNode(handle))
        {
            if (Transform(change, static_cast<std::uint32_t>(handle), 1) == handle)
            {
                return handle;
            }
            // 片側の子を更新するときも、反対側は一様値のまま保持する。
            const auto new_handle = kNodeBit | static_cast<Handle>(nodes_.size());
            nodes_.push_back({handle, handle, CurrentSum(handle, end - start), {}});
            handle = new_handle;
        }

        auto &node = nodes_[Index(handle)];
        const auto middle = start + (end - start) / 2;
        if (!node.pending.identity())
        {
            node.left = Apply(node.left, middle - start, node.pending);
            node.right = Apply(node.right, end - middle, node.pending);
            node.pending = {};
        }
        if (left < middle)
        {
            node.left = Update(node.left, start, middle, left, right, change);
        }
        if (middle < right)
        {
            node.right = Update(node.right, middle, end, left, right, change);
        }
        node.sum = Add(CurrentSum(node.left, middle - start), CurrentSum(node.right, end - middle));
        return handle;
    }

    std::uint32_t Query(Handle handle, std::uint64_t start, std::uint64_t end,
                        std::uint64_t left, std::uint64_t right, Affine inherited) const noexcept
    {
        if (!IsNode(handle))
        {
            const auto length = std::min(end, right) - std::max(start, left);
            return Scale(Transform(inherited, static_cast<std::uint32_t>(handle), 1), length);
        }

        const auto &node = nodes_[Index(handle)];
        if (left <= start && end <= right)
        {
            return Transform(inherited, node.sum, end - start);
        }
        inherited = Compose(inherited, node.pending);
        const auto middle = start + (end - start) / 2;
        std::uint32_t result = 0;
        if (left < middle)
        {
            result = Query(node.left, start, middle, left, right, inherited);
        }
        if (middle < right)
        {
            result = Add(result, Query(node.right, middle, end, left, right, inherited));
        }
        return result;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
