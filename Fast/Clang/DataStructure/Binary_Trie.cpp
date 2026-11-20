// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Binary_Trie.cpp
// 参考: https://nyaannyaan.github.io/library/data-structure/binary-trie.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_trie.md
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// [速度優先版 BinaryTrie<T,MAX_LOG,NODES>]
// ノードとID用ブロックをnewで確保し、破棄時も解放しない。プロセス終了まで累積する。
// SIRKIT_FAST_TRIE_INLINE_IDS: 最初の32個までのID枠を内蔵し、初回ID追加の確保を省く。
// 既定はID未使用時のオブジェクトを小さく保つ。内蔵時もIDのvector容量は解放しない。
// 正常な入力・十分なNODES・確保成功・size_typeに収まる件数が前提。例外時の状態復元は保証しない。
// C++20以降。重複を許す非負整数の二分トライ。T は bool / const / volatile を除く整数型。
// キーと XOR マスクは 0 <= x < 2^(MAX_LOG + 1)。MAX_LOG は最高位ビット番号。
// MAX_LOGの既定値はstd::numeric_limits<T>::digits - 1。第2テンプレート引数で手動指定も可。
// NODESは実ノード/ID枠の確保上限。必要数が上限内であることは利用側が保証。以下B=MAX_LOG+1。
// 配布のFast版は入力検査・容量検査・仮想二分ノード数の管理を除く。
// get_kth は重複込み・1始まり。ID は重複可、-1 は匿名（ID一覧には含めない）。
// ID省略の削除は匿名を優先し、なければ任意のID付き要素を削除する。
// ID一覧は順序不定の読み取り専用参照。次の更新または破棄まで有効。
// BinaryTrie(): 空の集合を構築する。
// size(): 重複・匿名要素を含む要素数を返す（O(1)）。
// empty(): 空ならtrueを返す（O(1)）。
// add(x, id=-1): xを1個追加する。id=-1は匿名。
// insert_unique(x,id=-1): 未登録時だけ追加してtrue。登録済みならIDも変えずfalse。
// del(x, id=-1): xを1個削除し、削除できればtrue。id指定時はそのIDを削除する。
// find(x): {xの個数, ID一覧}を返す。未登録なら{0, 空配列}。
// min_element(): {最小値, ID一覧}を返す。空でないこと。
// max_element(): {最大値, ID一覧}を返す。空でないこと。
// get_kth(k): {昇順でk番目の値, ID一覧}を返す。重複込み・1始まり。
// operate_xor(x): 全要素の値にXORを適用する（O(1)）。
// count(x): xの個数を返す。匿名要素・重複も数える。
// contains(x): xが1個以上あればtrueを返す。
// count_less(upper): upper未満の要素数を重複込みで返す。
// min_xor(x): {x ^ yの最小値, 対応するyのID一覧}を返す。空でないこと。
// max_xor(x): {x ^ yの最大値, 対応するyのID一覧}を返す。空でないこと。
// count_xor_less(x, upper): x ^ y < upperとなる要素数を重複込みで返す。
// 値の検索・順位・XORクエリはO(B)。整数型のupperが0以下なら個数は0。
// 構築時に最初のノードブロックを確保する。コピー・ムーブは不可。
// add/insert_uniqueは償却O(B)、ID追加は償却O(1)。確保失敗時の保証はない。
// delはID省略ならO(B)、指定IDの削除はO(B+その値のID数)。値またはIDがなければfalse。
// get_kthは1<=k<=size()が前提。operate_xor後にadd(y)する値はy。
// countはID一覧を参照しない。upperはキー型より広い整数型も可、キー範囲を超えればsize()。
// min_xor/max_xor/count_xor_lessは状態を変更しない。返す値は保存値yではなくx^y。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BINARY_TRIE_DEFINED
#define SIRKIT_BINARY_TRIE_DEFINED
template <typename T, int MAX_LOG = std::numeric_limits<T>::digits - 1,
          std::size_t NODES = 16777216>
class BinaryTrie
{
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>,
                  "T must be an integer type other than bool");
    static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                  "T must not be const or volatile");
    static_assert(MAX_LOG >= 0 && MAX_LOG < std::numeric_limits<T>::digits,
                  "MAX_LOG is outside the nonnegative range of T");
    static_assert(NODES > 0, "NODES must include at least the root");

public:
    using Container = std::vector<int>;
    using size_type = std::size_t;
    using FindResult = std::pair<size_type, const Container &>;
    using ElementResult = std::pair<T, const Container &>;

