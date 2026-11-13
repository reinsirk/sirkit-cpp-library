// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/implicit_treap.cpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using SumIds = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;

static std::uint64_t checks = 0;
static void require(bool value) {
    ++checks;
    if (!value) throw std::runtime_error("implicit treap ID reference mismatch");
}
struct Entry { int id; std::int64_t value; };

static void validate(SumIds& tree, const std::vector<Entry>& model) {
    require(tree.size() == static_cast<int>(model.size()));
    require(tree.empty() == model.empty());
    std::int64_t sum = 0;
    for (int p = 0; p < static_cast<int>(model.size()); ++p) {
        require(tree.get_id(p) == model[p].id);
        const auto value = tree.get(p);
        require(value.sum == model[p].value && value.size == 1);
        sum += model[p].value;
    }
    require(tree.all_prod().sum == sum);
    require(tree.all_prod().size == static_cast<std::int64_t>(model.size()));
    for (int id = -2; id <= 129; ++id) {
        const auto it = std::find_if(model.begin(), model.end(),
                                     [id](const Entry& x) { return x.id == id && id >= 0; });
        const int expected = it == model.end() ? -1 : static_cast<int>(it - model.begin());
        require(tree.position_of(id) == expected);
        require(tree.contains_id(id) == (expected != -1));
    }
    require(tree.position_of(std::numeric_limits<int>::max()) == -1);
    require(!tree.contains_id(std::numeric_limits<int>::max()));
}

#ifndef SKIP_RUNTIME_CHECKS
template<class F> static void rejected(F function) {
    bool threw = false;
    try { function(); } catch (const std::exception&) { threw = true; }
    require(threw);
}
#endif

static void lifecycle() {
    SumIds built(std::vector<RangeAddSum::S>{{4, 1}, {5, 1}, {6, 1}});
    built.insert_at_with_id(1, 7, {8, 1});
    built.reserve_ids(128);
    built.reserve_ids(2);
    validate(built, {{-1, 4}, {7, 8}, {-1, 5}, {-1, 6}});
    built.reverse(0, built.size());
    require(built.position_of(7) == 2);
    indexed_implicit_treap<> minimum;
    minimum.insert_at_with_id(0, 4, 11);
    minimum.insert_after(4, 7, 3);
    require(minimum.erase_prod(0, 2) == 3 && minimum.empty());
    require(!minimum.contains_id(4) && !minimum.contains_id(7));
    SumIds a;
    a.reserve(64);
    a.insert_at_with_id(0, 0, {1, 1});
    a.insert_at_with_id(1, 1, {2, 1});
    a.insert_after(0, 2, {3, 1});
    validate(a, {{0, 1}, {2, 3}, {1, 2}});
#ifndef SKIP_RUNTIME_CHECKS
    rejected([&] { a.insert_at_with_id(0, -1, {7, 1}); });
    rejected([&] { a.insert_at_with_id(0, 2, {7, 1}); });
    rejected([&] { a.insert_after(-1, 3, {7, 1}); });
    rejected([&] { a.insert_after(99, 3, {7, 1}); });
    rejected([&] { a.insert_after(0, -1, {7, 1}); });
    rejected([&] { a.insert_after(0, 2, {7, 1}); });
    rejected([&] { (void)a.erase_prod(-1, 1); });
    rejected([&] { (void)a.erase_prod(0, 4); });
    rejected([&] { (void)a.erase_prod(2, 1); });
    validate(a, {{0, 1}, {2, 3}, {1, 2}});
#endif
    SumIds b = a;
    b.set(0, {10, 1});
    require(a.get(0).sum == 1 && b.get_id(0) == 0);
    a.reverse(0, 3);
    require(a.position_of(1) == 0 && a.position_of(0) == 2);
    SumIds c;
    c.insert_at_with_id(0, 99, {99, 1});
    c = a;
    require(!c.contains_id(99));
    validate(c, {{1, 2}, {2, 3}, {0, 1}});
    c = c;
    SumIds moved(std::move(c));
    require(c.empty() && c.position_of(1) == -1);
    c.insert_at_with_id(0, 1, {8, 1});
    require(moved.position_of(1) == 0 && c.position_of(1) == 0);
    b = std::move(c);
    require(c.empty() && !c.contains_id(1));
    validate(b, {{1, 8}});
    swap(b, moved);
    validate(b, {{1, 2}, {2, 3}, {0, 1}});
    validate(moved, {{1, 8}});
    b = std::move(b);
    validate(b, {{1, 2}, {2, 3}, {0, 1}});
    const auto erased = b.erase_prod(0, b.size());
    require(erased.sum == 6 && erased.size == 3);
    require(!b.contains_id(0) && !b.contains_id(1) && !b.contains_id(2));
    b.insert_at_with_id(0, 2, {7, 1});
    validate(b, {{2, 7}});
    b.clear();
    validate(b, {});
    b.insert_at_with_id(0, 2, {11, 1});
    validate(b, {{2, 11}});
    b.erase_at(0);
    b.push_back({22, 1});
    validate(b, {{-1, 22}});
    b.clear();
    b.insert_at_with_id(0, 127, {1, 1});
    b.push_back({3, 1});
    b.insert_sorted({2, 1});
    validate(b, {{127, 1}, {-1, 2}, {-1, 3}});
    b.erase_val({1, 1});
    require(!b.contains_id(127));
    b.insert_at_with_id(0, 127, {0, 1});
    validate(b, {{127, 0}, {-1, 2}, {-1, 3}});
}

