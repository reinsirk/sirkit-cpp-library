// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_COVERAGE_ZETA_SOURCE
#define SIRKIT_COVERAGE_ZETA_SOURCE "../../../Math/Zeta/Set_Transform.cpp"
#endif
#include SIRKIT_COVERAGE_ZETA_SOURCE

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

template <class T>
void check(std::size_t size) {
    std::vector<T> original(size);
    for (std::size_t i = 0; i < size; ++i)
        original[i] = static_cast<T>((i * 13 + 7) % 17);
    for (const bool subset : {false, true}) {
        std::vector<T> expected(size);
        for (std::size_t i = 0; i < size; ++i) {
            for (std::size_t j = 0; j < size; ++j) {
                if ((i & j) == (subset ? j : i)) expected[i] += original[j];
            }
        }
        auto actual = original;
        if (subset) sirkit::subset_zeta_transform(actual);
        else sirkit::superset_zeta_transform(actual);
        assert(actual == expected);
        if (subset) sirkit::subset_mobius_transform(expected);
        else sirkit::superset_mobius_transform(expected);
        assert(expected == original);
    }
}

} // namespace

int main() {
    for (const std::size_t size : {0, 1, 2, 4, 8, 16, 32, 64, 128}) {
        check<std::int32_t>(size);
        check<std::uint64_t>(size);
        check<double>(size);
    }
    using Values = std::vector<std::int32_t>;
    for (const auto transform : {sirkit::subset_zeta_transform<std::int32_t>,
                                 sirkit::subset_mobius_transform<std::int32_t>,
                                 sirkit::superset_zeta_transform<std::int32_t>,
                                 sirkit::superset_mobius_transform<std::int32_t>}) {
        Values values(24, 2);
#ifndef SKIP_RUNTIME_CHECKS
        try {
            transform(values);
            assert(false);
        } catch (const std::invalid_argument&) {
            assert(values == Values(24, 2));
        }
#elif !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
        // 検査省略時も、固定長のSIMD用処理には異なる長さを渡さない。
        transform(values);
        assert(values == Values(24, 2));
#else
        (void)transform;
#endif
    }
}
