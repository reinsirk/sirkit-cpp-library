// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Exact_Length_Walk_Fold.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::ExactLengthWalkArc;
using sirkit::fold_exact_length_walks;

template <typename T, typename Add, typename Multiply>
std::vector<T> linear_walk_length(int n, const std::vector<ExactLengthWalkArc<T>> &edges, int source,
                                  int k, T zero, T one, Add add, Multiply multiply)
{
    std::vector<std::vector<T>> length(k + 1, std::vector<T>(n, zero));
    length[0][source] = one;
    for (int step = 0; step < k; ++step)
    {
        for (const auto &edge : edges)
        {
            length[step + 1][edge.to] = add(length[step + 1][edge.to], multiply(length[step][edge.from], edge.weight));
        }
    }
    return length.back();
}

template <typename Error, typename Action>
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

using Matrix = std::array<unsigned, 4>;
constexpr Matrix kZero{};
constexpr Matrix kOne{1, 0, 0, 1};

Matrix add_matrices(const Matrix &u, const Matrix &v)
{
    return {(u[0] + v[0]) % 17, (u[1] + v[1]) % 17, (u[2] + v[2]) % 17, (u[3] + v[3]) % 17};
}

Matrix multiply_matrices(const Matrix &u, const Matrix &v)
{
    return {(u[0] * v[0] + u[1] * v[2]) % 17, (u[0] * v[1] + u[1] * v[3]) % 17,
            (u[2] * v[0] + u[3] * v[2]) % 17, (u[2] * v[1] + u[3] * v[3]) % 17};
}
} // namespace

int main()
{
    assert(sirkit::detail::walk_fold_matrix_area(0, 0) == 0);
    assert(sirkit::detail::walk_fold_matrix_area(5, 25) == 25);
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::detail::walk_fold_matrix_area(5, 24);
                                    });

    const auto plus = std::plus<unsigned>{};
    const auto times = std::multiplies<unsigned>{};
    assert((fold_exact_length_walks(4, {}, 1, 0, 0U, 1U, plus, times) == std::vector<unsigned>{0, 1, 0, 0}));
    assert((fold_exact_length_walks(2, {}, 1, 50, 0U, 1U, plus, times) == std::vector<unsigned>{0, 0}));
    const std::vector<ExactLengthWalkArc<unsigned>> directed{{0, 1, 2}, {0, 1, 3}, {1, 2, 4}, {2, 2, 1}};
    for (const int steps : {2, 4, 5, 6, 31})
    {
        assert((fold_exact_length_walks(3, directed, 0, steps, 0U, 1U, plus, times) ==
                std::vector<unsigned>{0, 0, 20}));
    }

    const std::vector<ExactLengthWalkArc<bool>> alternating{{0, 1, true}, {1, 0, true}, {0, 0, false}};
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const auto either = std::logical_or<bool>{};
    const auto both = std::logical_and<bool>{};
    assert((fold_exact_length_walks(3, alternating, 0, kMax, false, true, either, both) ==
            std::vector<bool>{false, true, false}));
    assert((fold_exact_length_walks(3, alternating, 0, kMax - 1, false, true, either, both) ==
            std::vector<bool>{true, false, false}));
    assert((fold_exact_length_walks(3, alternating, 0, 1, false, true, either, both) ==
            std::vector<bool>{false, true, false}));

    const Matrix a{1, 2, 0, 1};
    const Matrix b{1, 0, 3, 1};
    assert(multiply_matrices(a, b) != multiply_matrices(b, a));
    const std::vector<ExactLengthWalkArc<Matrix>> matrix_arcs{{0, 1, a}, {1, 2, b}, {2, 0, a}, {0, 0, b}, {1, 1, kOne}};
    for (int step = 0; step < 70; ++step)
    {
        const auto actual = fold_exact_length_walks(3, matrix_arcs, 0, step, kZero, kOne,
                                                    add_matrices, multiply_matrices);
        assert(actual == linear_walk_length(3, matrix_arcs, 0, step, kZero, kOne, add_matrices, multiply_matrices));
    }

    constexpr unsigned kInf = 1U << 30;
    const auto minimum = [](unsigned x, unsigned y)
    {
        return std::min(x, y);
    };
    const auto maximum = [](unsigned x, unsigned y)
    {
        return std::max(x, y);
    };
    std::mt19937 random(23605385);
    for (int trial = 0; trial < 850; ++trial)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        const auto m = random() % (n * n * 2 + 1);
        std::vector<ExactLengthWalkArc<unsigned>> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             static_cast<unsigned>(random() % 19)});
        }
        const int source = static_cast<int>(random() % n);
        const int step = static_cast<int>(random() % (4 * n + 1));
        assert(fold_exact_length_walks(n, edges, source, step, 0U, 1U, plus, times) ==
               linear_walk_length(n, edges, source, step, 0U, 1U, plus, times));
        assert(fold_exact_length_walks(n, edges, source, step, kInf, 0U, minimum, maximum) ==
               linear_walk_length(n, edges, source, step, kInf, 0U, minimum, maximum));
    }

    expect_error<std::length_error>([&]
                                    {
                                        (void)fold_exact_length_walks(-1, directed, 0, 0, 0U, 1U, plus, times);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 3})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)fold_exact_length_walks(3, directed, bad, 0, 0U, 1U, plus, times);
                                        });
        const std::vector<ExactLengthWalkArc<unsigned>> from{{bad, 0, 1}}, to{{0, bad, 1}};
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)fold_exact_length_walks(3, from, 0, 0, 0U, 1U, plus, times);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)fold_exact_length_walks(3, to, 0, 3, 0U, 1U, plus, times);
                                        });
    }
#endif
}
