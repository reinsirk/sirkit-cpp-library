#include <cstdint>
#include <span>

#ifndef SIRKIT_RUN_LENGTH_Z_DEFINED
#define SIRKIT_RUN_LENGTH_Z_DEFINED

class RunLengthZ
{
public:
    struct Run
    {
        std::uint8_t byte;
        std::uint64_t count;
        friend constexpr bool operator==(const Run &left, const Run &right) noexcept
        {
            return left.byte == right.byte && left.count == right.count;
        }
    };

    explicit RunLengthZ(std::span<const Run> input)
    {
        std::vector<Run> packed;
        std::uint64_t total = 0;
        for (const auto run : input)
        {
            if (run.count == 0)
            {
                continue;
            }
            if (run.count > std::numeric_limits<std::uint64_t>::max() - total)
            {
                throw std::length_error("RunLengthZ: expanded length overflows");
            }
            total += run.count;
            if (!packed.empty() && packed.back().byte == run.byte)
            {
                packed.back().count += run.count;
            }
            else
            {
                packed.push_back(run);
            }
        }
        if (packed.empty())
        {
            return;
        }
        runs_.reserve(packed.size());
        std::uint64_t end = 0;
        for (const auto run : packed)
        {
            end += run.count;
            runs_.push_back({run, end, 0});
        }

        const auto first = packed.front();
        const auto equal_runs = z_algorithm(std::span<const Run>(packed).subspan(1));
        for (std::size_t i = 0; i < equal_runs.size(); ++i)
        {
            if (packed[i].byte != first.byte || packed[i].count < first.count)
            {
                continue;
            }
            // 最初の連続区間の長さが同じとなる開始位置だけ、次の区間へ進める。
            const auto matched = static_cast<std::size_t>(equal_runs[i]);
            auto matched_bytes = runs_[matched].end - first.count;
            const auto next = i + matched + 1;
            if (next < packed.size() && packed[next].byte == packed[matched + 1].byte)
            {
                matched_bytes += std::min(packed[next].count, packed[matched + 1].count);
            }
            runs_[i].extension = matched_bytes;
        }
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return runs_.empty() ? 0 : runs_.back().end;
    }

    [[nodiscard]] std::uint64_t prefix_at(std::uint64_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position > size())
        {
            throw std::out_of_range("RunLengthZ: position outside text");
        }
#endif
        if (position == size())
        {
            return 0;
        }
        const auto it = std::upper_bound(runs_.begin(), runs_.end(), position,
                                         [](std::uint64_t value, const Summary &run) { return value < run.end; });
        const auto first = runs_.front().run;
        if (it->run.byte != first.byte)
        {
            return 0;
        }
        const auto remaining = it->end - position;
        return remaining == first.count ? first.count + it->extension : std::min(remaining, first.count);
    }

    [[nodiscard]] std::uint32_t sum_prefix_lengths(std::uint32_t modulus = 1'000'000'007) const
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("RunLengthZ: modulus must be positive");
        }
        const auto product = [&](std::uint64_t left, std::uint64_t right) {
            return (left % modulus) * (right % modulus) % modulus;
        };
        std::uint64_t result = 0;
        for (const auto &summary : runs_)
        {
            const auto first = runs_.front().run;
            if (summary.run.byte != first.byte)
            {
                continue;
            }
            const auto common = std::min(summary.run.count, first.count);
            const auto triangle = product(common / 2 + (common % 2), common | std::uint64_t{1});
            const auto plateau = product(summary.run.count - common, common);
            result = (result + triangle + plateau + summary.extension % modulus) % modulus;
        }
        return static_cast<std::uint32_t>(result);
    }

private:
    struct Summary
    {
        Run run;
        std::uint64_t end;
        std::uint64_t extension;
    };
    std::vector<Summary> runs_;
};

#endif
