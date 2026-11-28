#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 4559e393b9b38fadeef98b76094879242f3890af76a11aa8265899d226a87f35
// Verify: https://atcoder.jp/contests/awc0100/tasks/awc0100_m

// Library: DataStructure/Persistent_Weighted_Union_Find.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考: https://atcoder.jp/contests/awc0100/editorial/21308
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_weighted_union_find.md
//
// API一覧
// PersistentWeightedUnionFind<T,Add,Subtract,Shift>(n,zero=T{},add={},subtract={}): n頂点を独立に構築。
// 版を省略する操作は現在版を使う。Versionは非所有の版ハンドル。初期版はnullptr。version()/restore(v)はO(1)。
// root(i[,v]): 代表の頂点番号。size(i[,v]): 成分サイズ。same/issame(a,b[,v]): 同一成分か。
// root_with_weight(i[,v]): {代表, iと代表の差}。try_diff(a,b[,v]): bとaの差。別成分ならnullopt。
// unite_checked(a,b,d[,v]): bとaの差をdに制約し、Merged/AlreadyConsistent/Contradictionを返す。
// unite_checkedの後は、結合時は新しい版、それ以外は操作元の版が現在版になる。過去の版は保持される。
// B=2^Shift、L=1+log_B(n+1)。構築はO(B+L)。操作はO(log(n+1)L)、結合時にO(BL)の償却時間・追加領域。
// Tはコピー構築・代入可能な可換加法群。既定の符号付き整数の演算は中間値の桁あふれで例外。符号なし整数は型の幅で剰余。
// 版は同じ保存領域を所有するコピーが残る間有効。コピー同士の現在版は独立。移動元は空。
//
// [同梱の永続配列]
// [PersistentArray<T,shift>]
// PersistentArray<T,shift=4>(n=0,ID=T{}): [0,n)の初期値ID、O(2^shift + log_(2^shift)(n+1))時間。Tはコピー構築・代入可能。
// PersistentArray(vector,ID): 配列から構築。B=2^shiftとしてO(B+n+(n/B+1)log_B(n+1))時間、O(n+B)領域。
// get(k)/get(version,k): 現在/指定版の値を返す。O(log_(2^shift)(n+1))。
// get_two(a,b)/get_two(version,a,b): 2要素をpair<T,T>で返す。2経路を並行参照、同じ計算量。
// update(k,val)/update(version,k,val): 現在/指定版から分岐して現在版を更新しNode*を返す。
// update_two(a,x,b,y)/update_two(version,a,x,b,y): 2点を同時更新。同じ添字はyを優先しNode*を返す。
// update/update_twoはO(2^shift log_(2^shift)(n+1))償却時間・償却追加領域。size(): 要素数、O(1)。
// restore(version): 現在版を選択。nullptrは全要素IDの版。O(1)。
// rootは現在版の非所有ハンドル。nullptrを指定した版は全要素ID。コピー同士は保存領域を共有し現在版は独立。
// ハンドルは同じ保存領域を所有するインスタンスが残る間有効。移動元は空。全版は最後の所有者と同時に反復解放。
// 別保存領域・寿命切れのハンドルは使用不可。並行更新は外部同期が必要。1<=shift<=8。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <concepts>
#include <functional>
#include <optional>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_ARRAY_DEFINED
#define SIRKIT_PERSISTENT_ARRAY_DEFINED
namespace detail {
inline void CheckPersistentArrayNodeCount(std::size_t size) {
    if (size == std::numeric_limits<std::size_t>::max())
        throw std::length_error("PersistentArray: node capacity");
}

inline std::int64_t CheckedPersistentArraySize(std::size_t size) {
    if (size > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
        throw std::length_error("PersistentArray: size overflow");
    return static_cast<std::int64_t>(size);
}
}

template<typename T, int shift = 4>
class PersistentArray {
    friend struct PersistentUnionFind;
    static_assert(shift >= 1 && shift <= 8);
    static constexpr std::size_t width = std::size_t{1} << shift;
#if defined(__GNUC__) && !defined(__clang__)
    static constexpr bool compact_nodes = std::is_trivially_copyable_v<T>;
#else
    static constexpr bool compact_nodes = false;
#endif
    struct Storage;
public:
    struct Node {
    private:
        friend class PersistentArray;
        union TrivialContent {
            std::array<Node*, width> children;
            std::array<T, width> values;
            TrivialContent(std::in_place_index_t<0>, std::array<Node*, width> a) : children(a) {}
            TrivialContent(std::in_place_index_t<1>, std::array<T, width> a) : values(a) {}
        };
        using Content = std::conditional_t<compact_nodes, TrivialContent,
                                          std::variant<std::array<Node*, width>, std::array<T, width>>>;
        Content content;
        auto& children() {
            if constexpr (compact_nodes) return content.children;
            else return std::get<0>(content);
        }
        auto& values() {
            if constexpr (compact_nodes) return content.values;
            else return std::get<1>(content);
        }
#ifndef SKIP_RUNTIME_CHECKS
        const Storage* owner;
#endif
        static std::array<T, width> filled(const T& value) {
            return [&value]<std::size_t... I>(std::index_sequence<I...>) {
                return std::array<T, width>{((void)I, value)...};
            }(std::make_index_sequence<width>{});
        }
    public:
        Node(bool leaf, const T& value, const Storage* storage)
            : content(leaf ? decltype(content)(std::in_place_index<1>, filled(value))
                           : decltype(content)(std::in_place_index<0>, std::array<Node*, width>{}))
#ifndef SKIP_RUNTIME_CHECKS
              , owner(storage)
#endif
        { (void)storage; }
        Node(const Node&) = default;
    };
private:
    // 内側vectorの容量内にだけ追加し、全版のNode*を安定させる。
    struct Storage {
        T id;
        std::vector<std::vector<Node>> blocks;
        std::size_t node_count = 0;
        explicit Storage(T value) : id(std::move_if_noexcept(value)) {}
    };
    std::shared_ptr<Storage> storage_;
    std::int64_t size_ = 0;
    int levels_ = 0;
    void check_index(std::int64_t k) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (k < 0 || k >= size_) throw std::out_of_range("PersistentArray: index");
#else
        (void)k;
#endif
    }
    void check_root(Node* n) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (n && n->owner != storage_.get()) throw std::invalid_argument("PersistentArray: foreign version");
#else
        (void)n;
#endif
    }
    Node* allocate(Node* original, bool leaf) {
        detail::CheckPersistentArrayNodeCount(storage_->node_count);
        if (storage_->blocks.empty() || storage_->blocks.back().size() == storage_->blocks.back().capacity()) {
            std::vector<Node> block;
            block.reserve(std::min<std::size_t>(512, std::size_t{4} << std::min<std::size_t>(storage_->blocks.size(), 9)));
            storage_->blocks.push_back(std::move(block));
        }
        auto& block = storage_->blocks.back();
        if (original) block.emplace_back(*original);
        else block.emplace_back(leaf, storage_->id, storage_.get());
        ++storage_->node_count;
        return &block.back();
    }
