// SPDX-License-Identifier: CC0-1.0
// 参考（多パターン検索の用途・Verify。実装は独自構成）:
// https://judge.yosupo.jp/problem/aho_corasick
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/aho_corasick.md
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// 速度優先版: GCCでは最後の主要配列の破棄・解放を省き、成長時はvectorとして旧領域を回収する。
// Clangでは従来のFastBufferを使い、成長中の旧領域も保持する。最後の領域はプロセス終了まで累積する。
// 格納型の破棄の副作用を必要としないこと。
// 入力・容量・件数の前提と確保成功を利用側が保証する。例外時の状態復元は保証しない。
// AhoCorasick<Alphabet=26,First='a'>: 符号なしバイトFirst..First+Alphabet-1を扱う。Alphabetは1..256。
// ByteAhoCorasick: 全バイト版AhoCorasick<256,0>。各状態にAlphabet本の遷移を保持する。
// add(pattern): 構築前に語を追加し0始まりIDを返す。空語・同じ語の重複も別ID。O(語長+新状態数*Alphabet)。
// build(): 遷移・failure・出力リンクを確定。O(状態数*Alphabet)時間・O(状態数)作業領域、再呼出しはO(1)。
// build後はaddしない。clear()で空の未構築状態へ戻せる。コピーは独立、move元は空の未構築状態。
// step(state,c): 1文字進んだ状態。failure(state)/output_count(state): failure先/その状態の一致語数。各O(1)。
// parent(state): 元Trieの親を返す（rootの親は0）。build前でもO(1)で参照可。
// count_total(text): 重なり・重複込みの総一致数。空語は全境界に一致。O(text長)、uint64_t上限超過は例外。
// match_counts(text): 各IDの一致数。O(text長+状態数+語数)時間・O(状態数+語数)領域。
// for_each_match(text,callback): callback(id,begin,end)へ半開区間を通知。O(text長+一致数)。callbackから構造を変更しないこと。
// find_all(text): AhoMatch{pattern,begin,end}のvectorを返す。終了位置の昇順、同位置内のID順は保証しない。
// size()/empty()/state_count(): 語数/語が空か/rootを含む状態数。pattern_length(id)/pattern_state(id): 語長/終端状態。
// reserve_nodes(n)/reserve_patterns(n): 確保枠を予約。node_capacity(): 状態の確保枠。swap(other): O(1)で交換。
// 検索・step・failure・output_countにはbuild済みが必要。text/語は指定アルファベット内。検査は通常処理へ統合。
// 各ID・状態はint。枠の上限は検査省略時も確認。addの失敗時は未登録の接頭ノードが残る場合がある。

