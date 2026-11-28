#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc183/tasks/abc183_f

// Library: DataStructure/Union_Find_Order_Statistics.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Union_Find_Order_Statistics.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/union_find_order_statistics.md
// 同梱UnionFindの出典: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/union-find
//
// API一覧
// UnionFindOrderStatistics<T>(labels): 初期ラベルを固定し、全頂点を独立成分にする。頂点は0始まり。
// Tはコピー可能な全順序型。異なる値は座標圧縮し、同じ値は多重度に数える。
// merge(a,b): 新規結合ならtrue。全結合での木の併合時間はO(n log(m+1))。成分探索は償却O(α(n))。
// root(v)/issame(a,b)/size(v): 代表/同一成分/成分サイズ。償却O(α(n))。
// size()/component_count(): 全頂点数/成分数。O(1)。
// kth(v,k): 成分内のラベルの昇順で0始まりk番目。範囲外はnullopt。
// count(v,label)/count_less(v,label): 成分内で同値/未満のラベル数。
// 照会はTの比較とコピーをO(1)としてO(log(m+1)+α(n))償却。
// nは頂点数でINT_MAX以下、mは異なるラベルの個数。構築O(n log(n+1))時間・O(n log(m+1))領域。
//
// [同梱のUnionFind]
// [UnionFind]
// UnionFind(n=0): n要素を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// root(i)/leader(i): 代表、unite(a,b): 結合（戻り値なし）、merge(a,b): 新規結合ならtrue。償却O(α(n+1))。
// issame(a,b)/same(a,b): 同一成分か、size(i): 成分サイズ。償却O(α(n+1))。探索時に経路を圧縮する。
// size()/empty()/component_count(): 全要素数/空か/成分数、O(1)。groups(): 頂点昇順の成分一覧、O(1+n α(n+1))償却。
// reset(n)/reset(): n要素/現在の要素数へ初期化。容量を再利用、O(旧n+n+1)上界。確保失敗時は元を保つ。
// clear(): 空へ戻して容量保持、O(旧n+1)上界、noexcept。swap(other): 内容交換、O(1)、noexcept。
// コピーはO(n+1)で独立、移動元は空。自己代入・自己ムーブは内容を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNION_FIND_DEFINED
#define SIRKIT_UNION_FIND_DEFINED
class UnionFind {
    std::vector<int> param;
    int components_ = 0;
    static std::size_t checked_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("UnionFind: too many vertices");
        return n;
    }
    void check(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(i) >= param.size())
            throw std::out_of_range("UnionFind: vertex");
#else
        (void)i;
#endif
    }
    int leader_unchecked(int i) {
        while (param[i] >= 0) {
            const int p = param[i];
            if (param[p] < 0) return p;
            i = param[i] = param[p];
        }
        return i;
    }
    // 単一成分の連続処理を分離し、通常の成分列挙のコード生成を保つ。
    void list_single_component(std::vector<int>& vertices) {
        const int root = leader_unchecked(0);
        const int negative_size = param[root];
        std::fill(param.begin(), param.end(), root);
        param[root] = negative_size;
        std::iota(vertices.begin(), vertices.end(), 0);
    }
public:
    explicit UnionFind(std::size_t n = 0)
        : param(checked_size(n), -1), components_(static_cast<int>(n)) {}
    UnionFind(const UnionFind&) = default;
    UnionFind& operator=(const UnionFind&) = default;
    UnionFind(UnionFind&& other) noexcept
        : param(std::move(other.param)), components_(std::exchange(other.components_, 0)) { other.param.clear(); }
    UnionFind& operator=(UnionFind&& other) noexcept {
        if (this != &other) {
            param = std::move(other.param);
            components_ = std::exchange(other.components_, 0); other.param.clear();
        }
        return *this;
    }
    void reset(std::size_t n) {
        const auto count = checked_size(n);
        param.reserve(count);
        param.clear(); param.resize(count, -1);
        components_ = static_cast<int>(count);
    }
    void reset() { reset(param.size()); }
    void clear() noexcept { param.clear(); components_ = 0; }
    bool empty() const noexcept { return param.empty(); }
    void swap(UnionFind& other) noexcept {
        param.swap(other.param); std::swap(components_, other.components_);
    }
    friend void swap(UnionFind& left, UnionFind& right) noexcept { left.swap(right); }
    int root(int i) { check(i); return leader_unchecked(i); }
    int leader(int i) { return root(i); }
    bool merge(int a, int b) {
        check(a); check(b);
        a = leader_unchecked(a); b = leader_unchecked(b);
        if (a == b) return false;
        if (param[a] > param[b]) std::swap(a, b);
        param[a] += param[b]; param[b] = a; --components_;
        return true;
    }
    void unite(int a, int b) { (void)merge(a, b); }
    bool same(int a, int b) { check(a); check(b); return leader_unchecked(a) == leader_unchecked(b); }
    bool issame(int a, int b) { return same(a, b); }
    int size(int i) { return -param[root(i)]; }
    int size() const noexcept { return static_cast<int>(param.size()); }
    int component_count() const noexcept { return components_; }
    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result;
        result.reserve(static_cast<std::size_t>(components_));
        if (components_ == size()) {
            for (int v = 0; v < size(); ++v) result.emplace_back(std::size_t{1}, v);
            return result;
        }
        if (components_ == 1) {
            result.emplace_back(param.size());
            list_single_component(result[0]);
            return result;
        }
        std::vector<int> leaders(param.size()), counts(param.size(), 0);
        for (int i = 0; i < size(); ++i) ++counts[leaders[i] = leader_unchecked(i)];
        for (int i = 0; i < size(); ++i) if (counts[i]) {
            const int count = counts[i];
            counts[i] = static_cast<int>(result.size());
            result.emplace_back(); result.back().reserve(count);
        }
        for (int i = 0; i < size(); ++i) result[counts[leaders[i]]].push_back(i);
        return result;
    }
};
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <concepts>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED
#define SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED

