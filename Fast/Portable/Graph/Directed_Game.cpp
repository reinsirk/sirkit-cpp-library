// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Directed_Game.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/directed_game.md
//
// API一覧
// solve_directed_game(n,edges): 2人が有向辺を交互にたどり、動けない側が負けるゲームを解く。
//   edgesは0始まりの(from,to)。同じ頂点や辺を再訪可。時間・領域O(n+m)。
//   各頂点のDirectedGamePosition{outcome,plies,next}を返す。
//   outcomeは手番側のwin/lose/draw。無限に続けて互いに負けを避けられるならdraw。
//   pliesは勝者が早く終わらせ、敗者が粘る場合の残り手数。drawなら-1。
//   nextはその手数を達成できる移動先。drawでは引き分けを保つ行先。終端は-1。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DIRECTED_GAME_DEFINED
#define SIRKIT_DIRECTED_GAME_DEFINED

enum class DirectedGameOutcome : std::uint8_t
{
    win,
    lose,
    draw,
};

struct DirectedGamePosition
{
    DirectedGameOutcome outcome = DirectedGameOutcome::draw;
    int plies = -1;
    int next = -1;
};

inline std::vector<DirectedGamePosition> solve_directed_game(
    int n, std::span<const std::pair<int, int>> edges)
{
    using Outcome = DirectedGameOutcome;
    std::vector<std::size_t> remaining(n);
    std::vector<std::vector<int>> predecessors(n);
    for (const auto &[u, v] : edges)
    {
        ++remaining[u];
        predecessors[v].push_back(u);
    }

    std::vector<DirectedGamePosition> result(n);
    std::vector<int> pending;
    pending.reserve(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        if (remaining[vertex] == 0)
        {
            result[vertex] = {Outcome::lose, 0, -1};
            pending.push_back(vertex);
        }
    }

    // 確定頂点を手数の昇順で処理する。勝ち側は最初の負け遷移、負け側は最後の勝ち遷移を選ぶ。
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int v = pending[i];
        const auto position = result[v];
        for (const int u : predecessors[v])
        {
            if (result[u].outcome != Outcome::draw)
            {
                continue;
            }
            if (position.outcome == Outcome::lose)
            {
                result[u] = {Outcome::win, position.plies + 1, v};
                pending.push_back(u);
            }
            else if (--remaining[u] == 0)
            {
                result[u] = {Outcome::lose, position.plies + 1, v};
                pending.push_back(u);
            }
        }
    }

    for (const auto &[u, v] : edges)
    {
        if (result[u].outcome == Outcome::draw && result[u].next == -1 && result[v].outcome == Outcome::draw)
        {
            result[u].next = v;
        }
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
