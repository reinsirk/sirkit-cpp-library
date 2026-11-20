// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
    std::int64_t work{};
    int original_count{}, director_count{};
    verify_in >> work >> original_count;
    std::vector<std::int64_t> originals(original_count);
    for (auto& capacity : originals) verify_in >> capacity;
    verify_in >> director_count;
    std::vector<std::int64_t> directors(director_count);
    for (auto& capacity : directors) verify_in >> capacity;
    const int source = original_count + director_count, sink = source + 1;
    mf_graph<std::int64_t> network(sink + 1);
    for (int i = 0; i < original_count; ++i) network.add_edge(source, i, originals[i]);
    for (int i = 0; i < director_count; ++i) {
        network.add_edge(original_count + i, sink, directors[i]);
        int count{}; verify_in >> count;
        std::vector<bool> forbidden(original_count);
        while (count--) { int index{}; verify_in >> index; forbidden[index - 1] = true; }
        for (int j = 0; j < original_count; ++j)
            if (!forbidden[j]) network.add_edge(j, original_count + i, work);
    }
    verify_out << (network.flow(source, sink, work) == work ? "SHIROBAKO\n" : "BANSAKUTSUKITA\n");
}
