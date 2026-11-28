// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

void Print(const std::vector<int> &values)
{
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << values[i];
    }
    verify_out << '\n';
}

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto code = sirkit::permutation_lehmer_code(values);
    Print(code);
    Print(sirkit::permutation_from_lehmer_code(code));
}
