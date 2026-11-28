#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2085

// Library: Graph/Tournament_Path.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tournament_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tournament_path.md
//
// API一覧
// tournament_path(n, edge): 0以上n未満の全頂点を通る有向パスを返す。
// edge(u,v)はuからvへの有向辺の有無を返し、相異なる頂点間には反対向きの辺と合わせて常に1本あること。
// edgeをO(1)とするとO(n log(n+1))時間、O(n)領域。n=500なら問い合わせは最大3989回。
// n=0なら空を返す。n<0はチェック有効時に例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOURNAMENT_PATH_DEFINED
#define SIRKIT_TOURNAMENT_PATH_DEFINED

template <class Edge>
[[nodiscard]] std::vector<int> tournament_path(int n, Edge &&edge)
{
    std::vector<int> path(n), buffer(n);
    std::iota(path.begin(), path.end(), 0);
    const auto merge_paths = [&](const auto &self, int left, int right) -> void
    {
        if (right - left < 2)
        {
            return;
        }
        const int middle = left + (right - left) / 2;
        self(self, left, middle);
        self(self, middle, right);
        int first = left, second = middle, write = left;
        // 選んだ先頭は他方の先頭へも、自分のパスの次の頂点へも進める。
        while (first < middle && second < right)
        {
            if (edge(path[first], path[second]))
            {
                buffer[write++] = path[first++];
            }
            else
            {
                buffer[write++] = path[second++];
            }
        }
        auto tail = std::copy(path.begin() + first, path.begin() + middle, buffer.begin() + write);
        std::copy(path.begin() + second, path.begin() + right, tail);
        std::copy(buffer.begin() + left, buffer.begin() + right, path.begin() + left);
    };
    merge_paths(merge_paths, 0, n);
    return path;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <iostream>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    if (!(std::cin >> n))
    {
        return 0;
    }
    const auto path = tournament_path(n, [](int from, int to)
                                      {
                                          std::cout << "? " << from + 1 << ' ' << to + 1 << std::endl;
                                          int answer;
                                          if (!(std::cin >> answer) || answer < 0)
                                          {
                                              std::exit(0);
                                          }
                                          return answer == 1;
                                      });
    std::cout << "!\n"
              << n - 1 << '\n';
    for (int i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            std::cout << ' ';
        }
        std::cout << path[i] + 1;
    }
    std::cout << std::endl;
}
