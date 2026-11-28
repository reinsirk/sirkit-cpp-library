#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
#define NDEBUG
// Verify: https://atcoder.jp/contests/abc370/tasks/abc370_e

// Library: Algorithm/Forbidden_Sum_Partitions.cpp
// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 生成版: Clang用。
// 生成元: Algorithm/Forbidden_Sum_Partitions.cpp
// 同梱するGenericHash::mixはSebastiano VignaのSplitMix64。原著者のPD提供声明と無条件許諾に従う。
// 原典: https://prng.di.unimi.it/splitmix64.c
// 提供条件: https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// 参考（禁止和を避ける分割）: https://atcoder.jp/contests/abc370/tasks/abc370_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forbidden_sum_partitions.md
//
// API一覧
// count_partitions_avoiding_sums<Count>(values,forbidden): int64_t列を空でない区間に分け、どの区間和も禁止値に含まれない分割数。
// forbiddenは単一値または列。順不同・重複可。空のvaluesは1。Countの加減算で数え、累積和はint64_tに収める。
// nを長さ、kを禁止値の指定数とし、型の演算がO(1)なら期待O(1+n*(k+1)+k log(k+1))時間、O(n+k+1)領域。
//
// [同梱のHashMap]
// [HashMap<Key,Val,Hasher,KeyEqual>]
// Flat table。検索・追加と次要素を探さない削除は期待償却O(1)。列キーのハッシュ計算は列長に比例する。
// 次iteratorを返す削除は空きバケットを走査し、最悪O(capacity)。返却iteratorを順に使う全削除はO(capacity)。
// 再配置・clear・swap・ムーブでiterator/参照は無効。eraseは対象だけを無効にする。
// Key/Valはコピー可能、またはvalue_typeのムーブ構築がnoexceptであること。
// Hasher/KeyEqualは同値なキーに同じハッシュ値を返し、格納中に規則を変えないこと。
// string_view等の借用キーは登録中、参照先の寿命と内容を保つ。削除後は参照先を解放できる。
// 再配置を伴う新規追加の引数には、本表の要素への参照を渡さず先にコピーする。
// HashMap(): 空の表を構築する。必要になるまで配列は確保しない。
// HashMap(n, hash, equal): n要素分を予約し、指定したハッシュ・同値判定で構築する。
// operator[](key): 未登録ならVal{}で追加し、値の参照を返す。
// find(key): 対応するiteratorを返す。未登録はend()。const版は変更できない。
// contain(key)/contains(key): 登録済みならtrueを返す。追加はしない。
// count(key): 登録済みなら1、未登録なら0を返す。
// at(key): 値の参照を返す。未登録はout_of_range。const版は変更できない。
// insert({key, value})/emplace(key, value): 未登録なら追加し、対応するiteratorを返す。
// try_emplace(key, args...): 未登録時だけ値を構築し、{iterator, 追加の有無}を返す。
// insert_or_assign(key, value): 追加または上書きし、{iterator, 追加の有無}を返す。
// get_or_default(key, fallback): 登録値またはfallbackのコピーを返す。追加はしない。
// erase(it, next=true): 対象を削除し次のiteratorを返す。false指定ならend()。
// erase(key): キーを削除し、次のiteratorを返す。未登録・erase(end())はend()。
// erase(key, next): falseなら次要素を探さず削除してend()、trueならerase(key)と同じ。
// begin()/end()/cbegin()/cend(): 格納要素を順序不定で列挙する。キーは変更できない。
// 空表のbeginはO(1)、それ以外は先頭要素まで走査。全列挙はO(C)、C=バケット数、N=登録数。
// size()/empty(): 登録数・空かどうかをO(1)で返す。
// clear(): 全要素を破棄し容量を再利用。O(C)、要素も墓石もなければO(1)。
// reserve(n): n要素分を予約し縮小しない。予約済みはO(1)、再配置は期待O(C+C')。
// rehash(n): nバケット以上・登録数に足りる容量C'へ再配置し墓石を除く。期待O(C+C')。
// 再配置の最悪時間は衝突によりO(C+C'+N^2)。ハッシュ・要素操作をO(1)とした費用。
// capacity()/load_factor(): バケット数・使用率をO(1)で返す。
// swap(other): 内容とハッシュ・同値判定を交換する。コピーは独立した複製。
// swapはHasher/KeyEqualのswapが例外を送出しないことを要求する。
//
// [GenericHash]
// GenericHash(): 実行時に選んだ種で構築する。同じオブジェクトでは結果は一定。
// GenericHash(seed): 指定した種で構築する。std::hash対応型の結果はその実装にも依存する。
// operator()(x): 整数・列挙・浮動小数・tuple・range・std::hash対応型を再帰的にハッシュ化する。
// unorderedコンテナの要素順は無視する。独自の同値関係には対応するHasherも用意する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GENERIC_HASH_DEFINED
#define SIRKIT_GENERIC_HASH_DEFINED
struct GenericHash {
private:
    std::uint64_t seed_;
    static constexpr std::uint64_t mix(std::uint64_t x) noexcept {
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        return x ^ (x >> 31);
    }
    template <class> static constexpr bool dependent_false = false;
    template <class T> static constexpr bool integral_pair = requires (const T& value) {
        value.first; value.second;
        requires std::is_integral_v<std::remove_cvref_t<decltype(value.first)>>;
        requires std::is_integral_v<std::remove_cvref_t<decltype(value.second)>>;
    };
    template <class T>
    std::uint64_t raw(const T& value) const {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::is_integral_v<U>) {
            static_assert(sizeof(U) <= sizeof(std::uint64_t), "GenericHash: integer exceeds 64 bits");
            return static_cast<std::uint64_t>(value);
        } else if constexpr (std::is_enum_v<U>) {
            return static_cast<std::uint64_t>(static_cast<std::underlying_type_t<U>>(value));
        } else if constexpr (requires { typename std::tuple_size<U>::type; }) {
            std::uint64_t result = seed_;
            std::apply([&](const auto&... elements) {
                ((result = result * UINT64_C(11995408973635179863) + raw(elements)), ...);
            }, value);
            return mix(result ^ std::tuple_size_v<U>);
        } else if constexpr (requires { typename U::hasher; typename U::key_equal; std::begin(value); std::end(value); }) {
            // 同じ要素集合は列挙順にかかわらず同じ値になる。
            std::uint64_t sum = 0, second = 0, count = 0;
            for (const auto& element : value) {
                const auto h = mix(raw(element) + seed_);
                sum += h;
                second += h * h;
                ++count;
            }
            return mix(sum ^ std::rotl(second, 23) ^ count);
        } else if constexpr (requires { { std::hash<U>{}(value) } -> std::convertible_to<std::size_t>; }) {
            // 浮動小数の+0/-0もstd::hashに従い等しい値にする。
            return static_cast<std::uint64_t>(std::hash<U>{}(value));
        } else if constexpr (requires { std::begin(value); std::end(value); }) {
            std::uint64_t result = seed_, count = 0;
            for (const auto& element : value) {
                result = result * UINT64_C(11995408973635179863) + raw(element);
                ++count;
            }
            return mix(result ^ count);
        } else {
            static_assert(dependent_false<U>, "GenericHash: provide std::hash or a custom Hasher for this key");
        }
    }
