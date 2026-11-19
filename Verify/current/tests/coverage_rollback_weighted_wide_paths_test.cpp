// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/rollback-union-findw.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace {

using Weight = std::int64_t;
using UnionFind = sirkit::RollbackWeightedUnionFind<Weight>;
constexpr auto kMaximum = std::numeric_limits<Weight>::max();
constexpr auto kMinimum = std::numeric_limits<Weight>::min();

struct HistoryLimitMember {
    friend constexpr auto member_pointer(HistoryLimitMember);
};
struct ParentStorageMember {
    friend constexpr auto member_pointer(ParentStorageMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<HistoryLimitMember, &UnionFind::check_history_capacity>;
template struct MemberAccess<ParentStorageMember, &UnionFind::parent_>;

template <class Operation>
void rejects_overflow(Operation operation) {
    try {
        operation();
    } catch (const std::overflow_error&) {
        return;
    }
    assert(false);
}

UnionFind balanced_component(Weight gap) {
    UnionFind groups(17);
    for (int step = 1; step < 16; step *= 2) {
        for (int root = 0; root < 16; root += 2 * step) {
            assert(groups.unite(root, root + step, gap));
        }
    }
    return groups;
}

void high_word_paths() {
    const auto positive = balanced_component(kMaximum);
    // 0,7,11,15はそれぞれ0,3M,3M,4M。表現範囲内の差だけを取り出す。
    assert(positive.diff(7, 11) == 0);
    assert(positive.diff(7, 15) == kMaximum);
    assert(positive.diff(15, 7) == -kMaximum);
    assert(positive.consistent(7, 15, kMaximum));
    assert(!positive.consistent(7, 15, -kMaximum));
    assert(!positive.consistent(0, 15, 0));
    assert(positive.consistent(15, 16, 0));
    assert(positive.consistent(16, 15, 0));
    assert(!positive.try_diff(16, 15));
    assert(!positive.try_diff(15, 16));
    rejects_overflow([&] { (void)positive.diff(1, 15); });
    rejects_overflow([&] { (void)positive.diff(15, 1); });
    rejects_overflow([&] { (void)positive.get_weight(15); });
#ifndef SKIP_RUNTIME_CHECKS
    bool disconnected = false;
    try {
        (void)positive.diff(15, 16);
    } catch (const std::invalid_argument&) {
        disconnected = true;
    }
    assert(disconnected);
#endif

    const auto negative = balanced_component(kMinimum);
    assert(negative.diff(7, 15) == kMinimum);
    assert(negative.try_diff(7, 15) == kMinimum);
    assert(negative.consistent(7, 15, kMinimum));
    assert(negative.diff(7, 11) == 0);
    rejects_overflow([&] { (void)negative.diff(15, 7); });
}

void rejected_merge_preserves_state() {
    UnionFind groups(4);
    assert(groups.unite(0, 1, kMaximum));
    assert(groups.unite(2, 3, -kMaximum));
    const auto state = groups.get_state();
    groups.reserve_history(state);
    rejects_overflow([&] { (void)groups.unite(1, 3, 0); });
    assert(groups.get_state() == state);
    assert(groups.component_count() == 2 && !groups.same(1, 3));
    assert(groups.diff(0, 1) == kMaximum && groups.diff(2, 3) == -kMaximum);

    UnionFind siblings(4);
    assert(siblings.unite(0, 1, kMaximum));
    assert(siblings.unite(0, 2, -kMaximum));
    // 狭い経路和は両方表現できるが、差と新しい孤立頂点の重みは表現できない。
    rejects_overflow([&] { (void)siblings.diff(1, 2); });
    rejects_overflow([&] { (void)siblings.try_diff(2, 1); });
    rejects_overflow([&] { (void)siblings.unite(1, 3, 1); });
    assert(siblings.get_state() == 2 && !siblings.same(1, 3));
}

void narrow_and_reverse_paths() {
    UnionFind groups(6);
    assert(groups.unite(0, 1, -kMaximum));
    assert(groups.unite(2, 3, kMaximum));
    assert(groups.unite(2, 4, 3));
    assert(!groups.try_diff(0, 2) && groups.consistent(0, 2, 77));
    assert(groups.consistent(2, 4, 3) && !groups.consistent(2, 4, 4));
    const auto state = groups.get_state();
    // 右側の成分が大きく、途中で差が溢れても保存する根間の差は表現できる。
    assert(groups.unite_checked(1, 3, kMaximum) == UnionFind::MergeResult::Merged);
    assert(groups.get_weight(1) == 0 && groups.diff(1, 3) == kMaximum);
    groups.rollback(state);
    assert(!groups.same(0, 2) && groups.diff(0, 1) == -kMaximum && groups.diff(2, 3) == kMaximum);
#ifndef SKIP_RUNTIME_CHECKS
    bool disconnected = false;
    try {
        (void)groups.diff(0, 2);
    } catch (const std::invalid_argument&) {
        disconnected = true;
    }
    assert(disconnected);
#endif
}

void capacity_checks() {
    const auto check = member_pointer(HistoryLimitMember{});
    const auto index_limit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    const auto reject = [](auto call) {
        try {
            call();
        } catch (const std::length_error&) {
            return;
        }
        assert(false);
    };
    check(3, 4);
    check(index_limit - 1, index_limit + 1);
    reject([&] { check(index_limit, index_limit + 1); });
    reject([&] { check(4, 4); });

    UnionFind groups(3);
    groups.unite(0, 1, 5);
    // vectorの余剰容量が要素型ごとに違う環境と同じ状態を作る。
    (groups.*member_pointer(ParentStorageMember{})).reserve(40);
    groups.reset(10);
    assert(groups.size() == 10 && groups.component_count() == 10 && groups.get_state() == 0);
    assert(groups.unite(2, 9, 37) && groups.diff(2, 9) == 37);
#ifdef SKIP_RUNTIME_CHECKS
    // 入力検査省略時も範囲外の容量を確保しない。
    reject([&] { (void)UnionFind(-1); });
    reject([&] { groups.reset(-1); });
    assert(groups.size() == 10 && groups.get_state() == 1 && groups.diff(2, 9) == 37);
#endif
}

} // namespace

int main() {
    using Result = UnionFind::MergeResult;
    constexpr auto maximum = kMaximum;

    UnionFind groups(8);
    assert(groups.unite(0, 1, maximum));
    assert(groups.unite(2, 3, maximum));
    // 0,1,2,3のポテンシャルは数学上0, M, 1, M+1。
    assert(groups.unite(0, 2, 1));
    assert(groups.diff(1, 3) == 1);
    assert(groups.try_diff(3, 1) == -1);
    assert(groups.consistent(3, 1, -1));
    assert(!groups.consistent(3, 1, 0));
    assert(!groups.try_diff(3, 7).has_value());
    const int state = groups.get_state();
    assert(groups.unite_checked(3, 1, -1) == Result::AlreadyConsistent);
    assert(groups.unite_checked(3, 1, 0) == Result::Contradiction);
    assert(!groups.unite(3, 1, 1));
    assert(groups.get_state() == state + 3);
    groups.rollback(state);

    assert(groups.unite(4, 5, 2));
    const int before_attach = groups.get_state();
    // 小さい成分から大きい成分へ結合され、4,5のポテンシャルは1,3になる。
    assert(groups.unite_checked(4, 3, maximum) == Result::Merged);
    assert(groups.diff(0, 4) == 1);
    assert(groups.diff(0, 5) == 3);
    // 中間のM+1が表現範囲を超えても、保存する差は1に収まる。
    assert(groups.unite_checked(3, 6, -maximum) == Result::Merged);
    assert(groups.diff(0, 6) == 1);
    assert(groups.component_count() == 2);
    groups.rollback(before_attach);
    assert(!groups.try_diff(3, 4));
    assert(!groups.try_diff(3, 6));
    assert(groups.diff(4, 5) == 2);
    bool overflow = false;
    try {
        (void)groups.diff(0, 3);
    } catch (const std::overflow_error&) {
        overflow = true;
    }
    assert(overflow);
#ifndef SKIP_RUNTIME_CHECKS
    bool disconnected = false;
    try {
        (void)groups.diff(3, 7);
    } catch (const std::invalid_argument&) {
        disconnected = true;
    }
    assert(disconnected);
#endif
    high_word_paths();
    rejected_merge_preserves_state();
    narrow_and_reverse_paths();
    capacity_checks();
}
