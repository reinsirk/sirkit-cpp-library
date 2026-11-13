// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Hash_set.cpp
// SplitMix64部分（GenericHash::mix）はCC0ではなく、原著者のPD提供声明・無条件許諾に従う。
// CC0-1.0はそれ以外の部分に適用する。SplitMix64の提供条件:
// https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// Original Source（GenericHash::mixのみ。Sebastiano Vigna、PD提供声明・無条件許諾）:
// https://prng.di.unimi.it/splitmix64.c
// 参考: https://nyaannyaan.github.io/library/hashmap/hashset.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hash_set.md
//
// API一覧
// 速度優先版: 確保・構築成功と有効な引数が前提。例外時の状態復元は保証しない。
// eraseは非自明な登録値を破棄する。clear・表の破棄では登録値の破棄を省く。
// 再配置後の旧配列だけ解放し、最後の表と省略した値の資源は回収しない。
// 格納型のデストラクタの副作用・資源回収を必要としない処理で使う。
// [HashSet<Key,Hasher,KeyEqual>]
// Flat table。検索・追加・次iteratorを求めない削除は期待償却O(1)。列キーのハッシュ計算は列長に比例する。
// 次iteratorへの移動は空き区間長に比例する。削除では自動縮小しないため、疎になればrehash(0)で縮小する。
// 再配置・clear・swap・ムーブでiterator/参照は無効。eraseは対象だけを無効にする。
// Keyはコピー可能、またはムーブ構築がnoexceptであること。
// Hasher/KeyEqualは同値なキーに同じハッシュ値を返し、格納中に規則を変えないこと。
// iterator/参照の無効化後の使用は禁止。検査でも全ての使い回しは検出しない。
// HashSet()/HashSet(n, hash, equal): 空集合を作り、後者はn要素分を予約する。
// HashSet({keys}, hash, equal)/HashSet(first,last,hash,equal): 指定したキー列から構築する。
// insert(const Key&)/insert(Key&&): コピー/ムーブで追加しiteratorを返す。重複時は既存要素を返す。
// insert_checked(const Key&)/insert_checked(Key&&): コピー/ムーブで追加し、{iterator, 追加の有無}を返す。
// emplace(args...): Keyを構築して追加し、{iterator, 追加の有無}を返す。
// insert_range(first,last): 入力iteratorの範囲を追加する。同じ表のiteratorは渡さないこと。
// find(key): 対応するiteratorを返す。未登録はend()。キーの変更は禁止する。
// contain(key)/contains(key)/count(key): 登録済みか、または登録数0/1を返す。
// erase(it, next=true): 削除して次のiteratorを返す。falseならO(1)でend()。erase(end())もend()。
// erase(key, next=true): キーを削除して次のiteratorを返す。falseなら期待O(1)でend()。未登録もend()。
// erase_if(pred): pred(const Key&)がtrueの要素を削除し、削除数を返す。空ならO(1)、他はO(capacity())。predは表を変更しないこと。
// begin()/end()/cbegin()/cend(): 順序不定で列挙する。逆向き移動もでき、キーはconst参照。
// 空集合のbeginはO(1)、それ以外は先頭要素までのバケットを走査する。
// size()/empty()/capacity()/load_factor(): 要素数・空か・バケット数・使用率をO(1)で返す。
// clear(): O(capacity())で登録状態を消し、容量を再利用する。値のデストラクタは呼ばない。
// reserve(n): n要素分を予約する。縮小しない。再配置はO(旧容量+新容量)。
// rehash(n): nバケット以上・現登録数に必要な容量へ再配置し、墓石を除く。
// HashSet(other)/operator=(other): 独立したコピーを作る。コピーにはKeyのコピー構築が必要。
// HashSet(move(other))/operator=(move(other)): 内容を移動し、移動元は空で再利用可能。
// swap(other): 内容とハッシュ・同値判定を交換する。O(1)。
// operator==(other)/!=: 要素数と両方向のcontainsで比較する。期待O(両表の容量)。
// swapはHasher/KeyEqualのswapが例外を送出しないことを要求する。
//
// [GenericHash]
// GenericHash(): 実行時に選んだ種で構築する。同じオブジェクトでは結果は一定。
// GenericHash(seed): 指定した種で構築する。std::hash対応型の結果はその実装にも依存する。
// operator()(x): 整数・列挙・浮動小数・tuple・range・std::hash対応型を再帰的にハッシュ化する。
// unorderedコンテナの要素順は無視する。独自の同値関係には対応するHasherも用意する。

