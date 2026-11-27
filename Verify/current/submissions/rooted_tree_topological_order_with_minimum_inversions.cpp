#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/rooted_tree_topological_order_with_minimum_inversions
// Submission: https://judge.yosupo.jp/submission/419925

// Library: Graph/Tree_Topological_Order_Minimum_Inversions.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（木で制約された仕事の順序付け）: https://doi.org/10.1137/0123021
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_topological_order_minimum_inversions.md
//
// API一覧
// minimum_inversion_topological_order(n,parent,c,d): 親を先に並べ、先行するuと後続vのd[u]*c[v]の積和を最小にする。
// parent[v]は頂点vの親（0始まり）、根だけ-1。c,dは非負整数の頂点重み。各列の長さはn。
// .inversionsは最小値、.orderはその値を達成する頂点順。空グラフは積和0・順序空。
// c,dそれぞれの総和や最小値がuint64_tの範囲外ならoverflow_error。
// O(n log(n+1))時間・O(n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <limits>
#include <numeric>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_TOPOLOGICAL_ORDER_MINIMUM_INVERSIONS_DEFINED
#define SIRKIT_TREE_TOPOLOGICAL_ORDER_MINIMUM_INVERSIONS_DEFINED

struct MinimumInversionTopologicalOrder
{
    std::uint64_t inversions;
    std::vector<int> order;
};

namespace detail
{
using TreeOrderWeight = std::uint64_t;
inline constexpr TreeOrderWeight kTreeOrderMaxWeight = std::numeric_limits<TreeOrderWeight>::max();

inline void tree_order_add(TreeOrderWeight &total, TreeOrderWeight value)
{
    if (value > kTreeOrderMaxWeight - total)
    {
        throw std::overflow_error("minimum_inversion_topological_order: sum does not fit uint64_t");
    }
    total += value;
}

inline std::pair<TreeOrderWeight, TreeOrderWeight> tree_order_product(TreeOrderWeight a, TreeOrderWeight b)
{
    // 32ビットずつの積を組み立て、128ビット積を上位・下位64ビットで返す。
    constexpr TreeOrderWeight kMask = (TreeOrderWeight{1} << 32) - 1;
    const auto a_low = a & kMask, a_high = a >> 32;
    const auto b_low = b & kMask, b_high = b >> 32;
    const auto low = a_low * b_low;
    const auto middle = a_high * b_low + (low >> 32);
    const auto carry = a_low * b_high + (middle & kMask);
    return {a_high * b_high + (middle >> 32) + (carry >> 32), (carry << 32) | (low & kMask)};
}

struct TreeOrderBlock
{
    TreeOrderWeight c;
    TreeOrderWeight d;
    int vertex;
    int version;

    TreeOrderWeight denominator() const
    {
        return (c | d) == 0 ? 1 : d;
    }
};

struct TreeOrderDensityLess
{
    bool wide;

