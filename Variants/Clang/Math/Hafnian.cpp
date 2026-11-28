// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Hafnian.cpp
// 参考（包除原理で頂点対を消去）: https://arxiv.org/abs/1107.4466
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hafnian.md
//
// API一覧
// hafnian(matrix): 対称行列の上三角から、完全マッチングごとの辺重みの積を合計する。
// N×Nのvector<vector<T>>で、N<=40。対角・下三角の値は参照しない。N=0は1、奇数なら0。
// 可換環Tの加減乗算を使う。O(N²・2^(N/2))回のT演算、O(N^4)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HAFNIAN_DEFINED
#define SIRKIT_HAFNIAN_DEFINED

namespace detail
{
template <class T>
class HafnianComputation
{
public:
    static constexpr std::size_t kMaxOrder = 40;

private:
    using Poly = std::array<T, kMaxOrder / 2 + 1>;
    using Matrix = std::vector<Poly>;

    std::size_t order_;
    std::size_t pairs_;
    std::vector<Matrix> levels_;

    const Poly &edge(const Matrix &matrix, std::size_t i, std::size_t j) const
    {
        return matrix[i * order_ + j];
    }

    static T product_coefficient(const Poly &left, const Poly &right, std::size_t degree)
    {
        T value{};
        for (std::size_t l = 0; l <= degree; ++l)
        {
            value += left[l] * right[degree - l];
        }
        return value;
    }

    T four_vertices(const Matrix &matrix, std::size_t a, const Poly &weight) const
    {
        const auto b = a + 1;
        const auto c = a + 2;
        const auto d = a + 3;
        T result{};
        for (std::size_t i = 0; i <= pairs_ - 2; ++i)
        {
            const auto degree = pairs_ - 2 - i;
            const auto matchings = product_coefficient(edge(matrix, a, b), edge(matrix, c, d), degree) +
                                   product_coefficient(edge(matrix, a, c), edge(matrix, b, d), degree) +
                                   product_coefficient(edge(matrix, a, d), edge(matrix, b, c), degree);
            result += weight[i] * matchings;
        }
        return result;
    }

    T remove_pairs(const Matrix &matrix, std::size_t a, const Poly &weight)
    {
        const auto b = a + 1;
        if (a + 2 == order_)
        {
            return product_coefficient(weight, edge(matrix, a, b), pairs_ - 1);
        }
        if (a + 4 == order_)
        {
            return four_vertices(matrix, a, weight);
        }

        // z付きで頂点対を消去する。負側との差を取ると、その頂点対を使わない項が消える。
        // zの次数N/2では、各頂点対をちょうど1回使った完全マッチングだけが残る。
        const auto without = remove_pairs(matrix, a + 2, weight);
        Poly with_weight = weight;
        for (std::size_t d = 1; d <= pairs_; ++d)
        {
            with_weight[d] += product_coefficient(weight, edge(matrix, a, b), d - 1);
        }
        auto &contracted = levels_[a / 2 + 1];
        for (std::size_t i = a + 2; i < order_; ++i)
        {
            for (std::size_t j = i + 1; j < order_; ++j)
            {
                auto &next = contracted[i * order_ + j];
                const auto &original = edge(matrix, i, j);
                next[0] = original[0];
                for (std::size_t d = 1; d < pairs_; ++d)
                {
                    next[d] = original[d] + product_coefficient(edge(matrix, a, i), edge(matrix, b, j), d - 1) +
                              product_coefficient(edge(matrix, a, j), edge(matrix, b, i), d - 1);
                }
            }
        }
        return remove_pairs(contracted, a + 2, with_weight) - without;
    }

public:
    explicit HafnianComputation(const std::vector<std::vector<T>> &matrix)
        : order_(matrix.size()), pairs_(order_ / 2), levels_(pairs_, Matrix(order_ * order_))
    {
        for (std::size_t i = 0; i < order_; ++i)
        {
            for (std::size_t j = i + 1; j < order_; ++j)
            {
                levels_[0][i * order_ + j][0] = matrix[i][j];
            }
        }
    }

    T evaluate()
    {
        Poly weight{};
        weight[0] = T{1};
        return remove_pairs(levels_[0], 0, weight);
    }
};
} // namespace detail

template <class T>
[[nodiscard]] T hafnian(const std::vector<std::vector<T>> &matrix)
{
    const auto order = matrix.size();
    if (order > detail::HafnianComputation<T>::kMaxOrder)
    {
        throw std::length_error("hafnian: order exceeds 40");
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &row : matrix)
    {
        if (row.size() != order)
        {
            throw std::invalid_argument("hafnian: expected a square matrix");
        }
    }
#endif
    if (order == 0)
    {
        return T{1};
    }
    if (order % 2 != 0)
    {
        return T{};
    }
    return detail::HafnianComputation<T>(matrix).evaluate();
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
