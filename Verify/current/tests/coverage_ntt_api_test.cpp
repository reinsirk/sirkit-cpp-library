// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <source_location>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using Residues = std::vector<std::uint32_t>;

void require(bool condition, std::source_location where = std::source_location::current()) {
    if (condition) return;
    std::cerr << where.file_name() << ':' << where.line() << '\n';
    std::abort();
}

template<class Exception, class Function>
void rejects(Function operation, std::source_location where = std::source_location::current()) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    }
    require(false, where);
}

template<std::uint32_t Mod, class Int>
std::uint32_t residue(Int value) {
    if constexpr (std::is_signed_v<Int>) {
        const auto remainder = static_cast<std::int64_t>(value) % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>((remainder + Mod) % Mod);
    } else {
        return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % Mod);
    }
}

template<std::uint32_t Mod, class Left, class Right>
Residues schoolbook(const std::vector<Left>& left, const std::vector<Right>& right) {
    if (left.empty() || right.empty()) return {};
    Residues result(left.size() + right.size() - 1);
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto x = residue<Mod>(static_cast<Left>(left[i]));
        for (std::size_t j = 0; j < right.size(); ++j) {
            const auto y = residue<Mod>(static_cast<Right>(right[j]));
            result[i + j] = static_cast<std::uint32_t>((result[i + j] + std::uint64_t{x} * y) % Mod);
        }
    }
    return result;
}

std::uint64_t power(std::uint64_t base, std::uint64_t exponent, std::uint32_t mod) {
    std::uint64_t result = 1;
    while (exponent) {
        if (exponent % 2) result = result * base % mod;
        base = base * base % mod;
        exponent /= 2;
    }
    return result;
}

template<std::uint32_t Mod, std::uint32_t Root>
Residues direct_transform(const Residues& input, bool inverse) {
    const auto size = input.size();
    const auto exponent = (Mod - 1) / size;
    const auto omega = power(Root, inverse ? Mod - 1 - exponent : exponent, Mod);
    const auto scale = inverse ? power(size, Mod - 2, Mod) : 1;
    Residues result(size);
    std::uint64_t step = 1;
    for (auto& coefficient : result) {
        std::uint64_t sum = 0;
        std::uint64_t weight = 1;
        for (const auto value : input) {
            sum = (sum + weight * value) % Mod;
            weight = weight * step % Mod;
        }
        coefficient = static_cast<std::uint32_t>(sum * scale % Mod);
        step = step * omega % Mod;
    }
    return result;
}

template<std::uint32_t Mod>
Residues values(std::size_t size) {
    std::mt19937 random(713 + static_cast<std::uint32_t>(size));
    Residues result(size);
    for (auto& item : result) item = random() % Mod;
    result.front() = 0;
    result.back() = Mod - 1;
    return result;
}

template<std::uint32_t Mod, std::uint32_t Root>
void check_transforms() {
    sirkit::NTT<Mod, Root> ntt;
    require(ntt.modulus() == Mod);
    Residues empty;
    ntt.transform(empty);
    ntt.transform(empty, true);
    ntt.prepare(0);
    ntt.prepare(1);
    require(ntt.roots_capacity() == 0);
    for (std::size_t size = 1; size <= 512 && size <= ntt.max_length(); size *= 2) {
        const auto original = values<Mod>(size);
        for (bool inverse : {false, true}) {
            auto actual = original;
            ntt.transform(actual, inverse);
            require(actual == direct_transform<Mod, Root>(original, inverse));
        }
    }

    const auto cache = ntt.roots_capacity();
    auto copy = ntt;
    auto moved = std::move(copy);
    ntt.clear_cache();
    require(ntt.roots_capacity() == 0);
    require(moved.roots_capacity() == cache);
    const Residues input{1, 2};
    const auto expected = direct_transform<Mod, Root>(input, false);
    for (auto* engine : {&ntt, &copy, &moved}) {
        auto actual = input;
        engine->transform(actual);
        require(actual == expected);
    }

    if constexpr (sirkit::NTT<Mod, Root>::max_length() >= 256) {
        // 空のキャッシュから一度に根を生成する経路も確認する。
        ntt.clear_cache();
        ntt.prepare(256);
        require(ntt.roots_capacity() >= 256);
        auto actual = values<Mod>(256);
        const auto direct = direct_transform<Mod, Root>(actual, false);
        ntt.transform(actual);
        require(actual == direct);
    }
}

