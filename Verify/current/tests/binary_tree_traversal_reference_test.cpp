// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Binary_Tree_Traversal.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Tree = sirkit::IndexedBinaryTree;

void traverse(const Tree &tree, int root, std::array<std::vector<int>, 3> &orders)
{
    if (root == -1)
    {
        return;
    }
    const auto [left, right] = tree.children[root];
    orders[0].push_back(root);
    traverse(tree, left, orders);
    orders[1].push_back(root);
    traverse(tree, right, orders);
    orders[2].push_back(root);
}

std::vector<Tree> tree_shapes(int n, int first, int last)
{
    if (first == last)
    {
        return {{-1, std::vector<std::array<int, 2>>(n, {-1, -1})}};
    }
    std::vector<Tree> shapes;
    for (int root = first; root < last; ++root)
    {
        for (const Tree &left : tree_shapes(n, first, root))
        {
            for (const Tree &right : tree_shapes(n, root + 1, last))
            {
                Tree merged{root, left.children};
                merged.children[root] = {left.root, right.root};
                std::copy(right.children.begin() + root + 1, right.children.end(), merged.children.begin() + root + 1);
                shapes.push_back(std::move(merged));
            }
        }
    }
    return shapes;
}

void validate(const std::optional<Tree> &actual, const Tree &expected)
{
    assert(actual && actual->root == expected.root && actual->children == expected.children);
}
} // namespace

int main()
{
    using sirkit::binary_tree_from_postorder_inorder;
    using sirkit::binary_tree_from_preorder_inorder;

    for (int n = 0; n <= 6; ++n)
    {
        std::vector<int> inorder(n);
        std::iota(inorder.begin(), inorder.end(), 0);
        std::array<std::map<std::vector<int>, Tree>, 2> expectations;
        for (const Tree &shape : tree_shapes(n, 0, n))
        {
            std::array<std::vector<int>, 3> sequences;
            traverse(shape, shape.root, sequences);
            assert(sequences[1] == inorder);
            assert(expectations[0].emplace(sequences[0], shape).second);
            assert(expectations[1].emplace(sequences[2], shape).second);
        }
        auto order = inorder;
        do
        {
            const std::array actual{binary_tree_from_preorder_inorder(order, inorder),
                                    binary_tree_from_postorder_inorder(order, inorder)};
            for (std::size_t i = 0; i < actual.size(); ++i)
            {
                const auto position = expectations[i].find(order);
                assert(actual[i].has_value() == (position != expectations[i].end()));
                if (actual[i])
                {
                    validate(actual[i], position->second);
                }
            }
        } while (std::next_permutation(order.begin(), order.end()));
    }

    std::mt19937 random(2556570);
    for (int trial = 0; trial < 960; ++trial)
    {
        const int n = static_cast<int>(random() % 64) + 1;
        std::vector<int> labels(n);
        std::iota(labels.begin(), labels.end(), 0);
        std::shuffle(labels.begin(), labels.end(), random);
        Tree generated{labels[0], std::vector<std::array<int, 2>>(n, {-1, -1})};
        std::vector<std::pair<int, int>> available{{labels[0], 0}, {labels[0], 1}};
        for (int i = 1; i < n; ++i)
        {
            const auto slot = random() % available.size();
            const auto [v, side] = available[slot];
            available[slot] = available.back();
            available.pop_back();
            generated.children[v][side] = labels[i];
            available.emplace_back(labels[i], 0);
            available.emplace_back(labels[i], 1);
        }
        std::array<std::vector<int>, 3> sequences;
        traverse(generated, generated.root, sequences);
        validate(binary_tree_from_preorder_inorder(sequences[0], sequences[1]), generated);
        validate(binary_tree_from_postorder_inorder(sequences[2], sequences[1]), generated);
    }

    constexpr std::array good{0, 1, 2};
    constexpr std::array duplicate{0, 1, 1};
    constexpr std::array negative{-1, 1, 2};
    constexpr std::array out{0, 1, 3};
    for (const auto invalid : {duplicate, negative, out})
    {
        assert(!binary_tree_from_preorder_inorder(good, invalid));
        assert(!binary_tree_from_preorder_inorder(invalid, good));
        assert(!binary_tree_from_postorder_inorder(good, invalid));
        assert(!binary_tree_from_postorder_inorder(invalid, good));
    }
    assert(!binary_tree_from_postorder_inorder(good, {}));
    const auto largest = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(sirkit::detail::checked_binary_tree_size(largest) == static_cast<int>(largest));
    bool caught = false;
    try
    {
        (void)sirkit::detail::checked_binary_tree_size(largest + 1);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}
