// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: DataStructure/Weighted_Slope_Trick.cpp
// 参考（重み付き折点による表現）: https://nyaannyaan.github.io/library/data-structure/slope-trick-weighted.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_slope_trick.md
//
// API一覧
// WeightedSlopeTrick(): 凸区分線形関数f=0。折点数をKとする。
// add_left_hinge(a,w=1), add_right_hinge(a,w=1): w*max(a-x,0), w*max(x-a,0)を加える。
// add_abs(a,w=1): w*|x-a|を加える。移す折点がC個なら償却O((C+1)log(K+2))時間、一時領域O(C+1)。
// add_constant(c), min_value(), argmin(): 定数加算、最小値、最小点の閉区間。端がnulloptなら、その側は無限。
// shift(a), shift(a,b): f(x-a)、またはmin_{x-b<=y<=x-a} f(y)へ変換。a<=b。いずれもO(1)。
// clear_left(), clear_right(): min_{x<=y} f(y)、min_{y<=x} f(y)。消す折点数に比例。
// eval(x): f(x)をO(K+1)で求める。格納領域は折点数の過去最大に比例。重みはuint64_t、座標と値はint64_t。
// 整数範囲や確保に失敗した更新では関数を保つ。shiftの順序検査のみSKIP_RUNTIME_CHECKSで省略。
// コピーはO(K+1)で独立。moveはO(1)で移動元はf=0。swapはO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <limits>
#include <optional>
#include <vector>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_WEIGHTED_SLOPE_TRICK_DEFINED
#define SIRKIT_WEIGHTED_SLOPE_TRICK_DEFINED

class WeightedSlopeTrick
{
    using Value = std::int64_t;
    using Weight = std::uint64_t;
    struct Knot
    {
        Value position;
        Weight weight;
    };

    struct Knots
    {
        std::vector<Knot> values;
        Value opposite = 0;
    };
    static constexpr Value kMin = std::numeric_limits<Value>::min();
    static constexpr Value kMax = std::numeric_limits<Value>::max();

public:
    struct Argmin
    {
        std::optional<Value> lower, upper;
    };

    WeightedSlopeTrick() = default;
    WeightedSlopeTrick(const WeightedSlopeTrick &) = default;
    WeightedSlopeTrick(WeightedSlopeTrick &&other) noexcept
    {
        swap(other);
    }

    WeightedSlopeTrick &operator=(WeightedSlopeTrick other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(WeightedSlopeTrick &other) noexcept
    {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(left_shift_, other.left_shift_);
        std::swap(right_shift_, other.right_shift_);
        std::swap(minimum_, other.minimum_);
    }

    void add_left_hinge(Value x, Weight weight = 1)
    {
        AddHinges(x, weight, 0);
    }

    void add_right_hinge(Value x, Weight weight = 1)
    {
        AddHinges(x, 0, weight);
    }

    void add_abs(Value x, Weight weight = 1)
    {
        AddHinges(x, weight, weight);
    }

    void add_constant(Value value)
    {
        minimum_ = Add(minimum_, value);
    }

    [[nodiscard]] Value min_value() const noexcept
    {
        return minimum_;
    }

    [[nodiscard]] Argmin argmin() const noexcept
    {
        Argmin range;
        if (!left_.values.empty())
        {
            range.lower = left_.values.front().position + left_shift_;
        }
        if (!right_.values.empty())
        {
            range.upper = right_.values.front().position + right_shift_;
        }
        return range;
    }

    void shift(Value amount)
    {
        shift(amount, amount);
    }

    void shift(Value lower, Value upper)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (lower > upper)
        {
            throw std::invalid_argument("WeightedSlopeTrick: invalid shift interval");
        }
#endif
        const auto next_left = ShiftedOffset(left_, left_shift_, lower);
        const auto next_right = ShiftedOffset(right_, right_shift_, upper);
        left_shift_ = next_left;
        right_shift_ = next_right;
    }

    void clear_left() noexcept
    {
        left_ = Knots{};
        left_shift_ = 0;
    }

    void clear_right() noexcept
    {
        right_ = Knots{};
        right_shift_ = 0;
    }

    [[nodiscard]] Value eval(Value x) const
    {
        auto value = minimum_;
        for (const auto &[key, weight] : left_.values)
        {
            const auto position = key + left_shift_;
            if (position > x)
            {
                value = Raise(value, Distance(position, x), weight);
            }
        }
        for (const auto &[key, weight] : right_.values)
        {
            const auto position = key + right_shift_;
            if (position < x)
            {
                value = Raise(value, Distance(x, position), weight);
            }
        }
        return value;
    }

private:
    Knots left_, right_;
    Value left_shift_ = 0, right_shift_ = 0, minimum_ = 0;

    [[nodiscard]] static Value Add(Value a, Value b)
    {
        if ((b > 0 && a > kMax - b) || (b < 0 && a < kMin - b))
        {
            throw std::overflow_error("WeightedSlopeTrick: addition out of range");
        }
        return a + b;
    }

    [[nodiscard]] static Value ToKey(Value position, Value shift)
    {
        if ((shift > 0 && position < kMin + shift) || (shift < 0 && position > kMax + shift))
        {
            throw std::overflow_error("WeightedSlopeTrick: key out of range");
        }
        return position - shift;
    }

    [[nodiscard]] static Weight Distance(Value high, Value low) noexcept
    {
        return static_cast<Weight>(high) - static_cast<Weight>(low);
    }