public:
    Node* root = nullptr;
    explicit PersistentArray(std::int64_t n = 0, T id = T{}) {
        if (n < 0) throw std::length_error("PersistentArray: negative size");
        size_ = n;
        std::uint64_t top = n ? static_cast<std::uint64_t>(n - 1) : 0;
        while (top >= width) { ++levels_; top >>= shift; }
        storage_ = std::make_shared<Storage>(std::move_if_noexcept(id)); root = allocate(nullptr, levels_ == 0);
    }
    explicit PersistentArray(const std::vector<T>& values, T id = T{})
        : PersistentArray(vector_size(values), std::move_if_noexcept(id)) {
        // 同じ葉の値は続けて代入し、根から葉までの再走査を減らす。
        for (std::size_t first = 0; first < values.size();) {
            Node* node = root;
            for (int d = levels_; d > 0; --d) {
                auto& next = node->children()[(first >> (d * shift)) & (width - 1)];
                if (!next) next = allocate(nullptr, d == 1);
                node = next;
            }
            const auto count = std::min(width, values.size() - first);
            auto& leaf = node->values();
            for (std::size_t j = 0; j < count; ++j) leaf[j] = values[first + j];
            first += count;
        }
    }
    PersistentArray(const PersistentArray&) = default;
    PersistentArray& operator=(const PersistentArray&) = default;
    PersistentArray(PersistentArray&& other) noexcept
        : storage_(std::move(other.storage_)), size_(std::exchange(other.size_, 0)),
          levels_(std::exchange(other.levels_, 0)), root(std::exchange(other.root, nullptr)) {}
    PersistentArray& operator=(PersistentArray&& other) noexcept {
        if (this != &other) {
            storage_ = std::move(other.storage_); size_ = std::exchange(other.size_, 0);
            levels_ = std::exchange(other.levels_, 0); root = std::exchange(other.root, nullptr);
        }
        return *this;
    }
    void restore(Node* r) { check_root(r); root = r; }
    std::int64_t size() const noexcept { return size_; }
    T get(Node* n, std::int64_t k) const {
        check_index(k); check_root(n); return get_unchecked(n, k);
    }
    T get(std::int64_t k) const { return get(root, k); }
    std::pair<T,T> get_two(Node* base, std::int64_t a, std::int64_t b) const {
        check_index(a); check_index(b); check_root(base);
        return get_two_unchecked(base, a, b);
    }
    std::pair<T,T> get_two(std::int64_t a, std::int64_t b) const { return get_two(root, a, b); }
    Node* update(Node* base, std::int64_t k, const T& value) {
        check_index(k); check_root(base);
        std::array<Node*, 64> path;
        std::array<std::size_t, 64> slots;
        const auto index = static_cast<std::uint64_t>(k);
        Node* n = base;
        for (int d = levels_; d > 0; --d) {
            const auto slot = (index >> (d * shift)) & (width - 1);
            path[d] = n; slots[d] = slot;
            n = n ? n->children()[slot] : nullptr;
        }
        Node* result = allocate(n, true);
        result->values()[index & (width - 1)] = value;
        for (int d = 1; d <= levels_; ++d) {
            Node* parent = allocate(path[d], false);
            parent->children()[slots[d]] = result; result = parent;
        }
        return root = result;
    }
    Node* update(std::int64_t k, const T& value) { return update(root, k, value); }
    Node* update_two(Node* base, std::int64_t a, const T& x, std::int64_t b, const T& y) {
        check_index(a); check_index(b); check_root(base);
        return root = update_two(base, static_cast<std::uint64_t>(a), x,
                                 static_cast<std::uint64_t>(b), y, levels_);
    }
    Node* update_two(std::int64_t a, const T& x, std::int64_t b, const T& y) {
        return update_two(root, a, x, b, y);
    }