#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>
#ifndef SIRKIT_FAST_HASH_RELEASED_ARRAY_DEFINED
#define SIRKIT_FAST_HASH_RELEASED_ARRAY_DEFINED
namespace reinsirk::detail {
template<class T>class FastHashReleasedArray {
    T* data_=nullptr;
    std::size_t size_=0;
public:
    FastHashReleasedArray() noexcept=default;
    explicit FastHashReleasedArray(std::size_t n):data_(n?new T[n]:nullptr),size_(n){}
    FastHashReleasedArray(std::size_t n,const T& value):FastHashReleasedArray(n){if(n)std::fill_n(data_,n,value);}
    FastHashReleasedArray(const FastHashReleasedArray&)=delete;
    FastHashReleasedArray& operator=(const FastHashReleasedArray&)=delete;
    FastHashReleasedArray(FastHashReleasedArray&& other)noexcept:data_(std::exchange(other.data_,nullptr)),size_(std::exchange(other.size_,0)){}
    FastHashReleasedArray& operator=(FastHashReleasedArray&& other)noexcept{if(this!=&other){data_=std::exchange(other.data_,nullptr);size_=std::exchange(other.size_,0);}return *this;}
    ~FastHashReleasedArray()=default;
    static constexpr std::size_t max_size()noexcept{return static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())/sizeof(T);}
    std::size_t size()const noexcept{return size_;}
    bool empty()const noexcept{return size_==0;}
    T*data()noexcept{return data_;}const T*data()const noexcept{return data_;}
    T*begin()noexcept{return data_;}const T*begin()const noexcept{return data_;}
    T*end()noexcept{return size_?data_+size_:data_;}const T*end()const noexcept{return size_?data_+size_:data_;}
    T&operator[](std::size_t i)noexcept{return data_[i];}const T&operator[](std::size_t i)const noexcept{return data_[i];}
    void release_storage()noexcept{delete[] data_;data_=nullptr;size_=0;}
    void swap(FastHashReleasedArray& other)noexcept{std::swap(data_,other.data_);std::swap(size_,other.size_);}
};
}
#endif

#if !defined(SIRKIT_DISABLE_SIMD) && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
#include <emmintrin.h>
#endif

#include <algorithm>
#include <bit>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
#include <emmintrin.h>
#endif

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

