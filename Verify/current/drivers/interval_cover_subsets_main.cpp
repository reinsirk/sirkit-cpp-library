// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::IntervalCoverChoice<VerifyMatrixField>> choices(m);
    for (auto &choice : choices)
    {
        verify_in >> choice.left >> choice.right;
    }
    const auto answer = sirkit::count_interval_cover_subsets<VerifyMatrixField>(1, n, choices);
    verify_out << answer.get() << '\n';
}