private:
    using U = std::make_unsigned_t<T>;
    static constexpr size_type kBits = MAX_LOG + 1;
    static constexpr size_type kLevels = (kBits + 1) / 2;
    static constexpr size_type kBlockSize = 8192;
    static constexpr size_type kInitialNodes = std::min(size_type{2048}, NODES);

    // 内部では子、葉ではID枠への非所有参照を保持する。
    struct Node
    {
        struct IdData { Container *ptr; size_type slot; }; // ptrはID枠内への非所有ポインタ。
        union Data
        {
            Node *child[4]; // ノードブロック内への非所有ポインタ。
            IdData ids;
            Data() noexcept : child{nullptr, nullptr, nullptr, nullptr} {}
        } data;
        size_type count = 0;
    };
    // 最初の最大2048ノードもブロックとして確保し、解放しない。
    using InitialNode = Node;
    struct InitialBlock { size_type padding = 0; std::array<InitialNode, kInitialNodes> nodes; };

    // ID配列も解放しないブロックへ置き、拡張時の参照を維持する。
    struct IdSlot
    {
        Container ids;
        size_type next = std::numeric_limits<size_type>::max();
    };
    static constexpr size_type kNoSlot = std::numeric_limits<size_type>::max();
    static constexpr size_type kInitialIdSlots = std::min(size_type{32}, NODES / 2);
#ifdef SIRKIT_FAST_TRIE_INLINE_IDS
    union InitialIdStorage
    {
        std::array<IdSlot, kInitialIdSlots> slots;
        InitialIdStorage() : slots{} {}
        ~InitialIdStorage() {} // 初期ID配列のvector容量も解放しない。
    };
#endif
    static constexpr size_type kIdBlockSize = 4096;

    IdSlot &id_slot(size_type i) noexcept
    {
        if (i < kInitialIdSlots) return initial_ids_[i];
        const auto index = i - kInitialIdSlots;
        return id_blocks_[index / kIdBlockSize][index % kIdBlockSize];
    }
    void append_id(Node *leaf, int id)
    {
#ifndef SIRKIT_FAST_TRIE_INLINE_IDS
        if (!initial_ids_ && kInitialIdSlots != 0) initial_ids_ = new IdSlot[kInitialIdSlots];
#endif
        const bool recycled = free_id_slot_ != kNoSlot;
        const auto index = recycled ? free_id_slot_ : next_id_;
        if (!recycled && next_id_ == id_capacity_)
        {
            const auto n = std::min(kIdBlockSize, NODES - id_capacity_);
            id_blocks_.push_back(new IdSlot[n]);
            id_capacity_ += n;
        }
        auto &slot = id_slot(index);
        slot.ids.push_back(id);
        if (recycled) free_id_slot_ = slot.next;
        else ++next_id_;
        leaf->data.ids = {&slot.ids, index};
    }

    // 確保したブロックの参照を保持し、拡張時もアドレスを維持する。
    std::vector<Node*> blocks_;
    size_type capacity_ = 0;
    size_type next_unused_ = 0;
    Node *free_list_ = nullptr;
    Node *root_ = nullptr;
    U lazy_ = 0;
    InitialBlock* initial_ = nullptr;
    IdSlot* initial_ids_ = nullptr;
    std::vector<IdSlot*> id_blocks_;
    size_type id_capacity_ = kInitialIdSlots, next_id_ = 0;
    size_type free_id_slot_ = kNoSlot;
#ifdef SIRKIT_FAST_TRIE_INLINE_IDS
    InitialIdStorage initial_storage_;
