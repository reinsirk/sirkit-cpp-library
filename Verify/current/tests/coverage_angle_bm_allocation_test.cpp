// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/angle_sort.cpp"
#include "../../../Math/Berlekamp_Massey.cpp"

#include <cassert>
#include <cstdlib>
#include <new>

namespace {
bool fail_allocation = false;

template <class Function>
void expect_bad_alloc(Function operation) {
    fail_allocation = true;
    bool caught = false;
    try {
        operation();
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    fail_allocation = false;
    assert(caught);
}
}  // namespace

[[gnu::noinline]] void* operator new(std::size_t size) {
    if (!fail_allocation) {
        if (void* memory = std::malloc(size == 0 ? 1 : size)) return memory;
    }
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

int main() {
    const std::vector<std::pair<std::int64_t, std::int64_t>> points{{0, 1}, {1, 0}};
    const auto original = points;
    expect_bad_alloc([&] { (void)sirkit::angle_sort_indices(points); });
    assert(points == original);
    assert((sirkit::angle_sort_indices(points) == std::vector<std::size_t>{1, 0}));

    const std::vector<std::uint32_t> sequence{1, 2, 4, 8};
    const auto original_sequence = sequence;
    expect_bad_alloc([&] { (void)sirkit::BerlekampMassey(sequence); });
    assert(sequence == original_sequence);
    assert((sirkit::BerlekampMassey(sequence) == std::vector<std::uint32_t>{2}));
}
