#ifndef SIRKIT_XOR_CONVOLUTION_WITHOUT_DIVISION_DEFINED
#define SIRKIT_XOR_CONVOLUTION_WITHOUT_DIVISION_DEFINED

template <bool CharacteristicTwo = false, class T>
    requires(!std::same_as<T, bool>)
[[nodiscard]] std::vector<T> xor_convolution_without_division(std::vector<T> a, std::vector<T> b)
{
    const auto size = a.size();
    if (size != b.size())
    {
        throw std::invalid_argument("xor_convolution_without_division: inputs differ in length");
    }
    if (size == 0)
    {
        return {};
    }
    if (!std::has_single_bit(size))
    {
        throw std::invalid_argument("xor_convolution_without_division: input length is not a power of two");
    }

    // x_i²=1 の基底から y_i=1+x_i に替えると y_i²=2*y_i。
    superset_mobius_transform(a);
    superset_mobius_transform(b);
    const auto dimensions = static_cast<std::size_t>(std::countr_zero(size));
    std::vector<std::uint8_t> ranks(size);
    std::vector<std::vector<T>> left(dimensions + 1), right(dimensions + 1);
    for (std::size_t rank = 0; rank <= dimensions; ++rank)
    {
        left[rank].resize(size);
        right[rank].resize(size);
    }
    for (std::size_t mask = 0; mask < size; ++mask)
    {
        const auto rank = static_cast<std::uint8_t>(std::popcount(mask));
        ranks[mask] = rank;
        left[rank][mask] = a[mask];
        right[rank][mask] = b[mask];
        a[mask] = T{};
    }
    for (std::size_t rank = 0; rank <= dimensions; ++rank)
    {
        subset_zeta_transform(left[rank]);
        subset_zeta_transform(right[rank]);
    }

    // 次数の和dの寄与を逆変換後に得て、残った重複数d-|mask|だけ2倍する。
    const auto max_degree = CharacteristicTwo ? dimensions : 2 * dimensions;
    std::vector<T> combined(size);
    for (std::size_t remaining = max_degree + 1; remaining != 0; --remaining)
    {
        const auto degree = remaining - 1;
        const auto first = degree > dimensions ? degree - dimensions : 0U;
        const auto last = std::min(degree, dimensions);
        std::fill(combined.begin(), combined.end(), T{});
        for (std::size_t rank = first; rank <= last; ++rank)
        {
            const auto &l = left[rank];
            auto &r = right[degree - rank];
            for (std::size_t mask = 0; mask < size; ++mask)
            {
                auto product = l[mask];
                detail::bitwise_multiply_assign(product, r[mask]);
                combined[mask] += product;
            }
        }
        subset_mobius_transform(combined);
        for (std::size_t mask = 0; mask < size; ++mask)
        {
            if constexpr (CharacteristicTwo)
            {
                if (ranks[mask] == degree)
                {
                    a[mask] = combined[mask];
                }
            }
            else
            {
                if (ranks[mask] <= degree)
                {
                    a[mask] += a[mask];
                    a[mask] += combined[mask];
                }
            }
        }
    }
    superset_zeta_transform(a);
    return a;
}

#endif
