// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Dynamic_Prefix_Filter.cpp
// 参考（禁止接頭辞の用途）: https://atcoder.jp/contests/abc403/editorial/12832
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_prefix_filter.md
//
// API一覧
// DynamicPrefixFilter(): 単語多重集合と禁止接頭辞集合を空で用意。O(1)。
// insert(word,copies=1): 追加後のwordの個数。erase(word,copies=1): 足りなければfalseで変更しない。
// count(word): wordの総個数。size(): 禁止にかかわらず登録した全個数。O(1)。
// block(prefix)/unblock(prefix): 禁止接頭辞に追加/解除。集合が変わればtrue。禁止中の単語も保持する。
// is_blocked(word): 禁止接頭辞を一つでも持つか。未登録の単語も照会可。
// allowed_count(): 禁止接頭辞を持たない登録単語の総個数。O(1)。
// 各文字列は任意のバイト列。空語も可、空接頭辞を禁止すると全ての単語が対象。
// 文字列操作は長さLに対しO(1+256L)、挿入時は確保を含め償却。領域は一度でも追加した接頭辞の種類数に比例。
// 総個数がSIZE_MAXを超える追加はlength_error。コピーは独立、ムーブ元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_PREFIX_FILTER_DEFINED
#define SIRKIT_DYNAMIC_PREFIX_FILTER_DEFINED

class DynamicPrefixFilter
{
    static constexpr auto kMissing = std::numeric_limits<std::size_t>::max();

    struct Node
    {
        std::size_t first_child = 0;
        std::size_t next_sibling = 0;
        std::size_t parent = 0;
        std::size_t words = 0;
        std::size_t allowed_children = 0;
        unsigned char byte = 0;
        bool blocked = false;

        [[nodiscard]] std::size_t allowed() const noexcept
        {
            return blocked ? 0 : words + allowed_children;
        }
    };

    std::vector<Node> nodes_;
    std::size_t size_ = 0;

    [[nodiscard]] std::size_t child(std::size_t at, unsigned char byte) const noexcept
    {
        for (auto next = nodes_[at].first_child; next != 0; next = nodes_[next].next_sibling)
        {
            if (nodes_[next].byte == byte)
            {
                return next;
            }
        }
        return 0;
    }

    [[nodiscard]] std::size_t find(std::string_view word) const noexcept
    {
        if (nodes_.empty())
        {
            return kMissing;
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            at = child(at, byte);
            if (at == 0)
            {
                return kMissing;
            }
        }
        return at;
    }

    std::size_t ensure(std::string_view word)
    {
        if (nodes_.empty())
        {
            nodes_.emplace_back();
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            auto next = child(at, byte);
            if (next == 0)
            {
                next = nodes_.size();
                nodes_.push_back(Node{.next_sibling = nodes_[at].first_child, .parent = at, .byte = byte});
                nodes_[at].first_child = next;
            }
            at = next;
        }
        return at;
    }

    // 禁止されたノードも直下の集計を保ち、解除時に子孫を走査せず復元する。
    void propagate(std::size_t at, std::size_t previous) noexcept
    {
        auto current = nodes_[at].allowed();
        while (at != 0 && current != previous)
        {
            at = nodes_[at].parent;
            auto &parent = nodes_[at];
            const auto parent_previous = parent.allowed();
            parent.allowed_children = parent.allowed_children - previous + current;
            previous = parent_previous;
            current = parent.allowed();
        }
    }

    bool set_blocked(std::size_t at, bool blocked) noexcept
    {
        auto &node = nodes_[at];
        if (node.blocked == blocked)
        {
            return false;
        }
        const auto previous = node.allowed();
        node.blocked = blocked;
        propagate(at, previous);
        return true;
    }

public:
    DynamicPrefixFilter() = default;
    DynamicPrefixFilter(const DynamicPrefixFilter &) = default;

    DynamicPrefixFilter(DynamicPrefixFilter &&other) noexcept
        : nodes_(std::move(other.nodes_)), size_(std::exchange(other.size_, 0))
    {
        other.nodes_.clear();
    }

    DynamicPrefixFilter &operator=(DynamicPrefixFilter other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(size_, other.size_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::size_t allowed_count() const noexcept
    {
        return nodes_.empty() ? 0 : nodes_[0].allowed();
    }

    [[nodiscard]] std::size_t count(std::string_view word) const noexcept
    {
        const auto at = find(word);
        return at == kMissing ? 0 : nodes_[at].words;
    }

    std::size_t insert(std::string_view word, std::size_t copies = 1)
    {
        if (copies == 0)
        {
            return count(word);
        }
        if (copies > std::numeric_limits<std::size_t>::max() - size_)
        {
            throw std::length_error("DynamicPrefixFilter: too many words");
        }
        const auto at = ensure(word);
        auto &node = nodes_[at];
        const auto previous = node.allowed();
        node.words += copies;
        size_ += copies;
        propagate(at, previous);
        return node.words;
    }

    bool erase(std::string_view word, std::size_t copies = 1) noexcept
    {
        if (copies == 0)
        {
            return true;
        }
        const auto at = find(word);
        if (at == kMissing || nodes_[at].words < copies)
        {
            return false;
        }
        auto &node = nodes_[at];
        const auto previous = node.allowed();
        node.words -= copies;
        size_ -= copies;
        propagate(at, previous);
        return true;
    }

    bool block(std::string_view prefix)
    {
        return set_blocked(ensure(prefix), true);
    }

    bool unblock(std::string_view prefix) noexcept
    {
        const auto at = find(prefix);
        return at != kMissing && set_blocked(at, false);
    }

    [[nodiscard]] bool is_blocked(std::string_view word) const noexcept
    {
        if (nodes_.empty())
        {
            return false;
        }
        std::size_t at = 0;
        for (const unsigned char byte : word)
        {
            if (nodes_[at].blocked)
            {
                return true;
            }
            at = child(at, byte);
            if (at == 0)
            {
                return false;
            }
        }
        return nodes_[at].blocked;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
