// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Affine_Range_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_affine_range_sum.md
//
// API一覧
// PersistentAffineRangeSum<Mod>(values): 初期列を構築。O(n+1)時間・領域。法の既定値998244353。
// initial_root(): 初期版のRoot。size()/allocated_nodes(): 長さ・共有領域の節点数。各O(1)。
// range_affine(root,l,r,a,b): 版rootの[l,r)にx→a*x+bを適用した新しい版。
// range_copy(dst,src,l,r): dstの同じ区間にsrcの値を写した新しい版。区間外も両元版も保つ。
// range_sum(root,l,r) / all_sum(root): 指定版の区間和／全体和の非負剰余。const、確保なし。
// 更新・コピーは償却O(log(n+1))時間・追加領域。照会はO(log(n+1))、全体和O(1)。
// 全区間更新・コピーと空区間操作はO(1)で追加節点なし。添字は0始まり、n<=INT_MAX。
// コピーした木は保存領域を共有。Rootは非所有で、保存領域を持つ木が残る間有効。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_AFFINE_RANGE_SUM_DEFINED
#define SIRKIT_PERSISTENT_AFFINE_RANGE_SUM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPersistentAffineSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("PersistentAffineRangeSum: too many elements");
    }
    return size;
}

inline std::size_t CheckedPersistentAffineBudget(std::size_t used, std::size_t extra, std::size_t limit)
{
    if (extra > limit - used)
    {
        throw std::length_error("PersistentAffineRangeSum: too many nodes");
    }
    return used + extra;
}
}

template <std::uint32_t Mod = 998244353>
class PersistentAffineRangeSum
{
    static_assert(Mod > 0);
    using Index = std::uint32_t;
    static constexpr std::uint32_t kOne = 1 % Mod;

    struct Affine
    {
        std::uint32_t a, b;

        [[nodiscard]] bool identity() const noexcept
        {
            return a == kOne && b == 0;
        }

        friend constexpr bool operator==(Affine left, Affine right) noexcept
        {
            return left.a == right.a && left.b == right.b;
        }
    };

    struct View
    {
        Index index;
        Affine pending;

        friend constexpr bool operator==(View left, View right) noexcept
        {
            return left.index == right.index && left.pending == right.pending;
        }
    };

    struct Node
    {
        View left, right;
        std::uint32_t sum;
    };

    struct Storage
    {
        static constexpr unsigned kBlockBits = 12;
        static constexpr std::size_t kBlockSize = std::size_t{1} << kBlockBits;
        static constexpr std::size_t kLimit = std::numeric_limits<Index>::max();

        std::vector<std::unique_ptr<Node[]>> blocks;
        std::size_t used = 0;
        int n = 0;
        View initial{0, {kOne, 0}};

        void reserve_more(std::size_t extra)
        {
            const auto needed = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPersistentAffineBudget(used, extra, kLimit);
            const auto count = needed / kBlockSize + (needed % kBlockSize != 0);
            while (blocks.size() < count)
            {
                blocks.push_back(std::make_unique_for_overwrite<Node[]>(kBlockSize));
            }
        }

        [[nodiscard]] const Node &at(Index index) const noexcept
        {
            return blocks[index >> kBlockBits][index & (kBlockSize - 1)];
        }

        View append(const Node &node) noexcept
        {
            const auto index = static_cast<Index>(used++);
            blocks[index >> kBlockBits][index & (kBlockSize - 1)] = node;
            return {index, {kOne, 0}};
        }
    };

public:
    class Root
    {
        friend class PersistentAffineRangeSum;

    public:
        Root() = default;

    private:
        View view_{0, {kOne, 0}};
    };

