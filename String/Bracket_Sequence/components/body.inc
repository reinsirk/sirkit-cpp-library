#include <string_view>

#ifndef SIRKIT_BRACKET_SEQUENCE_DEFINED
#define SIRKIT_BRACKET_SEQUENCE_DEFINED

struct BracketSummary
{
    int matched_pairs = 0;
    int unmatched_open = 0;
    int unmatched_close = 0;

    [[nodiscard]] constexpr int matched_length() const noexcept
    {
        return 2 * matched_pairs;
    }

    [[nodiscard]] constexpr bool balanced() const noexcept
    {
        return unmatched_open == 0 && unmatched_close == 0;
    }

    [[nodiscard]] constexpr int minimum_insertions() const noexcept
    {
        return unmatched_open + unmatched_close;
    }
};

namespace detail
{
inline constexpr BracketSummary merge_bracket_summaries(BracketSummary left, BracketSummary right) noexcept
{
    const auto matches = std::min(left.unmatched_open, right.unmatched_close);
    return {left.matched_pairs + right.matched_pairs + matches, left.unmatched_open + right.unmatched_open - matches,
            left.unmatched_close + right.unmatched_close - matches};
}

inline constexpr BracketSummary empty_bracket_summary() noexcept
{
    return {};
}

inline BracketSummary bracket_leaf(char symbol)
{
    if (symbol == '(')
    {
        return {0, 1, 0};
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (symbol != ')')
    {
        throw std::invalid_argument("BracketSequence: expected an opening or closing bracket");
    }
#endif
    return {0, 0, 1};
}

inline std::vector<BracketSummary> bracket_leaves(std::string_view text)
{
    static_cast<void>(CheckedSegmentArrayCount(text.size(), "BracketSequence: text is too long"));
    std::vector<BracketSummary> leaves;
    leaves.reserve(text.size());
    for (const char symbol : text)
    {
        leaves.push_back(bracket_leaf(symbol));
    }
    return leaves;
}
} // namespace detail

class BracketSequence
{
public:
    explicit BracketSequence(std::string_view text) : tree_(detail::bracket_leaves(text))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return tree_.size();
    }

    [[nodiscard]] char get(int index) const
    {
        return tree_.get(index).unmatched_open == 0 ? ')' : '(';
    }

    void set(int index, char symbol)
    {
        tree_.set(index, detail::bracket_leaf(symbol));
    }

    void swap(int first, int second)
    {
        const auto left = tree_.get(first);
        const auto right = tree_.get(second);
        if (left.unmatched_open != right.unmatched_open)
        {
            tree_.set(first, right);
            tree_.set(second, left);
        }
    }

    [[nodiscard]] BracketSummary query(int left, int right) const
    {
        return tree_.prod(left, right);
    }

    [[nodiscard]] int first_unmatched_close(int left) const
    {
        return tree_.max_right(left, [](BracketSummary summary) { return summary.unmatched_close == 0; });
    }

    [[nodiscard]] int last_unmatched_open(int right) const
    {
        return tree_.min_left(right, [](BracketSummary summary) { return summary.unmatched_open == 0; }) - 1;
    }

private:
    segtree<BracketSummary, detail::merge_bracket_summaries, detail::empty_bracket_summary> tree_;
};
#endif
