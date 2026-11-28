// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Poisson_Binomial.cpp
// 参考（成功数の確率）: https://atcoder.jp/contests/dp/tasks/dp_i
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/poisson_binomial.md
//
// API一覧
// poisson_binomial_distribution(probabilities): 独立試行の成功数が0..nとなる各確率を返す。O(1+n^2)時間・O(n+1)領域。
// poisson_binomial_at_least(probabilities,k): 成功がk回以上起きる確率。0<k<=nでO(n*min(k,n-k+1))時間。
// 追加領域はO(1+min(k,n-k+1))。k=0なら1、k>nなら0。範囲検査はO(n+1)。
// 入力は長さを取得できる浮動小数点の範囲。各要素は[0,1]。返り値の要素型は入力と同じ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POISSON_BINOMIAL_DEFINED
#define SIRKIT_POISSON_BINOMIAL_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <typename Range>
concept ProbabilityRange = std::ranges::forward_range<const Range> && std::ranges::sized_range<const Range> &&
                           std::floating_point<std::ranges::range_value_t<const Range>>;

template <ProbabilityRange Range>
void ValidatePoissonProbabilities(const Range &probabilities)
{
    (void)probabilities;
}

inline std::size_t PoissonBinomialTableSize(std::size_t last, std::size_t limit)
{
    if (last >= limit)
    {
        throw std::length_error("poisson_binomial: too many states");
    }
    return last + 1;
}

template <std::floating_point Probability>
std::vector<Probability> InitialPoissonMass(std::size_t limit)
{
    std::vector<Probability> mass;
    mass.resize(PoissonBinomialTableSize(limit, mass.max_size()));
    mass[0] = Probability{1};
    return mass;
}

template <std::floating_point Probability>
void UpdatePoissonMass(std::vector<Probability> &mass, std::size_t last,
                       Probability advance, Probability stay)
{
    for (std::size_t i = last; i != 0; --i)
    {
        mass[i] = mass[i] * stay + mass[i - 1] * advance;
    }
    mass[0] *= stay;
}
} // namespace reinsirk::detail

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ProbabilityRange Range>
[[nodiscard]] auto poisson_binomial_distribution(const Range &probabilities)
{
    using Probability = std::ranges::range_value_t<const Range>;
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ValidatePoissonProbabilities(probabilities);
    auto mass = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::InitialPoissonMass<Probability>(std::ranges::size(probabilities));
    std::size_t seen = 0;
    for (const auto probability : probabilities)
    {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::UpdatePoissonMass(mass, ++seen, probability, Probability{1} - probability);
    }
    return mass;
}

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ProbabilityRange Range>
[[nodiscard]] auto poisson_binomial_at_least(const Range &probabilities, std::size_t threshold)
{
    using Probability = std::ranges::range_value_t<const Range>;
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ValidatePoissonProbabilities(probabilities);
    const auto size = static_cast<std::size_t>(std::ranges::size(probabilities));
    if (threshold == 0)
    {
        return Probability{1};
    }
    if (threshold > size)
    {
        return Probability{0};
    }

    std::size_t seen = 0;
    const std::size_t failures = size - threshold;
    if (threshold <= failures + 1)
    {
        // k回以上の事象を最後の欄にまとめて、余事象からの引き算を避ける。
        auto mass = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::InitialPoissonMass<Probability>(threshold);
        for (const auto probability : probabilities)
        {
            mass.back() += mass[threshold - 1] * probability;
            /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::UpdatePoissonMass(mass, std::min(++seen, threshold - 1),
                                               probability, Probability{1} - probability);
        }
        return std::min(Probability{1}, mass.back());
    }

    auto mass = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::InitialPoissonMass<Probability>(failures);
    for (const auto probability : probabilities)
    {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::UpdatePoissonMass(mass, std::min(++seen, failures),
                                           Probability{1} - probability, probability);
    }
    return std::min(Probability{1}, std::accumulate(mass.begin(), mass.end(), Probability{}));
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
