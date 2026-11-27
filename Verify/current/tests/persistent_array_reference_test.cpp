// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PERSISTENT_ARRAY_TEST_SOURCE
#define SIRKIT_PERSISTENT_ARRAY_TEST_SOURCE "../../../DataStructure/Persistent_Array.cpp"
#define SIRKIT_TEST_SHARED_PERSISTENT_ARRAY
#endif
#include SIRKIT_PERSISTENT_ARRAY_TEST_SOURCE
#ifdef SIRKIT_TEST_SHARED_PERSISTENT_ARRAY
#include "../../../DataStructure/persistent-union-find.cpp"
#endif
#include <cassert>
#include <limits>
#include <string>

namespace
{
template <class Error, class Function>
void expect_exception(Function function)
{
    try
    {
        function();
    }
    catch (const Error &)
    {
        return;
    }
    assert(false);
}
}

int main()
{
    using Array = sirkit::PersistentArray<int, 2>;
    std::vector<int> input(35);
    for (std::size_t i = 0; i < input.size(); ++i)
    {
        input[i] = static_cast<int>(i) - 7;
    }
    Array array(input, -1);
    auto *initial = array.root;
    for (std::size_t i = 0; i < input.size(); ++i)
    {
        assert(array.get(static_cast<std::int64_t>(i)) == input[i]);
    }
    assert(array.size() == 35);
    auto *first = array.update(0, 100);
    auto *second = array.update(initial, 34, 200);
    assert(array.get(initial, 0) == -7 && array.get(first, 0) == 100);
    assert(array.get(second, 0) == -7 && array.get(34) == 200);
    auto *different_subtrees = array.update_two(first, 1, 13, 32, 14);
    auto *adjacent_leaves = array.update_two(different_subtrees, 5, 15, 9, 16);
    auto *same_leaf = array.update_two(0, 6, 1, 18);
    auto *same_index = array.update_two(same_leaf, 1, 19, 1, 20);
    assert((array.get_two(different_subtrees, 1, 32) == std::pair{13, 14}));
    assert((array.get_two(adjacent_leaves, 5, 9) == std::pair{15, 16}));
    assert((array.get_two(0, 1) == std::pair{6, 20}));
    assert(array.get(same_index, 1) == 20 && array.get(same_leaf, 1) == 18);

    Array sparse(64, -9);
    assert(sparse.get(0) == -9 && sparse.get(nullptr, 63) == -9);
    auto *sparse_first = sparse.update(nullptr, 0, 21);
    assert((sparse.get_two(sparse_first, 0, 63) == std::pair{21, -9}));
    assert((sparse.get_two(sparse_first, 63, 0) == std::pair{-9, 21}));
    assert((sparse.get_two(nullptr, 0, 63) == std::pair{-9, -9}));
    assert(sparse.get(63) == -9);
    sparse.update(4, 22);
    assert(sparse.get(1) == -9 && sparse.get(4) == 22);
    Array copy = sparse;
    Array assigned;
    assigned = copy;
    copy.restore(sparse_first);
    assert(sparse.get(4) == 22 && copy.get(4) == -9);
    Array moved(std::move(assigned));
    assert(assigned.size() == 0 && moved.get(4) == 22);
    assigned = std::move(moved);
    assert(moved.size() == 0 && assigned.get(4) == 22);
    auto &same = assigned;
    assigned = std::move(same);
    assert(assigned.get(4) == 22);
    assigned.restore(nullptr);
    assert(assigned.get(0) == -9);

    sirkit::PersistentArray<std::string> strings(2, "初期");
    auto *old = strings.root;
    strings.update(1, "更新");
    assert(strings.get(old, 1) == "初期" && strings.get(1) == "更新");

#ifdef SIRKIT_TEST_SHARED_PERSISTENT_ARRAY
    sirkit::PersistentUnionFind union_find(32);
    assert(union_find.unite(0, 1));
    assert(union_find.unite(16, 17));
    const auto before = union_find.version();
    assert(union_find.root(1) == 0);
    assert(!union_find.issame(1, 17));
    assert(union_find.unite(1, 17));
    assert(union_find.issame(1, 17));
    assert(!union_find.issame(1, 17, before));
#endif

    expect_exception<std::length_error>([]
                                        {
                                            Array bad(-1);
                                        });
    expect_exception<std::length_error>([]
                                        {
                                            sirkit::detail::CheckPersistentArrayNodeCount(std::numeric_limits<std::size_t>::max());
                                        });
    if constexpr (sizeof(std::size_t) >= sizeof(std::int64_t))
    {
        expect_exception<std::length_error>([]
                                            {
                                                sirkit::detail::CheckedPersistentArraySize(std::numeric_limits<std::size_t>::max());
                                            });
    }
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&]
                                        {
                                            array.get(-1);
                                        });
    expect_exception<std::out_of_range>([&]
                                        {
                                            array.get(35);
                                        });
    expect_exception<std::invalid_argument>([&]
                                            {
                                                array.restore(sparse_first);
                                            });
    assert(array.root == same_index);
#endif
}
