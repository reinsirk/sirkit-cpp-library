// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_DISJOINT_TEST_SOURCE
#define SIRKIT_DISJOINT_TEST_SOURCE "../../../DataStructure/Disjoint_Sparse_Table.cpp"
#endif
#include SIRKIT_DISJOINT_TEST_SOURCE
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <random>
#include <source_location>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
std::uint64_t checks = 0;
void check(bool ok, const std::source_location where = std::source_location::current()) {
    ++checks;
    if (!ok) {
        std::fprintf(stderr, "Disjoint storage check failed: %s:%u\n", where.file_name(), where.line());
        std::abort();
    }
}

struct Affine {
    std::uint64_t a = 1, b = 0;
    friend bool operator==(const Affine&, const Affine&) = default;
};
struct Composition {
    static constexpr std::uint64_t modulus = 998244353;
    Affine operator()(const Affine& left, const Affine& right) const {
        return {left.a * right.a % modulus, (left.b * right.a + right.b) % modulus};
    }
};

struct NoDefault {
    std::int64_t value;
    NoDefault() = delete;
    explicit NoDefault(std::int64_t v) : value(v) {}
    friend bool operator==(const NoDefault&, const NoDefault&) = default;
};
struct AddNoDefault {
    NoDefault operator()(const NoDefault& a, const NoDefault& b) const {
        return NoDefault(a.value + b.value);
    }
};
struct CopyOnly {
    std::int64_t value;
    CopyOnly() = delete;
    explicit CopyOnly(std::int64_t v) : value(v) {}
    CopyOnly(const CopyOnly&) = default;
    CopyOnly(CopyOnly&&) = delete;
    CopyOnly& operator=(const CopyOnly&) = default;
    friend bool operator==(const CopyOnly&, const CopyOnly&) = default;
};
struct AddCopyOnly {
    CopyOnly operator()(const CopyOnly& a, const CopyOnly& b) const {
        return CopyOnly(a.value + b.value);
    }
};
static_assert(!std::is_default_constructible_v<NoDefault>);
static_assert(!std::is_default_constructible_v<CopyOnly>);
static_assert(std::is_copy_constructible_v<CopyOnly> && !std::is_move_constructible_v<CopyOnly>);

struct Event {
    char kind;
    std::uint64_t left, right;
    friend bool operator==(const Event&, const Event&) = default;
};
using Events = std::vector<Event>;
struct TraceSum {
    std::shared_ptr<Events> events;
    std::uint64_t operator()(std::uint64_t a, std::uint64_t b) {
        events->push_back({'M', a, b});
        return a + b;
    }
    std::uint64_t operator()(std::uint64_t a, std::uint64_t b) const {
        events->push_back({'C', a, b});
        return a + b;
    }
};
struct TraceBits {
    std::shared_ptr<Events> events;
    bool operator()(bool a, std::vector<bool>::reference b) {
        events->push_back({'L', a, bool(b)});
        return a != bool(b);
    }
    bool operator()(std::vector<bool>::reference a, bool b) {
        events->push_back({'R', bool(a), b});
        return bool(a) != b;
    }
    bool operator()(bool a, bool b) const {
        events->push_back({'C', a, b});
        return a != b;
    }
};

// 保持行を使わず、原列の区間和から旧構築順の引数を求める。
// parity=trueなら和の偶奇が区間XORとなる。
Events expected_build(const std::vector<std::uint64_t>& a, bool parity) {
    Events result;
    if (a.size() <= 1) return result;
    std::vector<std::uint64_t> prefix(a.size() + 1);
    for (std::size_t i = 0; i < a.size(); ++i) prefix[i + 1] = prefix[i] + a[i];
    const auto fold = [&](std::size_t l, std::size_t r) {
        const auto sum = prefix[r] - prefix[l];
        return parity ? sum % 2 : sum;
    };
    for (int bit = 0; bit < std::bit_width(a.size() - 1); ++bit) {
        const auto half = std::size_t{1} << bit;
        for (std::size_t begin = 0; begin < a.size();) {
            const auto middle = begin + std::min(half, a.size() - begin);
            const auto end = middle + std::min(half, a.size() - middle);
            for (auto i = middle; i > begin + 1; --i)
                result.push_back({parity ? 'L' : 'M', a[i - 2], fold(i - 1, middle)});
            if (middle < end)
                for (auto i = middle + 1; i < end; ++i)
                    result.push_back({parity ? 'R' : 'M', fold(middle, i), a[i]});
            begin = end;
        }
    }
    return result;
}

