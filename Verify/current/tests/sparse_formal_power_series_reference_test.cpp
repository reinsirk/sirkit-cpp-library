// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SPARSE_FORMAL_POWER_SERIES_DEFINED
#include "../../../Math/Sparse_Formal_Power_Series.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Term = sirkit::SparseFpsTerm;
using Coefficients = std::vector<std::uint32_t>;
constexpr std::uint64_t kMod = 998244353;

std::uint32_t product(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
}

Coefficients multiply(const Coefficients &a, const Coefficients &b)
{
    Coefficients result(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; i + j < a.size(); ++j)
        {
            result[i + j] = (result[i + j] + product(a[i], b[j])) % kMod;
        }
    }
    return result;
}

std::uint32_t power(std::uint32_t a, std::uint64_t exponent)
{
    if (exponent == 0)
    {
        return 1;
    }
    const auto half = power(a, exponent / 2);
    return product(product(half, half), exponent % 2 == 0 ? 1 : a);
}

Coefficients dense(std::size_t n, const std::vector<Term> &terms)
{
    Coefficients result(n);
    for (const auto &[degree, coefficient] : terms)
    {
        if (degree < n)
        {
            result[degree] = (std::uint64_t{result[degree]} + coefficient) % kMod;
        }
    }
    return result;
}

Coefficients exp_by_definition(const Coefficients &f)
{
    Coefficients result(f.size()), f_power(f.size());
    result[0] = 1;
    f_power[0] = 1;
    std::uint32_t factorial = 1;
    for (std::size_t k = 1; k < f.size(); ++k)
    {
        f_power = multiply(f_power, f);
        factorial = product(factorial, static_cast<std::uint32_t>(k));
        const auto coefficient = power(factorial, kMod - 2);
        for (std::size_t i = 0; i < f.size(); ++i)
        {
            result[i] = (result[i] + product(f_power[i], coefficient)) % kMod;
        }
    }
    return result;
}

Coefficients log_by_definition(Coefficients f)
{
    Coefficients result(f.size()), f_power(f.size());
    f[0] = 0;
    f_power[0] = 1;
    for (std::size_t k = 1; k < f.size(); ++k)
    {
        f_power = multiply(f_power, f);
        const auto coefficient = power(static_cast<std::uint32_t>(k), kMod - 2);
        for (std::size_t i = 0; i < f.size(); ++i)
        {
            const auto term = product(f_power[i], coefficient);
            result[i] = (result[i] + (k % 2 != 0 ? term : kMod - term)) % kMod;
        }
    }
    return result;
}

Coefficients pow_by_definition(Coefficients base, std::uint64_t exponent)
{
    Coefficients result(base.size());
    result[0] = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if (exponent % 2 != 0)
        {
            result = multiply(result, base);
        }
        base = multiply(base, base);
    }
    return result;
}

std::vector<Term> list_terms(const Coefficients &values)
{
    std::vector<Term> terms;
    for (std::size_t index = 0; index < values.size(); ++index)
    {
        if (values[index] != 0)
        {
            terms.push_back({index, values[index]});
        }
    }
    return terms;
}

template <class Exception, class Call>
void expect_failure(Call call)
{
    bool caught = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    using sirkit::sparse_fps_exp;
    using sirkit::sparse_fps_inverse;
    using sirkit::sparse_fps_log;
    using sirkit::sparse_fps_pow;
    using sirkit::sparse_fps_sqrt;
    assert(sparse_fps_inverse(0, {}).empty());
    assert(sparse_fps_exp(0, {}).empty());
    assert(sparse_fps_log(0, {}).empty());
    assert(sparse_fps_pow(0, {}, 0).empty());
    assert(sparse_fps_sqrt(0, {}) == Coefficients{});
    assert(sparse_fps_log(5, std::array{Term{0, 1}}) == Coefficients(5));
    assert(sparse_fps_pow(4, {}, 0) == (Coefficients{1, 0, 0, 0}));
    assert(sparse_fps_pow(4, {}, 12345) == Coefficients(4));
    assert(sparse_fps_pow(4, std::array{Term{2, 1}}, std::numeric_limits<std::uint64_t>::max()) == Coefficients(4));
    assert(sparse_fps_pow(4, std::array{Term{1, 2}, Term{2, 3}}, 3) == (Coefficients{0, 0, 0, 8}));
    assert(sparse_fps_pow(4, std::array{Term{1, 2}}, 2) == (Coefficients{0, 0, 4, 0}));
    assert(sparse_fps_sqrt(2, {}) == Coefficients(2));
    assert(sparse_fps_sqrt(3, std::array{Term{0, 1}}) == (Coefficients{1, 0, 0}));
    assert(sparse_fps_sqrt(3, std::array{Term{2, 4}}) == (Coefficients{0, 2, 0}));
    assert(!sparse_fps_sqrt(2, std::array{Term{1, 1}}));
    assert(!sparse_fps_sqrt(1, std::array{Term{0, 3}}));
    assert(sparse_fps_inverse(5, std::array{Term{0, 1}}) == (Coefficients{1, 0, 0, 0, 0}));
    assert(sparse_fps_exp(5, {}) == (Coefficients{1, 0, 0, 0, 0}));
    assert(sparse_fps_exp(2, std::array{Term{0, 1}, Term{0, kMod - 1}, Term{99, 2}, Term{1, kMod}}) == (Coefficients{1, 0}));
#ifndef SKIP_RUNTIME_CHECKS
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_inverse(1, {});
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_inverse(2, std::array{Term{1, 1}});
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_exp(1, std::array{Term{0, 1}});
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_log(1, {});
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_log(2, std::array{Term{1, 1}});
                                          });
    expect_failure<std::invalid_argument>([]
                                          {
                                              (void)sparse_fps_log(1, std::array{Term{0, 2}});
                                          });
