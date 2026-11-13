// SPDX-License-Identifier: CC0-1.0
// 参考（多パターン検索の用途・Verify。実装は独自構成）:
// https://judge.yosupo.jp/problem/aho_corasick
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/aho_corasick.md
//
// API一覧
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
    std::vector<reinsirk::detail::AhoNode<Alphabet>> nodes_;
    std::vector<reinsirk::detail::AhoPattern> patterns_;
    std::vector<int> order_;
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
        std::vector<int> order; order.reserve(nodes_.size()); order.push_back(0);
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
