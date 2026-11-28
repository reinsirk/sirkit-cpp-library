// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Dirichlet_Prefix_Sums.cpp
// 参考（積xyz<=Nの分割）: https://codeforces.com/blog/entry/117635?#comment-1041002
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dirichlet_prefix_sums.md
//
// API一覧
// [DirichletPrefixSums<T>]
// DirichletPrefixSums(n): 正整数nについて、相異なるfloor(n/i)を昇順に並べた座標を準備する。O(log log(n+2))時間・O(1)領域。
// size(): 座標数。quotient(index): 0始まりの指定座標。どちらもO(1)時間。
// convolution(left,right): 座標順の2つの累積和から、Dirichlet積の累積和を返す。
// divide(product,divisor,inverse_at_one): divisorとのDirichlet積がproductとなる数列の累積和を返す。
// inverse(divisor,inverse_at_one): Dirichlet積の逆元の累積和。inverse_at_oneはdivisorの第1項の乗法逆元。
// 累積和の入出力はsize()要素。T{}は0、T{1}は1。Tに可換環の加減乗算と等値比較が必要。
// 積・商・逆元はO(n^(2/3))回のT演算・O(sqrt(n))追加領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DIRICHLET_PREFIX_SUMS_DEFINED
#define SIRKIT_DIRICHLET_PREFIX_SUMS_DEFINED
template <class T>
class DirichletPrefixSums
{
    using u64 = std::uint64_t;

    u64 n_;
    u64 small_;
    u64 large_;

    static u64 square_root(u64 value) noexcept
    {
        u64 upper = u64{1} << ((std::bit_width(value) + 1) / 2);
        for (u64 next = (upper + value / upper) / 2; next < upper; next = (upper + value / upper) / 2)
        {
            upper = next;
        }
        return upper;
    }

    std::size_t table_size() const
    {
        if constexpr (sizeof(std::size_t) < sizeof(u64))
        {
            if (size() >= std::numeric_limits<std::size_t>::max())
            {
                throw std::length_error("DirichletPrefixSums: table size");
            }
        }
        return static_cast<std::size_t>(size());
    }

    void check_shape([[maybe_unused]] std::span<const T> values) const
    {
    }

    static const T &prefix(std::span<const T> values, u64 one_based)
    {
        return values[static_cast<std::size_t>(one_based - 1)];
    }

    std::vector<T> coefficients(std::span<const T> values) const
    {
        std::vector<T> result(static_cast<std::size_t>(small_) + 1);
        result[1] = values[0];
        for (u64 i = 2; i <= small_; ++i)
        {
            result[i] = prefix(values, i) - prefix(values, i - 1);
        }
        return result;
    }

    std::vector<T> diagonal_prefix(const std::vector<T> &left, const std::vector<T> &right) const
    {
        std::vector<T> result(static_cast<std::size_t>(small_) + 1);
        for (u64 i = 1; i <= small_; ++i)
        {
            result[i] = result[i - 1] + left[i] * right[i];
        }
        return result;
    }

    // 出力座標がx=n/zのとき、max(a,b)<=zとなる項を区間加算する。
    std::vector<T> small_pairs(const std::vector<T> &left, const std::vector<T> &right) const
    {
        std::vector<T> differences(static_cast<std::size_t>(large_) + 1);
        for (u64 b = 1; b <= large_; ++b)
        {
            const u64 after = large_ - b + 1;
            const u64 limit = std::min(b, n_ / b / b);
            T total{};
            for (u64 a = 1; a <= limit; ++a)
            {
                const auto product = a * b;
                const auto first = product <= small_ ? 0 : large_ - n_ / product;
                const auto value = a == b ? left[a] * right[b] : left[a] * right[b] + left[b] * right[a];
                differences[first] += value;
                total += value;
            }
            differences[after] -= total;
        }
        return differences;
    }

