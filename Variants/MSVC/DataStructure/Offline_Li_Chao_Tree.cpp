// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: DataStructure/Offline_Li_Chao_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_li_chao_tree.md
//
// API一覧
// OfflineLiChaoTree<X, Function=OfflineLiChaoLine<X>>(): 空のクエリ列を準備する。
// OfflineLiChaoLine<T>{slope,intercept}: slope*x+interceptを返す既定の関数型。
// add_line(function): 全域で有効な関数を追加し、削除用IDを返す。償却O(1)。
// add_segment(l,r,function): l<=x<rで有効な関数を追加し、削除用IDを返す。償却O(1)。
// erase(id): 有効なIDを削除する。O(1)。IDは追加したオブジェクトの履歴内で使用する。
// query(x): 現在の最小値照会を登録し、その回答の添字を返す。償却O(1)。
// solve(): 照会順に最小値（候補なしはnullopt）を返す。履歴は保持し、追記・再実行可。
// 関数の比較は任意の2関数について、両方が有効な区間で高々1回だけ真偽が変わること。
// 既定の整数型では登録済み座標への評価の中間・最終結果がすべて型内に収まること。
// Q=照会数、X=異なる照会座標数、A=全域追加数、S=区間追加数とする。
// 時間O(Q log(Q+1)+(A+S)log(X+1)+log(Q+1)*(A log(X+1)+S log^2(X+1)))。
// 追加領域O(Q+X+(A+S)log(Q+1)+A log(X+1)+S log^2(X+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_LI_CHAO_TREE_DEFINED
#define SIRKIT_OFFLINE_LI_CHAO_TREE_DEFINED

namespace detail
{
inline std::size_t offline_li_chao_time_base(std::size_t queries, std::size_t capacity)
{
    if (queries > std::bit_floor((capacity - 1) / 2))
    {
        throw std::length_error("OfflineLiChaoTree: too many queries");
    }
    return std::bit_ceil(queries);
}

inline void offline_li_chao_add_count(std::size_t &count, std::size_t increment, std::size_t capacity)
{
    if (increment > capacity - count)
    {
        throw std::length_error("OfflineLiChaoTree: too many active intervals");
    }
    count += increment;
}
}

template <class T>
struct OfflineLiChaoLine
{
    T slope;
    T intercept;

    [[nodiscard]] T operator()(const T &x) const
    {
        return slope * x + intercept;
    }
};

template <class X, class Function = OfflineLiChaoLine<X>>
class OfflineLiChaoTree
{
public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<const Function &, const X &>>;
    using id_type = std::size_t;

    OfflineLiChaoTree() = default;
    OfflineLiChaoTree(const OfflineLiChaoTree &)
        requires std::is_copy_constructible_v<Function>
    = default;
    OfflineLiChaoTree(OfflineLiChaoTree &&) noexcept = default;

    OfflineLiChaoTree &operator=(OfflineLiChaoTree other) noexcept
    {
        functions_.swap(other.functions_);
        queries_.swap(other.queries_);
        return *this;
    }

    id_type add_line(Function function)
    {
        return add(std::move(function), std::nullopt);
    }

    id_type add_segment(X left, X right, Function function)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (right < left)
        {
            throw std::invalid_argument("OfflineLiChaoTree: reversed segment");
        }
#endif
        return add(std::move(function), std::pair{std::move(left), std::move(right)});
    }

    void erase(id_type id)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= functions_.size() || functions_[id].finish != kOngoing)
        {
            throw std::out_of_range("OfflineLiChaoTree: inactive function");
        }
#endif
        functions_[id].finish = queries_.size();
    }

    std::size_t query(X x)
    {
        queries_.push_back(std::move(x));
        return queries_.size() - 1;
    }

    [[nodiscard]] std::vector<std::optional<value_type>> solve() const
    {
        if (queries_.empty())
        {
            return {};
        }
        const std::size_t capacity = std::vector<std::size_t>().max_size();
        const std::size_t base = detail::offline_li_chao_time_base(queries_.size(), capacity);
        std::vector<std::size_t> offsets(2 * base + 1);
        visit_intervals(base, [&](std::size_t node, id_type)
                        {
                            detail::offline_li_chao_add_count(offsets[node + 1], 1, capacity);
                        });
        for (std::size_t node = 1; node < offsets.size(); ++node)
        {
            detail::offline_li_chao_add_count(offsets[node], offsets[node - 1], capacity);
        }
        std::vector<id_type> ids(offsets.back());
        {
            auto cursor = offsets;
            visit_intervals(base, [&](std::size_t node, id_type id)
                            {
                                ids[cursor[node]++] = id;
                            });
        }

        auto coordinates = queries_;
        std::ranges::sort(coordinates);
        coordinates.erase(std::unique(coordinates.begin(), coordinates.end()), coordinates.end());
        const auto index = [&](const X &x)
        {
            return static_cast<std::size_t>(std::ranges::lower_bound(coordinates, x) - coordinates.begin());
        };
        std::vector<std::pair<std::size_t, std::size_t>> domains;
        domains.reserve(functions_.size());
        for (const auto &record : functions_)
        {
            domains.push_back(record.domain ? std::pair{index(record.domain->first), index(record.domain->second)}
                                            : std::pair{std::size_t{0}, coordinates.size()});
        }
        SpatialTree storage(functions_, coordinates);
        // 時間木の葉を左からたどり、照会順に結果を追加する。
        std::vector<std::optional<value_type>> answers;
        answers.reserve(queries_.size());
        const auto search = [&](const auto &visit, std::size_t node) -> void
        {
            if (node >= base && node - base >= queries_.size())
            {
                return;
            }
            const auto before = storage.snapshot();
            for (auto position = offsets[node]; position < offsets[node + 1]; ++position)
            {
                const auto id = ids[position];
                const auto [left, right] = domains[id];
                if (left < right)
                {
                    storage.add(left, right, id);
                }
            }
            if (node >= base)
            {
                const auto time = node - base;
                answers.push_back(storage.get_min(index(queries_[time])));
            }
            else
            {
                visit(visit, 2 * node);
                visit(visit, 2 * node + 1);
            }
            storage.roll_back(before);
        };
        search(search, 1);
        return answers;
    }