private:
    Node* update_one(Node* base, std::uint64_t index, const T& value, int level) {
        Node* result = allocate(base, level == 0);
        Node* current = result;
        for (int d = level; d > 0; --d) {
            auto& child = current->children()[(index >> (d * shift)) & (width - 1)];
            child = allocate(child, d == 1);
            current = child;
        }
        current->values()[index & (width - 1)] = value;
        return result;
    }
    // 共通経路は一度だけ複製する。変更は全て新規ノード内で、rootは呼出元が最後に公開。
    Node* update_two(Node* base, std::uint64_t a, const T& x, std::uint64_t b, const T& y, int level) {
        Node* result = allocate(base, level == 0);
        Node* current = result;
        for (int d = level; d > 0; --d) {
            const auto sa = (a >> (d * shift)) & (width - 1);
            const auto sb = (b >> (d * shift)) & (width - 1);
            auto& children = current->children();
            if (sa != sb) {
                children[sa] = update_one(children[sa], a, x, d - 1);
                children[sb] = update_one(children[sb], b, y, d - 1);
                return result;
            }
            children[sa] = allocate(children[sa], d == 1);
            current = children[sa];
        }
        auto& values = current->values();
        values[a & (width - 1)] = x;
        values[b & (width - 1)] = y;
        return result;
    }
    T get_existing(Node* n, std::int64_t k) const {
        const auto index = static_cast<std::uint64_t>(k);
        for (int d = levels_; d > 0; --d)
            n = n->children()[(index >> (d * shift)) & (width - 1)];
        return n->values()[index & (width - 1)];
    }
    T get_unchecked(Node* n, std::int64_t k) const {
        const auto index = static_cast<std::uint64_t>(k);
        for (int d = levels_; d > 0; --d) {
            if (!n) return storage_->id;
            n = n->children()[(index >> (d * shift)) & (width - 1)];
        }
        return n ? n->values()[index & (width - 1)] : storage_->id;
    }
    // 独立な2本の参照を同じ反復で進める。
    std::pair<T,T> get_two_unchecked(Node* n, std::int64_t a, std::int64_t b) const {
        Node* left = n;
        Node* right = n;
        const auto ia = static_cast<std::uint64_t>(a);
        const auto ib = static_cast<std::uint64_t>(b);
        for (int d = levels_; d > 0; --d) {
            if (left) left = left->children()[(ia >> (d * shift)) & (width - 1)];
            if (right) right = right->children()[(ib >> (d * shift)) & (width - 1)];
        }
        return {left ? left->values()[ia & (width - 1)] : storage_->id,
                right ? right->values()[ib & (width - 1)] : storage_->id};
    }
    // UFの親番号は同じ版で明示更新済みなので、祖先の配列経路は必ず存在する。
    std::pair<T,T> get_two_existing(Node* n, std::int64_t a, std::int64_t b) const {
        Node* left = n; Node* right = n;
        const auto ia = static_cast<std::uint64_t>(a), ib = static_cast<std::uint64_t>(b);
        for (int d = levels_; d > 0; --d) {
            left = left->children()[(ia >> (d * shift)) & (width - 1)];
            right = right->children()[(ib >> (d * shift)) & (width - 1)];
        }
        return {left->values()[ia & (width - 1)], right->values()[ib & (width - 1)]};
    }
    static std::int64_t vector_size(const std::vector<T>& values) {
        return detail::CheckedPersistentArraySize(values.size());
    }
};
#endif