public:
    GenericHash() : GenericHash(static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count())) {}
    explicit constexpr GenericHash(std::uint64_t seed) noexcept : seed_(mix(seed) | 1) {}
    template <class T>
    std::uint64_t operator()(const T& value) const noexcept(
        std::is_integral_v<T> || std::is_enum_v<T> || integral_pair<T>) {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::is_integral_v<U> || std::is_enum_v<U>) {
            return (raw(value) ^ seed_) * UINT64_C(11995408973635179863);
        } else if constexpr (integral_pair<U>) {
            return (raw(value.first) ^ seed_) * UINT64_C(11995408973635179863) +
                   (raw(value.second) ^ seed_) * UINT64_C(10150724397891781847);
        } else return raw(value) * seed_;
    }
};
#endif

#ifndef SIRKIT_HASH_CAPACITY_DEFINED
#define SIRKIT_HASH_CAPACITY_DEFINED
namespace detail {
inline void CheckHashCapacity(std::size_t requested, std::size_t maximum, const char* message) {
    if (requested > maximum) throw std::length_error(message);
}
}
#endif

#ifndef SIRKIT_HASH_MAP_DEFINED
#define SIRKIT_HASH_MAP_DEFINED
template <class Key, class Val, class Hasher = GenericHash, class KeyEqual = std::equal_to<Key>>
class HashMap {
public:
    using key_type = Key;
    using mapped_type = Val;
    using value_type = std::pair<const Key, Val>;
    using Data = value_type;
    using size_type = std::size_t;
private:
    // 自明にコピー・破棄できる型は、GCCのx86では登録時に構築する。
    // 構築に失敗しうる型はoptionalで未構築状態を管理する。
    struct CompactSlot {
        value_type value{};
        value_type& operator*() noexcept { return value; }
        const value_type& operator*() const noexcept { return value; }
        value_type* operator->() noexcept { return std::addressof(value); }
        const value_type* operator->() const noexcept { return std::addressof(value); }
        template <class... Args> void emplace(Args&&... args) {
            value_type next(std::forward<Args>(args)...);
            std::destroy_at(std::addressof(value));
            std::construct_at(std::addressof(value), std::move(next));
        }
        void reset() noexcept {} // 自明な破棄なので、削除状態だけをstate_で変更する。
    };
    using Slot = std::conditional_t<
        std::is_nothrow_default_constructible_v<value_type> &&
        std::is_nothrow_move_constructible_v<value_type> &&
        std::is_trivially_copy_constructible_v<value_type> &&
        std::is_trivially_destructible_v<value_type>, CompactSlot, std::optional<value_type>>;
    static_assert(std::is_copy_constructible_v<value_type> ||
                  std::is_nothrow_move_constructible_v<value_type>,
                  "HashMap: entries must be copyable or nothrow movable");
    static_assert(std::is_copy_constructible_v<Hasher> && std::is_copy_constructible_v<KeyEqual> &&
                  std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>,
                  "HashMap: Hasher and KeyEqual must be copyable and nothrow swappable");
    std::vector<Slot> slots_;
    std::vector<std::uint8_t> state_; // 0=空、1=登録済み、2=削除済み
    size_type size_ = 0, occupied_ = 0;
    unsigned shift_ = 64;
    [[no_unique_address]] Hasher hash_;
    [[no_unique_address]] KeyEqual equal_;
#ifndef SKIP_RUNTIME_CHECKS
    size_type generation_ = 0;
#endif
    void invalidate() noexcept {
#ifndef SKIP_RUNTIME_CHECKS
        ++generation_;
#endif
    }
    size_type max_capacity() const noexcept {
        return std::bit_floor(std::min(slots_.max_size(), state_.max_size()));
    }
    static size_type threshold(size_type capacity) noexcept { return capacity == 0 ? 0 : (capacity - 1) / 2; }
    size_type bucket(const Key& key, unsigned shift) const noexcept(
        std::is_nothrow_invocable_v<const Hasher&, const Key&>) {
        const auto h = static_cast<std::uint64_t>(hash_(key));
        if constexpr (std::is_same_v<Hasher, GenericHash>) return static_cast<size_type>(h >> shift);
        else return static_cast<size_type>((h * UINT64_C(11995408973635179863)) >> shift);
    }
    size_type locate(const Key& key) const {
        if (slots_.empty()) return 0;
        size_type p = bucket(key, shift_);
        while (state_[p] == 1) {
            if (equal_(slots_[p]->first, key)) return p;
            p = (p + 1) & (slots_.size() - 1);
        }
        if (state_[p] == 0) return slots_.size();
        while (state_[p]) {
            if constexpr (std::is_same_v<Slot, CompactSlot> && std::is_integral_v<Key> &&
                          (std::is_same_v<KeyEqual, std::equal_to<Key>> ||
                           std::is_same_v<KeyEqual, std::equal_to<void>>)) {
                if (equal_(slots_[p]->first, key)) return state_[p] == 1 ? p : slots_.size();
            } else if (state_[p] == 1 && equal_(slots_[p]->first, key)) return p;
            p = (p + 1) & (slots_.size() - 1);
        }
        return slots_.size();
    }
    void rebuild(size_type capacity) {
        detail::CheckHashCapacity(capacity, max_capacity(), "HashMap: capacity overflow");
        std::vector<Slot> next(capacity);
        std::vector<std::uint8_t> state(capacity, 0);
        const auto shift = static_cast<unsigned>(64 - std::countr_zero(capacity));
        if constexpr (std::is_nothrow_move_constructible_v<value_type> &&
                      std::is_nothrow_invocable_v<const Hasher&, const Key&>) {
            for (size_type i = 0; i < slots_.size(); ++i) {
                if (state_[i] != 1) continue;
                size_type p = bucket(slots_[i]->first, shift);
                while (state[p]) p = (p + 1) & (capacity - 1);
                next[p].emplace(std::move(*slots_[i]));
                state[p] = 1;
            }
        } else {
            // 例外を送出しうるハッシュは、移動を始める前に全移動先を計算する。
            std::vector<std::pair<size_type, size_type>> positions;
            positions.reserve(size_);
            for (size_type i = 0; i < slots_.size(); ++i) {
                if (state_[i] != 1) continue;
                size_type p = bucket(slots_[i]->first, shift);
                while (state[p]) p = (p + 1) & (capacity - 1);
                state[p] = 1;
                positions.emplace_back(i, p);
            }
            for (const auto& [old, dest] : positions) {
                if constexpr (std::is_nothrow_move_constructible_v<value_type>)
                    next[dest].emplace(std::move(*slots_[old]));
                else next[dest].emplace(*slots_[old]);
            }
        }
        slots_.swap(next);
        state_.swap(state);
        occupied_ = size_;
        shift_ = shift;
        invalidate();
    }
    size_type insertion_slot(const Key& key) {
        if (slots_.empty()) rebuild(8);
        size_type p = bucket(key, shift_), first_deleted = slots_.size();
        while (state_[p]) {
            if constexpr (std::is_same_v<Slot, CompactSlot> && std::is_integral_v<Key> &&
                          (std::is_same_v<KeyEqual, std::equal_to<Key>> ||
                           std::is_same_v<KeyEqual, std::equal_to<void>>)) {
                // 外部参照のない整数と標準比較だけ、同値キーの墓石で止める。
                if (equal_(slots_[p]->first, key)) return p;
            } else if (state_[p] == 1 && equal_(slots_[p]->first, key)) return p;
            if (state_[p] == 2 && first_deleted == slots_.size()) first_deleted = p;
            p = (p + 1) & (slots_.size() - 1);
        }
        if (size_ == threshold(slots_.size())) {
            detail::CheckHashCapacity(slots_.size(), max_capacity() / 2, "HashMap: capacity overflow");
            rebuild(slots_.size() * 2);
            return insertion_slot(key);
        }
        if (first_deleted != slots_.size()) return first_deleted;
        if (occupied_ == threshold(slots_.size())) {
            rebuild(slots_.size());
            return insertion_slot(key);
        }
        return p;
    }
    void mark_inserted(size_type p) noexcept {
        if (state_[p] == 0) ++occupied_;
        state_[p] = 1;
        ++size_;
    }
public:
    template <bool Const>
    class BasicIterator {
        friend class HashMap;
        template <bool> friend class BasicIterator;
        using Owner = std::conditional_t<Const, const HashMap, HashMap>;
        Owner* owner_ = nullptr;
        size_type index_ = 0;
#ifndef SKIP_RUNTIME_CHECKS
        size_type generation_ = 0;
#endif
        BasicIterator(Owner* owner, size_type index) noexcept : owner_(owner), index_(index) {
#ifndef SKIP_RUNTIME_CHECKS
            generation_ = owner->generation_;
#endif
        }
        void check(bool element) const {
#ifndef SKIP_RUNTIME_CHECKS
            if (!owner_ || generation_ != owner_->generation_ || index_ > owner_->slots_.size() ||
                (element && index_ == owner_->slots_.size()) ||
                (index_ < owner_->slots_.size() && owner_->state_[index_] != 1))
                throw std::out_of_range("HashMap: invalid iterator");
#else
            (void)element;
#endif
        }
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept = std::bidirectional_iterator_tag;
        using value_type = typename HashMap::value_type;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<Const, const value_type&, value_type&>;
        using pointer = std::conditional_t<Const, const value_type*, value_type*>;
        BasicIterator() noexcept = default;
        template <bool Other> requires (Const && !Other)
        BasicIterator(const BasicIterator<Other>& other) noexcept : owner_(other.owner_), index_(other.index_) {
#ifndef SKIP_RUNTIME_CHECKS
            generation_ = other.generation_;
#endif
        }
        reference operator*() const { check(true); return *owner_->slots_[index_]; }
        pointer operator->() const { return std::addressof(operator*()); }
        BasicIterator& operator++() {
            check(true);
            do { ++index_; } while (index_ < owner_->slots_.size() && owner_->state_[index_] != 1);
            return *this;
        }
        BasicIterator operator++(int) { auto copy = *this; ++*this; return copy; }
        BasicIterator& operator--() {
            check(false);
            auto p = index_;
            while (p != 0) { --p; if (owner_->state_[p] == 1) { index_ = p; return *this; } }
#ifndef SKIP_RUNTIME_CHECKS
            throw std::out_of_range("HashMap: iterator before begin");
#else
            index_ = 0;
            return *this;
#endif
        }
        BasicIterator operator--(int) { auto copy = *this; --*this; return copy; }
        template <bool Other> bool operator==(const BasicIterator<Other>& other) const noexcept {
            return owner_ == other.owner_ && index_ == other.index_;
        }
    };
    using iterator = BasicIterator<false>;
    using const_iterator = BasicIterator<true>;
    using itr = iterator;

