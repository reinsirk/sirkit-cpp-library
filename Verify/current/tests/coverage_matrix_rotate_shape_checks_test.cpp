// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/rotate.cpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    const std::vector<std::string> original{"abc", "def"};
    const std::vector<std::string> ccw{"cf", "be", "ad"}, cw{"da", "eb", "fc"};
    assert(sirkit::rotate_quarter(original, -1) == ccw);
    assert(sirkit::rotate_quarter(original, 5) == cw);
    assert(sirkit::rotate_quarter(original, std::numeric_limits<int>::min()) == original);
#ifndef SKIP_RUNTIME_CHECKS
    for (const std::vector<std::string>& rows : {std::vector<std::string>{"abc", "d", "efg", "hij"},
                                                std::vector<std::string>{"abc", "def", "g", "hij"}}) {
        auto actual = rows;
        bool rejected = false;
        try {
            sirkit::flip_i_inplace(actual);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        // 外側の組は交換済み。幅の異なる内側の組は変更しない。
        assert(rejected && actual[0] == rows[3] && actual[3] == rows[0]);
        assert(actual[1] == rows[1] && actual[2] == rows[2]);
    }
#endif
}
