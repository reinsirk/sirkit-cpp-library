#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 649f1b5f10c1409551889420ca1ecf33dd4b275f91a4df2c2a804653c3f89c88
// Verify: https://atcoder.jp/contests/abc253/tasks/abc253_c

// Library: DataStructure/Ordered_Multiset.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（多重集合の削除と順位照会）: https://atcoder.jp/contests/abc253/tasks/abc253_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ordered_multiset.md
//
// API一覧
// OrderedMultiset<T,Compare>(compare={}): 空のAVL木。Compareはconstで呼べる狭義弱順序。
// size()/empty(): 重複込みの要素数(uint64_t)/空か。O(1)。
// insert(value,copies=1): 同値な要素をcopies個追加。総数の範囲超過はlength_error。
// erase(value,copies=1): 最大copies個削除し、削除数を返す。不足分は無視。count(value): 同値な要素数。
// count_less(value)/count_less_equal(value): 比較順でvalue未満/以下の要素数。
// kth(rank): 比較順の0始まり順位の値をconst T*で返す。範囲外はnullptr。
// ポインタはその値の最後の1個を消すまで有効。所有権は木にある。count以降と追加・削除はO(1+log(u+1))、uは異なるキー数。
// コピー不可。比較器が例外なく移動できる場合は移動可で、移動元は空。領域O(u+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ORDERED_MULTISET_DEFINED
#define SIRKIT_ORDERED_MULTISET_DEFINED

template <typename T, typename Compare = std::less<>>
class OrderedMultiset
{
    using Count = std::uint64_t;

    struct Node
    {
        T key;
        Count copies;
        Count total;
        int height = 1;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node(T &&value, Count times)
            : key(std::move(value)), copies(times), total(times)
        {
        }
    };

    using Link = std::unique_ptr<Node>;
    [[no_unique_address]] Compare compare_;
    Link root_;

    [[nodiscard]] bool less(const T &a, const T &b) const
    {
        return std::invoke(compare_, a, b);
    }

    static int height(const Link &link) noexcept
    {
        return link ? link->height : 0;
    }

    static Count total(const Link &link) noexcept
    {
        return link ? link->total : 0;
    }

    static void pull(Node &node) noexcept
    {
        node.height = 1 + std::max(height(node.left), height(node.right));
        node.total = total(node.left) + node.copies + total(node.right);
    }

    static void rotate_left(Link &link) noexcept
    {
        auto pivot = std::move(link->right);
        link->right = std::move(pivot->left);
        pull(*link);
        pivot->left = std::move(link);
        pull(*pivot);
        link = std::move(pivot);
    }

    static void rotate_right(Link &link) noexcept
    {
        auto pivot = std::move(link->left);
        link->left = std::move(pivot->right);
        pull(*link);
        pivot->right = std::move(link);
        pull(*pivot);
        link = std::move(pivot);
    }

    static void rebalance(Link &link) noexcept
    {
        pull(*link);
        const int difference = height(link->left) - height(link->right);
        if (difference > 1)
        {
            if (height(link->left->left) < height(link->left->right))
            {
                rotate_left(link->left);
            }
            rotate_right(link);
        }
        else if (difference < -1)
        {
            if (height(link->right->right) < height(link->right->left))
            {
                rotate_right(link->right);
            }
            rotate_left(link);
        }
    }

    void add(Link &link, T &value, Count copies)
    {
        if (!link)
        {
            link = std::make_unique<Node>(std::move(value), copies);
            return;
        }
        if (less(value, link->key))
        {
            add(link->left, value, copies);
        }
        else if (less(link->key, value))
        {
            add(link->right, value, copies);
        }
        else
        {
            link->copies += copies;
        }
        rebalance(link);
    }

    static Link detach_first(Link &link) noexcept
    {
        if (!link->left)
        {
            auto first = std::move(link);
            link = std::move(first->right);
            return first;
        }
        auto first = detach_first(link->left);
        rebalance(link);
        return first;
    }