#ifndef SIRKIT_PERSISTENT_WEIGHTED_UNION_FIND_DEFINED
#define SIRKIT_PERSISTENT_WEIGHTED_UNION_FIND_DEFINED

namespace detail
{
template <class T>
struct PersistentWeightAdd
{
    T operator()(const T &a, const T &b) const
    {
        if constexpr (std::signed_integral<T>)
        {
            if ((b > 0 && a > std::numeric_limits<T>::max() - b) ||
                (b < 0 && a < std::numeric_limits<T>::min() - b))
            {
                throw std::overflow_error("PersistentWeightedUnionFind: potential sum");
            }
        }
        return static_cast<T>(a + b);
    }
};

template <class T>
struct PersistentWeightSubtract
{
    T operator()(const T &a, const T &b) const
    {
        if constexpr (std::signed_integral<T>)
        {
            if ((b > 0 && a < std::numeric_limits<T>::min() + b) ||
                (b < 0 && a > std::numeric_limits<T>::max() + b))
            {
                throw std::overflow_error("PersistentWeightedUnionFind: potential difference");
            }
        }
        return static_cast<T>(a - b);
    }
};
}

template <std::equality_comparable T, class Add = detail::PersistentWeightAdd<T>,
          class Subtract = detail::PersistentWeightSubtract<T>, int Shift = 4>
class PersistentWeightedUnionFind
{
    struct Entry
    {
        int parent;
        T delta;
    };
    struct Group
    {
        T zero;
        [[no_unique_address]] Add add;
        [[no_unique_address]] Subtract subtract;
    };

    using Array = PersistentArray<Entry, Shift>;

public:
    using Version = typename Array::Node *;

    enum class MergeResult
    {
        Merged,
        AlreadyConsistent,
        Contradiction,
    };

    explicit PersistentWeightedUnionFind(std::size_t n = 0, T zero = T{}, Add add = {}, Subtract subtract = {})
        : entries_(CheckedSize(n), Entry{-1, zero}),
          group_(std::make_shared<const Group>(Group{std::move(zero), std::move(add), std::move(subtract)}))
    {
        entries_.restore(nullptr);
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(entries_.size());
    }

    [[nodiscard]] Version version() const noexcept
    {
        return entries_.root;
    }

    void restore(Version base)
    {
        entries_.restore(base);
    }

    [[nodiscard]] int root(int vertex, Version base) const
    {
        return FindRoot(vertex, base).first;
    }

    [[nodiscard]] int root(int vertex) const
    {
        return root(vertex, version());
    }

    [[nodiscard]] int size(int vertex, Version base) const
    {
        return -FindRoot(vertex, base).second;
    }

    [[nodiscard]] int size(int vertex) const
    {
        return size(vertex, version());
    }

    [[nodiscard]] bool same(int from, int to, Version base) const
    {
        return root(from, base) == root(to, base);
    }

    [[nodiscard]] bool same(int from, int to) const
    {
        return same(from, to, version());
    }

    [[nodiscard]] bool issame(int from, int to, Version base) const
    {
        return same(from, to, base);
    }

    [[nodiscard]] bool issame(int from, int to) const
    {
        return same(from, to);
    }