template <std::totally_ordered T>
    requires std::copyable<T>
class UnionFindOrderStatistics
{
    using Index = std::size_t;

public:
    explicit UnionFindOrderStatistics(const std::vector<T> &labels)
        : groups_(labels.size()), values_(labels), roots_(labels.size())
    {
        std::sort(values_.begin(), values_.end());
        values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
        if (!labels.empty())
        {
            const auto path_nodes = std::bit_width(values_.size() - 1) + 1;
            const auto estimated = static_cast<std::uint64_t>(labels.size()) * path_nodes;
            nodes_.reserve(static_cast<Index>(std::min<std::uint64_t>(estimated, nodes_.max_size())));
        }
        for (Index v = 0; v < labels.size(); ++v)
        {
            roots_[v] = BuildSingleton(0, values_.size(), LowerBound(labels[v]));
        }
    }

    UnionFindOrderStatistics(const UnionFindOrderStatistics &) = default;
    UnionFindOrderStatistics(UnionFindOrderStatistics &&) noexcept = default;

    UnionFindOrderStatistics &operator=(UnionFindOrderStatistics other) noexcept
    {
        groups_.swap(other.groups_);
        values_.swap(other.values_);
        roots_.swap(other.roots_);
        nodes_.swap(other.nodes_);
        return *this;
    }

    bool merge(int a, int b)
    {
        a = groups_.root(a);
        b = groups_.root(b);
        if (!groups_.merge(a, b))
        {
            return false;
        }
        const auto joined = MergeTrees(roots_[a], roots_[b]);
        roots_[a] = joined;
        roots_[b] = joined;
        return true;
    }

    int root(int vertex)
    {
        return groups_.root(vertex);
    }

    bool issame(int a, int b)
    {
        return groups_.issame(a, b);
    }

    int size(int vertex)
    {
        return groups_.size(vertex);
    }

    int size() const noexcept
    {
        return groups_.size();
    }

    int component_count() const noexcept
    {
        return groups_.component_count();
    }

    std::optional<T> kth(int vertex, int k)
    {
        auto id = roots_[groups_.root(vertex)];
        if (k < 0 || k >= SizeOf(id))
        {
            return std::nullopt;
        }
        Index from = 0;
        Index to = values_.size();
        while (to - from > 1)
        {
            const auto &node = nodes_[id - 1];
            const auto mid = from + (to - from) / 2;
            const int left_count = SizeOf(node.left);
            if (k < left_count)
            {
                id = node.left;
                to = mid;
            }
            else
            {
                k -= left_count;
                id = node.right;
                from = mid;
            }
        }
        return values_[from];
    }

    int count(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        const auto end = LowerBound(label);
        if (end == values_.size() || values_[end] != label)
        {
            return 0;
        }
        return Prefix(id, 0, values_.size(), end + 1) - Prefix(id, 0, values_.size(), end);
    }

    int count_less(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        return Prefix(id, 0, values_.size(), LowerBound(label));
    }

private:
    struct Node
    {
        Index left = 0;
        Index right = 0;
        int count = 1;
    };

    UnionFind groups_;
    std::vector<T> values_;
    std::vector<Index> roots_;
    std::vector<Node> nodes_;

    Index LowerBound(const T &label) const
    {
        return static_cast<Index>(std::lower_bound(values_.begin(), values_.end(), label) - values_.begin());
    }

    int SizeOf(Index id) const
    {
        return id == 0 ? 0 : nodes_[id - 1].count;
    }

    Index BuildSingleton(Index from, Index to, Index position)
    {
        Node node;
        if (to - from > 1)
        {
            const auto mid = from + (to - from) / 2;
            if (position < mid)
            {
                node.left = BuildSingleton(from, mid, position);
            }
            else
            {
                node.right = BuildSingleton(mid, to, position);
            }
        }
        nodes_.push_back(node);
        return nodes_.size();
    }

    Index MergeTrees(Index first, Index second)
    {
        if (first == 0)
        {
            return second;
        }
        if (second == 0)
        {
            return first;
        }
        auto &left = nodes_[first - 1];
        const auto &right = nodes_[second - 1];
        left.left = MergeTrees(left.left, right.left);
        left.right = MergeTrees(left.right, right.right);
        left.count += right.count;
        return first;
    }

    int Prefix(Index id, Index from, Index to, Index end) const
    {
        if (id == 0 || end <= from)
        {
            return 0;
        }
        const auto &node = nodes_[id - 1];
        if (to <= end)
        {
            return node.count;
        }
        const auto mid = from + (to - from) / 2;
        return Prefix(node.left, from, mid, end) + Prefix(node.right, mid, to, end);
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


#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &key : values)
    {
        verify_in >> key;
    }
    sirkit::UnionFindOrderStatistics tree(values);
    while (q-- > 0)
    {
        int type, a, b;
        verify_in >> type >> a >> b;
        if (type == 1)
        {
            tree.merge(a - 1, b - 1);
        }
        else
        {
            verify_out << tree.count(a - 1, b) << '\n';
        }
    }
}
