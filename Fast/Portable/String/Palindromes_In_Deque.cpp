// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Palindromes_In_Deque.cpp
// 参考: Q. Wang, M. Yang, X. Zhu, Double-Ended Palindromic Trees in Linear Time
// (https://arxiv.org/abs/2210.02292) の surface recording。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromes_in_deque.md
//
// API一覧
// PalindromesInDeque(expected_pushes=0): 空の英小文字列を構築。追加回数の見込みを渡すと回文頂点を予約。
// push_front(c)/push_back(c): 英小文字を指定の端へ追加。償却O(1)。
// pop_front()/pop_back(): 指定の端から1文字を除く。O(1)。
// size()/empty(): 現在の長さ/空かどうか。O(1)。
// distinct_palindrome_count(): 空でない相異なる回文部分文字列数。O(1)。
// longest_prefix_length()/longest_suffix_length(): 最長回文接頭辞/接尾辞の長さ。空なら0。O(1)。
// 通常版では範囲外の文字をinvalid_argument、空からの削除をout_of_rangeで通知。
// 過去の回文頂点を再利用する。予約なしで追加回数Uに対しO(26U+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMES_IN_DEQUE_DEFINED
#define SIRKIT_PALINDROMES_IN_DEQUE_DEFINED

class PalindromesInDeque
{
    using Index = std::size_t;
    static constexpr Index kAlphabet = 26;
    static constexpr Index kOdd = 0;
    static constexpr Index kEmpty = 1;

    struct Node
    {
        // quick[c]は、この回文内で直前の文字がcとなる最長の真の回文接尾辞。
        std::array<Index, kAlphabet> quick{};
        Index length = 0;
        Index suffix_link = kEmpty;
        Index terminal_count = 0; // 各接頭辞の最長回文接尾辞として現れる回数。
        Index first_child = kOdd;
        Index next_sibling = kOdd;
        Index letter = 0;
    };

    struct Cell
    {
        Index letter;
        Index prefix = kEmpty;
        Index suffix = kEmpty;
    };

public:
    explicit PalindromesInDeque(std::size_t expected_pushes = 0) : nodes_(2)
    {
        if (expected_pushes > nodes_.max_size() - 2)
        {
            throw std::length_error("PalindromesInDeque: node reservation too large");
        }
        nodes_.reserve(expected_pushes + 2);
    }

    PalindromesInDeque(const PalindromesInDeque &) = default;

    PalindromesInDeque(PalindromesInDeque &&other) : PalindromesInDeque()
    {
        Swap(other);
    }

    PalindromesInDeque &operator=(PalindromesInDeque other)
    {
        Swap(other);
        return *this;
    }

    std::size_t size() const noexcept
    {
        return text_.size();
    }

    bool empty() const noexcept
    {
        return text_.empty();
    }

    std::size_t distinct_palindrome_count() const noexcept
    {
        return distinct_;
    }

    std::size_t longest_prefix_length() const noexcept
    {
        return empty() ? 0 : nodes_[text_.front().prefix].length;
    }

    std::size_t longest_suffix_length() const noexcept
    {
        return empty() ? 0 : nodes_[text_.back().suffix].length;
    }

    void push_front(char character)
    {
        Push(character, true);
    }

    void push_back(char character)
    {
        Push(character, false);
    }

    void pop_front()
    {
        Pop(true);
    }

    void pop_back()
    {
        Pop(false);
    }

private:
    std::vector<Node> nodes_;
    std::deque<Cell> text_;
    Index distinct_ = 0;

    void Swap(PalindromesInDeque &other) noexcept
    {
        nodes_.swap(other.nodes_);
        text_.swap(other.text_);
        std::swap(distinct_, other.distinct_);
    }

    Cell &At(Index offset, bool front)
    {
        return text_[front ? offset : size() - 1 - offset];
    }

    // 更新する端を先頭と見なし、その向きで回文区間の開始と終了を読む。
    Index &Starts(Index offset, bool front)
    {
        auto &cell = At(offset, front);
        return front ? cell.prefix : cell.suffix;
    }

    Index &Ends(Index offset, bool front)
    {
        auto &cell = At(offset, front);
        return front ? cell.suffix : cell.prefix;
    }

    Index Transition(Index parent, Index letter) const
    {
        auto child = nodes_[parent].first_child;
        while (child != kOdd && nodes_[child].letter != letter)
        {
            child = nodes_[child].next_sibling;
        }
        return child;
    }

    Index FindOrCreate(Index inner, Index letter, bool front)
    {
        const auto child = Transition(inner, letter);
        if (child != kOdd)
        {
            return child;
        }

        Node node;
        node.length = inner == kOdd ? 1 : nodes_[inner].length + 2;
        if (inner != kOdd)
        {
            node.suffix_link = Transition(nodes_[inner].quick[letter], letter);
        }
        const Node &suffix = nodes_[node.suffix_link];
        node.quick = suffix.quick;
        const auto following = suffix.length == 0 ? letter : At(suffix.length - 1, front).letter;
        node.quick[following] = node.suffix_link;
        node.letter = letter;
        node.next_sibling = nodes_[inner].first_child;

        const auto id = nodes_.size();
        nodes_.push_back(node);
        nodes_[inner].first_child = id;
        return id;
    }

    void Push(char character, bool front)
    {
        const auto letter = static_cast<Index>(character - 'a');
        auto inner = empty() ? kEmpty : Starts(0, front);
        const auto length = nodes_[inner].length;
        if (length == size() || At(length, front).letter != letter)
        {
            inner = nodes_[inner].quick[letter];
        }
        const auto id = FindOrCreate(inner, letter, front);
        if (front)
        {
            text_.push_front({letter});
        }
        else
        {
            text_.push_back({letter});
        }

        auto &node = nodes_[id];
        const auto &suffix = nodes_[node.suffix_link];
        const auto suffix_start = node.length - suffix.length;
        // 以前の境界回文が新しい回文の接尾辞になった場合、その開始位置の記録を消す。
        if (suffix.length != 0 && Starts(suffix_start, front) == node.suffix_link)
        {
            Starts(suffix_start, front) = kEmpty;
        }
        Starts(0, front) = id;
        Ends(node.length - 1, front) = id;
        // 最初の出現はその位置で最長の回文接尾辞になる。より長い回文があれば、
        // その接頭辞として同じ回文が先に現れてしまうため。
        if (node.terminal_count++ == 0)
        {
            ++distinct_;
        }
    }

    void Pop(bool front)
    {
        const auto id = Starts(0, front);
        auto &node = nodes_[id];
        const auto &suffix = nodes_[node.suffix_link];
        const auto suffix_start = node.length - suffix.length;

        // 消える境界回文の真の接尾辞が別の回文に覆われなければ、そこを新しい記録とする。
        Index restored = kEmpty;
        if (suffix.length != 0 && nodes_[Starts(suffix_start, front)].length < suffix.length)
        {
            restored = node.suffix_link;
            Starts(suffix_start, front) = restored;
        }
        Ends(node.length - 1, front) = restored;
        if (--node.terminal_count == 0)
        {
            --distinct_;
        }
        if (front)
        {
            text_.pop_front();
        }
        else
        {
            text_.pop_back();
        }
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
