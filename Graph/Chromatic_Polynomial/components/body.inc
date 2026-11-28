#ifndef SIRKIT_CHROMATIC_POLYNOMIAL_DEFINED
#define SIRKIT_CHROMATIC_POLYNOMIAL_DEFINED

inline std::vector<std::uint32_t> chromatic_polynomial(
    int n, std::span<const std::pair<int, int>> edges)
{
    constexpr int kMaxVertices = 20;
    constexpr auto kMod = detail::kSetSeriesMod;
    if (n < 0 || n > kMaxVertices)
    {
        throw std::length_error("chromatic_polynomial: vertex count must be in [0,20]");
    }
    using Mask = std::uint32_t;
    std::array<Mask, kMaxVertices> adjacent{};
    bool has_loop = false;
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("chromatic_polynomial: vertex out of range");
        }
#endif
        has_loop |= u == v;
        adjacent[u] |= Mask{1} << v;
        adjacent[v] |= Mask{1} << u;
    }
    if (has_loop)
    {
        return std::vector<std::uint32_t>(n + 1);
    }
    if (n == 0)
    {
        return {1};
    }

    const Mask count = Mask{1} << n;
    std::vector<unsigned char> independent(count);
    independent[0] = 1;
    for (Mask mask = 1; mask < count; ++mask)
    {
        const Mask rest = mask & (mask - 1);
        independent[mask] = independent[rest] & ((adjacent[std::countr_zero(mask)] & rest) == 0);
    }

    std::vector<std::uint32_t> partitions(n + 1);
    // 最小頂点を含む独立集合を1色塗る。最初の遷移は残りが全頂点の1状態だけ。
    std::vector<std::uint32_t> remaining(count / 2);
    const auto high_bits = static_cast<Mask>(remaining.size()) - 1;
    for (Mask mask = 0; mask < remaining.size(); ++mask)
    {
        remaining[mask] = independent[2 * (high_bits ^ mask) + 1];
    }
    for (int colors = 1; colors < n; ++colors)
    {
        partitions[colors] = remaining[0];
        // 残りの最小頂点eで分け、eを次の独立集合に入れる。
        // 補集合を取った残り配列との畳み込みで、遷移先Tを含む状態から寄与を集める。
        std::vector<std::uint32_t> next(remaining.size() / 2);
        std::vector<std::uint32_t> eligible(next.size()), reversed(next.size());
        for (int e = colors; e < n; ++e)
        {
            const int shift = e - colors;
            const auto length = next.size() >> shift;
            const auto full = length - 1;
            for (std::size_t mask = 0; mask < length; ++mask)
            {
                eligible[mask] = independent[(2 * mask + 1) << e];
                reversed[full ^ mask] = remaining[(2 * mask + 1) << shift];
            }
            const auto product = subset_convolution(std::span(eligible).first(length),
                                                    std::span(reversed).first(length));
            for (std::size_t mask = 0; mask < length; ++mask)
            {
                auto &value = next[mask << shift];
                const auto sum = value + product[full ^ mask];
                value = sum >= kMod ? sum - kMod : sum;
            }
        }
        remaining = std::move(next);
    }
    partitions[n] = remaining[0];

    // k個の色クラスに色を割り当てる係数はx(x-1)...(x-k+1)。入れ子にして展開する。
    std::vector<std::uint32_t> coefficients(n + 1);
    coefficients[0] = partitions[n];
    const auto subtract_scaled = [](std::uint32_t a, std::uint32_t b, int scale) {
        return static_cast<std::uint32_t>((a + kMod - std::uint64_t{b} * scale % kMod) % kMod);
    };
    for (int k = n - 1; k >= 0; --k)
    {
        for (int degree = n - k; degree > 0; --degree)
        {
            coefficients[degree] = subtract_scaled(coefficients[degree - 1], coefficients[degree], k);
        }
        coefficients[0] = subtract_scaled(partitions[k], coefficients[0], k);
    }
    return coefficients;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
