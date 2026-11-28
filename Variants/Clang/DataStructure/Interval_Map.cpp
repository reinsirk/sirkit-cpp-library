// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Interval_Map.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_map.md
//
// API一覧
// IntervalMap<T>(): 空列。IntervalMap(n,value): 長さnの一様な列。各O(1)。
// IntervalMap(values): 初期列を連続する同値ごとにまとめて構築。O(n)時間。
// size()/empty()/run_count(): 列の長さ/空か/極大同値区間数。O(1)。
// get(i): 位置iの値へのconst参照。run_at(i): iを含む極大同値区間Run{left,right,value}のコピー。各O(log(r+1))。
// assign(l,r,value): 半開区間を置き換え、隣接する同値区間を統合。O(log(区間数+1)+削除する境界数)。
// runs(): 全区間のコピーを左端順に返す。O(区間数+1)時間と領域。
// Tはコピー構築可能。==は同値関係。列の保持領域は区間数に比例。更新の確保失敗時は内容を保つ。
// 0始まりの添字・半開区間。検査有効時に範囲外はout_of_range。コピーは独立、ムーブ元は空列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <iterator>
#include <map>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTERVAL_MAP_DEFINED
#define SIRKIT_INTERVAL_MAP_DEFINED

template <class T>
class IntervalMap
{
    using Map = std::map<std::size_t, T>;
    Map starts_;
    std::size_t size_ = 0;

    [[nodiscard]] typename Map::const_iterator locate(std::size_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size_)
        {
            throw std::out_of_range("IntervalMap: position");
        }
#endif
        return std::prev(starts_.upper_bound(position));
    }

    [[nodiscard]] std::size_t end_of(typename Map::const_iterator run) const
    {
        const auto next = std::next(run);
        return next == starts_.end() ? size_ : next->first;
    }

public:
    struct Run
    {
        std::size_t left, right;
        T value;
    };

    IntervalMap() = default;

    IntervalMap(std::size_t size, const T &value) : size_(size)
    {
        if (size_ != 0)
        {
            starts_.emplace(0, value);
        }
    }

    explicit IntervalMap(std::span<const T> values) : size_(values.size())
    {
        for (std::size_t i = 0; i < size_; ++i)
        {
            if (starts_.empty() || !(starts_.rbegin()->second == values[i]))
            {
                starts_.emplace_hint(starts_.end(), i, values[i]);
            }
        }
    }

    IntervalMap(const IntervalMap &) = default;

    IntervalMap(IntervalMap &&other) noexcept(std::is_nothrow_move_constructible_v<Map>)
        : starts_(std::move(other.starts_)), size_(std::exchange(other.size_, 0))
    {
        other.starts_.clear();
    }

    IntervalMap &operator=(IntervalMap other) noexcept
    {
        starts_.swap(other.starts_);
        std::swap(size_, other.size_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t run_count() const noexcept
    {
        return starts_.size();
    }

    [[nodiscard]] const T &get(std::size_t position) const
    {
        return locate(position)->second;
    }

    [[nodiscard]] Run run_at(std::size_t position) const
    {
        const auto run = locate(position);
        return {run->first, end_of(run), run->second};
    }

    [[nodiscard]] std::vector<Run> runs() const
    {
        std::vector<Run> result;
        result.reserve(starts_.size());
        for (auto it = starts_.begin(); it != starts_.end(); ++it)
        {
            result.push_back({it->first, end_of(it), it->second});
        }
        return result;
    }

    void assign(std::size_t left, std::size_t right, const T &value)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size_)
        {
            throw std::out_of_range("IntervalMap: interval");
        }
#endif
        if (left == right)
        {
            return;
        }

        auto erase_begin = starts_.lower_bound(left);
        auto erase_end = starts_.upper_bound(right);
        Map replacements;

        const bool joins_left = erase_begin != starts_.begin() && std::prev(erase_begin)->second == value;
        if (!joins_left)
        {
            if (erase_begin != starts_.end() && erase_begin->first == left && erase_begin->second == value)
            {
                ++erase_begin;
            }
            else
            {
                replacements.emplace(left, value);
            }
        }
        if (right < size_)
        {
            const auto right_run = std::prev(erase_end);
            if (!(right_run->second == value))
            {
                if (right_run->first == right)
                {
                    erase_end = right_run;
                }
                else
                {
                    replacements.emplace(right, right_run->second);
                }
            }
        }

        // 比較と最大2ノードの確保を済ませてから区間境界を入れ替える。
        starts_.erase(erase_begin, erase_end);
        starts_.merge(replacements);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
