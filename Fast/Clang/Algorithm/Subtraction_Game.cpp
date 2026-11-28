// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Subtraction_Game.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subtraction_game.md
// 参考（Verify問題）: https://atcoder.jp/contests/tessoku-book/tasks/tessoku_book_ah
//
// API一覧
// SubtractionGame(moves,limit=0): 指定個数の石を取り除く不偏ゲーム。movesに0があればinvalid_argument。
// extend(limit): 0〜limit個のGrundy数を用意する。作成済みの状態は再計算しない。
// grundy(stones): 山1つのGrundy数。O(1)。上限外はout_of_range。
// winning_move(piles): 複数の山について{pile,removed}を1組返す。先手が勝てなければnullopt。
// movesの長さK、山P個として初期化O(K log(K+1)+(K+1)*limit)時間・O(K+limit+1)領域、必勝手の取得O(P+K)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBTRACTION_GAME_DEFINED
#define SIRKIT_SUBTRACTION_GAME_DEFINED

struct SubtractionMove
{
    std::size_t pile = 0;
    std::size_t removed = 0;
};

class SubtractionGame
{
public:
    explicit SubtractionGame(std::span<const std::size_t> moves, std::size_t limit = 0)
        : moves_(moves.begin(), moves.end())
    {
        std::ranges::sort(moves_);
        if (!moves_.empty() && moves_.front() == 0)
        {
            throw std::invalid_argument("SubtractionGame: removal count must be positive");
        }
        moves_.erase(std::unique(moves_.begin(), moves_.end()), moves_.end());
        extend(limit);
    }

    void extend(std::size_t limit)
    {
        const auto old_count = values_.size();
        if (limit < old_count)
        {
            return;
        }
        if (limit >= values_.max_size())
        {
            throw std::length_error("SubtractionGame: too many stones");
        }
        std::vector<std::size_t> stamps(moves_.size() + 1);
        values_.resize(limit + 1);
        for (auto stones = std::max(old_count, std::size_t{1}); stones <= limit; ++stones)
        {
            for (auto move : moves_)
            {
                if (move > stones)
                {
                    break;
                }
                stamps[values_[stones - move]] = stones;
            }
            std::size_t mex = 0;
            while (stamps[mex] == stones)
            {
                ++mex;
            }
            values_[stones] = mex;
        }
    }

    [[nodiscard]] std::size_t grundy(std::size_t stones) const
    {
        return values_.at(stones);
    }

    [[nodiscard]] std::optional<SubtractionMove> winning_move(std::span<const std::size_t> piles) const
    {
        std::size_t total = 0;
        for (auto stones : piles)
        {
            total ^= grundy(stones);
        }
        const auto winning_pile = std::ranges::find_if(piles, [&](auto stones)
                                                       {
                                                           return (values_[stones] ^ total) < values_[stones];
                                                       });
        if (winning_pile == piles.end())
        {
            return std::nullopt;
        }
        const auto stones = *winning_pile;
        const auto target = values_[stones] ^ total;
        const auto last = std::ranges::upper_bound(moves_, stones); // mexより小さい値を作る合法手は必ず存在する。
        const auto move = std::find_if(moves_.begin(), last, [&](auto removed)
                                       {
                                           return values_[stones - removed] == target;
                                       });
        return SubtractionMove{static_cast<std::size_t>(winning_pile - piles.begin()), *move};
    }

private:
    std::vector<std::size_t> moves_;
    std::vector<std::size_t> values_{0};
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
