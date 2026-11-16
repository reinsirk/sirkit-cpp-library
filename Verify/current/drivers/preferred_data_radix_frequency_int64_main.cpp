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
        const unsigned bits=std::min<unsigned>(16,std::bit_width(static_cast<std::uint32_t>(keys.size())));
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
    int n{},q{}; verify_in >> n >> q;
    std::vector<std::int64_t> packed(n);
    for (int i=0; i<n; ++i) {
        std::uint32_t value{}; verify_in >> value;
        packed[i]=static_cast<std::int64_t>((std::uint64_t{value}<<32)|static_cast<std::uint32_t>(i));
    }
    RadixSort(packed);
    if (n==0 || static_cast<std::uint64_t>(packed.front())>>32==static_cast<std::uint64_t>(packed.back())>>32) {
        const auto value=n ? static_cast<std::uint32_t>(static_cast<std::uint64_t>(packed.front())>>32) : 0;
        while (q--) {
            std::uint32_t l{},r{},x{}; verify_in >> l >> r >> x;
            verify_out << (n && x==value ? r-l : 0) << '\n';
        }
        return 0;
    }
    // 照会が少なければ索引の追加構築を避け、連結キーを直接探索する。
    if (q<n/16) {
        while (q--) {
            std::uint32_t l{}, r{}, x{}; verify_in >> l >> r >> x;
            const auto lo = static_cast<std::int64_t>((std::uint64_t{x} << 32) | l);
            const auto hi = static_cast<std::int64_t>((std::uint64_t{x} << 32) | r);
            verify_out << std::lower_bound(packed.begin(), packed.end(), hi) - std::lower_bound(packed.begin(), packed.end(), lo) << '\n';
        }
        return 0;
    }
    std::vector<std::uint32_t> keys,offsets,positions(n);
    keys.reserve(n); offsets.reserve(static_cast<std::size_t>(n)+1);
    for (int i=0; i<n; ++i) {
        const auto value=static_cast<std::uint32_t>(static_cast<std::uint64_t>(packed[i])>>32);
        if (keys.empty() || keys.back()!=value) {
            keys.push_back(value); offsets.push_back(static_cast<std::uint32_t>(i));
        }
        positions[i]=static_cast<std::uint32_t>(packed[i]);
    }
    offsets.push_back(static_cast<std::uint32_t>(n));
    const ValueDirectory directory(keys);
    while (q--) {
        std::uint32_t l{},r{},x{}; verify_in >> l >> r >> x;
        if (l==r) { verify_out << 0 << '\n'; continue; }
        const auto rank=directory.find(x);
        if (rank==keys.size()) { verify_out << 0 << '\n'; continue; }
        const auto begin=positions.begin()+offsets[rank],end=positions.begin()+offsets[rank+1];
        if (end-begin==1) {
            verify_out << static_cast<int>(l<=*begin && *begin<r) << '\n';
            continue;
        }
        const auto first=std::lower_bound(begin,end,l);
        verify_out << std::lower_bound(first,end,r)-first << '\n';
    }
}
