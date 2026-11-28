// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

struct Operation
{
    int first, last;
    std::uint32_t value;
    bool assign;
};

constexpr auto kAbsent = std::numeric_limits<std::uint32_t>::max();

template <int Bits>
void Solve(const std::vector<std::uint32_t> &values, const std::vector<Operation> &operations)
{
    sirkit::DynamicWaveletMatrix<std::uint32_t, Bits> matrix(values);
    for (const auto &op : operations)
    {
        if (op.assign)
        {
            matrix.set(op.first, op.value);
        }
        else
        {
            verify_out << (op.value == kAbsent ? 0 : matrix.count(op.first, op.last, op.value)) << '\n';
        }
    }
}

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<std::uint32_t> values(n);
    std::vector<std::uint32_t> keys;
    keys.reserve(static_cast<std::size_t>(n) + q);
    for (auto &value : values)
    {
        verify_in >> value;
        keys.push_back(value);
    }

    std::vector<Operation> operations(q);
    for (auto &op : operations)
    {
        unsigned type;
        verify_in >> type >> op.first;
        op.assign = type == 0;
        if (op.assign)
        {
            verify_in >> op.value;
            keys.push_back(op.value);
        }
        else
        {
            verify_in >> op.last >> op.value;
        }
    }

    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    const auto encode = [&](std::uint32_t value)
    {
        const auto it = std::lower_bound(keys.begin(), keys.end(), value);
        return it != keys.end() && *it == value ? static_cast<std::uint32_t>(it - keys.begin()) : kAbsent;
    };
    for (auto &value : values)
    {
        value = encode(value);
    }
    for (auto &op : operations)
    {
        op.value = encode(op.value);
    }
    if (keys.size() <= (1U << 8))
    {
        Solve<8>(values, operations);
    }
    else if (keys.size() <= (1U << 16))
    {
        Solve<16>(values, operations);
    }
    else
    {
        Solve<19>(values, operations);
    }
}