template<std::uint32_t Mod, std::uint32_t Root>
void check_products() {
    sirkit::NTT<Mod, Root> ntt;
    // 短い積と、変換長の1/4・1/2をまたぐ長さを両順序で検査する。
    constexpr std::pair<std::size_t, std::size_t> sizes[] = {
        {2, 3}, {3, 17}, {2, 14}, {32, 33}, {33, 33}, {33, 65},
        {64, 64}, {65, 65}, {33, 129}, {33, 520}, {255, 513},
    };
    for (const auto [left_size, right_size] : sizes) {
        const auto left = values<Mod>(left_size);
        const auto right = values<Mod>(right_size);
        const auto expected = schoolbook<Mod>(left, right);
        require(ntt.convolution(left, right) == expected);
        require(ntt.convolution_normalized(left, right) == expected);
        require(ntt.convolution_normalized(right, left) == expected);
        require(ntt.square(left) == schoolbook<Mod>(left, left));
        require(ntt.square(right) == schoolbook<Mod>(right, right));
    }
    const std::vector<std::int64_t> signed_values{
        std::numeric_limits<std::int64_t>::min(), -static_cast<std::int64_t>(Mod), -1, 0, 1,
        std::numeric_limits<std::int64_t>::max(),
    };
    const std::vector<std::uint64_t> unsigned_values{0, Mod, Mod + 1, std::numeric_limits<std::uint64_t>::max()};
    const std::vector<bool> bits{true, false, true};
    require(ntt.convolution(signed_values, unsigned_values) == schoolbook<Mod>(signed_values, unsigned_values));
    require(ntt.square(signed_values) == schoolbook<Mod>(signed_values, signed_values));
    require(sirkit::ConvolutionMod<Mod, Root>(bits, signed_values) == schoolbook<Mod>(bits, signed_values));
    require(ntt.convolution(Residues{}, unsigned_values).empty());
    require(ntt.convolution(signed_values, Residues{}).empty());
    require(ntt.convolution_normalized({}, {1}).empty());
    require(ntt.convolution_normalized({1}, {}).empty());
    require(ntt.square(Residues{}).empty());
}

void check_lengths() {
    sirkit::NTT<17, 3> ntt;
    static_assert(ntt.max_length() == 16);
    ntt.prepare(ntt.max_length());
    const auto cache = ntt.roots_capacity();
    for (const auto size : {std::size_t{3}, std::size_t{32}, std::numeric_limits<std::size_t>::max()}) {
        rejects<std::length_error>([&] { ntt.prepare(size); });
        require(ntt.roots_capacity() == cache);
    }
    Residues unsupported(3, 1);
    rejects<std::length_error>([&] { ntt.transform(unsupported); });
    require(unsupported == Residues(3, 1));
    const Residues small(32, 1);
    const Residues too_long(33, 1);
    require(ntt.convolution(small, too_long) == schoolbook<17>(small, too_long));
    rejects<std::length_error>([&] { ntt.convolution(too_long, too_long); });
    rejects<std::length_error>([&] { ntt.convolution_normalized(too_long, too_long); });
    rejects<std::length_error>([&] { ntt.square(too_long); });
    require(ntt.roots_capacity() == cache);
}

#ifndef SKIP_RUNTIME_CHECKS
template<std::uint32_t Mod, std::uint32_t Root>
void check_invalid_residues() {
    sirkit::NTT<Mod, Root> ntt;
    constexpr std::size_t lengths[] = {1, 2, 8, 16, 32, 64};
    for (auto size : lengths) {
        for (bool inverse : {false, true}) {
            for (const auto position : {std::size_t{0}, size / 2, size - 1}) {
                for (const auto invalid : {Mod, std::numeric_limits<std::uint32_t>::max()}) {
                    Residues input(size, 1);
                    input[position] = invalid;
                    rejects<std::invalid_argument>([&] { ntt.transform(input, inverse); });
                }
            }
        }
    }
    constexpr std::pair<std::size_t, std::size_t> lengths_by_side[] = {
        {2, 3}, {3, 17}, {33, 33}, {33, 65}, {33, 129},
    };
    for (const auto [first_size, second_size] : lengths_by_side) {
        for (bool swap : {false, true}) {
            Residues left(swap ? second_size : first_size, 1);
            Residues right(swap ? first_size : second_size, 1);
            for (auto* input : {&left, &right}) {
                for (const auto position : {std::size_t{0}, input->size() / 2, input->size() - 1}) {
                    (*input)[position] = Mod;
                    rejects<std::invalid_argument>([&] { ntt.convolution_normalized(left, right); });
                    (*input)[position] = 1;
                }
            }
            require(ntt.convolution_normalized(left, right) == schoolbook<Mod>(left, right));
        }
    }
}
#endif
} // namespace

int main() {
    const auto capacity = std::numeric_limits<std::size_t>::max();
    require(sirkit::detail::convolution_result_size(capacity, 1, "size") == capacity);
    rejects<std::length_error>([&] {
        (void)sirkit::detail::convolution_result_size(capacity, 2, "size");
    });
    check_transforms<17, 3>();
    check_transforms<998244353, 3>();
    check_transforms<2013265921, 31>();
    check_products<998244353, 3>();
    check_products<2013265921, 31>();
    check_lengths();
#ifndef SKIP_RUNTIME_CHECKS
    check_invalid_residues<998244353, 3>();
    check_invalid_residues<2013265921, 31>();
#endif
}