static void nested_reversals() {
    SumIds tree;
    constexpr int n = 48;
    for (int i = 0; i < n; ++i) tree.insert_at_with_id(i, i, {i, 1});
    std::vector<Entry> model;
    for (int i = 0; i < n; ++i) model.push_back({i, i});
    tree.apply(0, n, 13);
    for (auto& e : model) e.value += 13;
    for (int k = 0; k < 20; ++k) {
        const int l = k % 9, r = n - k % 7;
        tree.reverse(l, r);
        std::reverse(model.begin() + l, model.begin() + r);
    }
    // getやto_vectorで遅延反転を解消する前に、IDから位置を問い合わせる。
    for (int id = n - 1; id >= 0; --id) {
        const auto it = std::find_if(model.begin(), model.end(), [id](Entry e) { return e.id == id; });
        require(tree.position_of(id) == it - model.begin());
    }
    tree.rotate(3, 17, 45);
    std::rotate(model.begin() + 3, model.begin() + 17, model.begin() + 45);
    validate(tree, model);
}

static void random_reference(std::uint64_t seed) {
    SumIds tree;
    std::vector<Entry> model;
    std::mt19937_64 rng(seed);
    for (int step = 0; step < 3500; ++step) {
        const int n = static_cast<int>(model.size());
        int l = static_cast<int>(rng() % (n + 1));
        int r = static_cast<int>(rng() % (n + 1));
        if (l > r) std::swap(l, r);
        const std::int64_t value = static_cast<std::int64_t>(rng() % 101) - 50;
        int fresh = static_cast<int>(rng() % 128);
        while (std::any_of(model.begin(), model.end(), [fresh](Entry e) { return e.id == fresh; }))
            fresh = (fresh + 1) % 128;
        switch (rng() % 12) {
        case 0:
            if (n < 80) {
                tree.insert_at_with_id(l, fresh, {value, 1});
                model.insert(model.begin() + l, {fresh, value});
            }
            break;
        case 1:
            if (n < 80) {
                tree.insert_at(l, {value, 1});
                model.insert(model.begin() + l, {-1, value});
            }
            break;
        case 2:
            if (n < 80) {
                const auto it = std::find_if(model.begin(), model.end(), [](Entry e) { return e.id >= 0; });
                if (it != model.end()) {
                    const int p = static_cast<int>(it - model.begin());
                    tree.insert_after(it->id, fresh, {value, 1});
                    model.insert(model.begin() + p + 1, {fresh, value});
                }
            }
            break;
        case 3: {
            std::int64_t expected = 0;
            for (int p = l; p < r; ++p) expected += model[p].value;
            const auto erased = tree.erase_prod(l, r);
            require(erased.sum == expected && erased.size == r - l);
            model.erase(model.begin() + l, model.begin() + r);
            break;
        }
        case 4:
            tree.erase(l, r);
            model.erase(model.begin() + l, model.begin() + r);
            break;
        case 5:
            tree.apply(l, r, value);
            for (int p = l; p < r; ++p) model[p].value += value;
            break;
        case 6:
            tree.reverse(l, r);
            std::reverse(model.begin() + l, model.begin() + r);
            break;
        case 7: {
            const int middle = l + static_cast<int>(rng() % (r - l + 1));
            tree.rotate(l, middle, r);
            std::rotate(model.begin() + l, model.begin() + middle, model.begin() + r);
            break;
        }
        case 8:
            if (n) {
                const int p = static_cast<int>(rng() % n);
                tree.set(p, {value, 1});
                model[p].value = value;
            }
            break;
        case 9:
            if (n) {
                const int p = static_cast<int>(rng() % n);
                tree.erase_at(p);
                model.erase(model.begin() + p);
            }
            break;
        case 10: {
            SumIds copy(tree);
            validate(copy, model);
            tree = copy;
            break;
        }
        case 11:
            if (step % 41 == 0) { tree.clear(); model.clear(); }
            break;
        }
        if (step % 7 == 0) validate(tree, model);
    }
    validate(tree, model);
}

static std::string text_op(std::string a, std::string b) { return a + b; }
static std::string text_e() { return {}; }
static std::string text_map(int, std::string x) { return x; }
static int text_composition(int, int) { return 0; }
static int text_id() { return 0; }
using TextIds = indexed_implicit_treap<std::string, text_op, text_e, int,
                                      text_map, text_composition, text_id>;
static void ordered_aggregate() {
    TextIds tree;
    for (int i = 0; i < 5; ++i)
        tree.insert_at_with_id(i, i, std::string(1, static_cast<char>('a' + i)));
    tree.reverse(0, 5);
    tree.rotate(0, 2, 5);
    require(tree.all_prod() == "cbaed");
    require(tree.position_of(4) == 3 && tree.position_of(0) == 2);
    require(tree.erase_prod(1, 4) == "bae");
    require(tree.all_prod() == "cd");
    require(!tree.contains_id(0) && !tree.contains_id(1) && !tree.contains_id(4));
    require(tree.position_of(2) == 0 && tree.position_of(3) == 1);
}

int main() {
    lifecycle();
    nested_reversals();
    for (std::uint64_t seed : {1, 19, 151, 57623}) random_reference(seed);
    ordered_aggregate();
    std::cout << "{\"checks\":" << checks << ",\"status\":\"PASS\"}\n";
}
