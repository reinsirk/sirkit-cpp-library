// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Persistent_Stack.cpp"

#include <cassert>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
struct ThrowingCopy
{
    inline static bool reject = false;
    int value;

    explicit ThrowingCopy(int v) : value(v)
    {
    }

    ThrowingCopy(const ThrowingCopy &other) : value(other.value)
    {
        if (reject)
        {
            throw std::runtime_error("copy refused");
        }
    }

    ThrowingCopy &operator=(const ThrowingCopy &other)
    {
        value = other.value;
        if (reject)
        {
            throw std::runtime_error("assignment refused");
        }
        return *this;
    }
};

void check_assignment()
{
    sirkit::PersistentStack<ThrowingCopy> first, second;
    const auto saved = first.emplace(0, 7);
    const auto alternate = second.emplace(0, 3);
    ThrowingCopy::reject = true;
    bool rejected = false;
    try
    {
        first = second;
    }
    catch (const std::runtime_error &)
    {
        rejected = true;
    }
    ThrowingCopy::reject = false;
    assert(rejected && first.top(saved).value == 7);
    first = second;
    assert(first.top(alternate).value == 3);
}
} // namespace

int main()
{
    check_assignment();
    using Stack = sirkit::PersistentStack<int>;
    Stack stack;
    assert(stack.node_count() == 0 && stack.empty(Stack::kEmpty));
    std::vector<std::vector<int>> expected(1);
    std::mt19937 rng(82846162);
    for (int step = 0; step < 5000; ++step)
    {
        const auto base = rng() % expected.size();
        const int value = static_cast<int>(rng() % 100000);
        auto data = expected[base];
        data.push_back(value);
        const Stack::version_type version = step % 2 == 0 ? stack.push(base, value) : stack.push(base, value + 0);
        assert(version == expected.size() && stack.pop(version) == base);
        assert(stack.size(version) == data.size() && stack.top(version) == value);
        assert(stack.empty(base) == expected[base].empty());
        expected.push_back(std::move(data));
    }
    assert(stack.pop(Stack::kEmpty) == Stack::kEmpty);
    const int &first_element = stack.top(1);
    for (int i = 0; i < 30000; ++i)
    {
        (void)stack.emplace(Stack::kEmpty, -i);
    }
    assert(first_element == expected[1].back() && &first_element == &stack.top(1));
    assert(stack.node_count() == expected.size() - 1 + 30000);

    sirkit::PersistentStack<std::string> text;
    const auto version = text.emplace(0, 3, 'a');
    assert(text.top(version) == "aaa");
    sirkit::PersistentStack<std::unique_ptr<int>> move_only;
    const auto record = move_only.push(0, std::make_unique<int>(44));
    assert(*move_only.top(record) == 44 && move_only.pop(record) == 0);
    sirkit::PersistentStack<std::unique_ptr<int>> transferred;
    transferred = std::move(move_only);
    assert(*transferred.top(record) == 44);
#ifndef SKIP_RUNTIME_CHECKS
    bool empty_top_rejected = false, invalid_version_rejected = false;
    try
    {
        (void)stack.top(Stack::kEmpty);
    }
    catch (const std::out_of_range &)
    {
        empty_top_rejected = true;
    }
    try
    {
        (void)stack.emplace(stack.node_count() + 1, 17);
    }
    catch (const std::out_of_range &)
    {
        invalid_version_rejected = true;
    }
    assert(empty_top_rejected && invalid_version_rejected && stack.node_count() == expected.size() - 1 + 30000);
#endif
}
