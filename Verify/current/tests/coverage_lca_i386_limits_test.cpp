// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/LCA.cpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace {

static_assert(sizeof(int) == 4 && sizeof(std::ptrdiff_t) == 4 && sizeof(std::size_t) == 4);

template<class Operation>
void expect_length_error(std::string_view message, Operation operation) {
    try {
        operation();
    } catch (const std::length_error& error) {
        assert(error.what() == message);
        return;
    }
    assert(false);
}

void rejects_csr_row_capacity() {
    // CSRの行境界には頂点数に加えて2要素が必要。
    const auto vertex_count = static_cast<int>(std::vector<std::size_t>{}.max_size() - 1);
    expect_length_error("LCA: CSR capacity exceeded", [&] {
        sirkit::detail::LcaCsrGraph graph(vertex_count, {});
    });
    expect_length_error("LCA: CSR capacity exceeded", [] {
        sirkit::detail::CheckLcaCsrCapacity(0, 0, 1);
    });
    constexpr auto kMaxEdges =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / (2 * sizeof(int));
    sirkit::detail::CheckLcaCsrCapacity(0, kMaxEdges, 2);
    expect_length_error("LCA: CSR capacity exceeded", [] {
        sirkit::detail::CheckLcaCsrCapacity(0, kMaxEdges + 1, 2);
    });
}

void rejects_ladder_capacity() {
    // 祖先ラダーは頂点ごとに最大2要素を確保する。入力配列の参照前に容量を検査する。
    constexpr auto kVertexCount =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / (2 * sizeof(int)) + 1;
    expect_length_error("LCA: ancestor ladders too large", [] {
        sirkit::detail::AncestorLadder ladder;
        ladder.build(kVertexCount, nullptr, nullptr, [](int) { return 0; }, nullptr, nullptr);
    });
}

void rejects_ancestor_table_capacity() {
    // 祖先表を使う最大の深さ256では9段を確保する。
    constexpr unsigned int kDepth = 256;
    constexpr auto kLevels = std::bit_width(kDepth);
    constexpr auto kMaxEntries =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(int);
    constexpr auto kVertexLimit = kMaxEntries / kLevels;
    assert(sirkit::detail::LcaAncestorTableSize(kVertexLimit, kLevels) == kVertexLimit * kLevels);
    expect_length_error("LCA: ancestor table too large", [] {
        (void)sirkit::detail::LcaAncestorTableSize(kVertexLimit + 1, kLevels);
    });
}

void rejects_vertex_and_block_capacity() {
    constexpr auto kVertexLimit = sirkit::detail::kLcaMaximumVertices;
    sirkit::detail::CheckLcaVertexCount(kVertexLimit);
    expect_length_error("LCA: too many vertices", [] {
        sirkit::detail::CheckLcaVertexCount(kVertexLimit + 1);
    });
    expect_length_error("LCA: ancestor ladders too large", [] {
        sirkit::detail::AncestorLadder ladder;
        ladder.build(kVertexLimit + 1, nullptr, nullptr, [](int) { return 0; }, nullptr, nullptr);
    });
    constexpr auto kMaxEntries =
        static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(std::uint32_t);
    constexpr auto kVertices = kMaxEntries / 3;
    const auto block_count = kVertices / 32 + (kVertices % 32 != 0);
    const auto levels = static_cast<unsigned int>(std::bit_width(block_count));
    assert(sirkit::detail::LcaBlockTableSize(257, 9, 4) == 807);
    expect_length_error("LCA: block table too large", [=] {
        (void)sirkit::detail::LcaBlockTableSize(kVertices, block_count, levels);
    });
    expect_length_error("LCA: block table too large", [=] {
        (void)sirkit::detail::LcaBlockTableSize(kVertices + 1, block_count, levels);
    });
}

} // namespace

int main() {
    rejects_csr_row_capacity();
    rejects_ladder_capacity();
    rejects_ancestor_table_capacity();
    rejects_vertex_and_block_capacity();
}