    HashMap() = default;
    explicit HashMap(size_type expected, Hasher hash = {}, KeyEqual equal = {})
        : hash_(std::move(hash)), equal_(std::move(equal)) { reserve(expected); }
    HashMap(const HashMap&) = default;
    HashMap(HashMap&& other) noexcept(std::is_nothrow_copy_constructible_v<Hasher> &&
                                    std::is_nothrow_copy_constructible_v<KeyEqual> &&
                                    std::is_nothrow_swappable_v<Hasher> &&
                                    std::is_nothrow_swappable_v<KeyEqual>)
        : hash_(other.hash_), equal_(other.equal_) { swap(other); }
    HashMap& operator=(HashMap other) { swap(other); return *this; }
    void swap(HashMap& other) noexcept(std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>) {
        static_assert(std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>,
                      "HashMap: Hasher and KeyEqual must be nothrow swappable");
        using std::swap;
        swap(hash_, other.hash_); swap(equal_, other.equal_);
        slots_.swap(other.slots_); state_.swap(other.state_);
        swap(size_, other.size_); swap(occupied_, other.occupied_); swap(shift_, other.shift_);
        invalidate(); other.invalidate();
    }
    iterator begin() noexcept {
        if (size_ == 0) return end();
        size_type p = 0; while (state_[p] != 1) ++p;
        return iterator(this, p);
    }
    const_iterator begin() const noexcept {
        if (size_ == 0) return end();
        size_type p = 0; while (state_[p] != 1) ++p;
        return const_iterator(this, p);
    }
    iterator end() noexcept { return iterator(this, slots_.size()); }
    const_iterator end() const noexcept { return const_iterator(this, slots_.size()); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    iterator find(const Key& key) { return iterator(this, locate(key)); }
    const_iterator find(const Key& key) const { return const_iterator(this, locate(key)); }
    bool contains(const Key& key) const { return locate(key) != slots_.size(); }
    bool contain(const Key& key) const { return contains(key); }
    int count(const Key& key) const { return contains(key) ? 1 : 0; }
    size_type size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    size_type capacity() const noexcept { return slots_.size(); }
    double load_factor() const noexcept { return slots_.empty() ? 0.0 : static_cast<double>(size_) / slots_.size(); }
    Val& at(const Key& key) {
        const auto p = locate(key);
        if (p == slots_.size()) throw std::out_of_range("HashMap::at: missing key");
        return slots_[p]->second;
    }
    const Val& at(const Key& key) const {
        const auto p = locate(key);
        if (p == slots_.size()) throw std::out_of_range("HashMap::at: missing key");
        return slots_[p]->second;
    }
    Val get_or_default(const Key& key, Val fallback = Val{}) const {
        const auto p = locate(key);
        return p == slots_.size() ? std::move(fallback) : slots_[p]->second;
    }
    template <class... Args>
    std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args) {
        const auto p = insertion_slot(key);
        if (state_[p] == 1) return {iterator(this, p), false};
        slots_[p].emplace(std::piecewise_construct, std::forward_as_tuple(key),
                          std::forward_as_tuple(std::forward<Args>(args)...));
        mark_inserted(p);
        return {iterator(this, p), true};
    }
    Val& operator[](const Key& key) { return try_emplace(key).first->second; }
    iterator insert(const value_type& entry) { return try_emplace(entry.first, entry.second).first; }
    iterator insert(value_type&& entry) { return try_emplace(entry.first, std::move(entry.second)).first; }
    iterator emplace(const Key& key, const Val& value) { return try_emplace(key, value).first; }
    template <class V>
    std::pair<iterator, bool> insert_or_assign(const Key& key, V&& value) {
        auto result = try_emplace(key, std::forward<V>(value));
        if (!result.second) result.first->second = std::forward<V>(value);
        return result;
    }
    iterator erase(const_iterator it, bool get_next = true) {
#ifndef SKIP_RUNTIME_CHECKS
        if (it.owner_ != this || it.generation_ != generation_ || it.index_ > slots_.size())
            throw std::out_of_range("HashMap::erase: foreign or stale iterator");
#endif
        if (it.index_ == slots_.size()) return end();
#ifndef SKIP_RUNTIME_CHECKS
        if (state_[it.index_] != 1)
            throw std::out_of_range("HashMap::erase: invalid iterator");
#endif
        const auto p = it.index_;
        slots_[p].reset(); state_[p] = 2; --size_;
        if (!get_next) return end();
        auto next = p;
        do { ++next; } while (next < slots_.size() && state_[next] != 1);
        return iterator(this, next);
    }
    iterator erase(iterator it, bool get_next = true) { return erase(const_iterator(it), get_next); }
    iterator erase(const Key& key) { return erase(find(key)); }
    iterator erase(const Key& key, bool get_next) { return erase(find(key), get_next); }
    void clear() noexcept {
        if (occupied_ == 0) { invalidate(); return; }
        for (size_type i = 0; i < slots_.size(); ++i) if (state_[i] == 1) slots_[i].reset();
        std::fill(state_.begin(), state_.end(), std::uint8_t{0});
        size_ = occupied_ = 0; invalidate();
    }
    void reserve(size_type count) {
        if (count <= threshold(slots_.size())) return;
        if (count > threshold(max_capacity())) throw std::length_error("HashMap::reserve: capacity overflow");
        size_type capacity = 8;
        while (count > threshold(capacity)) capacity *= 2;
        rebuild(capacity);
    }
    void rehash(size_type requested) {
        if (requested > max_capacity()) throw std::length_error("HashMap::rehash: capacity overflow");
        size_type capacity = 8;
        while (requested > capacity || size_ > threshold(capacity)) capacity *= 2;
        rebuild(capacity);
    }
};
#endif

