// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int cases{};
    verify_in >> cases;
    while (cases-- > 0)
    {
        std::uint64_t n{};
        verify_in >> n;
        const sirkit::DirichletPrefixSums<VerifyMatrixField> layout(n);
        const auto size = static_cast<std::size_t>(layout.size());
        std::vector<VerifyMatrixField> left(size), right(size);
        for (auto *table : {&left, &right})
        {
            for (auto &value : *table)
            {
                std::uint32_t residue{};
                verify_in >> residue;
                value = residue;
            }
        }
        const auto result = layout.convolution(left, right);
        for (std::size_t i = 0; i < size; ++i)
        {
            verify_out << result[i].get() << (i + 1 == size ? '\n' : ' ');
        }
    }
}
