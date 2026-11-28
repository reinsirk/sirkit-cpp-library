// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Stack.cpp"

#include <cassert>

int main()
{
    using Stack = sirkit::PersistentStack<int>;
    Stack stack;
    const auto first = stack.push(Stack::kEmpty, 3);
    const auto a = stack.push(first, 7);
    const auto b = stack.push(first, 9);
    assert(stack.top(a) == 7 && stack.top(b) == 9);
    assert(stack.pop(b) == first && stack.top(first) == 3);
}