void small_ownership(std::size_t n) {
    std::vector<std::int64_t> input(n);
    for (std::size_t i = 0; i < n; ++i) input[i] = static_cast<std::int64_t>(i + 5);
    using Tree = DisjointSparseTable<std::int64_t, std::plus<std::int64_t>>;
    Tree original(input), copied(original);
    Tree moved(std::move(original));
    check(original.empty() && original.all_prod() == 0);
    check(moved.values() == input && copied.values() == input);
    original.assign(std::vector<std::int64_t>{3, 4});
    check(original.all_prod() == 7 && copied.values() == input);
    Tree assigned;
    assigned = copied;
    const auto* const copy_self = std::addressof(assigned);
    assigned = *copy_self;
    check(assigned.values() == input);
    Tree move_assigned(std::vector<std::int64_t>{99});
    move_assigned = std::move(assigned);
    check(assigned.empty() && move_assigned.values() == input);
    auto* const self = std::addressof(move_assigned);
    move_assigned = std::move(*self);
    check(move_assigned.values() == input);
    copied.swap(original);
    check(copied.values() == std::vector<std::int64_t>({3, 4}));
    check(original.values() == input && moved.values() == input);
    original.clear();
    check(original.empty() && original.prod(0, 0) == 0 && original.values().empty());
    original.assign(input);
    assigned.assign(input);
    check(original.values() == input && assigned.values() == input);
}

void value_types(std::size_t n) {
    std::vector<NoDefault> a;
    std::vector<CopyOnly> b;
    a.reserve(n); b.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        a.emplace_back(static_cast<std::int64_t>(i + 1));
        b.emplace_back(static_cast<std::int64_t>(i + 1));
    }
    DisjointSparseTable<NoDefault, AddNoDefault> table(a, NoDefault(0));
    DisjointSparseTable<CopyOnly, AddCopyOnly> copy_only(b, CopyOnly(0));
    check(table.size() == n && copy_only.size() == n);
    for (std::size_t l = 0; l <= n; ++l) {
        std::int64_t sum = 0;
        for (std::size_t r = l; r <= n; ++r) {
            check(table.prod(l, r).value == sum);
            check(copy_only.prod(l, r).value == sum);
            if (r < n) sum += static_cast<std::int64_t>(r + 1);
        }
    }
    auto duplicate = copy_only;
    auto moved = std::move(copy_only);
    copy_only.clear();
    check(copy_only.empty() && copy_only.all_prod().value == 0);
    check(duplicate.values() == b && moved.values() == b);
    table.assign(a);
    auto reassigned = table;
    reassigned = table;
    table.clear();
    check(reassigned.values() == a && table.empty());
}

