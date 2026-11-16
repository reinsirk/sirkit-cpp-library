// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
    int height{}, width{};
    verify_in >> height >> width;
    TwoVariableSubmodularOpt<std::int64_t> model(height * width);
    model.reserve_nodes(height * width + height + width + 2);
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            std::int64_t cost{};
            verify_in >> cost;
            model.add_single_cost(row * width + column, 0, cost);
        }
    }
    std::vector<int> cells;
    cells.reserve(height > width ? height : width);
    for (int row = 0; row < height; ++row) {
        std::int64_t reward{};
        verify_in >> reward;
        cells.clear();
        for (int column = 0; column < width; ++column)
            cells.push_back(row * width + column);
        model.add_all_true_profit(cells, reward);
    }
    for (int column = 0; column < width; ++column) {
        std::int64_t reward{};
        verify_in >> reward;
        cells.clear();
        for (int row = 0; row < height; ++row)
            cells.push_back(row * width + column);
        model.add_all_true_profit(cells, reward);
    }
    verify_out << -model.solve() << '\n';
}