#if defined(__GNUC__) && !defined(__clang__)
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_UNRELEASED_VECTOR_DEFINED
#define SIRKIT_UNRELEASED_VECTOR_DEFINED
namespace reinsirk::detail {
template<class T>
class UnreleasedVector {
    union {std::vector<T> values_;};
public:
    using value_type=T;
    using size_type=typename std::vector<T>::size_type;
    using difference_type=typename std::vector<T>::difference_type;
    using reference=typename std::vector<T>::reference;
    using const_reference=typename std::vector<T>::const_reference;
    using pointer=typename std::vector<T>::pointer;
    using const_pointer=typename std::vector<T>::const_pointer;
    using iterator=typename std::vector<T>::iterator;
    using const_iterator=typename std::vector<T>::const_iterator;
    using reverse_iterator=typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator=typename std::vector<T>::const_reverse_iterator;
    UnreleasedVector()noexcept:values_{}{}
    explicit UnreleasedVector(size_type n):values_(n){}
    UnreleasedVector(size_type n,const T&value):values_(n,value){}
    template<std::input_iterator It> UnreleasedVector(It first,It last):values_(first,last){}
    UnreleasedVector(std::initializer_list<T> values):values_(values){}
    UnreleasedVector(const std::vector<T>&values)requires std::is_copy_constructible_v<T>:values_(values){}
    UnreleasedVector(std::vector<T>&&values)noexcept:values_(std::move(values)){values.clear();}
    UnreleasedVector(const UnreleasedVector&other)requires std::is_copy_constructible_v<T>:values_(other.values_){}
    UnreleasedVector&operator=(const UnreleasedVector&other)requires(std::is_copy_constructible_v<T>&&std::is_copy_assignable_v<T>){if(this!=&other)values_=other.values_;return *this;}
    UnreleasedVector(UnreleasedVector&&other)noexcept:values_(std::move(other.values_)){other.values_.clear();}
    UnreleasedVector&operator=(UnreleasedVector&&other)noexcept{if(this!=&other){values_=std::move(other.values_);other.values_.clear();}return *this;}
    ~UnreleasedVector()noexcept{} // 意図的にactiveなvectorを破棄しない。
    UnreleasedVector&operator=(std::initializer_list<T> values){values_=values;return *this;}
    iterator begin()noexcept{return values_.begin();}
    const_iterator begin()const noexcept{return values_.begin();}
    const_iterator cbegin()const noexcept{return values_.cbegin();}
    iterator end()noexcept{return values_.end();}
    const_iterator end()const noexcept{return values_.end();}
    const_iterator cend()const noexcept{return values_.cend();}
    reverse_iterator rbegin()noexcept{return values_.rbegin();}
    const_reverse_iterator rbegin()const noexcept{return values_.rbegin();}
    const_reverse_iterator crbegin()const noexcept{return values_.crbegin();}
    reverse_iterator rend()noexcept{return values_.rend();}
    const_reverse_iterator rend()const noexcept{return values_.rend();}
    const_reverse_iterator crend()const noexcept{return values_.crend();}
    bool empty()const noexcept{return values_.empty();}
    size_type size()const noexcept{return values_.size();}
    size_type capacity()const noexcept{return values_.capacity();}
    size_type max_size()const noexcept{return values_.max_size();}
    T*data()noexcept requires(!std::is_same_v<T,bool>){return values_.data();}
    const T*data()const noexcept requires(!std::is_same_v<T,bool>){return values_.data();}
    reference operator[](size_type i)noexcept{return values_[i];}
    const_reference operator[](size_type i)const noexcept{return values_[i];}
    reference at(size_type i){return values_.at(i);}
    const_reference at(size_type i)const{return values_.at(i);}
    reference front()noexcept{return values_.front();}
    const_reference front()const noexcept{return values_.front();}
    reference back()noexcept{return values_.back();}
    const_reference back()const noexcept{return values_.back();}
    void reserve(size_type n){values_.reserve(n);}
    void shrink_to_fit(){values_.shrink_to_fit();}
    void clear()noexcept{values_.clear();}
    void resize(size_type n){values_.resize(n);}
    void resize(size_type n,const T&value){values_.resize(n,value);}
    template<class...Args>reference emplace_back(Args&&...args){return values_.emplace_back(std::forward<Args>(args)...);}
    void push_back(const T&value){values_.push_back(value);}
    void push_back(T&&value){values_.push_back(std::move(value));}
    void pop_back()noexcept{values_.pop_back();}
    void assign(size_type n,const T&value){values_.assign(n,value);}
    template<std::input_iterator It>void assign(It first,It last){values_.assign(first,last);}
    void assign(std::initializer_list<T> values){values_.assign(values);}
    iterator insert(const_iterator position,const T&value){return values_.insert(position,value);}
    iterator insert(const_iterator position,T&&value){return values_.insert(position,std::move(value));}
    iterator insert(const_iterator position,size_type n,const T&value){return values_.insert(position,n,value);}
    template<std::input_iterator It>iterator insert(const_iterator position,It first,It last){return values_.insert(position,first,last);}
    iterator insert(const_iterator position,std::initializer_list<T> values){return values_.insert(position,values);}
    template<class...Args>iterator emplace(const_iterator position,Args&&...args){return values_.emplace(position,std::forward<Args>(args)...);}
    iterator erase(const_iterator position){return values_.erase(position);}
    iterator erase(const_iterator first,const_iterator last){return values_.erase(first,last);}
    void swap(UnreleasedVector&other)noexcept{values_.swap(other.values_);}
    friend void swap(UnreleasedVector&a,UnreleasedVector&b)noexcept{a.swap(b);}
};
}
#endif

