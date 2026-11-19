// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Linear_Recurrence.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {

struct Ring {
    std::int64_t value;
    Ring& operator+=(Ring other) {
        value += other.value;
        return *this;
    }
    friend Ring operator*(Ring a, Ring b) { return {a.value * b.value}; }
    friend bool operator==(Ring, Ring) = default;
};

#ifndef SKIP_RUNTIME_CHECKS
template<class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const std::invalid_argument&) {
        return;
    }
    assert(false);
}
#endif

} // namespace

int main() {
    const std::vector<Ring> initial{{3}, {8}}, coefficients{{2}, {-1}};
    constexpr int index = 512;
    assert(sirkit::LinearRecurrenceTerm(initial, coefficients, index) == Ring{3 + 5 * index});
    const std::vector<Ring> expected_weights{{1 - index}, {index}};
    assert(sirkit::LinearRecurrenceWeights(coefficients, index) == expected_weights);
    const std::vector<Ring> expected_prefix{{3}, {8}, {13}, {18}};
    assert(sirkit::LinearRecurrencePrefix(initial, coefficients, 4) == expected_prefix);
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<Ring> single{{1}};
    rejects([&] { (void)sirkit::LinearRecurrenceTerm(initial, single, 0); });
    rejects([&] { (void)sirkit::LinearRecurrencePrefix(initial, single, 0); });
#endif

    sirkit::NTT<> ntt;
    const std::vector<std::uint32_t> short_input{2, 3};
    const std::vector<std::uint32_t> long_input(17, 1);
    std::vector<std::uint32_t> expected(18, 5);
    expected.front() = 2;
    expected.back() = 3;
    assert(ntt.convolution_normalized(short_input, long_input) == expected);
    assert(ntt.convolution_normalized(long_input, short_input) == expected);
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<std::uint32_t> invalid_short{2, ntt.modulus()};
    auto invalid_long = long_input;
    invalid_long.back() = ntt.modulus();
    rejects([&] { (void)ntt.convolution_normalized(invalid_short, long_input); });
    rejects([&] { (void)ntt.convolution_normalized(short_input, invalid_long); });
#endif
}
