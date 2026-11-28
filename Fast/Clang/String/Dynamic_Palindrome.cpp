// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Dynamic_Palindrome.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_palindrome.md
//
// API一覧
// [DynamicPalindrome] 固定長のバイト列への点代入と、部分文字列の回文照会。添字は0始まり。
// DynamicPalindrome(text[,seed]): 内容をコピーする。seedを指定しないと乱数で基数を選ぶ。O(n+1)時間・領域。
// set(index,byte): 1バイトを代入。O(1+log(n+1))時間。get(index): バイトを取得。O(1)。
// is_palindrome(left,right): 半開区間[left,right)が回文か。空区間もtrue。O(1+log(n+1))時間。
// 2つの素数法のローリングハッシュを使う。衝突時は回文でない区間をtrueと判定する可能性がある。
// size(): バイト数。O(1)。コピーは独立。移動元は空。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_PALINDROME_DEFINED
#define SIRKIT_DYNAMIC_PALINDROME_DEFINED

namespace detail
{
inline std::size_t dynamic_palindrome_nodes(std::size_t length)
{
    if (length > std::numeric_limits<std::size_t>::max() / 2)
    {
        throw std::length_error("DynamicPalindrome: text too long");
    }
    return length + 1;
}

inline std::uint64_t dynamic_palindrome_seed()
{
    std::random_device device;
    const auto high = static_cast<std::uint64_t>(device());
    return (high << 32) ^ device();
}
} // namespace detail

class DynamicPalindrome
{
    static constexpr std::uint32_t kModulus1 = 1'000'000'007;
    static constexpr std::uint32_t kModulus2 = 1'000'000'009;

    struct Hash
    {
        std::uint32_t first = 0;
        std::uint32_t second = 0;

        [[nodiscard]] std::uint64_t packed() const noexcept
        {
            return (static_cast<std::uint64_t>(first) << 32) | second;
        }

        Hash operator+(Hash other) const noexcept
        {
            return {(first + other.first) % kModulus1, (second + other.second) % kModulus2};
        }

        Hash operator-(Hash other) const noexcept
        {
            return {(first + kModulus1 - other.first) % kModulus1,
                    (second + kModulus2 - other.second) % kModulus2};
        }

        Hash operator*(Hash other) const noexcept
        {
            return {static_cast<std::uint32_t>(static_cast<std::uint64_t>(first) * other.first % kModulus1),
                    static_cast<std::uint32_t>(static_cast<std::uint64_t>(second) * other.second % kModulus2)};
        }
    };

    struct Node
    {
        Hash forward;
        Hash reverse;

        void accumulate(Node other) noexcept
        {
            forward = forward + other.forward;
            reverse = reverse + other.reverse;
        }
    };

  public:
    explicit DynamicPalindrome(std::string_view text) : DynamicPalindrome(text, detail::dynamic_palindrome_seed())
    {
    }

    DynamicPalindrome(std::string_view text, std::uint64_t seed)
        : powers_(detail::dynamic_palindrome_nodes(text.size())), tree_(powers_.size()), text_(text)
    {
        std::mt19937_64 random(seed);
        const Hash base{std::uniform_int_distribution<std::uint32_t>(257, kModulus1 - 2)(random),
                        std::uniform_int_distribution<std::uint32_t>(257, kModulus2 - 2)(random)};
        powers_[0] = {1, 1};
        for (std::size_t index = 1; index < powers_.size(); ++index)
        {
            powers_[index] = powers_[index - 1] * base;
        }

        for (std::size_t index = 1; index <= size(); ++index)
        {
            tree_[index].accumulate(weighted(index - 1, static_cast<std::uint8_t>(text_[index - 1])));
            const auto parent = index + lowbit(index);
            if (parent <= size())
            {
                tree_[parent].accumulate(tree_[index]);
            }
        }
    }

    DynamicPalindrome(const DynamicPalindrome &) = default;
    DynamicPalindrome(DynamicPalindrome &&other) noexcept
    {
        exchange(other);
    }

    DynamicPalindrome &operator=(DynamicPalindrome other) noexcept
    {
        exchange(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return text_.size();
    }

    [[nodiscard]] std::uint8_t get(std::size_t index) const
    {
        return static_cast<std::uint8_t>(text_[index]);
    }

    void set(std::size_t index, std::uint8_t byte)
    {
        const auto previous = get(index);
        if (previous == byte)
        {
            return;
        }
        const auto old_value = weighted(index, previous);
        const auto new_value = weighted(index, byte);
        const Node delta{new_value.forward - old_value.forward, new_value.reverse - old_value.reverse};
        for (auto node = index + 1; node <= size(); node += lowbit(node))
        {
            tree_[node].accumulate(delta);
        }
        text_[index] = static_cast<char>(byte);
    }

    [[nodiscard]] bool is_palindrome(std::size_t left, std::size_t right) const
    {
        if (right - left < 2)
        {
            return true;
        }
        const auto from = prefix(left);
        const auto to = prefix(right);
        // 順方向と逆方向で、区間に対応する多項式の最小次数をそろえる。
        const auto forward = (to.forward - from.forward) * powers_[size() - right];
        const auto reverse = (to.reverse - from.reverse) * powers_[left];
        return forward.packed() == reverse.packed();
    }

  private:
    static std::size_t lowbit(std::size_t index) noexcept
    {
        return index & (std::size_t{0} - index);
    }

    [[nodiscard]] Node weighted(std::size_t position, std::uint8_t byte) const noexcept
    {
        const Hash value{byte, byte};
        return {value * powers_[position], value * powers_[size() - 1 - position]};
    }

    [[nodiscard]] Node prefix(std::size_t size) const noexcept
    {
        Node total;
        for (auto node = size; node != 0; node -= lowbit(node))
        {
            total.accumulate(tree_[node]);
        }
        return total;
    }


    void exchange(DynamicPalindrome &other) noexcept
    {
        powers_.swap(other.powers_);
        tree_.swap(other.tree_);
        text_.swap(other.text_);
    }

    std::vector<Hash> powers_;
    std::vector<Node> tree_;
    std::string text_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