#endif
    expect_failure<std::length_error>([]
                                      {
                                          (void)sparse_fps_exp(kMod + 1, {});
                                      });
    std::mt19937 random(25789);
    for (int iteration = 0; iteration < 310; ++iteration)
    {
        const std::size_t n = 1 + random() % 40;
        std::vector<Term> terms;
        for (std::size_t i = 0; i < random() % 90; ++i)
        {
            terms.push_back({1 + random() % (n + 5), static_cast<std::uint32_t>(random())});
        }
        assert(sparse_fps_exp(n, terms) == exp_by_definition(dense(n, terms)));
        if (iteration % 2 == 0)
        {
            const auto exponent = random() % 5;
            assert(sparse_fps_pow(n, terms, exponent) == pow_by_definition(dense(n, terms), exponent));
        }
        terms.push_back({0, 1u + static_cast<std::uint32_t>(random() % (kMod - 1))});
        const auto f = dense(n, terms);
        assert(multiply(f, sparse_fps_inverse(n, terms)) == exp_by_definition(Coefficients(n)));
        const std::uint64_t exponent = (iteration % 3 == 0 ? kMod - 1 : std::uint64_t{random()} * random());
        assert(sparse_fps_pow(n, terms, exponent) == pow_by_definition(f, exponent));
        terms.back() = {0, 1};
        assert(sparse_fps_log(n, terms) == log_by_definition(dense(n, terms)));

        auto root = f;
        // 平方で次数のずれと上位に残る自由な係数を作る。
        for (std::size_t i = 0; i < n / 5; ++i)
        {
            root[i] = 0;
        }
        const auto square = multiply(root, root);
        auto sq_terms = list_terms(square);
        std::shuffle(sq_terms.begin(), sq_terms.end(), random);
        const auto restored = sparse_fps_sqrt(n, sq_terms);
        assert(restored && multiply(*restored, *restored) == square);
    }

    // 16項ごとの繰り返しの剰余を通る、法−1の密な係数列。
    std::vector<Term> dense_terms;
    for (std::size_t i = 0; i < 40; ++i)
    {
        dense_terms.push_back({i, kMod - 1});
    }
    assert(multiply(dense(48, dense_terms), sparse_fps_inverse(48, dense_terms)) == exp_by_definition(Coefficients(48)));
    dense_terms.erase(dense_terms.begin());
    assert(sparse_fps_exp(48, dense_terms) == exp_by_definition(dense(48, dense_terms)));

    constexpr std::size_t kMaxN = 1'000'000;
    const auto inverse = sparse_fps_inverse(kMaxN, std::array{Term{0, 1}, Term{7, kMod - 1}});
    const auto exponential = sparse_fps_exp(kMaxN, std::array{Term{7, 1}});
    const auto logarithm = sparse_fps_log(kMaxN, std::array{Term{0, 1}, Term{7, kMod - 1}});
    const auto squared = sparse_fps_pow(kMaxN, std::array{Term{0, 1}, Term{7, 1}}, 2);
    const auto restored = sparse_fps_sqrt(kMaxN, std::array{Term{4, 1}, Term{11, 2}, Term{18, 1}});
    assert(restored && restored->size() == kMaxN);
    std::uint32_t factorial = 1;
    for (std::size_t i = 0; i < kMaxN; ++i)
    {
        assert(inverse[i] == (i % 7 == 0 ? 1 : 0));
        if (i % 7 == 0 && i != 0)
        {
            factorial = product(factorial, static_cast<std::uint32_t>(i / 7));
        }
        assert(i % 7 == 0 ? product(factorial, exponential[i]) == 1 : exponential[i] == 0);
        assert(i != 0 && i % 7 == 0 ? product(static_cast<std::uint32_t>(i / 7), logarithm[i]) == kMod - 1
                                    : logarithm[i] == 0);
        assert(squared[i] == (i == 0 || i == 14 ? 1 : (i == 7 ? 2 : 0)));
        assert((*restored)[i] == (i == 2 || i == 9 ? 1 : 0));
    }
}
