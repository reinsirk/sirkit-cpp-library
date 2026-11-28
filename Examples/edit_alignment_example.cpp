// SPDX-License-Identifier: CC0-1.0
#include "../String/Edit_Alignment.cpp"

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>

int main()
{
    const std::string_view source = "kitten";
    const std::string_view target = "sitting";
    const auto alignment = sirkit::edit_alignment(source, target);
    assert(alignment.distance == 3);
    std::string restored;
    std::size_t i = 0, j = 0;
    for (const auto action : alignment.steps)
    {
        switch (action)
        {
        case sirkit::EditAction::kMatch:
            restored.push_back(source[i++]);
            ++j;
            break;
        case sirkit::EditAction::kErase:
            ++i;
            break;
        case sirkit::EditAction::kReplace:
            ++i;
            restored.push_back(target[j++]);
            break;
        case sirkit::EditAction::kInsert:
            restored.push_back(target[j++]);
            break;
        }
    }
    assert(i == source.size() && restored == target);
}