#else
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_FAST_BUFFER_DEFINED
#define SIRKIT_FAST_BUFFER_DEFINED
namespace reinsirk::detail {
template <class T>
class FastBuffer {
    T* data_ = nullptr;
    std::size_t size_ = 0, capacity_ = 0;
    void grow() {
        const auto limit = max_size();
        reserve(capacity_ == 0 ? 1 : capacity_ > limit - capacity_ ? limit : capacity_ * 2);
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reference = T&;
    using const_reference = const T&;
    FastBuffer() noexcept = default;
    explicit FastBuffer(std::size_t count) {
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_);
    }
    FastBuffer(std::size_t count, const T& value) {
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_, value);
    }
    template <std::input_iterator Iterator>
    FastBuffer(Iterator first, Iterator last) {
        if (first == last) return;
        if constexpr (std::forward_iterator<Iterator>) reserve(static_cast<std::size_t>(std::distance(first, last)));
        for (; first != last; ++first) emplace_back(*first);
    }
    FastBuffer(const std::vector<T>& values) : FastBuffer(values.begin(), values.end()) {}
    FastBuffer(const FastBuffer& other) : FastBuffer(other.begin(), other.end()) {}
    FastBuffer& operator=(const FastBuffer& other) {
        if (this != &other) { FastBuffer next(other); swap(next); }
        return *this;
    }
    FastBuffer(FastBuffer&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}
    FastBuffer& operator=(FastBuffer&& other) noexcept {
        if (this != &other) {
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0); capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }
    // デストラクタは、格納値の破棄もdeallocateも行わない。
    ~FastBuffer() = default;
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }
    static constexpr std::size_t max_size() noexcept {
        return static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(T);
    }
    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }
    T* begin() noexcept { return data_; }
    const T* begin() const noexcept { return data_; }
    T* end() noexcept { return size_ == 0 ? data_ : data_ + size_; }
    const T* end() const noexcept { return size_ == 0 ? data_ : data_ + size_; }
    const T* cbegin() const noexcept { return begin(); }
    const T* cend() const noexcept { return end(); }
    T& operator[](std::size_t index) noexcept { return data_[index]; }
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }
    T& front() noexcept { return data_[0]; }
    const T& front() const noexcept { return data_[0]; }
    T& back() noexcept { return data_[size_ - 1]; }
    const T& back() const noexcept { return data_[size_ - 1]; }
    void reserve(std::size_t count) {
        if (count <= capacity_) return;
        // allocator::allocateはT配列の領域を開始する。要素は使用時だけ構築する。
        T* next = std::allocator<T>{}.allocate(count);
        for (std::size_t i = 0; i < size_; ++i)
            std::construct_at(next + i, std::move_if_noexcept(data_[i]));
        data_ = next; capacity_ = count;
    }
    template <class... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            // 引数が自身の要素を参照していても、移動する前に新要素を作る。
            T value(std::forward<Args>(args)...);
            grow();
            T* added = std::construct_at(data_ + size_, std::move_if_noexcept(value));
            ++size_;
            return *added;
        }
        T* added = std::construct_at(data_ + size_, std::forward<Args>(args)...);
        ++size_;
        return *added;
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() noexcept { --size_; }
    void clear() noexcept { size_ = 0; }
    void resize(std::size_t count) {
        if (count < size_) { size_ = count; return; }
        reserve(count);
        for (; size_ < count; ++size_) std::construct_at(data_ + size_);
    }
    void resize(std::size_t count, const T& value) {
        if (count < size_) { size_ = count; return; }
        if (count > capacity_) { T copy(value); reserve(count); resize(count, copy); return; }
        for (; size_ < count; ++size_) std::construct_at(data_ + size_, value);
    }
    void assign(std::size_t count, const T& value) { FastBuffer next(count, value); swap(next); }
    template <std::input_iterator Iterator>
    void assign(Iterator first, Iterator last) { FastBuffer next(first, last); swap(next); }
    void swap(FastBuffer& other) noexcept {
        std::swap(data_, other.data_); std::swap(size_, other.size_); std::swap(capacity_, other.capacity_);
    }
    friend void swap(FastBuffer& a, FastBuffer& b) noexcept { a.swap(b); }
};
}
#endif

#endif
#ifndef SIRKIT_AHO_FAST_STORAGE_DEFINED
#define SIRKIT_AHO_FAST_STORAGE_DEFINED
namespace reinsirk::detail {
#if defined(__GNUC__) && !defined(__clang__)
template<class T>using AhoStorage=UnreleasedVector<T>;
#else
template<class T>using AhoStorage=FastBuffer<T>;
#endif
}
#endif
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_AHO_CORASICK_DETAIL_DEFINED
#define SIRKIT_AHO_CORASICK_DETAIL_DEFINED
namespace reinsirk::detail {
template <std::size_t Alphabet>
struct AhoNode {
    std::array<int, Alphabet> next;
    int failure = 0, dictionary = -1, head = -1, parent = 0;
    std::uint64_t terminal = 0, outputs = 0;
    AhoNode() noexcept { next.fill(-1); }
};
struct AhoPattern {
    int state;
    std::size_t length;
    int next;
};
}
#endif

