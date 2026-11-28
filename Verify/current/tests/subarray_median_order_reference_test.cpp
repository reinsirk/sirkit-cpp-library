// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Subarray_Median_Order.cpp"

#include <array>
#include <cassert>
#include <list>
#include <random>
#include <string>

static std::vector<int> medians(const std::vector<int> &values, sirkit::SubarrayMedianSide side)
{
    std::vector<int> answer;
    for (std::size_t left = 0; left < values.size(); ++left)
    {
        for (std::size_t right = left + 1; right <= values.size(); ++right)
        {
            std::vector<int> window(values.begin() + left, values.begin() + right);
            std::sort(window.begin(), window.end());
            const auto index = side == sirkit::SubarrayMedianSide::upper ? window.size() / 2 : (window.size() - 1) / 2;
            answer.push_back(window[index]);
        }
    }
    std::sort(answer.begin(), answer.end());
    return answer;
}

int main()
{
    using sirkit::SubarrayMedianSide;
    const std::array values{10, 30, 20};
    assert(sirkit::count_subarray_medians_at_least(values, 30) == 3);
    assert(sirkit::count_subarray_medians_at_least(values, 30, SubarrayMedianSide::lower) == 1);
    assert(sirkit::kth_subarray_median(values, 4) == 30);
    assert(sirkit::kth_subarray_median(values, 4, SubarrayMedianSide::lower) == 20);
    assert(!sirkit::kth_subarray_median(values, 0) && !sirkit::kth_subarray_median(values, 7));

    const std::vector<int> empty;
    assert(sirkit::count_subarray_medians_at_least(empty, 0) == 0);
    assert(!sirkit::kth_subarray_median(empty, 1));
    const std::vector<int> equal(100000, 8);
    assert(sirkit::count_subarray_medians_at_least(equal, 8) == UINT64_C(5000050000));
    assert(sirkit::count_subarray_medians_at_least(equal, 9) == 0);
    assert(sirkit::kth_subarray_median(equal, UINT64_C(5000050000)) == 8);
    assert(!sirkit::kth_subarray_median(equal, UINT64_C(5000050001)));

    const std::list<std::string> letters{"bbb", "aaa"};
    assert(sirkit::kth_subarray_median(letters, 2) == "bbb");
    const std::vector<bool> bits{0, 1, 0};
    assert(sirkit::count_subarray_medians_at_least(bits, true) == 3);
    assert(sirkit::kth_subarray_median(bits, 4) == true);
    assert(sirkit::kth_subarray_median(bits, 4, SubarrayMedianSide::lower) == false);

    constexpr auto kMax = std::numeric_limits<std::size_t>::max();
    const std::array oversized{std::pair{static_cast<std::size_t>(UINT64_C(1) << 32), kMax},
                               std::pair{std::size_t{0}, std::size_t{0}}, std::pair{std::size_t{3}, std::size_t{4}}};
    for (const auto &[n, storage] : oversized)
    {
        bool threw = false;
        try
        {
            (void)sirkit::detail::SubarrayMedianStorage(n, storage);
        }
        catch (const std::length_error &)
        {
            threw = true;
        }
        assert(threw);
    }

    std::mt19937 rng(65349);
    for (int test = 0; test < 900; ++test)
    {
        std::vector<int> row(rng() % 12);
        for (auto &value : row)
        {
            value = static_cast<int>(rng() % 13) - 6;
        }
        for (const auto side : {SubarrayMedianSide::lower, SubarrayMedianSide::upper})
        {
            const auto expected = medians(row, side);
            for (std::size_t rank = 1; rank <= expected.size(); ++rank)
            {
                assert(sirkit::kth_subarray_median(row, rank, side) == expected[rank - 1]);
            }
            for (int threshold = -7; threshold <= 7; ++threshold)
            {
                const auto from = std::lower_bound(expected.begin(), expected.end(), threshold);
                assert(sirkit::count_subarray_medians_at_least(row, threshold, side) ==
                       static_cast<std::uint64_t>(expected.end() - from));
            }
        }
    }
}
