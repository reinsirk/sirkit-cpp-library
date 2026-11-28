// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <optional>

namespace
{
struct Ratio
{
    std::uint64_t numerator;
    std::uint64_t denominator;

    friend bool operator<(const Ratio &a, const Ratio &b)
    {
        return a.numerator * b.denominator < b.numerator * a.denominator;
    }
};

struct Piece
{
    int position;
    std::uint64_t value = 0;
    bool increasing;

    Ratio operator()(int x) const
    {
        return {static_cast<std::uint64_t>(increasing ? x - position : position - x), value};
    }
};
}

int main()
{
    using Envelope = sirkit::OfflineLiChaoTree<int, Piece>;
    using Ids = std::array<Envelope::id_type, 2>;
    Envelope timeline;
    std::map<int, std::optional<Ids>> active;
    int q = 0;
    verify_in >> q;
    while (q-- > 0)
    {
        int type = 0, x = 0;
        verify_in >> type >> x;
        if (type == 1)
        {
            std::uint64_t value = 0;
            verify_in >> value;
            auto &ids = active[x];
            if (value != 0)
            {
                ids = Ids{timeline.add_segment(0, x, {x, value, false}),
                          timeline.add_segment(x + 1, 1'000'000'001, {x, value, true})};
            }
        }
        else if (type == 2)
        {
            const auto it = active.find(x);
            if (it->second)
            {
                for (const auto id : *it->second)
                {
                    timeline.erase(id);
                }
            }
            active.erase(it);
        }
        else
        {
            timeline.query(x);
        }
    }
    verify_out.fixed(12);
    for (const auto &answer : timeline.solve())
    {
        const auto efficiency = answer ? static_cast<double>(answer->denominator) / static_cast<double>(answer->numerator) : 0.0;
        verify_out << efficiency << '\n';
    }
}
