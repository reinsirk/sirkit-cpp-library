// SPDX-License-Identifier: CC0-1.0
// 参考: D. Wiedemann, Solving sparse linear equations over finite fields (1986); Chen et al. (2002), Theorem 4.2.
// https://doi.org/10.1109/TIT.1986.1057137
// https://kaltofen.math.ncsu.edu/bibliography/02/CEKSTV02.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_matrix_determinant.md
//
// API一覧
// SparseDeterminantEntry: row, columnは0始まり。valueは998244353で剰余を取り、同じ座標の項は合計する。
// sparse_determinant_mod998244353(n, entries, seed=0, attempts=4): 行列式が確定すれば値を、未確定ならnulloptを返す。
// 0×0の行列式は1。seedで試行を再現できる。1試行O(N(N+K))時間、O(N+K)領域。失敗確率の上界は文書参照。

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_SPARSE_MATRIX_DETERMINANT_DEFINED
#define SIRKIT_SPARSE_MATRIX_DETERMINANT_DEFINED

struct SparseDeterminantEntry
{
    std::size_t row;
    std::size_t column;
    std::uint32_t value;
};

namespace reinsirk::detail
{
using SparseDeterminantField = reinsirk::detail::RecurrenceMod998;

// 64ビット一様乱数のうち、各剰余の出現回数が同じになる範囲だけを使う。
template<class Random>
std::uint32_t SparseDeterminantUniform(Random &random, std::uint32_t range)
{
    const std::uint64_t threshold = (std::uint64_t{0} - range) % range;
    std::uint64_t sample;
    do
    {
        sample = random();
    }
    while (sample < threshold);
    return static_cast<std::uint32_t>(sample % range);
}

inline std::optional<std::uint32_t> SparseDeterminantFromRecurrence(
    std::span<const SparseDeterminantField> coeff, std::size_t n, SparseDeterminantField diagonal_product)
{
    using Field = SparseDeterminantField;
    if (coeff.empty())
    {
        return std::nullopt;
    }
    // 射影列の最小多項式は行列の最小多項式を割る。
    if (coeff.back() == Field{0})
    {
        return 0;
    }
    if (coeff.size() != n)
    {
        return std::nullopt;
    }
    const Field det_scaled = n % 2 == 0 ? Field{0} - coeff.back() : coeff.back();
    return (det_scaled / diagonal_product).value;
}
} // namespace reinsirk::detail

inline std::optional<std::uint32_t> sparse_determinant_mod998244353(
    std::size_t n, std::span<const SparseDeterminantEntry> entries, std::uint64_t seed = 0, int attempts = 4)
{
    using Field = reinsirk::detail::SparseDeterminantField;
    if (n > std::vector<Field>{}.max_size() / 2)
    {
        throw std::length_error("sparse_determinant_mod998244353: dimension is too large");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (attempts < 0)
    {
        throw std::invalid_argument("sparse_determinant_mod998244353: negative attempt count");
    }
    for (const auto &entry : entries)
    {
        if (entry.row >= n || entry.column >= n)
        {
            throw std::out_of_range("sparse_determinant_mod998244353: entry outside the matrix");
        }
    }
#endif
    if (n == 0)
    {
        return 1;
    }
    if (entries.size() < n)
    {
        return 0;
    }

    // 行ごとに連続に格納し、行列・ベクトル積を毎回 O(N+K) で求める。
    struct Entry
    {
        std::size_t column;
        Field value;
    };
    std::vector<std::size_t> offsets(n + 1);
    for (const auto &entry : entries)
    {
        ++offsets[entry.row + 1];
    }
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    auto cursor = offsets;
    std::vector<Entry> rows(entries.size());
    for (const auto &entry : entries)
    {
        rows[cursor[entry.row]++] = {entry.column, Field{entry.value}};
    }

    std::mt19937_64 random(seed);
    std::vector<Field> diagonal(n), projection(n), power(n), next(n), sequence(2 * n);
    for (int trial = 0; trial < attempts; ++trial)
    {
        Field product{1};
        for (std::size_t i = 0; i < n; ++i)
        {
            diagonal[i] = Field{1u + reinsirk::detail::SparseDeterminantUniform(random, Field::modulus - 1)};
            product *= diagonal[i];
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            projection[i] = Field{reinsirk::detail::SparseDeterminantUniform(random, Field::modulus)};
            power[i] = Field{reinsirk::detail::SparseDeterminantUniform(random, Field::modulus)};
        }

        // sequence[t] = projection^T (D*A)^t power_initial。
        const auto observe = [&]
        {
            return std::inner_product(projection.begin(), projection.end(), power.begin(), Field{0});
        };
        sequence[0] = observe();
        for (std::size_t t = 1; t < sequence.size(); ++t)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                Field row{};
                for (std::size_t p = offsets[i]; p < offsets[i + 1]; ++p)
                {
                    row += rows[p].value * power[rows[p].column];
                }
                next[i] = diagonal[i] * row;
            }
            power.swap(next);
            sequence[t] = observe();
        }
        const auto coeff = BerlekampMassey(sequence);
        if (const auto determinant = reinsirk::detail::SparseDeterminantFromRecurrence(coeff, n, product))
        {
            return determinant;
        }
    }
    return std::nullopt;
}

#endif
