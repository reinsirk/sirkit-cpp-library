// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

int main()
{
    constexpr std::array<std::string_view, 4> kVolumeLabels{"iti", "ni", "san", "yon"};
    std::array<std::vector<std::string>, kVolumeLabels.size()> grouped_titles;
    std::size_t n{};
    verify_in >> n;
    for (std::size_t i = 0; i < n; ++i)
    {
        std::string title, label;
        verify_in >> title >> label;
        const auto volume = std::find(kVolumeLabels.begin(), kVolumeLabels.end(), label) - kVolumeLabels.begin();
        grouped_titles[volume].push_back(std::move(title));
    }
    std::vector<std::string_view> titles, labels;
    titles.reserve(n);
    labels.reserve(n);
    for (std::size_t volume = 0; volume < kVolumeLabels.size(); ++volume)
    {
        for (const auto &title : grouped_titles[volume])
        {
            titles.push_back(title);
            labels.push_back(kVolumeLabels[volume]);
        }
    }
    for (const auto i : sirkit::string_radix_order(titles))
    {
        verify_out << titles[i] << ' ' << labels[i] << '\n';
    }
}
