// SPDX-License-Identifier: CC0-1.0
#include <iostream>
#include <vector>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t alphabet = 0, count = 0, limit = 0, queries = 0;
    std::cin >> alphabet >> count >> limit >> queries;
    const auto read_word = []
    {
        std::size_t length = 0;
        std::cin >> length;
        std::vector<int> word(length);
        for (auto &g : word)
        {
            std::cin >> g;
        }
        return word;
    };
    std::vector<MonoidRelation> relations;
    relations.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        auto left = read_word();
        relations.push_back({std::move(left), read_word()});
    }
    const auto graph = todd_coxeter(alphabet, relations, limit);
    if (!graph)
    {
        std::cout << "-1\n";
        return 0;
    }
    std::cout << graph->size() << '\n';
    for (std::size_t i = 0; i < queries; ++i)
    {
        const auto left = read_word(), right = read_word();
        std::cout << (graph->evaluate(left) == graph->evaluate(right)) << '\n';
    }
}
