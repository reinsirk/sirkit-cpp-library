// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Berlekamp_Massey.cpp
// 数学的参考: J. L. Massey, Shift-Register Synthesis and BCH Decoding (1969).
// https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI411.pdf
// 数学的参考（講座）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_recurrence.md
//
// API一覧
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
template<class T>
std::vector<T> BerlekampMassey(const std::vector<T>& sequence) {
    static_assert(!std::is_integral_v<T>,"BerlekampMassey<T>: use an exact field type, or the uint32_t overload");
    if(sequence.empty())return {};
    const auto n=sequence.size();
    if(n>=std::vector<T>{}.max_size())throw std::length_error("BerlekampMassey: size overflow");
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
        if(previous.size()>connection.max_size()-shift)
            throw std::length_error("BerlekampMassey: size overflow");
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
// SIRKIT_NAMESPACE_END_V1
