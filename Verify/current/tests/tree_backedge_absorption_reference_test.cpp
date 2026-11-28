// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Backedge_Absorption.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kPrime = 998244353;
using Edge = std::pair<int, int>;

std::uint64_t power(std::uint64_t value, std::uint32_t exponent, std::uint32_t modulus)
{
    std::uint64_t result = 1;
    while (exponent != 0)
    {
        if (exponent & 1)
        {
            result = result * value % modulus;
        }
        value = value * value % modulus;
        exponent >>= 1;
    }
    return result;
}

template <std::uint32_t Modulus>
struct Field
{
    std::uint32_t residue{};
    explicit Field(std::uint64_t value = 0) : residue(static_cast<std::uint32_t>(value % Modulus))
    {
    }
    Field &operator+=(Field other)
    {
        *this = Field(std::uint64_t{residue} + other.residue);
        return *this;
    }
    friend Field operator*(Field a, Field b)
    {
        return Field(std::uint64_t{a.residue} * b.residue);
    }
    friend Field operator-(Field a, Field b)
    {
        return Field(std::uint64_t{a.residue} + Modulus - b.residue);
    }
    friend Field operator/(Field a, Field b)
    {
        assert(b.residue != 0);
        return a * Field(power(b.residue, Modulus - 2, Modulus));
    }
    friend bool operator==(Field, Field) = default;
};

std::optional<std::uint64_t> solve_matrix(const std::vector<int> &parents, const std::vector<Edge> &back,
                                          const std::vector<std::uint32_t> &costs, std::uint32_t mod)
{
    const int n = static_cast<int>(parents.size());
    std::vector<std::vector<int>> outgoing(n);
    const int root = static_cast<int>(std::find(parents.begin(), parents.end(), -1) - parents.begin());
    for (int v = 0; v < n; ++v)
    {
        if (parents[v] != -1)
        {
            outgoing[parents[v]].push_back(v);
        }
    }
    for (const auto &[from, to] : back)
    {
        outgoing[from].push_back(to);
    }
    std::vector matrix(n, std::vector<std::uint64_t>(n + 1));
    for (int v = 0; v < n; ++v)
    {
        const std::uint64_t degree = outgoing[v].size();
        if (degree == 0)
        {
            matrix[v][v] = 1;
            continue;
        }
        matrix[v][v] = degree % mod;
        matrix[v][n] = degree * costs[v] % mod;
        for (const int destination : outgoing[v])
        {
            matrix[v][destination] = (matrix[v][destination] + mod - 1) % mod;
        }
    }
    for (int col = 0; col < n; ++col)
    {
        int row = col;
        while (row < n && matrix[row][col] == 0)
        {
            ++row;
        }
        if (row == n)
        {
            return std::nullopt;
        }
        std::swap(matrix[col], matrix[row]);
        const auto inv = power(matrix[col][col], mod - 2, mod);
        for (int j = col; j <= n; ++j)
        {
            matrix[col][j] = matrix[col][j] * inv % mod;
        }
        for (int i = 0; i < n; ++i)
        {
            if (i == col)
            {
                continue;
            }
            const auto factor = matrix[i][col];
            for (int j = col; j <= n; ++j)
            {
                matrix[i][j] = (matrix[i][j] + mod - factor * matrix[col][j] % mod) % mod;
            }
        }
    }
    return matrix[root][n];
}

void check(const std::vector<int> &parents, const std::vector<Edge> &back, const std::vector<std::uint32_t> &costs)
{
    const sirkit::TreeBackedgeAbsorption walk(parents, back);
    const std::vector<Field<kPrime>> values(costs.begin(), costs.end());
    const auto actual = walk.expected_cost<Field<kPrime>>(values);
    const auto expected = solve_matrix(parents, back, costs, kPrime);
    assert(actual && expected && actual->residue == *expected);

    const auto steps = walk.expected_steps<Field<kPrime>>();
    const auto all_one = solve_matrix(parents, back, std::vector<std::uint32_t>(parents.size(), 1), kPrime);
    assert(steps && all_one && steps->residue == *all_one);

    const auto mod_small = walk.expected_cost<Field<2>>(
        std::vector<Field<2>>(costs.begin(), costs.end()));
    if (mod_small)
    {
        const auto oracle = solve_matrix(parents, back, costs, 2);
        assert(oracle && mod_small->residue == *oracle);
    }
}

template <class Error, class Action>
void expect_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}