#ifndef SIRKIT_HASH_SET_DEFINED
#define SIRKIT_HASH_SET_DEFINED
template <class Key, class Hasher = GenericHash, class KeyEqual = std::equal_to<Key>>
class HashSet {
public:
    using key_type = Key;
    using value_type = Key;
    using Data = value_type;
    using size_type = std::size_t;
private:
    friend GenericHash;
    using hasher = Hasher;
    using key_equal = KeyEqual;
    template<bool Compact>union SlotStorage {
        std::remove_cv_t<value_type> value;
        SlotStorage()noexcept{}
        ~SlotStorage() requires std::is_trivially_destructible_v<value_type> = default;
        ~SlotStorage()noexcept requires (!std::is_trivially_destructible_v<value_type>) {} // Fast契約: 登録値も破棄しない。
        SlotStorage(const SlotStorage&)=delete;
        SlotStorage(SlotStorage&&)=delete;
        value_type&operator*()noexcept{return value;}
        const value_type&operator*()const noexcept{return value;}
        value_type*operator->()noexcept{return std::addressof(value);}
        const value_type*operator->()const noexcept{return std::addressof(value);}
        template<class...Args>void emplace(Args&&...args){std::construct_at(std::addressof(value),std::forward<Args>(args)...);}
        void reset()noexcept{if constexpr(!std::is_trivially_destructible_v<value_type>)std::destroy_at(std::addressof(value));}
    };
    using CompactSlot=SlotStorage<true>;
    using Slot=std::conditional_t<
        std::is_nothrow_default_constructible_v<value_type>&&
        std::is_nothrow_move_constructible_v<value_type>&&
        std::is_trivially_copy_constructible_v<value_type>&&
        std::is_trivially_destructible_v<value_type>,CompactSlot,SlotStorage<false>>;
    static_assert(std::is_copy_constructible_v<value_type> ||
                  std::is_nothrow_move_constructible_v<value_type>,
                  "HashSet: entries must be copyable or nothrow movable");
    static_assert(std::is_copy_constructible_v<Hasher> && std::is_copy_constructible_v<KeyEqual> &&
                  std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>,
                  "HashSet: Hasher and KeyEqual must be copyable and nothrow swappable");
    reinsirk::detail::FastHashReleasedArray<Slot> slots_;
    reinsirk::detail::FastHashReleasedArray<std::uint8_t> state_; // 0=空、1=登録済み、2=削除済み
    size_type size_ = 0, occupied_ = 0;
    unsigned shift_ = 64;
    [[no_unique_address]] Hasher hash_;
    [[no_unique_address]] KeyEqual equal_;
    void invalidate() noexcept {
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
        if (size_ == 0) return slots_.size();
        size_type p = bucket(key, shift_);
        while (state_[p]) {
            if (state_[p] == 1 && equal_(*slots_[p], key)) return p;
            p = (p + 1) & (slots_.size() - 1);
        }
        return slots_.size();
    }
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
    // 登録状態を16スロットずつ判定し、有効なキーだけを移す。
    void rebuild_simd(size_type capacity) {
        reinsirk::detail::FastHashReleasedArray<Slot> next(capacity);
        // 値の構築前に領域を一定間隔で準備する。
         {
            if (capacity >= 256) {
                constexpr size_type stride = std::max(size_type{1}, size_type{4096} / sizeof(Slot));
                for (size_type i = 0; i < capacity; i += stride)
                    *reinterpret_cast<volatile unsigned char*>(std::addressof(next[i])) = 0;
            }
        }

        reinsirk::detail::FastHashReleasedArray<std::uint8_t> state(capacity, 0);
        const auto shift = static_cast<unsigned>(64 - std::countr_zero(capacity));
        const auto live = _mm_set1_epi8(1);
        for (size_type base = 0; base < slots_.size(); base += 16) {
            const auto states = _mm_loadu_si128(reinterpret_cast<const __m128i*>(state_.data() + base));
            unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(states, live)));
            while (mask) {
                const size_type i = base + std::countr_zero(mask);
                size_type p = bucket(*slots_[i], shift);
                while (state[p]) p = (p + 1) & (capacity - 1);
                next[p].emplace(*slots_[i]);
                state[p] = 1;
                mask &= mask - 1;
            }
        }
        slots_.swap(next); state_.swap(state);
        occupied_ = size_; shift_ = shift; invalidate();
        next.release_storage();state.release_storage();
    }
