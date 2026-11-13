// SPDX-License-Identifier: CC0-1.0
// 独自入出力の十進変換と空白走査を、標準変換と1文字ずつの走査で照合する。
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include "../../../String/Z_Algorithm.cpp"
#define main verify_z_algorithm_main
#include "../drivers/z_algorithm_main.cpp"
#undef main

void check_number(std::uint32_t value) {
    std::array<char, 32> actual, expected;
    actual.fill('@');
    expected.fill('@');
    char* end = reinsirk::detail::verify_write_with_space(actual.data() + 8, value);
    char* ref = std::to_chars(expected.data() + 8, expected.data() + 18, value).ptr;
    *ref++ = ' ';
    assert(end - actual.data() == ref - expected.data());
    assert(std::memcmp(actual.data(), expected.data(), static_cast<std::size_t>(ref - expected.data())) == 0);
    for (std::size_t i = 19; i < actual.size(); ++i) assert(actual[i] == '@');
}

void check_space(const char* first, std::size_t n) {
    const char* ref = first;
    while (ref != first + n && static_cast<unsigned char>(*ref) > ' ') ++ref;
    assert(reinsirk::detail::verify_find_space(first, first + n) == ref);
}

int main() {
    for (std::uint32_t value = 0; value < 10'000'000; ++value) check_number(value);
    for (auto value : {std::uint32_t{10'000'000}, std::uint32_t{2'147'483'646},
                       std::uint32_t{2'147'483'647}, std::uint32_t{2'147'483'648},
                       std::numeric_limits<std::uint32_t>::max()}) check_number(value);
    std::mt19937_64 random(132719);
    for (int i = 0; i < 200'000; ++i) check_number(static_cast<std::uint32_t>(random()));
    std::array<char, 144> buffer;
    for (unsigned x = 0; x < 256; ++x) for (unsigned y = 0; y < 256; ++y) {
        buffer.fill('X');
        buffer[16] = static_cast<char>(x);
        buffer[17] = static_cast<char>(y);
        for (int alignment = 0; alignment < 8; ++alignment) check_space(buffer.data() + alignment, 40);
    }
    for (int i = 0; i < 200'000; ++i) {
        for (char& c : buffer) c = static_cast<char>(random());
        const std::size_t offset = random() % 8, n = random() % (buffer.size() - offset + 1);
        check_space(buffer.data() + offset, n);
    }
}