private:
    static constexpr std::size_t kOngoing = std::numeric_limits<std::size_t>::max();
    static constexpr id_type kEmpty = std::numeric_limits<id_type>::max();

    struct Record
    {
        Function function;
        std::optional<std::pair<X, X>> domain;
        std::size_t start;
        std::size_t finish;
    };

    static_assert(std::is_copy_constructible_v<X>);
    static_assert(std::is_copy_constructible_v<Record> || std::is_nothrow_move_constructible_v<Record>);

    class SpatialTree
    {
    public:
        SpatialTree(const std::vector<Record> &records, const std::vector<X> &coordinates)
            : records_(records), x_(coordinates), nodes_(4 * coordinates.size(), kEmpty)
        {
        }

        [[nodiscard]] std::size_t snapshot() const noexcept
        {
            return journal_.size();
        }

        void roll_back(std::size_t before) noexcept
        {
            while (journal_.size() > before)
            {
                const auto [node, old] = journal_.back();
                nodes_[node] = old;
                journal_.pop_back();
            }
        }

        void add(std::size_t first, std::size_t last, id_type id)
        {
            add_segment(1, 0, x_.size(), first, last, id);
        }

        [[nodiscard]] std::optional<value_type> get_min(std::size_t point) const
        {
            std::optional<value_type> best;
            std::size_t node = 1, left = 0, right = x_.size();
            for (;;)
            {
                if (nodes_[node] != kEmpty)
                {
                    auto candidate = eval(nodes_[node], point);
                    if (!best || candidate < *best)
                    {
                        best.emplace(std::move(candidate));
                    }
                }
                if (right - left == 1)
                {
                    return best;
                }
                const auto middle = left + (right - left) / 2;
                node *= 2;
                if (point < middle)
                {
                    right = middle;
                }
                else
                {
                    left = middle;
                    ++node;
                }
            }
        }

    private:
        [[nodiscard]] value_type eval(id_type id, std::size_t point) const
        {
            return records_[id].function(x_[point]);
        }

        void write(std::size_t node, id_type id)
        {
            journal_.emplace_back(node, nodes_[node]);
            nodes_[node] = id;
        }

        void add_line(std::size_t node, std::size_t left, std::size_t right, id_type id)
        {
            auto current = nodes_[node];
            if (current == kEmpty)
            {
                write(node, id);
                return;
            }
            const bool left_wins = eval(id, left) < eval(current, left);
            const bool right_wins = eval(id, right - 1) < eval(current, right - 1);
            if (left_wins == right_wins)
            {
                if (left_wins)
                {
                    write(node, id);
                }
                return;
            }

            const auto middle = left + (right - left) / 2;
            const bool center_wins = eval(id, middle) < eval(current, middle);
            if (center_wins)
            {
                write(node, id);
                id = current;
            }
            if (left_wins != center_wins)
            {
                add_line(2 * node, left, middle, id);
            }
            else
            {
                add_line(2 * node + 1, middle, right, id);
            }
        }

        void add_segment(std::size_t node, std::size_t left, std::size_t right,
                         std::size_t first, std::size_t last, id_type id)
        {
            if (first <= left && right <= last)
            {
                add_line(node, left, right, id);
                return;
            }
            const auto middle = left + (right - left) / 2;
            if (first < middle)
            {
                add_segment(2 * node, left, middle, first, last, id);
            }
            if (middle < last)
            {
                add_segment(2 * node + 1, middle, right, first, last, id);
            }
        }

        const std::vector<Record> &records_;
        const std::vector<X> &x_;
        std::vector<id_type> nodes_;
        std::vector<std::pair<std::size_t, id_type>> journal_;
    };

    id_type add(Function function, std::optional<std::pair<X, X>> domain)
    {
        const auto id = functions_.size();
        functions_.push_back({std::move(function), std::move(domain), queries_.size(), kOngoing});
        return id;
    }

    template <class Visitor>
    void visit_intervals(std::size_t base, Visitor visit) const
    {
        for (id_type id = 0; id < functions_.size(); ++id)
        {
            const auto &record = functions_[id];
            auto left = base + record.start;
            auto right = base + (record.finish == kOngoing ? queries_.size() : record.finish);
            for (; left < right; left /= 2, right /= 2)
            {
                if (left & 1)
                {
                    visit(left++, id);
                }
                if (right & 1)
                {
                    visit(--right, id);
                }
            }
        }
    }

    std::vector<Record> functions_;
    std::vector<X> queries_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
