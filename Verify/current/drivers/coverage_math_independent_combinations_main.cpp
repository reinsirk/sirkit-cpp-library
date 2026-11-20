// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

// 両側20頂点以下へ分け、各個数の部分集合をライブラリで列挙する。
int main() {
    int n{}, m{}; verify_in >> n >> m;
    const int left_size = n / 2, right_size = n - left_size;
    std::vector<std::uint32_t> left_edges(left_size), right_edges(right_size), across(left_size);
    while (m--) {
        int u{}, v{}; verify_in >> u >> v;
        if (u >= left_size && v >= left_size) {
            right_edges[u - left_size] |= std::uint32_t{1} << (v - left_size);
            right_edges[v - left_size] |= std::uint32_t{1} << (u - left_size);
        } else if (u < left_size && v < left_size) {
            left_edges[u] |= std::uint32_t{1} << v;
            left_edges[v] |= std::uint32_t{1} << u;
        } else {
            if (u >= left_size) { const int temporary = u; u = v; v = temporary; }
            across[u] |= std::uint32_t{1} << (v - left_size);
        }
    }
    std::vector<unsigned char> best(std::size_t{1} << right_size);
    for (int count = 1; count <= right_size; ++count)
        for_each_combination<std::uint32_t>(right_size, count, [&](std::uint32_t mask) {
            const int vertex = std::countr_zero(mask);
            const auto rest = mask & (mask - 1);
            const auto take = best[rest & ~right_edges[vertex]] + 1;
            const auto skip = best[rest];
            best[mask] = take > skip ? take : skip;
        });
    const auto all_right = (std::uint32_t{1} << right_size) - 1;
    std::vector<char> independent(std::size_t{1} << left_size);
    std::vector<std::uint32_t> allowed(independent.size());
    independent[0] = true; allowed[0] = all_right;
    std::uint32_t answer_left = 0, answer_right = all_right;
    int answer_size = best[all_right];
    for (int count = 0; count <= left_size; ++count)
        for_each_combination<std::uint32_t>(left_size, count, [&](std::uint32_t mask) {
            if (mask) {
                const int vertex = std::countr_zero(mask);
                const auto rest = mask & (mask - 1);
                independent[mask] = independent[rest] && !(left_edges[vertex] & rest);
                if (!independent[mask]) return;
                allowed[mask] = allowed[rest] & ~across[vertex];
            }
            if (!independent[mask]) return;
            const auto right = best[allowed[mask]];
            if (count + right > answer_size) {
                answer_left = mask; answer_right = allowed[mask];
                answer_size = count + right;
            }
        });
    // DPは個数だけを保持し、必要な右半分の頂点集合を最後に1回復元する。
    std::uint32_t chosen = 0;
    while (answer_right) {
        const int vertex = std::countr_zero(answer_right);
        const auto rest = answer_right & (answer_right - 1);
        if (best[answer_right] == best[rest]) answer_right = rest;
        else {
            chosen |= std::uint32_t{1} << vertex;
            answer_right = rest & ~right_edges[vertex];
        }
    }
    answer_right = chosen;
    verify_out << answer_size << '\n';
    for (int v = 0; v < left_size; ++v) if ((answer_left >> v) & 1) verify_out << v << ' ';
    for (int v = 0; v < right_size; ++v) if ((answer_right >> v) & 1) verify_out << left_size + v << ' ';
    verify_out << '\n';
}
