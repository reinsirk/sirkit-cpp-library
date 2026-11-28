// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Round_Robin_Schedule.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/round_robin_schedule.md
// 参考（完全グラフの辺彩色）: https://info.atcoder.jp/entry/algorithm_lectures/complete_graph_edge_coloring
//
// API一覧
// [RoundRobinSchedule]
// RoundRobinSchedule(n): n人の総当たり戦を最小のラウンド数で配置。1ラウンドにつき1人1試合まで。
// size()/round_count(): 選手数/ラウンド数。n<=1なら0ラウンド。
// partner(r,u): ラウンドrの選手uの対戦相手。試合がなければ-1。
// round_of(u,v): 選手u,vの試合のラウンド。同じ選手なら-1。
// 番号はすべて0始まり。負のnはinvalid_argument、範囲外の照会はout_of_range。
// 全操作O(1)時間、保持する領域もO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROUND_ROBIN_SCHEDULE_DEFINED
#define SIRKIT_ROUND_ROBIN_SCHEDULE_DEFINED

class RoundRobinSchedule
{
    int players_;

    void check_player(int player) const
    {
        if (player < 0 || player >= players_)
        {
            throw std::out_of_range("round-robin schedule: player out of range");
        }
    }

public:
    explicit RoundRobinSchedule(int players) : players_(players)
    {
        if (players < 0)
        {
            throw std::invalid_argument("round-robin schedule: negative number of players");
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return players_;
    }

    [[nodiscard]] int round_count() const noexcept
    {
        if (players_ <= 1)
        {
            return 0;
        }
        return players_ - (players_ % 2 == 0);
    }

    [[nodiscard]] int partner(int round, int player) const
    {
        check_player(player);
        const int circle = round_count();
        if (round < 0 || round >= circle)
        {
            throw std::out_of_range("round-robin schedule: round out of range");
        }
        if (player == round)
        {
            return circle < players_ ? circle : -1;
        }
        if (player == circle)
        {
            return round;
        }
        // 円周上の対称位置。偶数人のとき、円外の1人はそのラウンドの中心と対戦する。
        return static_cast<int>((2 * std::int64_t{round} + circle - player) % circle);
    }

    [[nodiscard]] int round_of(int u, int v) const
    {
        check_player(u);
        check_player(v);
        if (u == v)
        {
            return -1;
        }
        const int circle = round_count();
        if (u == circle)
        {
            return v;
        }
        if (v == circle)
        {
            return u;
        }
        const auto sum = (std::int64_t{u} + v) % circle;
        return static_cast<int>((sum + (sum % 2 == 0 ? 0 : circle)) / 2);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