    static void remove_node(Link &link) noexcept
    {
        if (!link->left)
        {
            auto right = std::move(link->right);
            link = std::move(right);
            return;
        }
        if (!link->right)
        {
            auto left = std::move(link->left);
            link = std::move(left);
            return;
        }
        // キーを移動せず、後継ノード自体を付け替える。
        auto replacement = detach_first(link->right);
        replacement->left = std::move(link->left);
        replacement->right = std::move(link->right);
        link = std::move(replacement);
        rebalance(link);
    }

    Count remove(Link &link, const T &value, Count copies)
    {
        if (!link)
        {
            return 0;
        }
        Count removed;
        if (less(value, link->key))
        {
            removed = remove(link->left, value, copies);
        }
        else if (less(link->key, value))
        {
            removed = remove(link->right, value, copies);
        }
        else
        {
            removed = std::min(copies, link->copies);
            if (removed == link->copies)
            {
                remove_node(link);
                return removed;
            }
            link->copies -= removed;
        }
        rebalance(link);
        return removed;
    }

    [[nodiscard]] Count rank(const T &value, bool include_equal) const
    {
        Count smaller = 0;
        const Node *node = root_.get();
        while (node)
        {
            const bool counted = include_equal ? !less(value, node->key) : less(node->key, value);
            if (counted)
            {
                smaller += total(node->left) + node->copies;
                node = node->right.get();
            }
            else
            {
                node = node->left.get();
            }
        }
        return smaller;
    }

public:
    explicit OrderedMultiset(Compare compare = {}) : compare_(std::move(compare))
    {
    }

    OrderedMultiset(const OrderedMultiset &) = delete;
    OrderedMultiset &operator=(const OrderedMultiset &) = delete;

    OrderedMultiset(OrderedMultiset &&) noexcept
        requires std::is_nothrow_move_constructible_v<Compare> = default;

    OrderedMultiset &operator=(OrderedMultiset &&other) noexcept
        requires std::is_nothrow_move_assignable_v<Compare>
    {
        if (this != &other)
        {
            compare_ = std::move(other.compare_);
            root_ = std::move(other.root_);
        }
        return *this;
    }

    [[nodiscard]] Count size() const noexcept
    {
        return total(root_);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return !root_;
    }

    void insert(T value, Count copies = 1)
    {
        if (copies == 0)
        {
            return;
        }
        if (copies > std::numeric_limits<Count>::max() - size())
        {
            throw std::length_error("OrderedMultiset: too many elements");
        }
        add(root_, value, copies);
    }

    Count erase(const T &value, Count copies = 1)
    {
        return copies == 0 ? 0 : remove(root_, value, copies);
    }

    [[nodiscard]] Count count(const T &value) const
    {
        const Node *node = root_.get();
        while (node)
        {
            if (less(value, node->key))
            {
                node = node->left.get();
            }
            else if (less(node->key, value))
            {
                node = node->right.get();
            }
            else
            {
                return node->copies;
            }
        }
        return 0;
    }

    [[nodiscard]] Count count_less(const T &value) const
    {
        return rank(value, false);
    }

    [[nodiscard]] Count count_less_equal(const T &value) const
    {
        return rank(value, true);
    }

    [[nodiscard]] const T *kth(Count rank) const noexcept
    {
        const Node *node = root_.get();
        while (node)
        {
            const auto before = total(node->left);
            if (rank < before)
            {
                node = node->left.get();
            }
            else
            {
                rank -= before;
                if (rank < node->copies)
                {
                    return std::addressof(node->key);
                }
                rank -= node->copies;
                node = node->right.get();
            }
        }
        return nullptr;
    }
};

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


#include <cstdint>

int main()
{
    int queries{};
    verify_in >> queries;
    sirkit::OrderedMultiset<int> values;
    for (int i = 0; i < queries; ++i)
    {
        int type{};
        verify_in >> type;
        if (type == 3)
        {
            verify_out << *values.kth(values.size() - 1) - *values.kth(0) << '\n';
            continue;
        }
        int x{};
        verify_in >> x;
        if (type == 1)
        {
            values.insert(x);
        }
        else
        {
            std::uint64_t amount{};
            verify_in >> amount;
            values.erase(x, amount);
        }
    }
}