#endif
    void rebuild(size_type capacity) {
        if (capacity > max_capacity()) throw std::length_error("HashSet: capacity overflow");
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr (std::is_same_v<Slot, CompactSlot>) {
            if (slots_.size() >= 64) { rebuild_simd(capacity); return; }
        }
#endif
        reinsirk::detail::FastHashReleasedArray<Slot> next(capacity);
        // 値の構築前に領域を一定間隔で準備する。
         {
            if (capacity >= 256) {
                constexpr size_type stride = std::max(size_type{1}, size_type{4096} / sizeof(Slot));
                for (size_type i = 0; i < capacity; i += stride)
                    *reinterpret_cast<volatile unsigned char*>(std::addressof(next[i])) = 0;
            }
        }

        reinsirk::detail::FastHashReleasedArray<std::uint8_t> state(capacity, 0);
        const auto shift = static_cast<unsigned>(64 - std::countr_zero(capacity));
        for(size_type i=0;i<slots_.size();++i){
            if(state_[i]!=1)continue;
            size_type p=bucket(*slots_[i],shift);
            while(state[p])p=(p+1)&(capacity-1);
            if constexpr(std::is_move_constructible_v<value_type>)next[p].emplace(std::move(*slots_[i]));
            else next[p].emplace(*slots_[i]);
            state[p]=1;
        }
        slots_.swap(next);
        state_.swap(state);
        occupied_ = size_;
        shift_ = shift;
        invalidate();
        next.release_storage();state.release_storage();
    }
    size_type insertion_slot(const Key& key) {
        if (slots_.empty()) rebuild(8);
        for (;;) {
            size_type p = bucket(key, shift_);
            // 削除前は墓石の分岐と探索位置の記録を省く。
            if (occupied_ == size_) {
                while (state_[p]) {
                    if (equal_(*slots_[p], key)) return p;
                    p = (p + 1) & (slots_.size() - 1);
                }
                if (size_ != threshold(slots_.size())) return p;
                if (slots_.size() > max_capacity() / 2) throw std::length_error("HashSet: capacity overflow");
                rebuild(slots_.size() * 2);
                continue;
            }
            size_type first_deleted = slots_.size();
            while (state_[p]) {
                if constexpr (std::is_same_v<Slot, CompactSlot> && std::is_integral_v<Key> &&
                              (std::is_same_v<KeyEqual, std::equal_to<Key>> ||
                               std::is_same_v<KeyEqual, std::equal_to<void>>)) {
                    // 外部参照のない整数と標準比較だけ、同値キーの墓石で止める。
                    if (equal_(*slots_[p], key)) return p;
                } else if (state_[p] == 1 && equal_(*slots_[p], key)) return p;
                if (state_[p] == 2 && first_deleted == slots_.size()) first_deleted = p;
                p = (p + 1) & (slots_.size() - 1);
            }
            if (first_deleted != slots_.size()) return first_deleted;
            if (occupied_ != threshold(slots_.size())) return p;
            if (size_ == occupied_) {
                if (slots_.size() > max_capacity() / 2) throw std::length_error("HashSet: capacity overflow");
                rebuild(slots_.size() * 2);
            } else rebuild(slots_.size());
        }
    }
    void mark_inserted(size_type p) noexcept {
        if (state_[p] == 0) ++occupied_;
        state_[p] = 1;
        ++size_;
    }
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
    size_type next_live(size_type p) const noexcept {
        const auto capacity = state_.size();
        if (capacity - p >= 4) {
            if (state_[p] == 1) return p;
            if (state_[p+1] == 1) return p+1;
            if (state_[p+2] == 1) return p+2;
            if (state_[p+3] == 1) return p+3;
            p += 4;
        }
        const auto live = _mm_set1_epi8(1);
        while (capacity - p >= 16) {
            const auto states = _mm_loadu_si128(reinterpret_cast<const __m128i*>(state_.data()+p));
            const unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(states,live)));
            if (mask) return p + std::countr_zero(mask);
            p += 16;
        }
        while (p < capacity && state_[p] != 1) ++p;
        return p;
    }
    size_type previous_live(size_type p) const noexcept {
        if (p >= 4) {
            if (state_[p-1] == 1) return p-1;
            if (state_[p-2] == 1) return p-2;
            if (state_[p-3] == 1) return p-3;
            if (state_[p-4] == 1) return p-4;
            p -= 4;
        }
        const auto live = _mm_set1_epi8(1);
        while (p >= 16) {
            const auto states = _mm_loadu_si128(reinterpret_cast<const __m128i*>(state_.data()+p-16));
            const unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(states,live)));
            if (mask) return p - 16 + std::bit_width(mask) - 1;
            p -= 16;
        }
        while (p != 0) { --p; if (state_[p] == 1) return p; }
        return state_.size();
    }
