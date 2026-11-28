#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc049/tasks/arc065_a

// Library: String/Dictionary_Segmenter.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Dictionary_Segmenter.cpp
// 参考（辞書語への分割）: https://atcoder.jp/contests/abc049/tasks/arc065_a
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dictionary_segmenter.md
//
// API一覧
// DictionarySegmenter<Alphabet=26,First='a'>(words): 辞書を構築する。空語は無視し、添字は元の語順のまま。
// ByteDictionarySegmenter: 全バイト用。
// minimum_segmentation(text) const: 最少の語数で分割し、0始まりの辞書語IDを先頭から返す。不可能ならnullopt。
// 空textは空の分割。辞書は内部で保持する。コピーは同じ読み取り専用辞書を共有、move元は空辞書相当。
// 辞書の語数k・総文字数L・状態数Sとして構築O(1+k+L+S*Alphabet)時間、O(1+k+S*Alphabet)領域。
// 長さn・一致数RのtextにO(1+n+R)時間、O(n+1)領域。
//
// [同梱のAho–Corasick]
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
// find_all(text): AhoMatch{pattern,begin,end}のvectorを返す。一致数RとしてO(text長+R)時間・O(R)返却領域。
// 終了位置の昇順、同位置内のID順は保証しない。
// size()/empty()/state_count(): 語数/語が空か/rootを含む状態数。pattern_length(id)/pattern_state(id): 語長/終端状態。
// reserve_nodes(n)/reserve_patterns(n): 確保枠を予約。node_capacity(): 状態の確保枠。swap(other): O(1)で交換。
// 検索・step・failure・output_countにはbuild済みが必要。text/語は指定アルファベット内。検査は通常処理へ統合。
// 各ID・状態はint。枠の上限は検査省略時も確認。addの失敗時は未登録の接頭ノードが残る場合がある。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#include <concepts>
#include <memory>
#include <optional>
#include <ranges>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_AHO_CORASICK_DETAIL_DEFINED
#define SIRKIT_AHO_CORASICK_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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
inline void CheckAhoIndexLimit(std::size_t size, const char* message) {
    if (size >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error(message);
}
inline std::uint64_t AddAhoMatchCount(std::uint64_t total, std::uint64_t additional) {
    if (total > std::numeric_limits<std::uint64_t>::max() - additional)
        throw std::overflow_error("AhoCorasick: match count overflow");
    return total + additional;
}
inline void CheckAhoCountTextSize(std::size_t size) {
    if (size >= std::numeric_limits<std::uint64_t>::max())
        throw std::length_error("AhoCorasick: text too long");
}
}
#endif

#ifndef SIRKIT_AHO_MATCH_DEFINED
#define SIRKIT_AHO_MATCH_DEFINED
struct AhoMatch {
    int pattern;
    std::size_t begin, end;
    friend constexpr bool operator==(const AhoMatch& a, const AhoMatch& b) noexcept {
        return a.pattern == b.pattern && a.begin == b.begin && a.end == b.end;
    }
};
#endif