    bool operator()(const TreeOrderBlock &a, const TreeOrderBlock &b) const
    {
        const auto da = a.denominator(), db = b.denominator();
        const auto lhs = wide ? tree_order_product(a.c, db) : std::pair{TreeOrderWeight{0}, a.c * db};
        const auto rhs = wide ? tree_order_product(b.c, da) : std::pair{TreeOrderWeight{0}, b.c * da};
        return lhs == rhs ? a.vertex > b.vertex : lhs < rhs;
    }
};

inline int tree_order_root(std::span<const int> parent)
{
    const auto n = static_cast<int>(parent.size());
    int root = -1;
    for (int v = 0; v < n; ++v)
    {
        const int p = parent[v];
#ifndef SKIP_RUNTIME_CHECKS
        if (p < -1 || p >= n)
        {
            throw std::out_of_range("minimum_inversion_topological_order: parent out of range");
        }
#endif
        if (p == -1)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (root != -1)
            {
                throw std::invalid_argument("minimum_inversion_topological_order: multiple roots");
            }
#endif
            root = v;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (root == -1)
    {
        throw std::invalid_argument("minimum_inversion_topological_order: missing root");
    }
    std::vector<unsigned char> seen(n);
    seen[root] = 2;
    for (int v = 0; v < n; ++v)
    {
        int u = v;
        for (; seen[u] == 0; u = parent[u])
        {
            seen[u] = 1;
        }
        if (seen[u] == 1)
        {
            throw std::invalid_argument("minimum_inversion_topological_order: cycle in parents");
        }
        for (u = v; seen[u] == 1; u = parent[u])
        {
            seen[u] = 2;
        }
    }
#endif
    return root;
}

class TreeOrderBlocks
{
    std::vector<int> link_;
    std::vector<int> top_;

    int find(int v)
    {
        while (link_[v] >= 0)
        {
            const int p = link_[v];
            if (link_[p] >= 0)
            {
                link_[v] = link_[p];
            }
            v = link_[v];
        }
        return v;
    }

public:
    explicit TreeOrderBlocks(int n) : link_(n, -1), top_(n)
    {
        std::iota(top_.begin(), top_.end(), 0);
    }

    int top(int v)
    {
        return top_[find(v)];
    }

    void append(int parent, int child)
    {
        int a = find(parent), b = find(child);
        if (link_[a] > link_[b])
        {
            std::swap(a, b);
        }
        link_[a] += link_[b];
        link_[b] = a;
        top_[a] = parent;
    }
};
} // namespace detail

inline MinimumInversionTopologicalOrder minimum_inversion_topological_order(
    int n, std::span<const int> parent, std::span<const std::uint64_t> c, std::span<const std::uint64_t> d)
{
    if (n < 0 || parent.size() != static_cast<std::size_t>(n) ||
        c.size() != parent.size() || d.size() != parent.size())
    {
        throw std::invalid_argument("minimum_inversion_topological_order: inconsistent lengths");
    }
    if (n == 0)
    {
        return {0, {}};
    }
    const int root = detail::tree_order_root(parent);
    using Weight = detail::TreeOrderWeight;
    Weight total_c = 0, total_d = 0;
    std::vector<detail::TreeOrderBlock> blocks, initial;
    blocks.reserve(n);
    initial.reserve(n - 1);
    for (int v = 0; v < n; ++v)
    {
        detail::tree_order_add(total_c, c[v]);
        detail::tree_order_add(total_d, d[v]);
        blocks.push_back({c[v], d[v], v, 0});
        if (v != root)
        {
            initial.push_back(blocks.back());
        }
    }
    const bool wide = total_c != 0 && total_d > detail::kTreeOrderMaxWeight / total_c;
    std::priority_queue<detail::TreeOrderBlock, std::vector<detail::TreeOrderBlock>, detail::TreeOrderDensityLess>
        queue(detail::TreeOrderDensityLess{wide}, std::move(initial));

    detail::TreeOrderBlocks merged(n);
    std::vector<int> tail(n), next(n, -1);
    std::iota(tail.begin(), tail.end(), 0);
    while (!queue.empty())
    {
        const auto [child_c, child_d, child, stamp] = queue.top();
        queue.pop();
        if (blocks[child].version != stamp)
        {
            continue;
        }
        // 最大c/d比の子を親ブロックの直後に置いても、途中のブロックとの交換で積和が増えない。
        const int above = merged.top(parent[child]);
        auto &block = blocks[above];
        block.c += child_c;
        block.d += child_d;
        next[tail[above]] = child;
        tail[above] = tail[child];
        merged.append(above, child);
        blocks[child].version = -1;
        if (above != root)
        {
            ++block.version;
            queue.push(block);
        }
    }

    MinimumInversionTopologicalOrder result{0, {}};
    result.order.reserve(n);
    Weight prefix = 0;
    for (int v = root; v != -1; v = next[v])
    {
        result.order.push_back(v);
        if (wide)
        {
            if (c[v] != 0 && prefix > detail::kTreeOrderMaxWeight / c[v])
            {
                throw std::overflow_error("minimum_inversion_topological_order: inversion term does not fit uint64_t");
            }
            detail::tree_order_add(result.inversions, c[v] * prefix);
        }
        else
        {
            result.inversions += c[v] * prefix;
        }
        prefix += d[v];
    }
    return result;
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


#include <cstdint>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> parent(n);
    parent[0] = -1;
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parent[v];
    }
    std::vector<std::uint64_t> c(n), d(n);
    for (auto &value : c)
    {
        verify_in >> value;
    }
    for (auto &value : d)
    {
        verify_in >> value;
    }
    const auto best = minimum_inversion_topological_order(n, parent, c, d);
    verify_out << best.inversions << '\n';
    for (int i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << best.order[i];
    }
    verify_out << '\n';
}