#endif

    // 再利用リストか未使用領域から取得し、必要なら上限内でブロックを確保する。
    Node *acquire_node()
    {
        if (free_list_)
        {
            Node *node = free_list_;
            free_list_ = node->data.child[0];
            for (auto &child : node->data.child) child = nullptr;
            return node;
        }
        if (next_unused_ == capacity_)
        {
            const auto n = std::min(kBlockSize, NODES - capacity_);
            blocks_.push_back(new Node[n]);
            capacity_ += n;
        }
        const auto index = next_unused_++;
        if (index < kInitialNodes) return &initial_->nodes[index];
        const auto offset = index - kInitialNodes;
        return &blocks_[offset / kBlockSize + 1][offset % kBlockSize];
    }

    // ノードを初期化して再利用リストへ戻す。子の回収は呼び出し側で行う。
    void release_node(Node *node, bool leaf) noexcept
    {
        if (leaf && node->data.ids.ptr)
        {
            const auto slot = node->data.ids.slot;
            auto &entry = id_slot(slot);
            entry.ids.clear(); // 再利用枠の容量を保持する。
            entry.next = free_id_slot_;
            free_id_slot_ = slot;
        }
        node->count = 0;
        node->data.child[0] = free_list_;
        node->data.child[1] = nullptr;
        free_list_ = node;
    }

    // 値の範囲を検査し、内部用の符号なし型へ変換する。
    static U checked_key(T x)
    {
        return static_cast<U>(x);
    }

    static constexpr unsigned width(size_type depth) noexcept
    {
        return depth == 1 && kBits % 2 != 0 ? 1u : 2u;
    }
    static constexpr size_type shift(size_type depth) noexcept
    {
        return 2 * (kLevels - depth);
    }
    static unsigned digit(U key, size_type depth) noexcept
    {
        return static_cast<unsigned>((key >> shift(depth)) & (width(depth) == 1 ? U{1} : U{3}));
    }
    static const Container &ids_of(const Node *node) noexcept
    {
        static constexpr Container empty;
        return node && node->data.ids.ptr ? *node->data.ids.ptr : empty;
    }
    const Node *find_leaf(U key) const noexcept
    {
        const Node *node = root_;
        for (size_type depth = 1; depth <= kLevels && node; ++depth)
            node = node->data.child[digit(key, depth)];
        return node;
    }
    ElementResult xor_extreme(U mask, bool maximum) const
    {
        const Node *node = root_;
        U value = 0;
        // 1要素なら物理キーの経路が確定し、照会マスクで分岐を変えなくてよい。
        if (node->count == 1) {
            for (size_type depth = 1; depth <= kLevels; ++depth) {
                unsigned d = 0;
                if (!node->data.child[0]) {
                    if (node->data.child[1]) d = 1;
                    else if (node->data.child[2]) d = 2;
                    else d = 3;
                }
                value |= static_cast<U>(d) << shift(depth);
                node = node->data.child[d];
            }
            return {static_cast<T>(value ^ mask), ids_of(node)};
        }
        for (size_type depth = 1; depth <= kLevels; ++depth)
        {
            const unsigned last = width(depth) == 1 ? 1u : 3u;
            const unsigned preferred = digit(mask, depth) ^ (maximum ? last : 0u);
            unsigned offset = 0;
            if (!node->data.child[preferred])
            {
                if (node->data.child[preferred ^ 1u]) offset = 1;
                else if (node->data.child[preferred ^ 2u]) offset = 2;
                else offset = 3;
            }
            value |= static_cast<U>(offset ^ (maximum ? last : 0u)) << shift(depth);
            node = node->data.child[preferred ^ offset];
        }
        return {static_cast<T>(value), ids_of(node)};
    }
    ElementResult extreme(bool maximum) const
    {
        return xor_extreme(lazy_, maximum);
    }
    template<class Bound>
    size_type xor_count_less(U mask, Bound upper) const noexcept
    {
        static_assert(std::is_integral_v<Bound>, "upper must be an integer");
        if constexpr (std::is_signed_v<Bound>) if (upper <= 0) return 0;
        const auto limit = static_cast<std::uintmax_t>(upper);
        if (!limit) return 0;
        if ((limit >> MAX_LOG) > std::uintmax_t{1}) return size();
        const U bound = static_cast<U>(limit);
        const Node *node = root_;
        size_type result = 0;
        for (size_type depth = 1; depth <= kLevels && node; ++depth)
        {
            const unsigned zero = digit(mask, depth);
            const unsigned n = digit(bound, depth);
            for (unsigned d = 0; d < n; ++d)
                if (const Node *child = node->data.child[zero ^ d]) result += child->count;
            node = node->data.child[zero ^ n];
        }
        return result;
    }

    bool insert(T x, int id, bool unique)
    {
        const U key = checked_key(x) ^ lazy_;
        std::array<Node*, kLevels + 1> path;
        path[0] = root_;
        size_type first_new = 0;
        for (size_type depth = 1; depth <= kLevels; ++depth)
        {
            const auto d = digit(key, depth);
            Node* child = path[depth - 1]->data.child[d];
            if (!child)
            {
                child = acquire_node();
                if (!first_new) first_new = depth;
                else path[depth - 1]->data.child[d] = child;
            }
            path[depth] = child;
        }
        if (unique && !first_new) return false;
        Node* leaf = path[kLevels];
        if (first_new) leaf->data.ids.ptr = nullptr;
        if (id != -1)
        {
            if (leaf->data.ids.ptr) leaf->data.ids.ptr->push_back(id);
            else append_id(leaf, id);
        }
        if (first_new) path[first_new - 1]->data.child[digit(key, first_new)] = path[first_new];
        for (Node* node : path) ++node->count;
        return true;
    }

