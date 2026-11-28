// SPDX-License-Identifier: CC0-1.0
#include "../String/Dynamic_Prefix_Filter.cpp"

#include <cassert>

int main()
{
    sirkit::DynamicPrefixFilter filter;
    filter.insert("apple", 2);
    filter.insert("banana");
    filter.block("app");
    assert(filter.allowed_count() == 1);
    filter.unblock("app");
    assert(filter.allowed_count() == 3);
}
