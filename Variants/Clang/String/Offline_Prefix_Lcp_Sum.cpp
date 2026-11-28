// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Offline_Prefix_Lcp_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_prefix_lcp_sum.md
//
// API一覧
// PrefixLcpAppend{index,byte}: 0始まりの文字列indexの末尾に1バイト追加する。
// PrefixLcpQuery{index}: 文字列indexと現在の各文字列との最長共通接頭辞長の総和を照会する。
// PrefixLcpOperation: 上の操作のvariant。空文字列や同じ内容の入力も可。総和に照会先自身も含める。
// offline_prefix_lcp_sum(initial,operations): 操作をまとめて受け取り、照会の答えを順に返す。
// 初期本数N、合計長S、操作数Q、作るTrieの頂点数MとしてO(N+S+(Q+1)log(M+1))時間・O(N+M+Q)領域。バイト集合は固定。
// NとMはINT_MAX以下。同梱Fenwick木のAPIはDocument/fenwick_tree.mdを参照。
//
// [同梱のFenwick木]
// [fenwick_tree<T>]
// fenwick_tree()/fenwick_tree(int n)/fenwick_tree(vector<T>): 空列/n要素の0/列から構築。O(n+1)時間・領域。
// add(int p,T x): 点加算。sum(int l,int r): 半開区間[l,r)の和をT値で返す。O(log(n+1))、空区間はT{}。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// ACL形式の公開APIをsirkit内に定義する。bool以外の整数は内部を符号なしにして型幅のmod 2^bitで計算する。
// その他のTはT{}が単位元の可換加法群とし、加減算・コピーをO(1)とする。要素数はintの範囲内。
// size()/empty(): 要素数/空か、O(1)。get(int p)/set(int p,T value): 点取得/代入、O(log(n+1))。
// lower_bound(target)/upper_bound(target): sum(0,i+1)>=target/>targetとなる最小int添字、なければsize()。O(log(n+1))。
// 境界探索は全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。全要素の検査はしない。
// values(): 列を復元して返す。O(n+1)時間・追加O(n)。reset(int n)/reset(): n要素/現在の長さを0にする。O(旧n+n+1)。
// clear()/swap(other): 空にして容量保持/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空として使える。
// 添字・区間は既定で検査しSKIP_RUNTIME_CHECKSで除く。容量・添字のオーバーフロー対策は残す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
namespace detail {
inline std::size_t CheckedFenwickStorageSize(std::size_t n, std::size_t limit) {
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= limit)
        throw std::length_error("fenwick_tree: size");
    return n == 0 ? 0 : n + 1;
}
}

template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("fenwick_tree: negative size");
#endif
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        return detail::CheckedFenwickStorageSize(n, std::vector<Storage>{}.max_size());
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || i >= size()) throw std::out_of_range("fenwick_tree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("fenwick_tree: range");
#else
        (void)l; (void)r;
#endif
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i;;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif

#ifndef SIRKIT_OFFLINE_PREFIX_LCP_SUM_DEFINED
#define SIRKIT_OFFLINE_PREFIX_LCP_SUM_DEFINED

struct PrefixLcpAppend
{
    std::size_t index;
    std::uint8_t byte;
};

struct PrefixLcpQuery
{
    std::size_t index;
};

using PrefixLcpOperation = std::variant<PrefixLcpAppend, PrefixLcpQuery>;

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
struct PrefixLcpNode
{
    int parent = 0;
    int child = 0;
    int sibling = 0;
    std::uint8_t byte = 0;
};

inline int prefix_lcp_checked_count(std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("offline_prefix_lcp_sum: too many strings or trie nodes");
    }
    return static_cast<int>(count);
}

inline int prefix_lcp_extend(std::vector<PrefixLcpNode> &nodes, int parent, std::uint8_t byte)
{
    for (auto index = nodes[parent].child; index != 0; index = nodes[index].sibling)
    {
        if (nodes[index].byte == byte)
        {
            return index;
        }
    }
    const auto next = prefix_lcp_checked_count(nodes.size() + 1) - 1;
    nodes.push_back({parent, 0, nodes[parent].child, byte});
    nodes[parent].child = next;
    return next;
}
} // namespace detail

inline std::vector<std::uint64_t> offline_prefix_lcp_sum(std::span<const std::string_view> initial,
                                                          std::span<const PrefixLcpOperation> operations)
{
    (void)detail::prefix_lcp_checked_count(initial.size());
    std::vector<detail::PrefixLcpNode> nodes(1);
    std::vector<int> current(initial.size());
    for (std::size_t index = 0; index < initial.size(); ++index)
    {
        for (const char byte : initial[index])
        {
            current[index] = detail::prefix_lcp_extend(nodes, current[index], static_cast<std::uint8_t>(byte));
        }
    }
    const auto starts = current;

    struct Event
    {
        int node;
        bool append;
    };
    std::vector<Event> events;
    events.reserve(operations.size());
    for (const auto &operation : operations)
    {
        const auto index = std::visit([](const auto &event) { return event.index; }, operation);
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= current.size())
        {
            throw std::out_of_range("offline_prefix_lcp_sum: string index");
        }
#endif
        if (const auto *append = std::get_if<PrefixLcpAppend>(&operation))
        {
            current[index] = detail::prefix_lcp_extend(nodes, current[index], append->byte);
            events.push_back({current[index], true});
        }
        else
        {
            events.push_back({current[index], false});
        }
    }

    const auto size = static_cast<int>(nodes.size());
    std::vector<std::uint64_t> frequency(size), base_sum(size);
    std::vector<int> subtree_size(size, 1);
    for (const auto node : starts)
    {
        ++frequency[node];
    }
    for (auto node = size - 1; node > 0; --node)
    {
        const auto parent = nodes[node].parent;
        frequency[parent] += frequency[node];
        subtree_size[parent] += subtree_size[node];
    }
    for (int node = 1; node < size; ++node)
    {
        base_sum[node] = base_sum[nodes[node].parent] + frequency[node];
    }

    std::vector<int> entry(size), pending{0};
    int order = 0;
    while (!pending.empty())
    {
        const auto node = pending.back();
        pending.pop_back();
        entry[node] = order++;
        for (auto child = nodes[node].child; child != 0; child = nodes[child].sibling)
        {
            pending.push_back(child);
        }
    }

    // 1字の追加で増える接頭辞の種類は一つ。その頂点の部分木の答えを+1する。
    fenwick_tree<std::int64_t> changes(size);
    std::vector<std::uint64_t> answer;
    for (const auto [node, append] : events)
    {
        const auto start = entry[node];
        if (append)
        {
            changes.add(start, 1);
            const auto end = start + subtree_size[node];
            if (end < size)
            {
                changes.add(end, -1);
            }
        }
        else
        {
            answer.push_back(base_sum[node] + static_cast<std::uint64_t>(changes.sum(0, start + 1)));
        }
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