#ifndef SIRKIT_FORBIDDEN_SUM_PARTITIONS_DEFINED
#define SIRKIT_FORBIDDEN_SUM_PARTITIONS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::int64_t AddPartitionPrefix(std::int64_t prefix, std::int64_t value)
{
#ifndef SKIP_RUNTIME_CHECKS
    if ((value > 0 && prefix > std::numeric_limits<std::int64_t>::max() - value) ||
        (value < 0 && prefix < std::numeric_limits<std::int64_t>::min() - value))
    {
        throw std::overflow_error("count_partitions_avoiding_sums: prefix overflow");
    }
#endif
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(prefix) + static_cast<std::uint64_t>(value));
}
} // namespace reinsirk::detail

template <typename Count>
[[nodiscard]] Count count_partitions_avoiding_sums(
    std::span<const std::int64_t> values, std::span<const std::int64_t> forbidden)
{
    std::vector<std::int64_t> banned(forbidden.begin(), forbidden.end());
    std::ranges::sort(banned);
    banned.erase(std::unique(banned.begin(), banned.end()), banned.end());

    HashMap<std::int64_t, Count> by_prefix;
    by_prefix.reserve(values.size() + 1);
    by_prefix[0] = Count{1};
    Count total{1};
    Count answer{1};
    std::int64_t prefix = 0;
    for (const auto value : values)
    {
        prefix = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddPartitionPrefix(prefix, value);
        answer = total;
        for (const auto bad_sum : banned)
        {
            // 減算結果が型の範囲外なら、該当する先行累積和も存在しない。
            if ((bad_sum > 0 && prefix < std::numeric_limits<std::int64_t>::min() + bad_sum) ||
                (bad_sum < 0 && prefix > std::numeric_limits<std::int64_t>::max() + bad_sum))
            {
                continue;
            }
            const auto past = by_prefix.find(prefix - bad_sum);
            if (past != by_prefix.end())
            {
                answer -= past->second;
            }
        }
        // 同じ累積和の切れ目はまとめて管理し、今の切れ目を最後に登録する。
        by_prefix[prefix] += answer;
        total += answer;
    }
    return answer;
}