#endif
public:
    class Iterator {
        friend class HashSet;
        const HashSet* owner_ = nullptr;
        size_type index_ = 0;
        Iterator(const HashSet* owner, size_type index) noexcept : owner_(owner), index_(index) {
        }
        void check(bool element) const {
            (void)element;
        }
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept = std::bidirectional_iterator_tag;
        using value_type = Key;
        using difference_type = std::ptrdiff_t;
        using reference = const Key&;
        using pointer = const Key*;
        Iterator() noexcept = default;
        reference operator*() const { check(true); return *owner_->slots_[index_]; }
        pointer operator->() const { return std::addressof(operator*()); }
        Iterator& operator++() {
            check(true);
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
            if constexpr (!std::is_same_v<Slot, CompactSlot>) {
                index_ = owner_->next_live(index_ + 1);
                return *this;
            }
#endif
            do { ++index_; } while (index_ < owner_->slots_.size() && owner_->state_[index_] != 1);
            return *this;
        }
        Iterator operator++(int) { auto copy = *this; ++*this; return copy; }
        Iterator& operator--() {
            check(false);
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
            if constexpr (!std::is_same_v<Slot, CompactSlot>) {
                const auto p = owner_->previous_live(index_);
                if (p != owner_->slots_.size()) { index_ = p; return *this; }
                index_ = 0;
                return *this;
            }
#endif
            auto p = index_;
            while (p != 0) { --p; if (owner_->state_[p] == 1) { index_ = p; return *this; } }
            index_ = 0;
            return *this;
        }
        Iterator operator--(int) { auto copy = *this; --*this; return copy; }
        bool operator==(const Iterator& other) const noexcept {
            return owner_ == other.owner_ && index_ == other.index_;
        }
    };
    using iterator = Iterator;
    using const_iterator = Iterator;
    using itr = iterator;

    HashSet() = default;
    explicit HashSet(size_type expected, Hasher hash = {}, KeyEqual equal = {})
        : hash_(std::move(hash)), equal_(std::move(equal)) { reserve(expected); }
    HashSet(std::initializer_list<Key> keys, Hasher hash = {}, KeyEqual equal = {})
        : HashSet(keys.size(), std::move(hash), std::move(equal)) { insert_range(keys.begin(), keys.end()); }
    template <std::input_iterator It, std::sentinel_for<It> Sentinel>
    HashSet(It first, Sentinel last, Hasher hash = {}, KeyEqual equal = {})
        : hash_(std::move(hash)), equal_(std::move(equal)) { insert_range(first, last); }
    HashSet(const HashSet& other) requires std::is_copy_constructible_v<value_type>
        : slots_(other.slots_.size()),state_(other.state_.size(),0),size_(other.size_),occupied_(other.occupied_),
          shift_(other.shift_),hash_(other.hash_),equal_(other.equal_) {
        if(!state_.empty())std::copy_n(other.state_.data(),state_.size(),state_.data());
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
        if(slots_.size()>=16) {
            for(size_type base=0;base<slots_.size();base+=16) {
                const auto states=_mm_loadu_si128(reinterpret_cast<const __m128i*>(state_.data()+base));
                unsigned mask;
                if constexpr(std::is_same_v<Slot,CompactSlot>)mask=static_cast<unsigned>(~_mm_movemask_epi8(_mm_cmpeq_epi8(states,_mm_setzero_si128())))&65535u;
                else mask=static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(states,_mm_set1_epi8(1))));
                while(mask) {const auto i=base+std::countr_zero(mask);slots_[i].emplace(*other.slots_[i]);mask&=mask-1;}
            }
            return;
        }