#ifndef SIRKIT_AHO_MATCH_DEFINED
#define SIRKIT_AHO_MATCH_DEFINED
struct AhoMatch {
    int pattern;
    std::size_t begin, end;
    friend bool operator==(const AhoMatch&, const AhoMatch&) = default;
};
#endif

#ifndef SIRKIT_AHO_CORASICK_DEFINED
#define SIRKIT_AHO_CORASICK_DEFINED
template <std::size_t Alphabet = 26, unsigned First = 'a'>
class AhoCorasick {
    static_assert(Alphabet > 0 && Alphabet <= 256 && First <= 256 - Alphabet,
                  "AhoCorasick alphabet must fit unsigned bytes");
    reinsirk::detail::AhoStorage<reinsirk::detail::AhoNode<Alphabet>> nodes_;
    reinsirk::detail::AhoStorage<reinsirk::detail::AhoPattern> patterns_;
    reinsirk::detail::AhoStorage<int> order_;
    bool built_ = false;
    static std::size_t letter(unsigned char c) {
#ifndef SKIP_RUNTIME_CHECKS
        if constexpr (First != 0 || Alphabet != 256) {
            if (c < First || static_cast<unsigned>(c) - First >= Alphabet)
                throw std::out_of_range("AhoCorasick: character outside alphabet");
        }
#endif
        return static_cast<unsigned>(c) - First;
    }
    void require_built() const {
#ifndef SKIP_RUNTIME_CHECKS
        if (!built_) throw std::logic_error("AhoCorasick: build before searching");
#endif
    }
    void check_state(int state) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (state < 0 || static_cast<std::size_t>(state) >= nodes_.size())
            throw std::out_of_range("AhoCorasick: state");
#else
        (void)state;
#endif
    }
    void check_pattern(int pattern) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (pattern < 0 || static_cast<std::size_t>(pattern) >= patterns_.size())
            throw std::out_of_range("AhoCorasick: pattern");
#else
        (void)pattern;
#endif
    }