template <typename Count>
[[nodiscard]] Count count_partitions_avoiding_sums(
    std::span<const std::int64_t> values, std::int64_t forbidden)
{
    return count_partitions_avoiding_sums<Count>(values, std::span{&forbidden, 1});
}

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


// ACL: atcoder/modint
#ifndef ATCODER_MODINT_HPP
#define ATCODER_MODINT_HPP 1

#include <cassert>
#include <numeric>
#include <type_traits>

#ifdef _MSC_VER
#include <intrin.h>
#endif

// ACL: atcoder/internal_math
#ifndef ATCODER_INTERNAL_MATH_HPP
#define ATCODER_INTERNAL_MATH_HPP 1

#include <utility>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace atcoder {

namespace internal {

// @param m `1 <= m`
// @return x mod m
constexpr long long safe_mod(long long x, long long m) {
    x %= m;
    if (x < 0) x += m;
    return x;
}

// Fast modular multiplication by barrett reduction
// Reference: https://en.wikipedia.org/wiki/Barrett_reduction
// NOTE: reconsider after Ice Lake
struct barrett {
    unsigned int _m;
    unsigned long long im;

    // @param m `1 <= m`
    explicit barrett(unsigned int m) : _m(m), im((unsigned long long)(-1) / m + 1) {}

