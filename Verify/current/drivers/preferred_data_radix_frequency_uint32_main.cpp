// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

// ソート済み値を値域の上位bitで区切り、同じ区間だけを探索する。
// 小さい配列は通常の二分探索。区間の境界はRadixSort結果から構築する。
class ValueDirectory {
    const std::vector<std::uint32_t>& keys_;
    std::vector<std::uint32_t> offsets_;
    std::uint32_t base_=0, maximum_=0;
    unsigned shift_=0;
public:
    explicit ValueDirectory(const std::vector<std::uint32_t>& keys):keys_(keys) {
        if (keys.empty()) return;
        base_=keys.front(); maximum_=keys.back();
        if (keys.size()<64) return;
        const unsigned bits=std::min<unsigned>(18,std::bit_width(static_cast<std::uint32_t>(keys.size())));
        const unsigned width=std::bit_width(maximum_-base_);
        shift_=width>bits ? width-bits : 0;
        const auto buckets=((maximum_-base_)>>shift_)+1;
        offsets_.resize(buckets+1);
        std::size_t position=0;
        for (std::uint32_t bucket=0; bucket<=buckets; ++bucket) {
            while (position<keys.size() && ((keys[position]-base_)>>shift_)<bucket) ++position;
            offsets_[bucket]=static_cast<std::uint32_t>(position);
        }
    }
    std::size_t find(std::uint32_t value) const {
        if (keys_.empty() || value<base_ || value>maximum_) return keys_.size();
        auto first=keys_.begin(),last=keys_.end();
        if (!offsets_.empty()) {
            const auto bucket=(value-base_)>>shift_;
            first+=offsets_[bucket]; last=keys_.begin()+offsets_[bucket+1];
        }
        const auto found=std::lower_bound(first,last,value);
        return found!=last && *found==value ? static_cast<std::size_t>(found-keys_.begin()) : keys_.size();
    }
};

int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<std::uint32_t> values(n);
    for (auto& x : values) verify_in >> x;
    auto keys = values;
    RadixSort(keys);
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    if (keys.size()<=1) {
        while (q--) {
            std::uint32_t l{},r{},x{}; verify_in >> l >> r >> x;
            verify_out << (!keys.empty() && x==keys[0] ? r-l : 0) << '\n';
        }
        return 0;
    }
    const ValueDirectory directory(keys);
    std::vector<std::uint32_t> ranks(n), offsets(keys.size() + 1);
    for (int i = 0; i < n; ++i) {
        ranks[i] = static_cast<std::uint32_t>(directory.find(values[i]));
        ++offsets[ranks[i] + 1];
    }
    for (std::size_t i = 1; i < offsets.size(); ++i) offsets[i] += offsets[i - 1];
    auto cursor = offsets;
    std::vector<std::uint32_t> positions(n);
    for (int i = 0; i < n; ++i) positions[cursor[ranks[i]]++] = static_cast<std::uint32_t>(i);
    while (q--) {
        std::uint32_t l{}, r{}, x{}; verify_in >> l >> r >> x;
        if (l==r) { verify_out << 0 << '\n'; continue; }
        const auto rank=directory.find(x);
        if (rank==keys.size()) { verify_out << 0 << '\n'; continue; }
        const auto begin = positions.begin() + offsets[rank], end = positions.begin() + offsets[rank + 1];
        if (end-begin==1) {
            verify_out << static_cast<int>(l<=*begin && *begin<r) << '\n';
            continue;
        }
        const auto first=std::lower_bound(begin,end,l);
        verify_out << std::lower_bound(first,end,r)-first << '\n';
    }
}