    explicit PersistentAffineRangeSum(std::span<const std::int64_t> values = {})
        : storage_(std::make_shared<Storage>())
    {
        const auto count = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPersistentAffineSize(values.size());
        storage_->n = static_cast<int>(count);
        if (count != 0)
        {
            storage_->reserve_more(2 * count - 1);
            storage_->initial = Build(values);
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return storage_ ? storage_->n : 0;
    }

    [[nodiscard]] std::size_t allocated_nodes() const noexcept
    {
        return storage_ ? storage_->used : 0;
    }

    [[nodiscard]] Root initial_root() const noexcept
    {
        return storage_ ? MakeRoot(storage_->initial) : Root{};
    }

    [[nodiscard]] Root range_affine(Root from, int left, int right, std::int64_t a, std::int64_t b)
    {
        Check(from, left, right);
        const Affine change{Normalize(a), Normalize(b)};
        if (left == right || change.identity())
        {
            return from;
        }
        if (left == 0 && right == size())
        {
            return MakeRoot(With(from.view_, change));
        }

        ReserveChange();
        return MakeRoot(Update(from.view_, 0, size(), left, right, change));
    }

    [[nodiscard]] Root range_copy(Root dst, Root src, int left, int right)
    {
        Check(dst, left, right);
        CheckOwner(src);
        if (left == right || dst.view_ == src.view_)
        {
            return dst;
        }
        if (left == 0 && right == size())
        {
            return src;
        }

        ReserveChange();
        return MakeRoot(Copy(dst.view_, src.view_, 0, size(), left, right));
    }

    [[nodiscard]] std::uint32_t range_sum(Root root, int left, int right) const
    {
        Check(root, left, right);
        if (left == right)
        {
            return 0;
        }
        return Query(root.view_, 0, size(), left, right);
    }

    [[nodiscard]] std::uint32_t all_sum(Root root) const
    {
        CheckOwner(root);
        return size() == 0 ? 0 : ReadSum(root.view_, size());
    }

private:
    std::shared_ptr<Storage> storage_;

    [[nodiscard]] Root MakeRoot(View view) const noexcept
    {
        Root root;
        root.view_ = view;
        return root;
    }

    void CheckOwner(Root root) const
    {
        (void)root;
    }

    void Check(Root root, int left, int right) const
    {
        CheckOwner(root);
        (void)left;
        (void)right;
    }

    void ReserveChange()
    {
        // 各深さで部分被覆の境界に接する節点だけを作る。
        storage_->reserve_more(2 * std::bit_width(static_cast<unsigned>(size())));
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b) noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Transform(Affine f, std::uint32_t sum, int length) noexcept
    {
        const auto product = std::uint64_t{f.a} * sum;
        const auto offset = std::uint64_t{f.b} * static_cast<unsigned>(length);
        if constexpr (Mod <= (std::uint64_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    // 親の変換は子が既に持つ変換より後に適用する。
    static View With(View view, Affine after) noexcept
    {
        if (!after.identity())
        {
            view.pending = {
                static_cast<std::uint32_t>(std::uint64_t{after.a} * view.pending.a % Mod),
                Transform(after, view.pending.b, 1),
            };
        }
        return view;
    }

    std::uint32_t ReadSum(View view, int length) const noexcept
    {
        const auto sum = storage_->at(view.index).sum;
        return view.pending.identity() ? sum : Transform(view.pending, sum, length);
    }

    View Combine(View left, View right, int left_size, int right_size) noexcept
    {
        return storage_->append({left, right, Add(ReadSum(left, left_size), ReadSum(right, right_size))});
    }

    View Build(std::span<const std::int64_t> values) noexcept
    {
        if (values.size() == 1)
        {
            return storage_->append({{}, {}, Normalize(values.front())});
        }
        const auto midpoint = values.size() / 2;
        const auto left = Build(values.first(midpoint));
        const auto right = Build(values.subspan(midpoint));
        return Combine(left, right, static_cast<int>(midpoint), static_cast<int>(values.size() - midpoint));
    }

    View Update(View current, int start, int end, int left, int right, Affine change) noexcept
    {
        if (left <= start && end <= right)
        {
            return With(current, change);
        }

        const auto &node = storage_->at(current.index);
        auto low = With(node.left, current.pending);
        auto high = With(node.right, current.pending);
        const auto midpoint = start + (end - start) / 2;
        if (left < midpoint)
        {
            low = Update(low, start, midpoint, left, right, change);
        }
        if (midpoint < right)
        {
            high = Update(high, midpoint, end, left, right, change);
        }
        return Combine(low, high, midpoint - start, end - midpoint);
    }

    View Copy(View target, View source, int start, int end, int left, int right) noexcept
    {
        if (target == source)
        {
            return target;
        }
        if (left <= start && end <= right)
        {
            return source;
        }

        const auto &dst = storage_->at(target.index);
        const auto &src = storage_->at(source.index);
        auto low = With(dst.left, target.pending);
        auto high = With(dst.right, target.pending);
        const auto midpoint = start + (end - start) / 2;
        if (left < midpoint)
        {
            low = Copy(low, With(src.left, source.pending), start, midpoint, left, right);
        }
        if (midpoint < right)
        {
            high = Copy(high, With(src.right, source.pending), midpoint, end, left, right);
        }
        return Combine(low, high, midpoint - start, end - midpoint);
    }

    std::uint32_t Query(View view, int start, int end, int left, int right) const noexcept
    {
        if (left <= start && end <= right)
        {
            return ReadSum(view, end - start);
        }

        const auto &node = storage_->at(view.index);
        const auto midpoint = start + (end - start) / 2;
        if (right <= midpoint)
        {
            return Query(With(node.left, view.pending), start, midpoint, left, right);
        }
        if (left >= midpoint)
        {
            return Query(With(node.right, view.pending), midpoint, end, left, right);
        }
        const auto low = Query(With(node.left, view.pending), start, midpoint, left, right);
        const auto high = Query(With(node.right, view.pending), midpoint, end, left, right);
        return Add(low, high);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
