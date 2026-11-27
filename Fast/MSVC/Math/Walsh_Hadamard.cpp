// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Walsh_Hadamard.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/walsh_hadamard.md
//
// API一覧
// walsh_hadamard_transform(values): 正規化しないXOR変換をその場で実行。値型は加減算が可能なもの。
// inverse_walsh_hadamard_transform(values): 上と同じ変換を行い各係数を配列長で割る。
// bitwise_xor_convolution(left, right): out[k]=Σ_(i^j=k) left[i]*right[j]。入力は同長。
// bitwise_xor_square(values): 同じ入力同士のXOR畳み込み。順変換は1回。
// すべて空列または長さn=2^k。逆変換・積では符号付き整数か、nで割れる係数型を用いる。
// 四則演算を定数時間とし、各操作O(n log(n+1))時間、入力配列を除いてO(1)補助領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_WALSH_HADAMARD_DEFINED
#define SIRKIT_WALSH_HADAMARD_DEFINED

template <class T, std::size_t Extent>
void walsh_hadamard_transform(std::span<T, Extent> values)
{
    const auto n = values.size();
    for (std::size_t half = 1; half < n; half *= 2)
    {
        for (std::size_t base = 0; base < n; base += 2 * half)
        {
            for (std::size_t j = 0; j < half; ++j)
            {
                auto &left = values[base + j];
                auto &right = values[base + half + j];
                const T sum = left + right;
                right = left - right;
                left = sum;
            }
        }
    }
}

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class T, std::size_t Extent>
    requires std::numeric_limits<T>::is_integer
void NormalizeWalshHadamard(std::span<T, Extent> values)
{
    const auto divisor = std::ssize(values);
    for (auto &value : values)
    {
        value /= divisor;
    }
}

template <class T, std::size_t Extent>
    requires(!std::numeric_limits<T>::is_integer)
void NormalizeWalshHadamard(std::span<T, Extent> values)
{
    const T inverse_length = T{1} / static_cast<T>(values.size());
    for (auto &value : values)
    {
        value *= inverse_length;
    }
}
} // namespace reinsirk::detail

template <class T, std::size_t Extent>
void inverse_walsh_hadamard_transform(std::span<T, Extent> values)
{
    static_assert(!std::numeric_limits<T>::is_integer || std::numeric_limits<T>::is_signed,
                  "use signed integers or a type in which the length is invertible");
    if (values.empty())
    {
        return;
    }
    walsh_hadamard_transform(values);
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NormalizeWalshHadamard(values);
}

template <class T>
std::vector<T> bitwise_xor_convolution(std::vector<T> left, std::vector<T> right)
{
    walsh_hadamard_transform(std::span(left));
    walsh_hadamard_transform(std::span(right));
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        left[i] *= right[i];
    }
    inverse_walsh_hadamard_transform(std::span(left));
    return left;
}

template <class T>
std::vector<T> bitwise_xor_square(std::vector<T> values)
{
    walsh_hadamard_transform(std::span(values));
    for (auto &value : values)
    {
        value *= value;
    }
    inverse_walsh_hadamard_transform(std::span(values));
    return values;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