    // 残りのmax(a,b)>zの項を、小さい方の添字ごとに区間和でまとめる。
    T large_pairs(u64 z, u64 x, u64 first, std::span<const T> left, std::span<const T> right,
                  const std::vector<T> &left_coefficients, const std::vector<T> &right_coefficients,
                  const std::vector<T> &diagonal) const
    {
        const u64 root = square_root(x);
        const u64 limit = std::min(root, x / (z + 1));
        const u64 large_limit = large_ / z;
        T value = root > z ? diagonal[root] - diagonal[z] : T{};
        for (u64 a = first; a <= limit; ++a)
        {
            const auto low = std::max(a, z);
            const auto high = a <= large_limit ? size() - a * z + 1 : x / a;
            value += left_coefficients[a] * (prefix(right, high) - prefix(right, low)) +
                     right_coefficients[a] * (prefix(left, high) - prefix(left, low));
        }
        return value;
    }

    template <class Product>
    std::vector<T> solve(std::span<const T> divisor, T inverse_at_one, const Product &product) const
    {
        const auto count = table_size();
        check_shape(divisor);
        const auto divisor_coefficients = coefficients(divisor);
        const auto dense_size = static_cast<std::size_t>(small_) + 1;
        std::vector<T> answer(count), solved(dense_size), pending(dense_size);
        T previous_product{}, total{};
        for (u64 i = 1; i <= small_; ++i)
        {
            const T next_product = product(i);
            solved[i] = (next_product - previous_product - pending[i]) * inverse_at_one;
            previous_product = next_product;
            total += solved[i];
            answer[i - 1] = total;
            for (u64 j = 2; j <= small_ / i; ++j)
            {
                pending[i * j] += solved[i] * divisor_coefficients[j];
            }
        }
        const auto diagonal = diagonal_prefix(divisor_coefficients, solved);
        const auto differences = small_pairs(divisor_coefficients, solved);
        T fixed{};
        for (u64 offset = 0; offset < large_; ++offset)
        {
            const auto z = large_ - offset;
            const auto rank = small_ + offset + 1;
            fixed += differences[offset];
            const auto tail = large_pairs(z, n_ / z, 2, divisor, answer, divisor_coefficients, solved, diagonal);
            // 大きい累積和で未知なのは、divisor(1)と掛かるanswer(rank)だけ。
            const auto cross = solved[1] * (prefix(divisor, rank) - prefix(divisor, z)) - divisor_coefficients[1] * prefix(answer, z);
            answer[rank - 1] = (product(rank) - fixed - tail - cross) * inverse_at_one;
        }
        return answer;
    }

public:
    explicit DirichletPrefixSums(u64 n) : n_(n)
    {
        small_ = square_root(n);
        large_ = n / (small_ + 1);
    }

    [[nodiscard]] u64 size() const noexcept
    {
        return small_ + large_;
    }

    [[nodiscard]] u64 quotient(u64 index) const
    {
        return index < small_ ? index + 1 : n_ / (size() - index);
    }

    [[nodiscard]] std::vector<T> convolution(std::span<const T> left, std::span<const T> right) const
    {
        const auto count = table_size();
        check_shape(left);
        check_shape(right);
        const auto left_coefficients = coefficients(left);
        const auto right_coefficients = coefficients(right);
        std::vector<T> answer(count);
        for (u64 a = 1; a <= small_; ++a)
        {
            for (u64 b = 1; b <= small_ / a; ++b)
            {
                answer[a * b - 1] += left_coefficients[a] * right_coefficients[b];
            }
        }
        for (u64 i = 1; i < small_; ++i)
        {
            answer[i] += answer[i - 1];
        }
        const auto diagonal = diagonal_prefix(left_coefficients, right_coefficients);
        const auto differences = small_pairs(left_coefficients, right_coefficients);
        T fixed{};
        for (u64 offset = 0; offset < large_; ++offset)
        {
            const auto z = large_ - offset;
            fixed += differences[offset];
            answer[small_ + offset] = fixed + large_pairs(z, n_ / z, 1, left, right, left_coefficients,
                                                          right_coefficients, diagonal);
        }
        return answer;
    }

    [[nodiscard]] std::vector<T> divide(std::span<const T> product, std::span<const T> divisor, T inverse_at_one) const
    {
        check_shape(product);
        return solve(divisor, inverse_at_one, [product](u64 rank)
                     {
                         return prefix(product, rank);
                     });
    }

    [[nodiscard]] std::vector<T> inverse(std::span<const T> divisor, T inverse_at_one) const
    {
        return solve(divisor, inverse_at_one, [](u64)
                     {
                         return T{1};
                     });
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