    // @return m
    unsigned int umod() const { return _m; }

    // @param a `0 <= a < m`
    // @param b `0 <= b < m`
    // @return `a * b % m`
    unsigned int mul(unsigned int a, unsigned int b) const {
        // [1] m = 1
        // a = b = im = 0, so okay

        // [2] m >= 2
        // im = ceil(2^64 / m)
        // -> im * m = 2^64 + r (0 <= r < m)
        // let z = a*b = c*m + d (0 <= c, d < m)
        // a*b * im = (c*m + d) * im = c*(im*m) + d*im = c*2^64 + c*r + d*im
        // c*r + d*im < m * m + m * im < m * m + 2^64 + m <= 2^64 + m * (m + 1) < 2^64 * 2
        // ((ab * im) >> 64) == c or c + 1
        unsigned long long z = a;
        z *= b;
#ifdef _MSC_VER
        unsigned long long x;
        _umul128(z, im, &x);
#else
        unsigned long long x =
            (unsigned long long)(((unsigned __int128)(z)*im) >> 64);
#endif
        unsigned long long y = x * _m;
        return (unsigned int)(z - y + (z < y ? _m : 0));
    }
};

// @param n `0 <= n`
// @param m `1 <= m`
// @return `(x ** n) % m`
constexpr long long pow_mod_constexpr(long long x, long long n, int m) {
    if (m == 1) return 0;
    unsigned int _m = (unsigned int)(m);
    unsigned long long r = 1;
    unsigned long long y = safe_mod(x, m);
    while (n) {
        if (n & 1) r = (r * y) % _m;
        y = (y * y) % _m;
        n >>= 1;
    }
    return r;
}

// Reference:
// M. Forisek and J. Jancina,
// Fast Primality Testing for Integers That Fit into a Machine Word
// @param n `0 <= n`
constexpr bool is_prime_constexpr(int n) {
    if (n <= 1) return false;
    if (n == 2 || n == 7 || n == 61) return true;
    if (n % 2 == 0) return false;
    long long d = n - 1;
    while (d % 2 == 0) d /= 2;
    constexpr long long bases[3] = {2, 7, 61};
    for (long long a : bases) {
        long long t = d;
        long long y = pow_mod_constexpr(a, t, n);
        while (t != n - 1 && y != 1 && y != n - 1) {
            y = y * y % n;
            t <<= 1;
        }
        if (y != n - 1 && t % 2 == 0) {
            return false;
        }
    }
    return true;
}
template <int n> constexpr bool is_prime = is_prime_constexpr(n);

// @param b `1 <= b`
// @return pair(g, x) s.t. g = gcd(a, b), xa = g (mod b), 0 <= x < b/g
constexpr std::pair<long long, long long> inv_gcd(long long a, long long b) {
    a = safe_mod(a, b);
    if (a == 0) return {b, 0};

    // Contracts:
    // [1] s - m0 * a = 0 (mod b)
    // [2] t - m1 * a = 0 (mod b)
    // [3] s * |m1| + t * |m0| <= b
    long long s = b, t = a;
    long long m0 = 0, m1 = 1;

    while (t) {
        long long u = s / t;
        s -= t * u;
        m0 -= m1 * u;  // |m1 * u| <= |m1| * s <= b

        // [3]:
        // (s - t * u) * |m1| + t * |m0 - m1 * u|
        // <= s * |m1| - t * u * |m1| + t * (|m0| + |m1| * u)
        // = s * |m1| + t * |m0| <= b

        auto tmp = s;
        s = t;
        t = tmp;
        tmp = m0;
        m0 = m1;
        m1 = tmp;
    }
    // by [3]: |m0| <= b/g
    // by g != b: |m0| < b/g
    if (m0 < 0) m0 += b / s;
    return {s, m0};
}

// Compile time primitive root
// @param m must be prime
// @return primitive root (and minimum in now)
constexpr int primitive_root_constexpr(int m) {
    if (m == 2) return 1;
    if (m == 167772161) return 3;
    if (m == 469762049) return 3;
    if (m == 754974721) return 11;
    if (m == 998244353) return 3;
    int divs[20] = {};
    divs[0] = 2;
    int cnt = 1;
    int x = (m - 1) / 2;
    while (x % 2 == 0) x /= 2;
    for (int i = 3; (long long)(i)*i <= x; i += 2) {
        if (x % i == 0) {
            divs[cnt++] = i;
            while (x % i == 0) {
                x /= i;
            }
        }
    }
    if (x > 1) {
        divs[cnt++] = x;
    }
    for (int g = 2;; g++) {
        bool ok = true;
        for (int i = 0; i < cnt; i++) {
            if (pow_mod_constexpr(g, (m - 1) / divs[i], m) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) return g;
    }
}
template <int m> constexpr int primitive_root = primitive_root_constexpr(m);

// @param n `n < 2^32`
// @param m `1 <= m < 2^32`
// @return sum_{i=0}^{n-1} floor((ai + b) / m) (mod 2^64)
unsigned long long floor_sum_unsigned(unsigned long long n,
                                      unsigned long long m,
                                      unsigned long long a,
                                      unsigned long long b) {
    unsigned long long ans = 0;
    while (true) {
        if (a >= m) {
            ans += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            ans += n * (b / m);
            b %= m;
        }

        unsigned long long y_max = a * n + b;
        if (y_max < m) break;
        // y_max < m * (n + 1)
        // floor(y_max / m) <= n
        n = (unsigned long long)(y_max / m);
        b = (unsigned long long)(y_max % m);
        std::swap(m, a);
    }
    return ans;
}

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_INTERNAL_MATH_HPP

// ACL: atcoder/internal_type_traits
#ifndef ATCODER_INTERNAL_TYPE_TRAITS_HPP
#define ATCODER_INTERNAL_TYPE_TRAITS_HPP 1

#include <cassert>
#include <numeric>
#include <type_traits>

namespace atcoder {

namespace internal {

#ifndef _MSC_VER
template <class T>
using is_signed_int128 =
    typename std::conditional<std::is_same<T, __int128_t>::value ||
                                  std::is_same<T, __int128>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using is_unsigned_int128 =
    typename std::conditional<std::is_same<T, __uint128_t>::value ||
                                  std::is_same<T, unsigned __int128>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using make_unsigned_int128 =
    typename std::conditional<std::is_same<T, __int128_t>::value,
                              __uint128_t,
                              unsigned __int128>;

template <class T>
using is_integral = typename std::conditional<std::is_integral<T>::value ||
                                                  is_signed_int128<T>::value ||
                                                  is_unsigned_int128<T>::value,
                                              std::true_type,
                                              std::false_type>::type;

template <class T>
using is_signed_int = typename std::conditional<(is_integral<T>::value &&
                                                 std::is_signed<T>::value) ||
                                                    is_signed_int128<T>::value,
                                                std::true_type,
                                                std::false_type>::type;

template <class T>
using is_unsigned_int =
    typename std::conditional<(is_integral<T>::value &&
                               std::is_unsigned<T>::value) ||
                                  is_unsigned_int128<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using to_unsigned = typename std::conditional<
    is_signed_int128<T>::value,
    make_unsigned_int128<T>,
    typename std::conditional<std::is_signed<T>::value,
                              std::make_unsigned<T>,
                              std::common_type<T>>::type>::type;

#else

template <class T> using is_integral = typename std::is_integral<T>;

template <class T>
using is_signed_int =
    typename std::conditional<is_integral<T>::value && std::is_signed<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using is_unsigned_int =
    typename std::conditional<is_integral<T>::value &&
                                  std::is_unsigned<T>::value,
                              std::true_type,
                              std::false_type>::type;

template <class T>
using to_unsigned = typename std::conditional<is_signed_int<T>::value,
                                              std::make_unsigned<T>,
                                              std::common_type<T>>::type;

#endif

template <class T>
using is_signed_int_t = std::enable_if_t<is_signed_int<T>::value>;

template <class T>
using is_unsigned_int_t = std::enable_if_t<is_unsigned_int<T>::value>;

template <class T> using to_unsigned_t = typename to_unsigned<T>::type;

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_INTERNAL_TYPE_TRAITS_HPP

namespace atcoder {

namespace internal {

struct modint_base {};
struct static_modint_base : modint_base {};

template <class T> using is_modint = std::is_base_of<modint_base, T>;
template <class T> using is_modint_t = std::enable_if_t<is_modint<T>::value>;

}  // namespace internal

template <int m, std::enable_if_t<(1 <= m)>* = nullptr>
struct static_modint : internal::static_modint_base {
    using mint = static_modint;

  public:
    static constexpr int mod() { return m; }
    static mint raw(int v) {
        mint x;
        x._v = v;
        return x;
    }

    static_modint() : _v(0) {}
    template <class T, internal::is_signed_int_t<T>* = nullptr>
    static_modint(T v) {
        long long x = (long long)(v % (long long)(umod()));
        if (x < 0) x += umod();
        _v = (unsigned int)(x);
    }
    template <class T, internal::is_unsigned_int_t<T>* = nullptr>
    static_modint(T v) {
        _v = (unsigned int)(v % umod());
    }

    int val() const { return _v; }

    mint& operator++() {
        _v++;
        if (_v == umod()) _v = 0;
        return *this;
    }
    mint& operator--() {
        if (_v == 0) _v = umod();
        _v--;
        return *this;
    }
    mint operator++(int) {
        mint result = *this;
        ++*this;
        return result;
    }
    mint operator--(int) {
        mint result = *this;
        --*this;
        return result;
    }

    mint& operator+=(const mint& rhs) {
        _v += rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator-=(const mint& rhs) {
        _v -= rhs._v;
        if (_v >= umod()) _v += umod();
        return *this;
    }
    mint& operator*=(const mint& rhs) {
        unsigned long long z = _v;
        z *= rhs._v;
        _v = (unsigned int)(z % umod());
        return *this;
    }
    mint& operator/=(const mint& rhs) { return *this = *this * rhs.inv(); }

    mint operator+() const { return *this; }
    mint operator-() const { return mint() - *this; }

    mint pow(long long n) const {
        assert(0 <= n);
        mint x = *this, r = 1;
        while (n) {
            if (n & 1) r *= x;
            x *= x;
            n >>= 1;
        }
        return r;
    }
    mint inv() const {
        if (prime) {
            assert(_v);
            return pow(umod() - 2);
        } else {
            auto eg = internal::inv_gcd(_v, m);
            assert(eg.first == 1);
            return eg.second;
        }
    }

    friend mint operator+(const mint& lhs, const mint& rhs) {
        return mint(lhs) += rhs;
    }
    friend mint operator-(const mint& lhs, const mint& rhs) {
        return mint(lhs) -= rhs;
    }
    friend mint operator*(const mint& lhs, const mint& rhs) {
        return mint(lhs) *= rhs;
    }
    friend mint operator/(const mint& lhs, const mint& rhs) {
        return mint(lhs) /= rhs;
    }
    friend bool operator==(const mint& lhs, const mint& rhs) {
        return lhs._v == rhs._v;
    }
    friend bool operator!=(const mint& lhs, const mint& rhs) {
        return lhs._v != rhs._v;
    }

  private:
    unsigned int _v;
    static constexpr unsigned int umod() { return m; }
    static constexpr bool prime = internal::is_prime<m>;
};

template <int id> struct dynamic_modint : internal::modint_base {
    using mint = dynamic_modint;

  public:
    static int mod() { return (int)(bt.umod()); }
    static void set_mod(int m) {
        assert(1 <= m);
        bt = internal::barrett(m);
    }
    static mint raw(int v) {
        mint x;
        x._v = v;
        return x;
    }

    dynamic_modint() : _v(0) {}
    template <class T, internal::is_signed_int_t<T>* = nullptr>
    dynamic_modint(T v) {
        long long x = (long long)(v % (long long)(mod()));
        if (x < 0) x += mod();
        _v = (unsigned int)(x);
    }
    template <class T, internal::is_unsigned_int_t<T>* = nullptr>
    dynamic_modint(T v) {
        _v = (unsigned int)(v % mod());
    }

    int val() const { return _v; }

    mint& operator++() {
        _v++;
        if (_v == umod()) _v = 0;
        return *this;
    }
    mint& operator--() {
        if (_v == 0) _v = umod();
        _v--;
        return *this;
    }
    mint operator++(int) {
        mint result = *this;
        ++*this;
        return result;
    }
    mint operator--(int) {
        mint result = *this;
        --*this;
        return result;
    }

    mint& operator+=(const mint& rhs) {
        _v += rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator-=(const mint& rhs) {
        _v += mod() - rhs._v;
        if (_v >= umod()) _v -= umod();
        return *this;
    }
    mint& operator*=(const mint& rhs) {
        _v = bt.mul(_v, rhs._v);
        return *this;
    }
    mint& operator/=(const mint& rhs) { return *this = *this * rhs.inv(); }

    mint operator+() const { return *this; }
    mint operator-() const { return mint() - *this; }

    mint pow(long long n) const {
        assert(0 <= n);
        mint x = *this, r = 1;
        while (n) {
            if (n & 1) r *= x;
            x *= x;
            n >>= 1;
        }
        return r;
    }
    mint inv() const {
        auto eg = internal::inv_gcd(_v, mod());
        assert(eg.first == 1);
        return eg.second;
    }

    friend mint operator+(const mint& lhs, const mint& rhs) {
        return mint(lhs) += rhs;
    }
    friend mint operator-(const mint& lhs, const mint& rhs) {
        return mint(lhs) -= rhs;
    }
    friend mint operator*(const mint& lhs, const mint& rhs) {
        return mint(lhs) *= rhs;
    }
    friend mint operator/(const mint& lhs, const mint& rhs) {
        return mint(lhs) /= rhs;
    }
    friend bool operator==(const mint& lhs, const mint& rhs) {
        return lhs._v == rhs._v;
    }
    friend bool operator!=(const mint& lhs, const mint& rhs) {
        return lhs._v != rhs._v;
    }

  private:
    unsigned int _v;
    static internal::barrett bt;
    static unsigned int umod() { return bt.umod(); }
};
template <int id> internal::barrett dynamic_modint<id>::bt(998244353);

using modint998244353 = static_modint<998244353>;
using modint1000000007 = static_modint<1000000007>;
using modint = dynamic_modint<-1>;

namespace internal {

template <class T>
using is_static_modint = std::is_base_of<internal::static_modint_base, T>;

template <class T>
using is_static_modint_t = std::enable_if_t<is_static_modint<T>::value>;

template <class> struct is_dynamic_modint : public std::false_type {};
template <int id>
struct is_dynamic_modint<dynamic_modint<id>> : public std::true_type {};

template <class T>
using is_dynamic_modint_t = std::enable_if_t<is_dynamic_modint<T>::value>;

}  // namespace internal

}  // namespace atcoder

#endif  // ATCODER_MODINT_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::int64_t forbidden{};
    verify_in >> n >> forbidden;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto result = sirkit::count_partitions_avoiding_sums<atcoder::modint998244353>(values, forbidden);
    verify_out << result.val() << '\n';
}
