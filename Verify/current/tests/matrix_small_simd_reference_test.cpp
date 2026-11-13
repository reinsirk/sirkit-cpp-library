// SPDX-License-Identifier: CC0-1.0
#ifndef MATRIX_HEADER
#define MATRIX_HEADER "../../../Algorithm/rotate.cpp"
#endif
#include MATRIX_HEADER
#include MATRIX_HEADER

#include <bit>
#include <cassert>
#include <cfenv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
enum class ByteCode : std::uint8_t {};
enum class ShortCode : std::uint16_t {};
struct SmallBox {
    std::uint16_t bits;
    friend bool operator==(SmallBox, SmallBox) = default;
};
static_assert(sizeof(SmallBox) == 2);
std::uint64_t checks = 0, matrices = 0;

template <class Value>
Value random_value(std::mt19937_64& random) {
    if constexpr (std::is_same_v<Value, bool>) return (random() & 1) != 0;
    else if constexpr (std::is_same_v<Value, SmallBox>) return {static_cast<std::uint16_t>(random())};
    else if constexpr (sizeof(Value) == 1) return std::bit_cast<Value>(static_cast<std::uint8_t>(random()));
    else return std::bit_cast<Value>(static_cast<std::uint16_t>(random()));
}

template <class Row>
void check_matrix(std::size_t h, std::size_t w, std::mt19937_64& random) {
    using Value = typename Row::value_type;
    std::vector<Row> input(h, Row(w, Value{}));
    for (auto& row : input) for (std::size_t j = 0; j < w; ++j) row[j] = random_value<Value>(random);
    const auto original = input;
    ++matrices;
    for (int mode : {0, 1, 3}) {
        std::vector<Row> expected;
        if (h == 0) {
            if (mode == 1) expected.resize(1);
        } else {
            expected.assign(w, Row(h, Value{}));
            // SIMDのタイル・unpackを使わず、元の座標から直接対応付ける。
            for (std::size_t i = 0; i < h; ++i) for (std::size_t j = 0; j < w; ++j) {
                if (mode == 1) expected[j][h - 1 - i] = input[i][j];
                else if (mode == 3) expected[w - 1 - j][i] = input[i][j];
                else expected[j][i] = input[i][j];
            }
        }
        std::feclearexcept(FE_ALL_EXCEPT);
        std::feraiseexcept(FE_DIVBYZERO);
        const int flags = std::fetestexcept(FE_ALL_EXCEPT);
        const auto actual = mode == 0 ? transpose(input) : mode == 1 ? rotate_90r(input) : rotate_90l(input);
        assert(std::fetestexcept(FE_ALL_EXCEPT) == flags);
        assert(actual == expected);
        assert(input == original);
        ++checks;
        if (h != 0 && mode != 0) {
            assert(rotate_quarter(input, mode) == expected);
            assert(rotate_quarter(input, mode - 4) == expected);
            assert(std::fetestexcept(FE_ALL_EXCEPT) == flags);
            assert(input == original);
            checks += 2;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (h > 1 && w > 1) for (std::size_t bad : {std::size_t{1}, h / 2, h - 1}) {
        for (bool longer : {false, true}) {
            auto ragged = input;
            if (longer) ragged[bad].push_back(Value{});
            else ragged[bad].pop_back();
            const auto before = ragged;
            for (int mode : {0, 1, 3}) {
                bool caught = false;
                try {
                    (void)(mode == 0 ? transpose(ragged) : mode == 1 ? rotate_90r(ragged) : rotate_90l(ragged));
                } catch (const std::invalid_argument&) { caught = true; }
                assert(caught);
                assert(ragged == before);
                ++checks;
            }
        }
    }
#endif
}
} // namespace

int main() {
    std::mt19937_64 random(UINT64_C(0x20261031A2C4));
    for (auto [h, w] : {std::pair{0, 0}, {1, 0}, {17, 0}, {1, 129}, {7, 67}, {8, 65}, {9, 63},
                       {15, 257}, {16, 257}, {17, 241}, {241, 17}, {31, 129}, {32, 31}, {32, 32},
                       {32, 33}, {32, 127}, {32, 128}, {32, 129}, {33, 127}, {47, 95}, {48, 96},
                       {49, 97}, {63, 65}, {64, 63}, {64, 64}, {64, 65}, {65, 63}, {65, 64},
                       {65, 65}, {65, 67}, {95, 97}, {96, 96}, {97, 95}, {127, 129}, {129, 127},
                       {257, 255}, {32, 4097}, {4097, 32}}) {
        check_matrix<std::vector<char>>(h, w, random);
        check_matrix<std::vector<std::int8_t>>(h, w, random);
        check_matrix<std::vector<std::uint8_t>>(h, w, random);
        check_matrix<std::vector<std::int16_t>>(h, w, random);
        check_matrix<std::vector<std::uint16_t>>(h, w, random);
        check_matrix<std::vector<char16_t>>(h, w, random);
        check_matrix<std::vector<ByteCode>>(h, w, random);
        check_matrix<std::vector<ShortCode>>(h, w, random);
        check_matrix<std::string>(h, w, random);
        check_matrix<std::vector<bool>>(h, w, random);
        check_matrix<std::vector<SmallBox>>(h, w, random);
    }
    std::printf("small matrix random-bit reference matrices=%ju checks=%ju\n",
                static_cast<std::uintmax_t>(matrices), static_cast<std::uintmax_t>(checks));
}
