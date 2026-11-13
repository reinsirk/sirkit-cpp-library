#include "../Graph/Dijkstra.cpp"
#include <iostream>

int main()
{
    WGraph graph(5);
    graph[0].emplace_back(1, 2);
    graph[0].emplace_back(2, 7);
    graph[1].emplace_back(2, 3);
    graph[2].emplace_back(3, 0);
    std::vector<std::int64_t> distances;
    std::vector<int> path;
    Dijkstra(graph, distances, 0, 3, path);
    for (const auto distance : distances)
    {
        if (distance == INF)
            std::cout << "INF\n";
        else
            std::cout << distance << '\n';
    }
    for (std::size_t i = 0; i < path.size(); ++i)
        std::cout << (i == 0 ? "" : " ") << path[i];
    std::cout << '\n'; // 0 1 2 3
    Dijkstra(graph, distances, 0); // 距離のみ。経路用配列の確保・更新を省く。

    std::vector<std::tuple<int,int,std::int64_t>> edges{
        {0,1,2},{0,2,7},{1,2,3},{2,3,0}};
    Dijkstra(5, edges, distances, 0, 3, path); // 入力が辺列なら内部CSRを使う。
    std::cout << distances[3] << ' ' << path.size() << '\n'; // 5 4
    Dijkstra(5, edges, distances, 0); // 辺列版も距離のみで呼べる。
}