    [[nodiscard]] std::pair<int, T> root_with_weight(int vertex, Version base) const
    {
        auto result = FindWeightedRoot(vertex, base);
        return {result.root, std::move(result.delta)};
    }

    [[nodiscard]] std::pair<int, T> root_with_weight(int vertex) const
    {
        return root_with_weight(vertex, version());
    }

    [[nodiscard]] std::optional<T> try_diff(int from, int to, Version base) const
    {
        const auto a = FindWeightedRoot(from, base);
        const auto b = FindWeightedRoot(to, base);
        if (a.root != b.root)
        {
            return std::nullopt;
        }
        return std::invoke(group_->subtract, b.delta, a.delta);
    }

    [[nodiscard]] std::optional<T> try_diff(int from, int to) const
    {
        return try_diff(from, to, version());
    }

    MergeResult unite_checked(int from, int to, const T &delta, Version base)
    {
        const auto a = FindWeightedRoot(from, base);
        const auto b = FindWeightedRoot(to, base);
        if (a.root == b.root)
        {
            const bool consistent = std::invoke(group_->subtract, b.delta, a.delta) == delta;
            restore(base);
            return consistent ? MergeResult::AlreadyConsistent : MergeResult::Contradiction;
        }
        const T root_delta = std::invoke(group_->subtract, std::invoke(group_->add, a.delta, delta), b.delta);
        const Entry joined{a.parent + b.parent, group_->zero};
        if (a.parent > b.parent)
        {
            const T reversed = std::invoke(group_->subtract, group_->zero, root_delta);
            entries_.update_two(base, b.root, joined, a.root, Entry{b.root, reversed});
        }
        else
        {
            entries_.update_two(base, a.root, joined, b.root, Entry{a.root, root_delta});
        }
        return MergeResult::Merged;
    }

    MergeResult unite_checked(int from, int to, const T &delta)
    {
        return unite_checked(from, to, delta, version());
    }

private:
    struct WeightedRoot
    {
        int root;
        int parent;
        T delta;
    };

    Array entries_;
    std::shared_ptr<const Group> group_;

    static int CheckedSize(std::size_t n)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("PersistentWeightedUnionFind: too many vertices");
        }
        return static_cast<int>(n);
    }

    [[nodiscard]] std::pair<int, int> FindRoot(int vertex, Version base) const
    {
        int parent = entries_.get(base, vertex).parent;
        while (parent >= 0)
        {
            vertex = parent;
            parent = entries_.get(base, vertex).parent;
        }
        return {vertex, parent};
    }

    [[nodiscard]] WeightedRoot FindWeightedRoot(int vertex, Version base) const
    {
        Entry entry = entries_.get(base, vertex);
        T delta = group_->zero;
        while (entry.parent >= 0)
        {
            delta = std::invoke(group_->add, delta, entry.delta);
            vertex = entry.parent;
            entry = entries_.get(base, vertex);
        }
        return {vertex, entry.parent, std::move(delta)};
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


#include <vector>

namespace
{
struct ModularAdd
{
    int modulus;

    int operator()(int a, int b) const
    {
        const int sum = a + b;
        return sum < modulus ? sum : sum - modulus;
    }
};

struct ModularSubtract
{
    int modulus;

    int operator()(int a, int b) const
    {
        const int difference = a - b;
        return difference >= 0 ? difference : difference + modulus;
    }
};
} // namespace

int main()
{
    int n = 0, modulus = 0, q = 0;
    verify_in >> n >> modulus >> q;
    using Weighted =
        sirkit::PersistentWeightedUnionFind<int, ModularAdd, ModularSubtract>;
    Weighted uf(n + 1, 0, ModularAdd{modulus}, ModularSubtract{modulus});
    std::vector<Weighted::Version> versions(q + 1, uf.version());
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, base = 0, l = 0, r = 0;
        verify_in >> type >> base >> l >> r;
        if (type == 0)
        {
            int score = 0;
            verify_in >> score;
            const auto status = uf.unite_checked(l - 1, r, score, versions[base]);
            verify_out << (status == Weighted::MergeResult::Contradiction ? "NO\n"
                                                                          : "YES\n");
            versions[i] = uf.version();
        }
        else
        {
            const auto value = uf.try_diff(l - 1, r, versions[base]);
            if (value)
            {
                verify_out << *value << '\n';
            }
            else
            {
                verify_out << "UNKNOWN\n";
            }
            versions[i] = versions[base];
        }
    }
}