    [[nodiscard]] static Value Raise(Value base, Weight distance, Weight weight)
    {
        const Weight room = Distance(kMax, base);
        if (distance > room / weight)
        {
            throw std::overflow_error("WeightedSlopeTrick: value out of range");
        }
        return static_cast<Value>(static_cast<Weight>(base) + distance * weight);
    }

    [[nodiscard]] static Value ShiftedOffset(const Knots &knots, Value shift, Value amount)
    {
        if (knots.values.empty())
        {
            return 0;
        }
        const auto next = Add(shift, amount);
        (void)Add(knots.values.front().position, next);
        (void)Add(knots.opposite, next);
        return next;
    }

    static void Reserve(Knots &knots, std::size_t more)
    {
        const auto needed = knots.values.size() + more;
        const auto capacity = knots.values.capacity();
        if (needed > capacity)
        {
            knots.values.reserve(std::max(needed, capacity + capacity / 2 + 1));
        }
    }

    template <bool Maximum>
    static constexpr auto Compare = [](const Knot &a, const Knot &b)
    {
        return Maximum ? a.position < b.position : a.position > b.position;
    };

    template <bool Maximum>
    static void Push(Knots &knots, Knot knot) noexcept
    {
        if (knots.values.empty() || Compare<Maximum>(knot, {knots.opposite, 0}))
        {
            knots.opposite = knot.position;
        }
        knots.values.push_back(knot);
        std::push_heap(knots.values.begin(), knots.values.end(), Compare<Maximum>);
    }

    void AddHinges(Value x, Weight left_weight, Weight right_weight)
    {
        if (left_weight == 0 && right_weight == 0)
        {
            return;
        }
        if (right_weight != 0 && !left_.values.empty() && left_.values.front().position + left_shift_ > x)
        {
            Exchange<true>(x, right_weight, left_weight);
        }
        else if (left_weight != 0 && !right_.values.empty() && right_.values.front().position + right_shift_ < x)
        {
            Exchange<false>(x, left_weight, right_weight);
        }
        else
        {
            const Value left_key = left_weight == 0 ? 0 : ToKey(x, left_shift_);
            const Value right_key = right_weight == 0 ? 0 : ToKey(x, right_shift_);
            Reserve(left_, left_weight != 0);
            Reserve(right_, right_weight != 0);
            if (left_weight != 0)
            {
                Push<true>(left_, {left_key, left_weight});
            }
            if (right_weight != 0)
            {
                Push<false>(right_, {right_key, right_weight});
            }
        }
    }

    template <bool ToRight>
    void Exchange(Value x, Weight requested, Weight extra)
    {
        auto &donor = ToRight ? left_ : right_;
        auto &receiver = ToRight ? right_ : left_;
        const Value donor_shift = ToRight ? left_shift_ : right_shift_;
        const Value receiver_shift = ToRight ? right_shift_ : left_shift_;

        // 失敗時に取り出した折点を戻せるよう、移す前と後の値を控える。
        struct Transfer
        {
            Knot original, relocated;
        };
        std::array<Transfer, 4> local;
        std::vector<Transfer> overflow;
        std::size_t local_count = 0;
        const auto for_each = [&](auto process)
        {
            for (std::size_t i = 0; i < local_count; ++i)
            {
                process(local[i]);
            }
            for (const auto &entry : overflow)
            {
                process(entry);
            }
        };
        Weight remaining = requested, partial = 0;
        Value next_minimum = minimum_, partial_key = 0, donor_key = 0, receiver_key = 0;
        try
        {
            while (remaining != 0 && !donor.values.empty())
            {
                const Knot current = donor.values.front();
                const auto position = current.position + donor_shift;
                if (ToRight ? position <= x : position >= x)
                {
                    break;
                }
                const Weight moved = std::min(remaining, current.weight);
                const Weight gap = ToRight ? Distance(position, x) : Distance(x, position);
                next_minimum = Raise(next_minimum, gap, moved);
                const auto key = ToKey(position, receiver_shift);
                remaining -= moved;
                if (moved < current.weight)
                {
                    partial = moved;
                    partial_key = key;
                    break;
                }
                const Transfer entry{current, {key, moved}};
                if (local_count < local.size())
                {
                    local[local_count++] = entry;
                }
                else
                {
                    overflow.push_back(entry);
                }
                std::pop_heap(donor.values.begin(), donor.values.end(), Compare<ToRight>);
                donor.values.pop_back();
            }
            donor_key = ToKey(x, donor_shift);
            if (remaining != 0)
            {
                receiver_key = ToKey(x, receiver_shift);
            }
            // ここで両側を確保し、以降の処理では割り当てが起きないようにする。
            Reserve(donor, 1 + (extra != 0));
            Reserve(receiver, local_count + overflow.size() + (partial != 0) + (remaining != 0));
        }
        catch (...)
        {
            for_each([&](const auto &entry)
                     {
                         Push<ToRight>(donor, entry.original);
                     });
            throw;
        }

        for_each([&](const auto &entry)
                 {
                     Push<!ToRight>(receiver, entry.relocated);
                 });
        if (partial != 0)
        {
            donor.values.front().weight -= partial;
            Push<!ToRight>(receiver, {partial_key, partial});
        }
        Push<ToRight>(donor, {donor_key, requested - remaining});
        if (extra != 0)
        {
            Push<ToRight>(donor, {donor_key, extra});
        }
        if (remaining != 0)
        {
            Push<!ToRight>(receiver, {receiver_key, remaining});
        }
        minimum_ = next_minimum;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
