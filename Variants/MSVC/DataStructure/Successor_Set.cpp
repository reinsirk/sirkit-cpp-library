// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: DataStructure/Successor_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/successor_set.md
// 参考（Parking Function）: https://info.atcoder.jp/entry/algorithm_lectures/parking_function
//
// API一覧
// SuccessorSet(n=0): 0..n-1が残る集合を作る。0<=n<INT_MAX。O(n+1)時間・領域。
// size()/remaining(): 最初の要素数/残っている要素数。O(1)。
// next(i): i以上の最小要素。該当なしならnullopt。0<=i<=n。
// erase(i): iを取り除く。存在したときtrue。take(i): next(i)を返し、見つかれば取り除く。
// take_cyclic(i): i以上がなければ先頭から探すtake。next/erase/take/take_cyclicは償却O(α(n+1))。
// parking_slots(preferences): 各希望位置から右に最初の空きへ順に配置する。失敗はnullopt。償却O(n α(n+1))時間・O(n)領域。
//
// [同梱のUnionFind]
// [UnionFind]
// UnionFind(n=0): n要素を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// root(i)/leader(i): 代表、unite(a,b): 結合（戻り値なし）、merge(a,b): 新規結合ならtrue。償却O(α(n+1))。
// issame(a,b)/same(a,b): 同一成分か、size(i): 成分サイズ。償却O(α(n+1))。探索時に経路を圧縮する。
// size()/empty()/component_count(): 全要素数/空か/成分数、O(1)。groups(): 頂点昇順の成分一覧、O(1+n α(n+1))償却。
// reset(n)/reset(): n要素/現在の要素数へ初期化。容量を再利用、O(旧n+n+1)上界。確保失敗時は元を保つ。
// clear(): 空へ戻して容量保持、O(旧n+1)上界、noexcept。swap(other): 内容交換、O(1)、noexcept。
// コピーはO(n+1)で独立、移動元は空。自己代入・自己ムーブは内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>
#include <optional>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNION_FIND_DEFINED
#define SIRKIT_UNION_FIND_DEFINED
class UnionFind {
    std::vector<int> param;
    int components_ = 0;
    static std::size_t checked_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("UnionFind: too many vertices");
        return n;
    }
    void check(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(i) >= param.size())
            throw std::out_of_range("UnionFind: vertex");
#else
        (void)i;
#endif
    }
    int leader_unchecked(int i) {
        while (param[i] >= 0) {
            const int p = param[i];
            if (param[p] < 0) return p;
            i = param[i] = param[p];
        }
        return i;
    }
public:
    explicit UnionFind(std::size_t n = 0)
        : param(checked_size(n), -1), components_(static_cast<int>(n)) {}
    UnionFind(const UnionFind&) = default;
    UnionFind& operator=(const UnionFind&) = default;
    UnionFind(UnionFind&& other) noexcept
        : param(std::move(other.param)), components_(std::exchange(other.components_, 0)) { other.param.clear(); }
    UnionFind& operator=(UnionFind&& other) noexcept {
        if (this != &other) {
            param = std::move(other.param);
            components_ = std::exchange(other.components_, 0); other.param.clear();
        }
        return *this;
    }
    void reset(std::size_t n) {
        const auto count = checked_size(n);
        param.reserve(count);
        param.clear(); param.resize(count, -1);
        components_ = static_cast<int>(count);
    }
    void reset() { reset(param.size()); }
    void clear() noexcept { param.clear(); components_ = 0; }
    bool empty() const noexcept { return param.empty(); }
    void swap(UnionFind& other) noexcept {
        param.swap(other.param); std::swap(components_, other.components_);
    }
    friend void swap(UnionFind& left, UnionFind& right) noexcept { left.swap(right); }
    int root(int i) { check(i); return leader_unchecked(i); }
    int leader(int i) { return root(i); }
    bool merge(int a, int b) {
        check(a); check(b);
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
        return true;
    }
    void unite(int a, int b) { (void)merge(a, b); }
    bool same(int a, int b) { check(a); check(b); return leader_unchecked(a) == leader_unchecked(b); }
    bool issame(int a, int b) { return same(a, b); }
    int size(int i) { return -param[root(i)]; }
    int size() const noexcept { return static_cast<int>(param.size()); }
    int component_count() const noexcept { return components_; }
    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result;
        result.reserve(static_cast<std::size_t>(components_));
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
            const int root = leader_unchecked(0);
            result.emplace_back(param.size());
            const int negative_size = param[root];
            std::fill(param.begin(), param.end(), root);
            param[root] = negative_size;
            std::iota(result[0].begin(), result[0].end(), 0);
            return result;
        }
        std::vector<int> slot(param.size());
        for (int i = 0; i < size(); ++i) if (param[i] < 0) {
            slot[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(static_cast<std::size_t>(-param[i]));
        }
        for (int i = 0; i < size(); ++i) result[slot[leader_unchecked(i)]].push_back(i);
        return result;
    }
};
#endif

#ifndef SIRKIT_SUCCESSOR_SET_DEFINED
#define SIRKIT_SUCCESSOR_SET_DEFINED

class SuccessorSet
{
public:
    explicit SuccessorSet(std::size_t n = 0)
        : groups_(capacity_with_sentinel(n)), rightmost_(n + 1), remaining_(static_cast<int>(n))
    {
        std::iota(rightmost_.begin(), rightmost_.end(), 0);
    }

    [[nodiscard]] int size() const noexcept
    {
        return std::max(0, groups_.size() - 1);
    }

    [[nodiscard]] int remaining() const noexcept
    {
        return std::min(size(), remaining_);
    }

    [[nodiscard]] std::optional<int> next(int position)
    {
        check_position(position);
        if (position == size())
        {
            return std::nullopt;
        }
        const int index = rightmost_[groups_.root(position)];
        return index == size() ? std::nullopt : std::optional(index);
    }

    bool erase(int position)
    {
        check_position(position);
        if (position == size())
        {
            return false;
        }
        const int component = groups_.root(position);
        if (rightmost_[component] != position)
        {
            return false;
        }
        const int following = groups_.root(position + 1);
        const int successor = rightmost_[following];
        groups_.unite(component, following);
        rightmost_[groups_.root(component)] = successor;
        --remaining_;
        return true;
    }

    std::optional<int> take(int first)
    {
        const auto result = next(first);
        if (result)
        {
            erase(*result);
        }
        return result;
    }

    std::optional<int> take_cyclic(int first)
    {
        return take(first).or_else([this]
                                   {
                                       return take(0);
                                   });
    }

private:
    static std::size_t capacity_with_sentinel(std::size_t n)
    {
        if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("SuccessorSet: capacity");
        }
        return n + 1;
    }

    void check_position(int position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position < 0 || position > size())
        {
            throw std::out_of_range("SuccessorSet: position");
        }
#else
        (void)position;
#endif
    }

    UnionFind groups_;
    std::vector<int> rightmost_;
    int remaining_;
};

inline std::optional<std::vector<int>> parking_slots(std::span<const int> preferences)
{
    SuccessorSet available(preferences.size());
    std::vector<int> answer;
    answer.reserve(preferences.size());
    for (const int preferred : preferences)
    {
        if (preferred < 0 || preferred >= available.size())
        {
            return std::nullopt;
        }
        const auto slot = available.take(preferred);
        if (!slot)
        {
            return std::nullopt;
        }
        answer.push_back(*slot);
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