void check_singular_prime()
{
    constexpr int kPower = 30;
    std::vector<int> parents(kPower + 2, -1);
    std::iota(parents.begin() + 1, parents.end(), 0);
    std::vector<Edge> resets;
    for (int vertex = 1; vertex <= kPower; ++vertex)
    {
        const bool terminal = ((kPrime - 1U) >> (kPower - vertex)) & 1;
        if (terminal)
        {
            parents.push_back(vertex);
        }
        else
        {
            resets.emplace_back(vertex, 0);
        }
    }
    const sirkit::TreeBackedgeAbsorption walk(parents, resets);
    assert(!walk.expected_steps<Field<kPrime>>());

    constexpr std::uint32_t kFallbackPrime = 1000000007;
    const auto actual = walk.expected_steps<Field<kFallbackPrime>>();
    const auto expected = ((3ULL << kPower) - 2) % kFallbackPrime * power(kPrime, kFallbackPrime - 2, kFallbackPrime) % kFallbackPrime;
    assert(actual && actual->residue == expected);
    const auto oracle = solve_matrix(parents, resets, std::vector<std::uint32_t>(parents.size(), 1), kFallbackPrime);
    assert(oracle && *oracle == expected);
}
} // namespace

int main()
{
    check({-1}, {}, {123});
    check({2, -1, 1}, {{2, 1}}, {100, 5, 2});
    check({-1, 0, 1, 1, 2, 3, 4, 4, 5}, {{3, 0}, {4, 0}, {4, 2}, {4, 1}, {5, 3}}, {1, 2, 3, 4, 5, 6, 7, 8, 9});
    check({-1, 0, 0}, {}, {1, 100, 1000});
    check_singular_prime();

    const std::array model{-1, 0, 1};
    sirkit::TreeBackedgeAbsorption valid(model);
    const auto copy = valid;
    const auto moved = std::move(valid);
    assert(copy.expected_steps<Field<kPrime>>()->residue == 2);
    assert(moved.expected_steps<Field<kPrime>>()->residue == 2);
    assert(valid.expected_steps<Field<kPrime>>()->residue == 0);
    assert(valid.expected_cost<Field<kPrime>>({})->residue == 0);
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)moved.expected_cost<Field<kPrime>>({});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::TreeBackedgeAbsorption({});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array multiple{-1, -1};
                                            (void)sirkit::TreeBackedgeAbsorption(multiple);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{-1, 2, 1};
                                            (void)sirkit::TreeBackedgeAbsorption(cycle);
                                        });
    expect_error<std::out_of_range>([]
                                    {
                                        const std::array outside{-1, 2};
                                        (void)sirkit::TreeBackedgeAbsorption(outside);
                                    });
    const std::array branched{-1, 0, 0, 1, 2};
    for (const auto &edge : {Edge{1, 1}, Edge{1, 2}, Edge{3, 1}})
    {
        expect_error<std::invalid_argument>([&]
                                            {
                                                const std::array back{edge};
                                                (void)sirkit::TreeBackedgeAbsorption(branched, back);
                                            });
    }
    expect_error<std::out_of_range>([&]
                                    {
                                        const std::array back{Edge{2, 15}};
                                        (void)sirkit::TreeBackedgeAbsorption(branched, back);
                                    });

    std::mt19937 random(6189187);
    for (int trial = 0; trial < 500; ++trial)
    {
        const int n = 2 + static_cast<int>(random() % 23);
        std::vector<int> parents(n, -1), indices(n), old_parents(n, -1);
        std::vector<bool> leaf(n, true);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), random);
        for (int vertex = 1; vertex < n; ++vertex)
        {
            const int parent = static_cast<int>(random() % vertex);
            old_parents[vertex] = parent;
            leaf[parent] = false;
            parents[indices[vertex]] = indices[parent];
        }
        std::vector<Edge> back;
        for (int vertex = 1; vertex < n; ++vertex)
        {
            if (leaf[vertex])
            {
                continue;
            }
            for (int ancestor = old_parents[vertex]; ancestor != -1; ancestor = old_parents[ancestor])
            {
                for (int count = static_cast<int>(random() % 4); count > 0; --count)
                {
                    back.emplace_back(indices[vertex], indices[ancestor]);
                }
            }
        }
        std::vector<std::uint32_t> costs(n);
        for (auto &cost : costs)
        {
            cost = static_cast<std::uint32_t>(random() % kPrime);
        }
        check(parents, back, costs);
    }
}
