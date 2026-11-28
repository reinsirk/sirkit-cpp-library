// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Persistent_Affine_Sequence.cpp"

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Rope = sirkit::PersistentAffineSequence<long long>;

void Expect(bool condition)
{
    if (!condition)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

void Check(const Rope &rope, const std::vector<long long> &model)
{
    Expect(rope.size() == model.size() && rope.empty() == model.empty());
    Expect(rope.total() == std::accumulate(model.begin(), model.end(), 0LL));
    Expect(rope.sum(0, rope.size()) == rope.total());
    for (std::size_t i = 0; i < model.size(); ++i)
    {
        Expect(rope.sum(i, i + 1) == model[i]);
    }
}

void RandomHistory()
{
    std::mt19937 random(3103477);
    for (int trial = 0; trial < 90; ++trial)
    {
        const int n = trial % 17;
        std::vector<long long> initial(n);
        for (auto &value : initial)
        {
            value = static_cast<int>(random() % 41) - 20;
        }
        std::vector<Rope> history{Rope(initial)};
        std::vector<std::vector<long long>> models{initial};
        for (int i = 0; i < 200; ++i)
        {
            const std::size_t version = (i % 6 == 0) ? random() % history.size() : history.size() - 1;
            const Rope &base = history[version];
            const auto &model = models[version];
            std::size_t left = random() % (model.size() + 1);
            std::size_t right = random() % (model.size() + 1);
            if (left > right)
            {
                std::swap(left, right);
            }
            auto updated = model;
            Rope next;
            switch (random() % 8)
            {
            case 0:
            {
                const int factor = static_cast<int>(random() % 3) - 1;
                const int shift = static_cast<int>(random() % 31) - 15;
                next = base.affine(left, right, factor, shift);
                for (std::size_t j = left; j < right; ++j)
                {
                    updated[j] = updated[j] * factor + shift;
                }
                break;
            }
            case 1:
                next = base.reverse(left, right);
                std::reverse(updated.begin() + left, updated.begin() + right);
                break;
            case 2:
                next = base.slice(left, right);
                updated = std::vector<long long>(model.begin() + left, model.begin() + right);
                break;
            case 3:
            {
                const auto [a, b] = base.split((left + right) / 2);
                Expect(a.size() == (left + right) / 2);
                next = b.concat(a);
                std::rotate(updated.begin(), updated.begin() + (left + right) / 2, updated.end());
                break;
            }
            case 4:
            {
                const auto other = random() % history.size();
                next = base.replace(left, right, history[other]);
                updated.erase(updated.begin() + left, updated.begin() + right);
                updated.insert(updated.begin() + left, models[other].begin(), models[other].end());
                break;
            }
            case 5:
            {
                const auto other = random() % history.size();
                next = history[other].concat(base);
                updated.insert(updated.begin(), models[other].begin(), models[other].end());
                break;
            }
            case 6:
            {
                const std::size_t other = random() % history.size();
                const std::size_t cap = history[other].size();
                const auto a = std::min(left, cap);
                const auto b = std::min(right, cap);
                next = base.copy_from(a, b, history[other]);
                std::copy(models[other].begin() + a, models[other].begin() + b, updated.begin() + a);
                break;
            }
            case 7:
                next = base;
                Expect(base.sum(left, right) == std::accumulate(model.begin() + left, model.begin() + right, 0LL));
                break;
            }
            Check(next, updated);
            if (next.size() > 65)
            {
                auto excess = next.size() - 41;
                next = next.slice(excess / 3, excess / 3 + 41);
                updated = std::vector<long long>(updated.begin() + excess / 3, updated.begin() + excess / 3 + 41);
                Check(next, updated);
            }
            Check(base, model);
            history.push_back(std::move(next));
            models.push_back(std::move(updated));
        }
    }
}

void LimitsAndCopies()
{
    Rope empty;
    Check(empty, {});
    const std::vector<long long> values{1, 2, 3, 4, 5, 6, 7, 8};
    Rope seq(values);
    auto middle = seq.affine(2, 7, 3, -2).reverse(0, 7);
    Check(middle, {19, 16, 13, 10, 7, 2, 1, 8});
    auto overlapping = middle.replace(1, 5, middle.slice(2, 6));
    Check(overlapping, {19, 13, 10, 7, 2, 2, 1, 8});
    Check(middle, {19, 16, 13, 10, 7, 2, 1, 8});
    Check(seq, values);
    Check(seq.copy_from(0, 0, empty), values);
    Check(seq.copy_from(0, 8, seq), values);
    Check(seq.copy_from(0, 8, middle), {19, 16, 13, 10, 7, 2, 1, 8});
    Check(seq.copy_from(1, 3, middle), {1, 16, 13, 4, 5, 6, 7, 8});
    Check(seq.copy_from(6, 8, middle), {1, 2, 3, 4, 5, 6, 1, 8});
    Check(seq.copy_from(2, 6, middle), {1, 2, 13, 10, 7, 2, 7, 8});
    Check(seq.copy_from(1, 3, Rope(values).reverse(0, 3)), {1, 2, 1, 4, 5, 6, 7, 8});
    Check(seq.copy_from(1, 3, middle.slice(0, 4)), {1, 16, 13, 4, 5, 6, 7, 8});
    Rope moved(std::move(middle));
    Expect(middle.empty());
    Expect(moved.sum(1, 7) == 49);

    const std::vector<long long> zero{0};
    auto shared = Rope(zero);
    for (int i = 0; i < 30; ++i)
    {
        shared = shared.concat(shared);
    }
    const std::size_t length = std::size_t{1} << 30;
    Expect(shared.size() == length && shared.sum(length - 5, length) == 0);
    Expect(shared.affine(length - 1, length, 1, 7).total() == 7);
    ExpectThrow<std::length_error>([&]
                                   {
                                       (void)shared.concat(shared);
                                   });
    ExpectThrow<std::length_error>([&]
                                   {
                                       (void)shared.replace(0, 0, shared);
                                   });
    Expect(shared.total() == 0 && shared.size() == length);
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)seq.reverse(4, 3);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)seq.slice(4, 9);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)empty.split(1);
                                   });
#endif
}
}

int main()
{
    RandomHistory();
    LimitsAndCopies();
}