public:
    BinaryTrie()
    {
#ifdef SIRKIT_FAST_TRIE_INLINE_IDS
        initial_ids_ = initial_storage_.slots.data();
#endif
        initial_ = new InitialBlock;
        blocks_.push_back(nullptr);
        capacity_ = kInitialNodes;
        root_ = acquire_node();
    }

    BinaryTrie(const BinaryTrie &) = delete;
    BinaryTrie &operator=(const BinaryTrie &) = delete;
    BinaryTrie(BinaryTrie &&) = delete;
    BinaryTrie &operator=(BinaryTrie &&) = delete;

    [[nodiscard]] size_type size() const noexcept { return root_->count; }
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    void add(T x, int id = -1) { static_cast<void>(insert(x, id, false)); }
    bool insert_unique(T x, int id = -1) { return insert(x, id, true); }

    bool del(T x, int id = -1)
    {
        const U key = checked_key(x) ^ lazy_;
        std::array<Node *, kLevels + 1> path;
        path[0] = root_;
        for (size_type depth = 1; depth <= kLevels; ++depth)
        {
            path[depth] = path[depth - 1]->data.child[digit(key, depth)];
            if (!path[depth]) return false;
        }
        Node *leaf = path[kLevels];
        if (id != -1)
        {
            if (!leaf->data.ids.ptr) return false;
            auto &ids = *leaf->data.ids.ptr;
            const auto it = std::find(ids.begin(), ids.end(), id);
            if (it == ids.end()) return false;
            *it = ids.back();
            ids.pop_back();
        }
        else if (leaf->data.ids.ptr && leaf->count == leaf->data.ids.ptr->size())
            leaf->data.ids.ptr->pop_back();
        for (Node *node : path) --node->count;
        for (size_type depth = kLevels; depth > 0; --depth)
        {
            if (path[depth]->count) break;
            const auto d = digit(key, depth);
            auto *parent = path[depth - 1];
            parent->data.child[d] = nullptr;
            release_node(path[depth], depth == kLevels);
        }
        return true;
    }

    [[nodiscard]] FindResult find(T x) const
    {
        const Node *leaf = find_leaf(checked_key(x) ^ lazy_);
        return {leaf ? leaf->count : 0, ids_of(leaf)};
    }
    [[nodiscard]] ElementResult min_element() const { return extreme(false); }
    [[nodiscard]] ElementResult max_element() const { return extreme(true); }
    [[nodiscard]] ElementResult get_kth(std::int64_t k) const
    {
        auto rank = static_cast<size_type>(k);
        const Node *node = root_;
        U value = 0;
        for (size_type depth = 1; depth <= kLevels; ++depth)
        {
            const unsigned zero = digit(lazy_, depth);
            const unsigned last = width(depth) == 1 ? 1u : 3u;
            unsigned d = 0;
            for (; d < last; ++d)
            {
                const Node *child = node->data.child[zero ^ d];
                const auto n = child ? child->count : 0;
                if (rank <= n) break;
                rank -= n;
            }
            value |= static_cast<U>(d) << shift(depth);
            node = node->data.child[zero ^ d];
        }
        return {static_cast<T>(value), ids_of(node)};
    }
    void operate_xor(T x) { lazy_ ^= checked_key(x); }

    [[nodiscard]] size_type count(T x) const
    {
        const Node *leaf = find_leaf(checked_key(x) ^ lazy_);
        return leaf ? leaf->count : 0;
    }

    [[nodiscard]] bool contains(T x) const
    {
        return find_leaf(checked_key(x) ^ lazy_) != nullptr;
    }

    template <class Bound>
    [[nodiscard]] size_type count_less(Bound upper) const noexcept
    {
        return xor_count_less(lazy_, upper);
    }

    [[nodiscard]] ElementResult min_xor(T x) const
    {
        return xor_extreme(checked_key(x) ^ lazy_, false);
    }

    [[nodiscard]] ElementResult max_xor(T x) const
    {
        return xor_extreme(checked_key(x) ^ lazy_, true);
    }

    template <class Bound>
    [[nodiscard]] size_type count_xor_less(T x, Bound upper) const
    {
        return xor_count_less(checked_key(x) ^ lazy_, upper);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
