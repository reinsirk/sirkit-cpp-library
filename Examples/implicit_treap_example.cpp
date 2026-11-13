#include "../DataStructure/implicit_treap.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using SumTreap = lazy_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;
using IdSumTreap = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;

int main() {
    SumTreap sequence(std::vector<RangeAddSum::S>{{1, 1}, {2, 1}, {3, 1}, {4, 1}});
    sequence.apply(1, 3, 10);
    sequence.reverse(0, 4);
    sequence.set(1, {5, 1});
    sequence.insert_at(2, {7, 1});
    sequence.erase_at(4);
    sequence.rotate(0, 1, 4); // [4,5,7,12] -> [5,7,12,4]。
    std::cout << sequence.prod(0, 3).sum << ' ' << sequence.all_prod().sum << '\n'; // 24 28
    std::cout << sequence.max_right(0, [](RangeAddSum::S x) { return x.sum <= 12; }) << '\n'; // 2
    std::cout << sequence.min_left(4, [](RangeAddSum::S x) { return x.sum <= 16; }) << '\n'; // 2
    for (auto x : sequence.to_vector()) std::cout << x.sum << ' '; // 5 7 12 4
    std::cout << '\n';
    SumTreap copy = sequence;
    SumTreap moved = std::move(copy);
    std::cout << copy.empty() << ' ' << moved.size() << '\n'; // 1 4
    moved.pop_front();
    moved.pop_back();
    std::cout << moved.get(0).sum << ' ' << moved.size() << '\n'; // 7 2
    SumTreap assigned;
    assigned.reserve(8);
    assigned = sequence;
    assigned.erase_at(0);
    assert(assigned.size() == 3 && sequence.size() == 4); // コピー代入後も独立した列。

    lazy_implicit_treap<> ordered;
    ordered.reserve(10);
    for (std::int64_t x : {3, 1, 3, 2}) ordered.insert_sorted(x);
    std::cout << ordered.get_rank(3) << ' ' << ordered.count(3) << ' ' << ordered.contains(2) << '\n'; // 2 2 1
    std::cout << ordered.lower_bound(3) << ' ' << ordered.upper_bound(3) << '\n'; // 2 4
    ordered.erase_val(3);
    ordered.erase_max();
    std::cout << ordered.size() << ' ' << ordered.all_prod() << '\n'; // 2 1

    // 区間を、移動前の位置を指定して移す。
    lazy_implicit_treap<> editing(std::vector<std::int64_t>{10, 20, 30, 40, 50});
    editing.move_range(1, 3, 5); // [10,40,50,20,30]。元の末尾へ[20,30]を移動。
    std::cout << editing.front() << ' ' << editing.back() << '\n'; // 10 30

    // IDは値を変更しても保持される。
    IdSumTreap ids;
    ids.insert_at_with_id(0, 10, {100, 1});
    ids.insert_after(10, 30, {300, 1});
    ids.insert_before(30, 20, {200, 1}); // 値は[100,200,300]、IDは[10,20,30]。
    ids.set_by_id(20, {250, 1});
    std::cout << ids.get_by_id(20).sum << '\n'; // 250
    std::cout << ids.prod_between(30, 10).sum << '\n'; // 250。端点の前後はどちらでもよい。
    std::cout << ids.erase_between(10, 30).sum << '\n'; // 250。端点は残す。
    std::cout << ids.erase_id(20) << '\n'; // 0。すでに削除済み。
    std::cout << ids.erase_id(30) << '\n'; // 1
}
