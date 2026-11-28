#include "../../../Algorithm/Clamped_Translation.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>

namespace
{
using sirkit::ClampedTranslation;
using Value = std::int64_t;

struct Step
{
    unsigned kind;
    Value amount;
};

ClampedTranslation build(std::span<const Step> steps)
{
    ClampedTranslation result;
    for (const auto &[kind, amount] : steps)
    {
        if (kind == 0)
        {
            result.then_add(amount);
        }
        else if (kind == 1)
        {
            result.then_chmax(amount);
        }
        else
        {
            result.then_chmin(amount);
        }
    }
    return result;
}

Value brute(Value value, std::span<const Step> steps)
{
    for (const auto &[kind, amount] : steps)
    {
        switch (kind)
        {
        case 0:
            value += amount;
            break;
        case 1:
            value = std::max(value, amount);
            break;
        default:
            value = std::min(value, amount);
            break;
        }
    }
    return value;
}

#ifndef SKIP_RUNTIME_CHECKS
template <typename Action>
void expect_overflow(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const std::overflow_error &)
    {
        caught = true;
    }
    assert(caught);
}
#endif
}

int main()
{
    constexpr auto low = std::numeric_limits<Value>::min();
    constexpr auto high = std::numeric_limits<Value>::max();
    const ClampedTranslation identity;
    assert(identity.apply(low) == low);
    assert(identity.apply(high) == high);

    ClampedTranslation rising;
    rising.then_add(high).then_chmin(6);
    assert(rising.apply(high) == 6);
    assert(rising.apply(low) == -1);
    ClampedTranslation falling;
    falling.then_add(low).then_chmax(-6);
    assert(falling.apply(low) == -6);
    assert(falling.apply(high) == -1);

    ClampedTranslation at_least_high;
    at_least_high.then_chmax(high);
    const auto constant = at_least_high.then(rising);
    assert(constant.apply(low) == 6 && constant.apply(high) == 6);
    ClampedTranslation descending_constant;
    descending_constant.then_chmax(9).then_chmin(4).then_add(-3);
    assert(descending_constant.apply(low) == 1 && descending_constant.apply(high) == 1);

    std::mt19937 random(196);
    for (int test = 0; test < 1800; ++test)
    {
        std::array<Step, 30> steps;
        for (auto &step : steps)
        {
            step = Step{static_cast<unsigned>(random() % 3), static_cast<Value>(random() % 151) - 75};
        }
        const auto count = static_cast<std::size_t>(random() % (steps.size() + 1));
        const auto all = std::span<const Step>(steps).first(count);
        const auto mid = static_cast<std::size_t>(random() % (all.size() + 1));
        const auto first = build(all.first(mid));
        const auto second = build(all.subspan(mid));
        const auto chained = first.then(second);
        const auto combined = build(all);
        const auto squared = first.then(first);
        for (Value input = -120; input <= 120; input += 13)
        {
            assert(chained.apply(input) == brute(input, all));
            assert(combined.apply(input) == brute(input, all));
            assert(squared.apply(input) == brute(brute(input, all.first(mid)), all.first(mid)));
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    ClampedTranslation move_high;
    move_high.then_add(1);
    expect_overflow([&] { (void)move_high.apply(high); });
    assert(move_high.apply(high - 1) == high);
    ClampedTranslation move_low;
    move_low.then_add(-1);
    expect_overflow([&] { (void)move_low.apply(low); });
    assert(move_low.apply(low + 1) == low);

    expect_overflow([&] { at_least_high.then_add(1); });
    assert(at_least_high.apply(low) == high);
    ClampedTranslation at_most_low;
    at_most_low.then_chmin(low);
    expect_overflow([&] { at_most_low.then_add(-1); });
    assert(at_most_low.apply(high) == low);

    ClampedTranslation extreme_shift;
    extreme_shift.then_add(high);
    expect_overflow([&] { extreme_shift.then_add(1); });
    assert(extreme_shift.apply(-high) == 0);
#endif
}