#endif
        for(size_type i=0;i<slots_.size();++i) {
            if constexpr(std::is_same_v<Slot,CompactSlot>) {
                if(state_[i])slots_[i].emplace(*other.slots_[i]);
            } else if(state_[i]==1)slots_[i].emplace(*other.slots_[i]);
        }
    }
    HashSet(HashSet&& other) noexcept(std::is_nothrow_copy_constructible_v<Hasher> &&
                                    std::is_nothrow_copy_constructible_v<KeyEqual> &&
                                    std::is_nothrow_swappable_v<Hasher> &&
                                    std::is_nothrow_swappable_v<KeyEqual>)
        : hash_(other.hash_), equal_(other.equal_) { swap(other); }
    HashSet& operator=(HashSet other) { swap(other); return *this; }
    void swap(HashSet& other) noexcept(std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>) {
        static_assert(std::is_nothrow_swappable_v<Hasher> && std::is_nothrow_swappable_v<KeyEqual>,
                      "HashSet: Hasher and KeyEqual must be nothrow swappable");
        using std::swap;
        swap(hash_, other.hash_); swap(equal_, other.equal_);
        slots_.swap(other.slots_); state_.swap(other.state_);
        swap(size_, other.size_); swap(occupied_, other.occupied_); swap(shift_, other.shift_);
        invalidate(); other.invalidate();
    }
    iterator begin() const noexcept {
        if (size_ == 0) return end();
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr (!std::is_same_v<Slot, CompactSlot>) return iterator(this,next_live(0));
#endif
        size_type p = 0; while (p < slots_.size() && state_[p] != 1) ++p;
        return iterator(this, p);
    }
    iterator end() const noexcept { return iterator(this, slots_.size()); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    friend iterator begin(const HashSet& set) noexcept { return set.begin(); }
    friend iterator end(const HashSet& set) noexcept { return set.end(); }
    iterator find(const Key& key) const { return iterator(this, locate(key)); }
    bool contains(const Key& key) const { return locate(key) != slots_.size(); }
    bool contain(const Key& key) const { return contains(key); }
    int count(const Key& key) const { return contains(key) ? 1 : 0; }
    size_type size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    size_type capacity() const noexcept { return slots_.size(); }
    double load_factor() const noexcept { return slots_.empty() ? 0.0 : static_cast<double>(size_) / slots_.size(); }
private:
    template <class K>
    std::pair<iterator, bool> insert_checked_impl(K&& key) {
        const auto p = insertion_slot(key);
        if (state_[p] == 1) return {iterator(this, p), false};
        slots_[p].emplace(std::forward<K>(key));
        mark_inserted(p);
        return {iterator(this, p), true};
    }
public:
    std::pair<iterator, bool> insert_checked(const Key& key) { return insert_checked_impl(key); }
    std::pair<iterator, bool> insert_checked(Key&& key) { return insert_checked_impl(std::move(key)); }
    iterator insert(const Key& key) { return insert_checked(key).first; }
    iterator insert(Key&& key) { return insert_checked(std::move(key)).first; }
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        Key key(std::forward<Args>(args)...);
        return insert_checked(std::move(key));
    }
    template <std::input_iterator It, std::sentinel_for<It> Sentinel>
    void insert_range(It first, Sentinel last) {
        for (; first != last; ++first) insert(*first);
    }
    iterator erase(const_iterator it, bool get_next = true) {
        if (it.index_ == slots_.size()) return end();
        it.check(true);
        const auto p = it.index_;
        slots_[p].reset(); state_[p] = 2; --size_;
        if (!get_next) return end();
#if (!defined(SIRKIT_DISABLE_SIMD)) && (defined(__SSE2__)) && ((defined(__x86_64__)) || (defined(__i386__)))
        if constexpr (!std::is_same_v<Slot, CompactSlot>) return iterator(this,next_live(p+1));
#endif
        auto next = iterator(this, p);
        do { ++next.index_; } while (next.index_ < slots_.size() && state_[next.index_] != 1);
        return next;
    }
    iterator erase(const Key& key, bool get_next = true) { return erase(find(key), get_next); }
    template <class Predicate>
    size_type erase_if(Predicate predicate) {
        if (size_ == 0) return 0;
        size_type removed = 0;
        for (size_type i = 0; i < slots_.size(); ++i) {
            if (state_[i] == 1 && std::invoke(predicate, std::as_const(*slots_[i]))) {
                slots_[i].reset(); state_[i] = 2; --size_; ++removed;
            }
        }
        return removed;
    }
    friend void swap(HashSet& left, HashSet& right) noexcept { left.swap(right); }
    friend bool operator==(const HashSet& left, const HashSet& right) {
        if (left.size_ != right.size_) return false;
        if (left.size_ == 0) return true;
        for (const auto& key : left) if (!right.contains(key)) return false;
        for (const auto& key : right) if (!left.contains(key)) return false;
        return true;
    }
    void clear() noexcept {
        if (occupied_ == 0) { invalidate(); return; }
        std::fill(state_.begin(), state_.end(), std::uint8_t{0});
        size_ = occupied_ = 0; invalidate();
    }
    void reserve(size_type count) {
        if (count <= threshold(slots_.size())) return;
        if (count > threshold(max_capacity())) throw std::length_error("HashSet::reserve: capacity overflow");
        size_type capacity = 8;
        while (count > threshold(capacity)) capacity *= 2;
        rebuild(capacity);
    }
    void rehash(size_type requested) {
        if (requested > max_capacity()) throw std::length_error("HashSet::rehash: capacity overflow");
        size_type capacity = 8;
        while (requested > capacity || size_ > threshold(capacity)) {
            if (capacity > max_capacity() / 2) throw std::length_error("HashSet::rehash: capacity overflow");
            capacity *= 2;
        }
        rebuild(capacity);
    }
};
#endif
