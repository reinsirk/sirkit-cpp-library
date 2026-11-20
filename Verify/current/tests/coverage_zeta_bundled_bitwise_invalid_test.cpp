// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Zeta_Mebius.cpp"

#include <cassert>
#include <stdexcept>
#include <vector>

int main() {
    const std::vector<int> input{1, 2, 3};
    for (auto transform : {sirkit::subset_zeta_transform<int>, sirkit::subset_mobius_transform<int>,
                           sirkit::superset_zeta_transform<int>, sirkit::superset_mobius_transform<int>}) {
        // 配列長の検査は計算を始める前に行う。
#ifndef SKIP_RUNTIME_CHECKS
        auto values = input;
        bool rejected = false;
        try {
            transform(values);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected && values == input);
#endif
        std::vector<int> empty;
        transform(empty);
        assert(empty.empty());
    }
}