void large_boundaries(std::size_t n, std::mt19937_64& random) {
    std::vector<Affine> functions(n);
    std::vector<std::int64_t> values(n), prefix(n + 1);
    std::vector<bool> bits(n);
    for (std::size_t i = 0; i < n; ++i) {
        functions[i] = {random() % Composition::modulus, random() % Composition::modulus};
        values[i] = static_cast<std::int64_t>(random() % 11);
        prefix[i + 1] = prefix[i] + values[i];
        bits[i] = (random() & 1) != 0;
    }
    DisjointSparseTable<Affine, Composition> affine(functions);
    DisjointSparseTable<std::int64_t, std::plus<std::int64_t>> sums(values);
    DisjointSparseTable<bool, std::bit_xor<bool>> boolean(bits, false);
    check(affine.values() == functions && sums.values() == values && boolean.values() == bits);
    for (int trial = 0; trial < 250; ++trial) {
        auto l = static_cast<std::size_t>(random() % (n + 1));
        auto r = static_cast<std::size_t>(random() % (n + 1));
        if (l > r) std::swap(l, r);
        if (trial == 0) { l = 0; r = n; }
        if (trial == 1 && n) { l = n - 1; r = n; }
        Affine expected;
        bool parity = false;
        for (auto i = l; i < r; ++i) {
            expected = Composition{}(expected, functions[i]);
            parity = parity != bool(bits[i]);
        }
        check(affine.prod(l, r) == expected);
        check(sums.prod(l, r) == prefix[r] - prefix[l]);
        check(boolean.prod(l, r) == parity);
        const auto limit = static_cast<std::int64_t>(random() % 71);
        auto right = l, left = r;
        while (right < n && prefix[right + 1] - prefix[l] <= limit) ++right;
        while (left > 0 && prefix[r] - prefix[left - 1] <= limit) --left;
        check(sums.max_right(l, [=](auto sum) { return sum <= limit; }) == right);
        check(sums.min_left(r, [=](auto sum) { return sum <= limit; }) == left);
    }
    auto independent = affine;
    auto moved = std::move(affine);
    check(affine.empty() && affine.all_prod() == Affine{});
    moved.clear();
    check(independent.values() == functions);
    affine.assign(functions);
    check(affine.values() == functions && moved.empty());
}

void call_order(std::size_t n, std::mt19937_64& random) {
    std::vector<std::uint64_t> numbers(n), bit_values(n), prefix(n + 1);
    std::vector<bool> bits(n);
    for (std::size_t i = 0; i < n; ++i) {
        numbers[i] = random() % 1009;
        prefix[i + 1] = prefix[i] + numbers[i];
        bits[i] = (random() & 1) != 0;
        bit_values[i] = bits[i];
    }
    auto number_events = std::make_shared<Events>();
    auto bit_events = std::make_shared<Events>();
    DisjointSparseTable<std::uint64_t, TraceSum> table(numbers, 0, TraceSum{number_events});
    DisjointSparseTable<bool, TraceBits> boolean(bits, false, TraceBits{bit_events});
    check(*number_events == expected_build(numbers, false));
    check(*bit_events == expected_build(bit_values, true));
    check(table.values() == numbers && boolean.values() == bits);
    for (int trial = 0; trial < 80; ++trial) {
        auto l = static_cast<std::size_t>(random() % (n + 1));
        auto r = static_cast<std::size_t>(random() % (n + 1));
        if (l > r) std::swap(l, r);
        if (trial == 0) { l = 0; r = n; }
        if (trial == 1 && n) { l = n - 1; r = n; }
        number_events->clear(); bit_events->clear();
        check(table.prod(l, r) == prefix[r] - prefix[l]);
        bool parity = false;
        for (auto i = l; i < r; ++i) parity = parity != bool(bits[i]);
        check(boolean.prod(l, r) == parity);
        if (r - l <= 1) {
            check(number_events->empty() && bit_events->empty());
        } else {
            const auto half = std::bit_floor(l ^ (r - 1));
            const auto middle = l / (2 * half) * (2 * half) + half;
            const Event expected{'C', prefix[middle] - prefix[l], prefix[r] - prefix[middle]};
            check(number_events->size() == 1 && number_events->front() == expected);
            bool left = false, right = false;
            for (auto i = l; i < middle; ++i) left = left != bool(bits[i]);
            for (auto i = middle; i < r; ++i) right = right != bool(bits[i]);
            check(bit_events->size() == 1 && bit_events->front() == Event{'C', left, right});
        }
    }
}
}

int main() {
    std::mt19937_64 random(0x6519a278ULL);
    for (const auto n : {0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
                         127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025,
                         2047, 2048, 2049, 4095, 4096, 4097}) {
        large_boundaries(static_cast<std::size_t>(n), random);
        call_order(static_cast<std::size_t>(n), random);
    }
    for (std::size_t n = 0; n <= 2; ++n) small_ownership(n);
    for (const auto n : {0, 1, 2, 3, 9, 33, 129}) value_types(static_cast<std::size_t>(n));
    std::printf("Disjoint storage/type/order checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
