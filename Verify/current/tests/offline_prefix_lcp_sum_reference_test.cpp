// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_OFFLINE_PREFIX_LCP_SUM_DEFINED
#include "../../../String/Offline_Prefix_Lcp_Sum.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace
{
void check(std::span<const std::string_view> initial, std::span<const sirkit::PrefixLcpOperation> operations)
{
    std::vector<std::string> active(initial.begin(), initial.end());
    std::vector<std::uint64_t> result;
    for (const auto &operation : operations)
    {
        if (const auto *append = std::get_if<sirkit::PrefixLcpAppend>(&operation))
        {
            active[append->index].push_back(static_cast<char>(append->byte));
            continue;
        }
        const auto &target = active[std::get<sirkit::PrefixLcpQuery>(operation).index];
        std::uint64_t total = 0;
        for (const auto &word : active)
        {
            const auto [first, ignored] = std::mismatch(target.begin(), target.end(), word.begin(), word.end());
            (void)ignored;
            total += static_cast<std::uint64_t>(first - target.begin());
        }
        result.push_back(total);
    }
    assert(sirkit::offline_prefix_lcp_sum(initial, operations) == result);
}

template<class Exception, class Function>
void check_error(Function run)
{
    bool threw = false;
    try
    {
        run();
    }
    catch (const Exception &)
    {
        threw = true;
    }
    assert(threw);
}
} // namespace

int main()
{
    using sirkit::PrefixLcpAppend;
    using sirkit::PrefixLcpOperation;
    using sirkit::PrefixLcpQuery;
    using namespace std::string_view_literals;
    check({}, {});
    check(std::array{""sv, ""sv}, std::array<PrefixLcpOperation, 2>{PrefixLcpQuery{0}, PrefixLcpQuery{1}});
    check(std::array{""sv, "ab"sv, "ab"sv, "c"sv},
          std::array<PrefixLcpOperation, 8>{PrefixLcpQuery{0}, PrefixLcpQuery{1}, PrefixLcpAppend{0, 'a'},
                                            PrefixLcpQuery{0}, PrefixLcpAppend{0, 'b'}, PrefixLcpQuery{1},
                                            PrefixLcpAppend{3, 255}, PrefixLcpQuery{3}});
    check(std::array{"a\0b"sv, "a\0"sv},
          std::array<PrefixLcpOperation, 4>{PrefixLcpQuery{1}, PrefixLcpAppend{1, 'b'},
                                            PrefixLcpAppend{0, 255}, PrefixLcpQuery{0}});
    std::mt19937 random(331013);
    for (int trial = 0; trial < 650; ++trial)
    {
        std::vector<std::string> words(random() % 25 + 1);
        std::vector<std::string_view> initial;
        for (auto &word : words)
        {
            word.resize(random() % 8);
            for (auto &byte : word)
            {
                byte = static_cast<char>(random() % 5);
            }
            initial.push_back(word);
        }
        std::vector<PrefixLcpOperation> operations;
        for (int i = 0; i < 75; ++i)
        {
            const auto index = random() % words.size();
            if (random() % 2 == 0)
            {
                operations.emplace_back(PrefixLcpAppend{index, static_cast<std::uint8_t>(random() % 5)});
            }
            else
            {
                operations.emplace_back(PrefixLcpQuery{index});
            }
        }
        check(initial, operations);
    }

    constexpr std::size_t kDepth = 100'000;
    std::vector<PrefixLcpOperation> chain(kDepth, PrefixLcpAppend{0, 'x'});
    chain.emplace_back(PrefixLcpQuery{0});
    chain.emplace_back(PrefixLcpQuery{1});
    const auto scaled = sirkit::offline_prefix_lcp_sum(std::array{"x"sv, "x"sv}, chain);
    assert((scaled == std::vector<std::uint64_t>{kDepth + 2, 2}));
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::out_of_range>([] {
        (void)sirkit::offline_prefix_lcp_sum({}, std::array<PrefixLcpOperation, 1>{PrefixLcpQuery{0}});
    });
#endif
    check_error<std::length_error>([] {
        (void)sirkit::detail::prefix_lcp_checked_count(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
    });
}
