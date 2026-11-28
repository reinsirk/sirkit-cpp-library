#include <cstdint>
#include <iterator>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef SIRKIT_MODULAR_SHIFT_MEX_DEFINED
#define SIRKIT_MODULAR_SHIFT_MEX_DEFINED

class ModularShiftMex
{
    struct Progression
    {
        std::uint32_t count;
        std::uint32_t modulus;
        std::uint32_t slope;
        std::uint32_t phase;
    };
    using Runs = std::map<std::uint32_t, std::uint32_t>;

public:
    ModularShiftMex(std::uint32_t count, std::uint32_t modulus, std::int64_t step, std::int64_t start = 0)
        : progression_{count, modulus, Normalize(step, modulus), Normalize(start, modulus)}
    {
    }

    ModularShiftMex(const ModularShiftMex &) = default;

    ModularShiftMex(ModularShiftMex &&other) noexcept(std::is_nothrow_default_constructible_v<Runs>)
        : progression_(other.progression_)
    {
        Swap(other);
    }

    ModularShiftMex &operator=(ModularShiftMex other) noexcept
    {
        Swap(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t sum() const noexcept
    {
        return total_;
    }

    [[nodiscard]] std::uint32_t size() const noexcept
    {
        return size_;
    }

    void clear() noexcept
    {
        runs_.clear();
        size_ = 0;
        total_ = 0;
    }

    bool insert(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before != runs_.end() && before->second > value)
        {
            return false;
        }
        const auto modulus = progression_.modulus;
        const auto next_total = size_ + 1 == modulus
                                    ? std::uint64_t{modulus} * progression_.count
                                    : total_ + AddedContribution(value, before, after);
        const auto next_value = value + 1;
        const bool touch_left = before != runs_.end() && before->second == value;
        const bool touch_right = after != runs_.end() && after->first == next_value;
        if (touch_left)
        {
            if (touch_right)
            {
                before->second = after->second;
                runs_.erase(after);
            }
            else
            {
                before->second = next_value;
            }
        }
        else if (touch_right)
        {
            MoveStart(after, value);
        }
        else
        {
            runs_.emplace_hint(after, value, next_value);
        }
        ++size_;
        total_ = next_total;
        return true;
    }

    bool erase(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before == runs_.end() || before->second <= value)
        {
            return false;
        }
        const auto next_value = value + 1;
        if (before->first == value)
        {
            if (before->second == next_value)
            {
                runs_.erase(before);
            }
            else
            {
                MoveStart(before, next_value);
            }
        }
        else
        {
            if (before->second != next_value)
            {
                runs_.emplace_hint(after, next_value, before->second);
            }
            before->second = value;
        }
        --size_;
        if (size_ + 1 == progression_.modulus)
        {
            total_ = MissingOneSum(value);
        }
        else
        {
            const auto next = runs_.upper_bound(value);
            total_ -= AddedContribution(value, Previous(next), next);
        }
        return true;
    }

private:
    void Swap(ModularShiftMex &other) noexcept
    {
        std::swap(progression_, other.progression_);
        runs_.swap(other.runs_);
        std::swap(size_, other.size_);
        std::swap(total_, other.total_);
    }

    static std::uint32_t Normalize(std::int64_t value, std::uint32_t modulus)
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("ModularShiftMex: zero modulus");
        }
        const auto remainder = value % static_cast<std::int64_t>(modulus);
        return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
    }

    void Check(std::uint32_t value) const
    {
        if (value >= progression_.modulus)
        {
            throw std::out_of_range("ModularShiftMex: residue outside modulus");
        }
    }

    Runs::iterator Previous(Runs::iterator after) noexcept
    {
        return after == runs_.begin() ? runs_.end() : std::prev(after);
    }

    void MoveStart(Runs::iterator run, std::uint32_t start)
    {
        auto node = runs_.extract(run);
        node.key() = start;
        runs_.insert(std::move(node));
    }

    [[nodiscard]] std::uint64_t FloorWithOffset(std::int64_t offset) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        return static_cast<std::uint64_t>(floor_sum(count, modulus, slope, offset + phase));
    }

    [[nodiscard]] std::uint64_t MissingOneSum(std::uint32_t missing) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        const std::uint64_t length = count;
        const auto linear = (length * (length - 1) / 2) * slope + length * (std::uint64_t{phase} + missing);
        return linear - modulus * FloorWithOffset(missing);
    }

    [[nodiscard]] std::uint64_t AddedContribution(
        std::uint32_t value, Runs::const_iterator before, Runs::const_iterator after) const
    {
        auto left = static_cast<std::int64_t>(value);
        auto right = left + 1;
        if (before != runs_.end() && before->second == value)
        {
            left = before->first;
        }
        if (after != runs_.end() && after->first == value + 1)
        {
            right = after->second;
        }
        const auto modulus = progression_.modulus;
        if (!runs_.empty())
        {
            if (left == 0 && runs_.rbegin()->second == modulus)
            {
                left -= modulus - runs_.rbegin()->first;
            }
            if (right == modulus && runs_.begin()->first == 0)
            {
                right += runs_.begin()->second;
            }
        }

        // 左端からvalueの間に0が移動した回だけ、mexの終点がvalueからrightへ動く。
        const auto occurrences = FloorWithOffset(value) - FloorWithOffset(left - 1);
        return occurrences * static_cast<std::uint64_t>(right - value);
    }

    Progression progression_;
    Runs runs_; // [first,second)の占有区間。円環をまたぐときは端の2区間に分ける。
    std::uint32_t size_ = 0;
    std::uint64_t total_ = 0;
};

#endif
