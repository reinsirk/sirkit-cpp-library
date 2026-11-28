// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <unordered_map>

int main()
{
    int q;
    verify_in >> q;
    using Stack = sirkit::PersistentStack<int>;
    Stack stack;
    Stack::version_type current = Stack::kEmpty;
    std::unordered_map<int, Stack::version_type> notebook;
    notebook.reserve(static_cast<std::size_t>(q));
    std::string command;
    for (int i = 0; i < q; ++i)
    {
        verify_in >> command;
        if (command == "DELETE")
        {
            current = stack.pop(current);
        }
        else
        {
            int argument;
            verify_in >> argument;
            if (command == "ADD")
            {
                current = stack.push(current, argument);
            }
            else if (command == "SAVE")
            {
                notebook[argument] = current;
            }
            else
            {
                const auto saved = notebook.find(argument);
                current = saved == notebook.end() ? Stack::kEmpty : saved->second;
            }
        }
        verify_out << (stack.empty(current) ? -1 : stack.top(current)) << (i + 1 == q ? '\n' : ' ');
    }
}
