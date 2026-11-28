// SPDX-License-Identifier: CC0-1.0
#include "Math/Todd_Coxeter.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <random>
#include <span>
#include <utility>
#include <vector>

namespace
{
using Transformation = std::array<int, 3>;

void check_transformations(std::span<const Transformation> generators, std::mt19937 &random)
{
    const Transformation identity{0, 1, 2};
    std::map<Transformation, std::size_t> index{{identity, 0}};
    std::vector<Transformation> transforms{identity};
    std::vector<std::vector<int>> representative(1);
    std::vector<std::vector<std::size_t>> transitions;
    std::vector<sirkit::MonoidRelation> relations;
    for (std::size_t i = 0; i < transforms.size(); ++i)
    {
        std::vector<std::size_t> row;
        for (std::size_t g = 0; g < generators.size(); ++g)
        {
            Transformation product{};
            for (std::size_t j = 0; j < product.size(); ++j)
            {
                product[j] = generators[g][transforms[i][j]];
            }
            auto word = representative[i];
            word.push_back(static_cast<int>(g));
            const auto [entry, fresh] = index.emplace(product, transforms.size());
            if (fresh)
            {
                transforms.push_back(product);
                representative.push_back(word);
            }
            const auto target = entry->second;
            row.push_back(target);
            relations.push_back({std::move(word), representative[target]});
        }
        transitions.push_back(std::move(row));
    }
    // A right multiplication table presents a finite monoid by the relations r_v*g=r_{v*g}.
    // The generated words are independent of the library's enumerated vertex numbering.
    std::shuffle(relations.begin(), relations.end(), random);
    for (auto &[left, right] : relations)
    {
        if (random() % 2 == 0)
        {
            left.swap(right);
        }
    }
    const auto graph = sirkit::todd_coxeter(generators.size(), relations, 100000);
    assert(graph && graph->size() == transforms.size());
    assert(graph->generators() == generators.size());
    std::vector<int> numbers;
    for (const auto &word : representative)
    {
        numbers.push_back(graph->evaluate(word));
    }
    auto sorted_numbers = numbers;
    std::sort(sorted_numbers.begin(), sorted_numbers.end());
    for (std::size_t i = 0; i < sorted_numbers.size(); ++i)
    {
        assert(sorted_numbers[i] == static_cast<int>(i));
    }
    assert(numbers.front() == 0);
    for (std::size_t i = 0; i < transitions.size(); ++i)
    {
        for (std::size_t g = 0; g < generators.size(); ++g)
        {
            const std::array word{static_cast<int>(g)};
            const auto expected = numbers[transitions[i][g]];
            assert(graph->transition(numbers[i], word[0]) == expected);
            assert(graph->evaluate(word, numbers[i]) == expected);
        }
    }
}

template <class Exception, class Function>
void expect_failure(Function action)
{
    try
    {
        action();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    using sirkit::MonoidRelation;
    using sirkit::todd_coxeter;
    const auto trivial = todd_coxeter(0, {}, 1);
    assert(trivial && trivial->size() == 1 && trivial->evaluate({}) == 0);
    assert(!todd_coxeter(0, {}, 0));
    assert(!todd_coxeter(2, {}, 40));

    const std::array<MonoidRelation, 3> braid{{{{0, 0}, {}}, {{1, 1}, {}}, {{0, 1, 0}, {1, 0, 1}}}};
    const auto s3 = todd_coxeter(2, braid, 100);
    assert(s3 && s3->size() == 6);
    assert(s3->evaluate(std::array{0, 1, 0}) == s3->evaluate(std::array{1, 0, 1}));
    assert(s3->evaluate(std::array{0, 1}) != s3->evaluate(std::array{1, 0}));
    assert(s3->evaluate(std::array{0, 1, 0, 1, 0, 1}) == 0);
    const std::array<MonoidRelation, 3> collapse{{{std::vector<int>(26000), {}}, {{0}, {}}, {{1}, {}}}};
    const auto one = todd_coxeter(2, collapse, 28000);
    assert(one && one->size() == 1);
    assert(!todd_coxeter(2, collapse, 2000));
    assert(one->evaluate(std::array{1, 0, 1}) == 0);

    std::mt19937 random(6127);
    for (int test = 0; test < 500; ++test)
    {
        std::vector<Transformation> generators(random() % 6);
        for (auto &function : generators)
        {
            for (auto &point : function)
            {
                point = static_cast<int>(random() % 3);
            }
        }
        check_transformations(generators, random);
    }

    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)s3->evaluate({}, -1);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)s3->transition(6, 0);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)s3->transition(0, -1);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)s3->evaluate(std::array{2});
                                      });
    const auto too_many = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
    expect_failure<std::length_error>([&]
                                      {
                                          (void)todd_coxeter(too_many, {}, 1);
                                      });
    expect_failure<std::length_error>([&]
                                      {
                                          (void)todd_coxeter(1, {}, too_many);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)todd_coxeter(1, std::array<MonoidRelation, 1>{{{{-1}, {}}}}, 10);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          (void)todd_coxeter(1, std::array<MonoidRelation, 1>{{{{}, {1}}}}, 10);
                                      });
}
