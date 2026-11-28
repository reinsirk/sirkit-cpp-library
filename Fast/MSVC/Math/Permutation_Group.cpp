// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Permutation_Group.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/permutation_group.md
// 参考（固定部分群の構築）: https://arxiv.org/abs/math/9201304
//
// API一覧
// PermutationGroup(n, generators): [0,n)の置換から生成される群を構築。生成元G個でO(n^5+G*n^2)時間、O(n^3)領域。
// degree(): 各置換の長さn。O(1)。
// contains(p): pが群に属するか。O(n^2)時間、O(n)補助領域。
// stabilizer_indices(): i未満の点をすべて固定する置換で、iを移せる点の数をiごとに返す。O(n)。
// order_mod(mod): 群の位数を正のmodで割った余り。O(n)。
// 長さ0の置換と空の生成元列も使える。各置換の要素に重複がなく、0以上n未満であること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERMUTATION_GROUP_DEFINED
#define SIRKIT_PERMUTATION_GROUP_DEFINED
class PermutationGroup
{
public:
    using Permutation = std::vector<int>;

    PermutationGroup(int n, std::span<const Permutation> generators) : degree_(n)
    {
        if (n < 0)
        {
            throw std::invalid_argument("PermutationGroup: degree must be nonnegative");
        }
        Permutation identity(static_cast<std::size_t>(n));
        std::iota(identity.begin(), identity.end(), 0);
        levels_.reserve(identity.size());
        for (int point = 0; point < n; ++point)
        {
            Level level;
            level.slot.assign(static_cast<std::size_t>(n - point), -1);
            level.slot.front() = 0;
            level.representatives.push_back({identity, 0});
            levels_.push_back(std::move(level));
        }
        for (const auto &generator : generators)
        {
            ExtendIfMissing(0, CopyPermutation(generator));
        }
    }

    [[nodiscard]] int degree() const noexcept
    {
        return degree_;
    }

    [[nodiscard]] bool contains(std::span<const int> permutation) const
    {
        return Sift(0, CopyPermutation(permutation));
    }

    [[nodiscard]] std::vector<int> stabilizer_indices() const
    {
        std::vector<int> result;
        result.reserve(levels_.size());
        for (const auto &level : levels_)
        {
            result.push_back(static_cast<int>(level.representatives.size()));
        }
        return result;
    }

    [[nodiscard]] std::uint32_t order_mod(std::uint32_t modulus) const
    {
        std::uint64_t order = 1U % modulus;
        for (const auto &level : levels_)
        {
            order = order * level.representatives.size() % modulus;
        }
        return static_cast<std::uint32_t>(order);
    }

private:
    struct Representative
    {
        Permutation inverse;
        std::size_t checked_generators;
    };

    struct Level
    {
        std::vector<int> slot;
        std::vector<Representative> representatives;
        std::vector<Permutation> generators;
    };

    int degree_;
    std::vector<Level> levels_;

    Permutation CopyPermutation(std::span<const int> permutation) const
    {
        return Permutation(permutation.begin(), permutation.end());
    }

    void Reduce(Permutation &permutation, int base, const Permutation &inverse) const
    {
        for (int point = base; point < degree_; ++point)
        {
            permutation[point] = inverse[permutation[point]];
        }
    }

    bool Sift(int first, Permutation permutation) const
    {
        for (int base = first; base < degree_; ++base)
        {
            const int image = permutation[base];
            if (image == base)
            {
                continue;
            }
            const auto &level = levels_[base];
            const int slot = level.slot[image - base];
            if (slot < 0)
            {
                return false;
            }
            Reduce(permutation, base, level.representatives[slot].inverse);
        }
        return true;
    }

    void ExtendIfMissing(int base, Permutation generator)
    {
        if (Sift(base, generator))
        {
            return;
        }
        auto &level = levels_[base];
        level.generators.push_back(std::move(generator));
        // 各代表元と生成元の積は、現れた順に1回だけ調べる。
        for (std::size_t index = 0; index < level.representatives.size(); ++index)
        {
            while (level.representatives[index].checked_generators < level.generators.size())
            {
                auto &representative = level.representatives[index];
                const auto &next_generator = level.generators[representative.checked_generators++];
                Permutation product = next_generator;
                for (int point = base; point < degree_; ++point)
                {
                    product[representative.inverse[point]] = next_generator[point];
                }
                RegisterProduct(base, std::move(product));
            }
        }
    }

    void RegisterProduct(int base, Permutation product)
    {
        auto &level = levels_[base];
        auto &slot = level.slot[product[base] - base];
        if (slot < 0)
        {
            Permutation inverse = product;
            for (int point = base; point < degree_; ++point)
            {
                inverse[product[point]] = point;
            }
            slot = static_cast<int>(level.representatives.size());
            level.representatives.push_back({std::move(inverse), 0});
            return;
        }
        const Permutation &inverse = level.representatives[slot].inverse;
        // 既知の代表元を取り除くと、先頭base+1点が固定される。
        Reduce(product, base, inverse);
        ExtendIfMissing(base + 1, std::move(product));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
