// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Prefix_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_sum.md
//
// API一覧
// PrefixSum<T>(values={}): 長さNの配列から累積和を作る。時間・領域O(N)。空配列も可。
// size(): 元の配列の要素数。O(1)。
// prefix(r): [0,r)の和。0<=r<=size()。O(1)。
// sum(l,r): [l,r)の和。0<=l<=r<=size()。O(1)。空区間の和は0。
// T{}は加法の単位元。累積和と差をTで表せること。
// 範囲外の問い合わせはstd::out_of_range。SKIP_RUNTIME_CHECKSで範囲検査を省く。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_SUM_DEFINED
#define SIRKIT_PREFIX_SUM_DEFINED

template <class T>
class PrefixSum
{
public:
    explicit PrefixSum(std::span<const T> values = {})
    {
        sums_.reserve(values.size());
        T total{};
        for (const T &value : values)
        {
            total = total + value;
            sums_.push_back(total);
        }
    }

    std::size_t size() const noexcept
    {
        return sums_.size();
    }

    T prefix(std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (right > size())
        {
            throw std::out_of_range("PrefixSum: prefix end");
        }
#endif
        return prefix_unchecked(right);
    }

    T sum(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("PrefixSum: range");
        }
#endif
        return prefix_unchecked(right) - prefix_unchecked(left);
    }

private:
    T prefix_unchecked(std::size_t right) const
    {
        return right == 0 ? T{} : sums_[right - 1];
    }

    std::vector<T> sums_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
