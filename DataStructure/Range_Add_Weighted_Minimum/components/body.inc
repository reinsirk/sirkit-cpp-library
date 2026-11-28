#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_RANGE_ADD_WEIGHTED_MINIMUM_DEFINED
#define SIRKIT_RANGE_ADD_WEIGHTED_MINIMUM_DEFINED

template <std::signed_integral T = std::int64_t, class Weight = std::uint64_t>
class RangeAddWeightedMinimum
{
public:
    struct Minimum
    {
        T value;
        Weight weight;
    };

    RangeAddWeightedMinimum(std::span<const T> values, std::span<const Weight> weights)
        : tree_(build(values, weights))
    {
    }

    explicit RangeAddWeightedMinimum(std::span<const Weight> weights)
        : tree_(build(weights))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return tree_.size();
    }

    void add(int first, int last, T delta)
    {
        tree_.apply(first, last, delta);
    }

    [[nodiscard]] std::optional<Minimum> minimum(int first, int last)
    {
        return tree_.prod(first, last);
    }

    [[nodiscard]] std::optional<Minimum> all_minimum()
    {
        return tree_.all_prod();
    }

private:
    using Summary = std::optional<Minimum>;

    static Summary merge(const Summary &left, const Summary &right)
    {
        if (!left)
        {
            return right;
        }
        if (!right)
        {
            return left;
        }
        if (left->value < right->value)
        {
            return left;
        }
        if (right->value < left->value)
        {
            return right;
        }
        return Minimum{left->value, static_cast<Weight>(left->weight + right->weight)};
    }

    static Summary identity() noexcept
    {
        return std::nullopt;
    }

    static Summary translate(T delta, const Summary &result)
    {
        return result.transform([delta](Minimum item)
                                {
                                    item.value += delta;
                                    return item;
                                });
    }

    static T compose(T delta, T previous) noexcept
    {
        return static_cast<T>(delta + previous);
    }

    static T zero() noexcept
    {
        return T{};
    }

    static std::vector<Summary> build(std::span<const T> values, std::span<const Weight> weights)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (values.size() != weights.size())
        {
            throw std::invalid_argument("RangeAddWeightedMinimum: value and weight counts");
        }
#endif
        std::vector<Summary> result;
        result.reserve(values.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            result.emplace_back(Minimum{values[i], weights[i]});
        }
        return result;
    }

    static std::vector<Summary> build(std::span<const Weight> weights)
    {
        std::vector<Summary> result;
        result.reserve(weights.size());
        for (const auto &weight : weights)
        {
            result.emplace_back(Minimum{T{}, weight});
        }
        return result;
    }

    lazy_segtree<Summary, merge, identity, T, translate, compose, zero> tree_;
};

#endif