#ifndef SIRKIT_AHO_CORASICK_DEFINED
#define SIRKIT_AHO_CORASICK_DEFINED
template <std::size_t Alphabet = 26, unsigned First = 'a'>
class AhoCorasick {
    static_assert(Alphabet > 0 && Alphabet <= 256 && First <= 256 - Alphabet,
                  "AhoCorasick alphabet must fit unsigned bytes");
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AhoNode<Alphabet>> nodes_;
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AhoPattern> patterns_;
    std::vector<int> order_;
    bool built_ = false;
    static std::size_t letter(unsigned char c) {
        return static_cast<unsigned>(c) - First;
    }
    void require_built() const {
    }
    void check_state(int state) const {
        (void)state;
    }
    void check_pattern(int pattern) const {
        (void)pattern;
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
        detail::CheckAhoIndexLimit(patterns_.size(), "AhoCorasick: too many patterns");
        if (nodes_.empty()) nodes_.emplace_back();
        int state = 0;
        for (unsigned char c : pattern) {
            const auto symbol = letter(c);
            int next = nodes_[state].next[symbol];
            if (next == -1) {
                detail::CheckAhoIndexLimit(nodes_.size(), "AhoCorasick: too many states");
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
            total = detail::AddAhoMatchCount(total, nodes_[state].outputs);
        }
        return total;
    }
    std::vector<std::uint64_t> match_counts(std::string_view text) const {
        require_built();
        detail::CheckAhoCountTextSize(text.size());
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

#ifndef SIRKIT_DICTIONARY_SEGMENTER_DEFINED
#define SIRKIT_DICTIONARY_SEGMENTER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t DictionarySegmentationBoundaryCount(std::size_t length, std::size_t limit)
{
    if (length >= limit)
    {
        throw std::length_error("DictionarySegmenter: text too long");
    }
    return length + 1;
}

template <typename Words>
concept DictionaryWordRange = std::ranges::input_range<const Words> &&
                              std::convertible_to<std::ranges::range_reference_t<const Words>, std::string_view>;
} // namespace reinsirk::detail

template <std::size_t Alphabet = 26, unsigned First = 'a'>
class DictionarySegmenter
{
    struct Data
    {
        AhoCorasick<Alphabet, First> matcher;
        std::vector<std::size_t> word_ids;

        template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::DictionaryWordRange Words>
        explicit Data(const Words &words)
        {
            std::size_t id = 0;
            for (const std::string_view word : words)
            {
                if (!word.empty())
                {
                    matcher.add(word);
                    word_ids.push_back(id);
                }
                ++id;
            }
            matcher.build();
        }
    };

    struct Choice
    {
        std::size_t count = std::numeric_limits<std::size_t>::max();
        int last_word = -1;
    };

    std::shared_ptr<const Data> data_;

public:
    template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::DictionaryWordRange Words>
    explicit DictionarySegmenter(const Words &words) : data_(std::make_shared<const Data>(words))
    {
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> minimum_segmentation(std::string_view text) const
    {
        if (text.empty())
        {
            return std::vector<std::size_t>{};
        }
        if (!data_)
        {
            return std::nullopt;
        }
        std::vector<Choice> best;
        best.resize(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::DictionarySegmentationBoundaryCount(text.size(), best.max_size()));
        best[0].count = 0;

        // 一致は終端の昇順。空語を除いたため、更新には確定済みの短い接頭辞だけを使う。
        data_->matcher.for_each_match(text, [&](int word, std::size_t begin, std::size_t end)
        {
            constexpr auto kUnreachable = std::numeric_limits<std::size_t>::max();
            if (best[begin].count == kUnreachable)
            {
                return;
            }
            const auto candidate = best[begin].count + 1;
            auto &choice = best[end];
            if (candidate < choice.count || (candidate == choice.count && word < choice.last_word))
            {
                choice = {candidate, word};
            }
        });
        if (best.back().last_word < 0)
        {
            return std::nullopt;
        }

        std::vector<std::size_t> result;
        result.reserve(best.back().count);
        for (std::size_t end = text.size(); end != 0;)
        {
            const int word = best[end].last_word;
            result.push_back(data_->word_ids[static_cast<std::size_t>(word)]);
            end -= data_->matcher.pattern_length(word);
        }
        std::ranges::reverse(result);
        return result;
    }
};

using ByteDictionarySegmenter = DictionarySegmenter<256, 0>;

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


#include <array>
#include <string>
#include <string_view>

int main()
{
    std::string text;
    verify_in >> text;
    constexpr std::array<std::string_view, 4> words{"dream", "dreamer", "erase", "eraser"};
    const sirkit::DictionarySegmenter<> segmenter(words);
    verify_out << (segmenter.minimum_segmentation(text) ? "YES" : "NO") << '\n';
}
