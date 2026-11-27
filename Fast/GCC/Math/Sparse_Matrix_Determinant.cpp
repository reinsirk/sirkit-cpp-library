// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Sparse_Matrix_Determinant.cpp
// 参考: D. Wiedemann, Solving sparse linear equations over finite fields (1986); Chen et al. (2002), Theorem 4.2.
// https://doi.org/10.1109/TIT.1986.1057137
// https://kaltofen.math.ncsu.edu/bibliography/02/CEKSTV02.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_matrix_determinant.md
//
// API一覧
// SparseDeterminantEntry: row, columnは0始まり。valueは998244353で剰余を取り、同じ座標の項は合計する。
// sparse_determinant_mod998244353(n, entries, seed=0, attempts=4): 行列式が確定すれば値を、未確定ならnulloptを返す。
// 0×0の行列式は1。seedで試行を再現できる。1試行O(N(N+K))時間、O(N+K)領域。失敗確率の上界は文書参照。
//
// [同梱Berlekamp–Massey]
// BerlekampMassey<T>(s): 体T上の有限列sに合う最短線形漸化式の係数cを返す。O(n²)時間・O(n)領域。
// 返却長dとし、d<=i<nでs[i]=sum(c[j]*s[i-j-1])。同じ最短次数の解は一意とは限らない。
// Tは正確な体の四則演算・0/1からの構築・等値比較を提供する。Mintは利用側が用意する。
// BerlekampMassey(vector<uint32_t>): 法998244353で同じ係数を返す。入力は自動で正規化する。
// 空列・全零列は{}。有限列の適合だけを保証し、未知の後続項や本来の漸化式の特定は保証しない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
#define SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct RecurrenceMod998 {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    RecurrenceMod998()=default;
    RecurrenceMod998(std::uint64_t x):value(static_cast<std::uint32_t>(x%modulus)){}
    RecurrenceMod998& operator+=(RecurrenceMod998 b)noexcept{
        value+=b.value;if(value>=modulus)value-=modulus;return *this;
    }
    RecurrenceMod998& operator-=(RecurrenceMod998 b)noexcept{
        value=value>=b.value?value-b.value:value+modulus-b.value;return *this;
    }
    RecurrenceMod998& operator*=(RecurrenceMod998 b)noexcept{
        value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;
    }
    friend RecurrenceMod998 operator+(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a+=b;}
    friend RecurrenceMod998 operator-(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a-=b;}
    friend RecurrenceMod998 operator*(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMod998,RecurrenceMod998)=default;
    friend RecurrenceMod998 operator/(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{
        RecurrenceMod998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BERLEKAMP_MASSEY_DEFINED
#define SIRKIT_BERLEKAMP_MASSEY_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline std::size_t recurrence_size_plus_one(std::size_t size, std::size_t capacity,
                                            const char* error) {
    if (size >= capacity) throw std::length_error(error);
    return size + 1;
}
}
template<class T>
std::vector<T> BerlekampMassey(const std::vector<T>& sequence) {
    static_assert(!std::is_integral_v<T>,"BerlekampMassey<T>: use an exact field type, or the uint32_t overload");
    if(sequence.empty())return {};
    const auto n=sequence.size();
    (void)/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::recurrence_size_plus_one(n, sequence.max_size(), "BerlekampMassey: size overflow");
    std::vector<T> connection{T{1}},previous{T{1}};
    std::size_t order=0,shift=1;
    T previous_inverse{1};
    for(std::size_t position=0;position<n;++position) {
        T discrepancy=sequence[position];
        for(std::size_t j=1;j<=order;++j)discrepancy+=connection[j]*sequence[position-j];
        if(discrepancy==T{0}){++shift;continue;}
        const T ratio=discrepancy*previous_inverse;
        const bool extend=order<=position/2;
        std::vector<T> saved;
        if(extend)saved=connection;
        // 最後の更新位置pで保存したpreviousはp+1項以下。shiftを足してもn+1項以下。
        if(connection.size()<previous.size()+shift)connection.resize(previous.size()+shift,T{0});
        for(std::size_t j=0;j<previous.size();++j)connection[j+shift]-=ratio*previous[j];
        if(extend) {
            order=position+1-order;
            previous=std::move(saved);
            previous_inverse=T{1}/discrepancy;
            shift=1;
        } else ++shift;
    }
    std::vector<T> result(order,T{0});
    for(std::size_t j=0;j<order;++j)result[j]=T{0}-connection[j+1];
    return result;
}
inline std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence) {
    using Field=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMod998;
    std::vector<Field> input;input.reserve(sequence.size());
    for(const auto value:sequence)input.emplace_back(value);
    const auto coefficients=BerlekampMassey(input);
    std::vector<std::uint32_t> result;result.reserve(coefficients.size());
    for(const auto value:coefficients)result.push_back(value.value);
    return result;
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <random>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_SPARSE_MATRIX_DETERMINANT_DEFINED
#define SIRKIT_SPARSE_MATRIX_DETERMINANT_DEFINED

struct SparseDeterminantEntry
{
    std::size_t row;
    std::size_t column;
    std::uint32_t value;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
using SparseDeterminantField = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMod998;

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
    using Field = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SparseDeterminantField;
    if (n > std::vector<Field>{}.max_size() / 2)
    {
        throw std::length_error("sparse_determinant_mod998244353: dimension is too large");
    }
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
            diagonal[i] = Field{1u + /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SparseDeterminantUniform(random, Field::modulus - 1)};
            product *= diagonal[i];
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            projection[i] = Field{/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SparseDeterminantUniform(random, Field::modulus)};
            power[i] = Field{/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SparseDeterminantUniform(random, Field::modulus)};
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
        if (const auto determinant = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SparseDeterminantFromRecurrence(coeff, n, product))
        {
            return determinant;
        }
    }
    return std::nullopt;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
