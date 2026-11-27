// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Persistent_Array.cpp
// 参考: https://nyaannyaan.github.io/library/data-structure/persistent-array.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_array.md
//
// API一覧
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
    static constexpr bool compact_nodes = std::is_trivially_copyable_v<T>;
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
