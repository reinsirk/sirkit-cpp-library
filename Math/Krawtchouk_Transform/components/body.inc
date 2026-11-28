#ifndef SIRKIT_KRAWTCHOUK_TRANSFORM_DEFINED
#define SIRKIT_KRAWTCHOUK_TRANSFORM_DEFINED

[[nodiscard]] inline FormalPowerSeries krawtchouk_transform(const FormalPowerSeries &series)
{
    const auto shifted = TaylorShift(series, 1);
    std::vector<std::uint32_t> mirror(shifted.size());
    constexpr auto kMod = FormalPowerSeries::modulus();
    std::uint32_t scale = 1;
    for (std::size_t i = 0; i < mirror.size(); ++i)
    {
        mirror[mirror.size() - 1 - i] = detail::fps_multiply(shifted[i], scale);
        scale = detail::fps_multiply(scale, kMod - 2);
    }
    // ((1-x)/(1+x))-1=-2x/(1+x)。係数を逆順にしてから (x+1) のべきに直す。
    const auto composed = TaylorShift(FormalPowerSeries(std::move(mirror)), 1);
    return FormalPowerSeries(std::vector(composed.coefficients().rbegin(), composed.coefficients().rend()));
}

[[nodiscard]] inline FormalPowerSeries inverse_krawtchouk_transform(const FormalPowerSeries &series)
{
    if (series.empty())
    {
        return {};
    }
    constexpr auto kHalf = (FormalPowerSeries::modulus() + 1) / 2;
    return krawtchouk_transform(series).scale(detail::fps_power(kHalf, series.size() - 1));
}

#endif
