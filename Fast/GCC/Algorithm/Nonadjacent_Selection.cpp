// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Nonadjacent_Selection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/nonadjacent_selection.md
// 参考（貪欲法）: https://atcoder.jp/contests/abc218/editorial/2633
//
// API一覧
// NonadjacentSelection<Score=int64_t>(weights,limit=上限なし): 隣接しない要素をちょうどk個取る最大和を準備する。
// sums(): sums()[k]が最大和。kの範囲は0〜min(limit,ceil(n/2))。参照をO(1)で返す。
// positions(k): 0始まりの添字の一例を昇順で返す。未計算・実行不能のkはnullopt。O(n+K)時間・領域。
// K=min(limit,ceil(n/2))。前処理O(n+K log(n+1))時間・O(n+K)領域。途中の加減算もScoreに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <ranges>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NONADJACENT_SELECTION_DEFINED
#define SIRKIT_NONADJACENT_SELECTION_DEFINED

template <class Score = std::int64_t>
class NonadjacentSelection
{
    static constexpr auto kNoIndex = std::numeric_limits<std::size_t>::max();

    struct Slot
    {
        std::size_t previous = 0;
        std::size_t next = 0;
        std::size_t id = kNoIndex;
        Score gain{};
    };

    struct Candidate
    {
        Score gain{};
        std::size_t position = 0;
        std::size_t id = 0;

        bool operator<(const Candidate &other) const
        {
            return gain < other.gain;
        }
    };

public:
    template <class Range>
        requires std::ranges::input_range<const Range>
    explicit NonadjacentSelection(const Range &weights, std::size_t limit = kNoIndex)
    {
        std::vector<Slot> slots;
        std::vector<Candidate> candidates;
        for (const auto &weight : weights)
        {
            const auto id = slots.size();
            const auto gain = static_cast<Score>(weight);
            slots.push_back({id == 0 ? kNoIndex : id - 1, id + 1, id, gain});
            candidates.push_back({gain, id, id});
        }
        size_ = slots.size();
        const auto count = std::min(limit, size_ / 2 + size_ % 2);
        sums_.reserve(count + 1);
        sums_.push_back(Score{});
        if (count == 0)
        {
            return;
        }

        const auto sentinel = size_;
        slots.front().previous = sentinel;
        slots.push_back({size_ - 1, 0, kNoIndex, Score{}});
        std::priority_queue heap(std::less<Candidate>{}, std::move(candidates));
        choices_.reserve(count);
        replacements_.reserve(count);
        auto link = [&](std::size_t left, std::size_t right)
        {
            slots[left].next = right;
            slots[right].previous = left;
        };
        for (;;)
        {
            const auto best = heap.top();
            heap.pop();
            auto &current = slots[best.position];
            if (current.id != best.id)
            {
                continue;
            }
            sums_.push_back(sums_.back() + best.gain);
            choices_.push_back(best.id);
            if (choices_.size() == count)
            {
                break;
            }
            const auto left = current.previous;
            const auto right = current.next;
            const auto before = left == sentinel ? sentinel : slots[left].previous;
            const auto after = right == sentinel ? sentinel : slots[right].next;
            if (left != sentinel && right != sentinel)
            {
                // 両隣へ入れ替える増分を1要素として残す。
                const auto id = size_ + replacements_.size();
                const Score gain = slots[left].gain + slots[right].gain - best.gain;
                replacements_.push_back({slots[left].id, best.id, slots[right].id});
                current = {before, after, id, gain};
                heap.push({gain, best.position, id});
                link(before, best.position);
                link(best.position, after);
            }
            else
            {
                current.id = kNoIndex;
                link(before, after);
            }
            slots[left].id = slots[right].id = kNoIndex;
        }
    }

    [[nodiscard]] const std::vector<Score> &sums() const noexcept
    {
        return sums_;
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> positions(std::size_t count) const
    {
        if (count >= sums_.size())
        {
            return std::nullopt;
        }
        std::vector<std::uint8_t> chosen(size_ + replacements_.size());
        for (std::size_t i = 0; i < count; ++i)
        {
            chosen[choices_[i]] ^= 1;
        }
        for (auto id = replacements_.size(); id > 0; --id)
        {
            if (chosen[size_ + id - 1] != 0)
            {
                for (auto child : replacements_[id - 1])
                {
                    chosen[child] ^= 1;
                }
            }
        }
        std::vector<std::size_t> result;
        result.reserve(count);
        for (std::size_t i = 0; i < size_; ++i)
        {
            if (chosen[i] != 0)
            {
                result.push_back(i);
            }
        }
        return result;
    }

private:
    std::size_t size_ = 0;
    std::vector<Score> sums_;
    std::vector<std::size_t> choices_;
    std::vector<std::array<std::size_t, 3>> replacements_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