public:
    AhoCorasick() = default;
    AhoCorasick(const AhoCorasick&) = default;
    AhoCorasick(AhoCorasick&& other) noexcept
        : nodes_(std::move(other.nodes_)), patterns_(std::move(other.patterns_)),
          order_(std::move(other.order_)), built_(std::exchange(other.built_, false)) {
        other.nodes_.clear(); other.patterns_.clear(); other.order_.clear();
    }
    AhoCorasick& operator=(AhoCorasick other) noexcept { swap(other); return *this; }
    void swap(AhoCorasick& other) noexcept {
        nodes_.swap(other.nodes_); patterns_.swap(other.patterns_); order_.swap(other.order_);
        std::swap(built_, other.built_);
    }
    void clear() noexcept {
        nodes_.clear(); patterns_.clear(); order_.clear(); built_ = false;
    }
    [[nodiscard]] std::size_t size() const noexcept { return patterns_.size(); }
    [[nodiscard]] bool empty() const noexcept { return patterns_.empty(); }
    [[nodiscard]] int state_count() const noexcept { return nodes_.empty() ? 1 : static_cast<int>(nodes_.size()); }
    [[nodiscard]] std::size_t node_capacity() const noexcept { return nodes_.capacity(); }
    void reserve_nodes(std::size_t n) {
        if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("AhoCorasick: state capacity");
        nodes_.reserve(n);
    }
    void reserve_patterns(std::size_t n) {
        if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("AhoCorasick: pattern capacity");
        patterns_.reserve(n);
    }
    [[nodiscard]] std::size_t pattern_length(int id) const { check_pattern(id); return patterns_[id].length; }
    [[nodiscard]] int pattern_state(int id) const { check_pattern(id); return patterns_[id].state; }
    [[nodiscard]] int parent(int state) const {
        if (state == 0 && nodes_.empty()) return 0;
        check_state(state); return nodes_[state].parent;
    }
    int add(std::string_view pattern) {
#ifndef SKIP_RUNTIME_CHECKS
        if (built_) throw std::logic_error("AhoCorasick: add after build");
#endif
        if (patterns_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("AhoCorasick: too many patterns");
        if (nodes_.empty()) nodes_.emplace_back();
        int state = 0;
        for (unsigned char c : pattern) {
            const auto symbol = letter(c);
            int next = nodes_[state].next[symbol];
            if (next == -1) {
                if (nodes_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
                    throw std::length_error("AhoCorasick: too many states");
                next = static_cast<int>(nodes_.size());
                nodes_.emplace_back();
                nodes_[next].parent = state;
                nodes_[state].next[symbol] = next;
            }
            state = next;
        }
        const int id = static_cast<int>(patterns_.size());
        patterns_.push_back({state, pattern.size(), nodes_[state].head});
        nodes_[state].head = id; ++nodes_[state].terminal;
        return id;
    }
    void build() {
        if (built_) return;
        if (nodes_.empty()) nodes_.emplace_back();
        reinsirk::detail::AhoStorage<int> order; order.reserve(nodes_.size()); order.push_back(0);
        nodes_[0].outputs = nodes_[0].terminal;
        for (std::size_t symbol = 0; symbol < Alphabet; ++symbol) {
            auto& next = nodes_[0].next[symbol];
            if (next == -1) next = 0;
            else {
                nodes_[next].failure = 0;
                nodes_[next].dictionary = nodes_[0].head != -1 ? 0 : -1;
                nodes_[next].outputs = nodes_[next].terminal + nodes_[0].outputs;
                order.push_back(next);
            }
        }
        for (std::size_t at = 1; at < order.size(); ++at) {
            const int state = order[at], fail = nodes_[state].failure;
            for (std::size_t symbol = 0; symbol < Alphabet; ++symbol) {
                auto& next = nodes_[state].next[symbol];
                if (next == -1) next = nodes_[fail].next[symbol];
                else {
                    const int ancestor = nodes_[fail].next[symbol];
                    nodes_[next].failure = ancestor;
                    nodes_[next].dictionary = nodes_[ancestor].head != -1 ? ancestor : nodes_[ancestor].dictionary;
                    nodes_[next].outputs = nodes_[next].terminal + nodes_[ancestor].outputs;
                    order.push_back(next);
                }
            }
        }
        order_.swap(order); built_ = true;
    }
    [[nodiscard]] int step(int state, unsigned char c) const {
        require_built(); check_state(state); return nodes_[state].next[letter(c)];
    }
    [[nodiscard]] int failure(int state) const { require_built(); check_state(state); return nodes_[state].failure; }
    [[nodiscard]] std::uint64_t output_count(int state) const { require_built(); check_state(state); return nodes_[state].outputs; }
    std::uint64_t count_total(std::string_view text) const {
        require_built();
        int state = 0; auto total = nodes_[0].terminal;
        for (unsigned char c : text) {
            state = nodes_[state].next[letter(c)];
            if (total > std::numeric_limits<std::uint64_t>::max() - nodes_[state].outputs)
                throw std::overflow_error("AhoCorasick: match count overflow");
            total += nodes_[state].outputs;
        }
        return total;
    }
    std::vector<std::uint64_t> match_counts(std::string_view text) const {
        require_built();
        if (text.size() >= std::numeric_limits<std::uint64_t>::max())
            throw std::length_error("AhoCorasick: text too long");
        std::vector<std::uint64_t> visits(nodes_.size()); visits[0] = 1;
        int state = 0;
        for (unsigned char c : text) { state = nodes_[state].next[letter(c)]; ++visits[state]; }
        for (std::size_t at = order_.size(); at > 1; --at) {
            const int current = order_[at - 1]; visits[nodes_[current].failure] += visits[current];
        }
        std::vector<std::uint64_t> result(patterns_.size());
        for (std::size_t i = 0; i < patterns_.size(); ++i) result[i] = visits[patterns_[i].state];
        return result;
    }
    template <class Callback>
    void for_each_match(std::string_view text, Callback&& callback) const {
        require_built();
        const auto emit = [&](int state, std::size_t end) {
            for (int current = state; current != -1; current = nodes_[current].dictionary)
                for (int id = nodes_[current].head; id != -1; id = patterns_[id].next)
                    callback(id, end - patterns_[id].length, end);
        };
        int state = 0; emit(0, 0);
        for (std::size_t i = 0; i < text.size(); ++i) {
            state = nodes_[state].next[letter(static_cast<unsigned char>(text[i]))]; emit(state, i + 1);
        }
    }
    std::vector<AhoMatch> find_all(std::string_view text) const {
        std::vector<AhoMatch> matches;
        for_each_match(text, [&](int id, std::size_t begin, std::size_t end) { matches.push_back({id, begin, end}); });
        return matches;
    }
};
#endif

#ifndef SIRKIT_BYTE_AHO_CORASICK_DEFINED
#define SIRKIT_BYTE_AHO_CORASICK_DEFINED
using ByteAhoCorasick = AhoCorasick<256, 0>;
#endif
