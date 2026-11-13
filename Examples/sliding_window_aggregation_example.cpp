// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sliding_Window_Aggregation.cpp"
#include <cassert>
#include <iostream>
#include <string>
struct Concat {
    std::string operator()(const std::string& a, const std::string& b) const { return a + b; }
};
int main() {
    SlidingWindowAggregation<std::string, Concat> queue("");
    queue.push_back("ab"); queue.push_back("cd"); queue.push_back("ef");
    assert(queue.front() == "ab" && queue.back() == "ef" && queue.size() == 3);
    std::cout << queue.all_prod() << '\n'; // abcdef
    queue.pop_front();
    std::cout << queue.all_prod() << '\n'; // cdef
    auto copy = queue;
    copy.clear(); queue.swap(copy);
    assert(queue.empty() && queue.all_prod().empty());
    queue.push_back("reuse");
    assert(queue.front() == "reuse");
}
