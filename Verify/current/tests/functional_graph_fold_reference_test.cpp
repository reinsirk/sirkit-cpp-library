// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Functional_Graph_Fold.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
struct Affine
{
    std::uint64_t factor;
    std::uint64_t add;
    Affine(std::uint64_t a, std::uint64_t b) : factor(a), add(b)
    {
    }
    bool operator==(const Affine &) const = default;
};

struct Compose
{
    Affine operator()(const Affine &a, const Affine &b) const
    {
        constexpr std::uint64_t kMod = 998244353;
        return {(b.factor * a.factor) % kMod, (b.factor * a.add + b.add) % kMod};
    }
};

using Sum = sirkit::FunctionalGraphFold<std::uint64_t>;

void check_random()
{
    std::mt19937 rng(241);
    const Affine identity{1, 0};
    const Compose compose;
    for (int trial = 0; trial < 700; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 31);
        std::vector<int> next(n);
        std::vector<Affine> label;
        for (int i = 0; i < n; ++i)
        {
            next[i] = static_cast<int>(rng() % n);
            label.emplace_back(rng() % 11, rng() % 13);
        }
        const sirkit::FunctionalGraphFold<Affine, Compose> walk(next, label, identity, compose);
        assert(walk.size() == n);
        for (int query = 0; query < 50; ++query)
        {
            const int source = static_cast<int>(rng() % n);
            const unsigned steps = rng() % 531;
            int v = source;
            auto result = identity;
            for (unsigned i = 0; i < steps; ++i)
            {
                result = compose(result, label[v]);
                v = next[v];
            }
            assert((walk.walk(source, steps) == std::pair{v, result}));
        }
    }
}

void check_boundaries()
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const Sum empty({}, {}, 0);
    assert(empty.size() == 0);

    const std::array next{1, 2, 1};
    const std::array<std::uint64_t, 3> values{7, 2, 3};
    const Sum walk(next, values, 0);
    assert((walk.walk(0, 0) == std::pair{0, std::uint64_t{0}}));
    assert((walk.walk(0, 3) == std::pair{1, std::uint64_t{12}}));
    assert((walk.walk(0, kMax) == std::pair{1, 7 + ((kMax - 1) / 2) * 5}));
    assert((walk.walk(2, kMax) == std::pair{1, 3 + ((kMax - 1) / 2) * 5}));

    const std::array stay{0};
    const std::array<std::uint64_t, 1> value{15};
    const Sum fixed(stay, value, 0);
    assert((fixed.walk(0, kMax) == std::pair{0, kMax * 15}));

    const std::array exact_n{1, 2, 3, 0};
    const std::array<std::uint64_t, 4> ring{1, 2, 4, 8};
    const Sum round(exact_n, ring, 0);
    assert((round.walk(0, 4) == std::pair{0, std::uint64_t{15}}));
    assert((round.walk(1, 6) == std::pair{3, std::uint64_t{21}}));
}

template <class Exception, class Invoke>
void must_throw(Invoke invoke)
{
    bool caught = false;
    try
    {
        invoke();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_invalid()
{
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::detail::functional_graph_fold_vertex_count(
                                          std::size_t{1} + std::numeric_limits<int>::max()));
                                  });
    const std::array next{1, 0};
    const std::array<std::uint64_t, 2> values{2, 4};
    must_throw<std::invalid_argument>([&]
                                      {
                                          static_cast<void>(Sum(next, {}, 0));
                                      });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int invalid : {-1, 2})
    {
        const std::array bad{0, invalid};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(Sum(bad, values, 0));
                                      });
        const Sum walk(next, values, 0);
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(walk.walk(invalid, 0));
                                      });
    }
#endif
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        const std::array next{0};
        static_cast<void>(Sum(next, {}, 0));
        return 1;
    }
    check_boundaries();
    check_random();
    check_invalid();
}
