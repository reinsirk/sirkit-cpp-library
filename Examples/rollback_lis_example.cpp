// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Rollback_LIS.cpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    sirkit::RollbackLIS<int> lis;
    lis.push_back(3);
    lis.push_back(1);
    lis.push_back(2);
    assert(lis.length() == 2);
    assert((lis.indices() == std::vector<std::size_t>{1, 2}));
    lis.rollback(1);
    assert(lis.length() == 1);
    assert((lis.indices() == std::vector<std::size_t>{0}));
}
