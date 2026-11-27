#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/point_set_range_composite_large_array

// Library: DataStructure/Sparse_Segment_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sparse_Segment_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_segment_tree.md
//
// API一覧
// SparseSegmentTree<S,op,e>(n=0): 長さnの全要素を単位元で初期化。O(1)時間・領域。
// set(p,value): pを代入。等値比較できる型では単位元の代入で点を削除。償却O(1+log(n+1))。
// erase(p): pを単位元に戻し、点を削除したときtrue。O(1+log(n+1))。
// get(p): pの値。未登録位置は単位元。O(1+log(n+1))。
// prod(l,r): 0始まり半開区間[l,r)を左から右へ集約。空区間は単位元。O(1+log(n+1))。
// size()/all_prod()/stored_points(): 長さ・全体の集約・保存点数。各O(1)。
// 長さ・端点はuint64_t。領域は同時に保存した最大点数Pに対してO(P+1)。
// opとSのコピー・ムーブの構築・代入はnoexcept。演算の途中の値もSで表現できること。単位元の自動削除に使う比較は設定前に行う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_SEGMENT_TREE_DEFINED
#define SIRKIT_SPARSE_SEGMENT_TREE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t NextSparseSegmentTreeCapacity(std::size_t used, std::size_t capacity, std::size_t extra,
                                                 std::size_t limit)
{
    if (extra > limit - used)
    {
        throw std::length_error("SparseSegmentTree: too many nodes");
    }
    if (used + extra <= capacity)
    {
        return capacity;
    }
    return std::max(used + extra, capacity + std::min(capacity, limit - capacity));
}
}

template <class S, auto op, auto e>
class SparseSegmentTree
{
    static_assert(std::is_nothrow_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S> &&
                  std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>);

public:
    explicit SparseSegmentTree(std::uint64_t n = 0) : length_(n), identity_(std::invoke(e))
    {
    }

    SparseSegmentTree(const SparseSegmentTree &) = default;

    SparseSegmentTree(SparseSegmentTree &&other) noexcept
        : nodes_(std::move(other.nodes_)), length_(std::exchange(other.length_, 0)), identity_(other.identity_),
          root_(std::exchange(other.root_, 0)), free_(std::exchange(other.free_, 0)),
          points_(std::exchange(other.points_, 0)), free_count_(std::exchange(other.free_count_, 0))
    {
        other.nodes_.clear();
    }

    SparseSegmentTree &operator=(SparseSegmentTree other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(length_, other.length_);
        std::swap(identity_, other.identity_);
        std::swap(root_, other.root_);
        std::swap(free_, other.free_);
        std::swap(points_, other.points_);
        std::swap(free_count_, other.free_count_);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t stored_points() const noexcept
    {
        return points_;
    }

    [[nodiscard]] S all_prod() const noexcept
    {
        return root_ != 0 ? At(root_).aggregate : identity_;
    }

    void set(std::uint64_t position, S value)
    {
        CheckPosition(position);
        if constexpr (std::equality_comparable<S>)
        {
            if (std::as_const(value) == std::as_const(identity_))
            {
                root_ = Erase(root_, position);
                return;
            }
        }
        root_ = Set(root_, position, value);
    }

    bool erase(std::uint64_t position)
    {
        CheckPosition(position);
        const auto old_points = points_;
        root_ = Erase(root_, position);
        return points_ != old_points;
    }

    [[nodiscard]] S get(std::uint64_t position) const
    {
        CheckPosition(position);
        auto node = root_;
        while (node != 0)
        {
            const auto &current = At(node);
            if (current.bits == 0)
            {
                return current.pivot == position ? current.aggregate : identity_;
            }
            node = position < current.pivot ? current.left : current.right;
        }
        return identity_;
    }

    [[nodiscard]] S prod(std::uint64_t left, std::uint64_t right) const
    {
        CheckRange(left, right);
        if (left == right || root_ == 0)
        {
            return identity_;
        }
        if (left == 0 && right == length_)
        {
            return At(root_).aggregate;
        }
        return Fold(root_, left, right);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        // 葉のpivotは位置。分岐では右半分の先頭位置を表す。
        std::uint64_t pivot = 0;
        Index left = 0, right = 0;
        S aggregate;
        unsigned bits = 0;
    };

    std::vector<Node> nodes_;
    std::uint64_t length_;
    S identity_;
    Index root_ = 0, free_ = 0;
    std::size_t points_ = 0, free_count_ = 0;

    void CheckPosition([[maybe_unused]] std::uint64_t position) const
    {
    }

    void CheckRange([[maybe_unused]] std::uint64_t left, [[maybe_unused]] std::uint64_t right) const
    {
    }

    void Reserve(std::size_t extra)
    {
        const auto slots = extra - std::min(extra, free_count_);
        const auto limit = std::min<std::size_t>(std::numeric_limits<Index>::max(), nodes_.max_size());
        nodes_.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NextSparseSegmentTreeCapacity(
            nodes_.size(), nodes_.capacity(), slots, limit));
    }

    [[nodiscard]] const Node &At(Index index) const noexcept
    {
        return nodes_[index - 1];
    }

    Node &At(Index index) noexcept
    {
        return nodes_[index - 1];
    }

    Index Allocate(Node node) noexcept
    {
        if (free_ == 0)
        {
            nodes_.push_back(std::move(node));
            return static_cast<Index>(nodes_.size());
        }
        const auto index = free_;
        free_ = At(index).left;
        --free_count_;
        At(index) = std::move(node);
        return index;
    }

    void Release(Index index) noexcept
    {
        auto &node = At(index);
        node.aggregate = identity_;
        node.left = free_;
        free_ = index;
        ++free_count_;
    }

    static S Product(const S &left, const S &right) noexcept
    {
        return std::invoke(op, left, right);
    }

    void Pull(Index index) noexcept
    {
        auto &node = At(index);
        node.aggregate = Product(At(node.left).aggregate, At(node.right).aggregate);
    }

    Index Set(Index index, std::uint64_t position, const S &value)
    {
        if (index == 0)
        {
            Reserve(1);
            ++points_;
            return Allocate({position, 0, 0, value});
        }
        const auto pivot = At(index).pivot;
        const auto bits = static_cast<unsigned>(std::bit_width(pivot ^ position));
        if (bits > At(index).bits)
        {
            const auto half = std::uint64_t{1} << (bits - 1);
            const auto split = (position | half) & ~(half - 1);
            Reserve(2);
            ++points_;
            const auto leaf = Allocate({position, 0, 0, value});
            const auto left = position < split ? leaf : index;
            const auto right = position < split ? index : leaf;
            return Allocate({split, left, right, Product(At(left).aggregate, At(right).aggregate), bits});
        }
        if (At(index).bits == 0)
        {
            At(index).aggregate = value;
            return index;
        }

        // 挿入でvectorが移動し得るため、子の添字を更新してから親を取り直す。
        if (position < pivot)
        {
            const auto child = Set(At(index).left, position, value);
            At(index).left = child;
        }
        else
        {
            const auto child = Set(At(index).right, position, value);
            At(index).right = child;
        }
        Pull(index);
        return index;
    }

    Index Erase(Index index, std::uint64_t position) noexcept
    {
        if (index == 0)
        {
            return 0;
        }
        auto &node = At(index);
        if (node.bits == 0)
        {
            if (node.pivot != position)
            {
                return index;
            }
            Release(index);
            --points_;
            return 0;
        }
        auto &child = position < node.pivot ? node.left : node.right;
        child = Erase(child, position);
        if (child == 0)
        {
            const auto sibling = node.left == 0 ? node.right : node.left;
            Release(index);
            return sibling;
        }
        Pull(index);
        return index;
    }

    [[nodiscard]] S Fold(Index index, std::uint64_t left, std::uint64_t right) const noexcept
    {
        const auto &node = At(index);
        if (node.bits == 0)
        {
            return left <= node.pivot && node.pivot < right ? node.aggregate : identity_;
        }
        // 右端を含む形にして、64ビット全体にまたがる区間でも加算であふれない。
        const auto half = std::uint64_t{1} << (node.bits - 1);
        const auto first = node.pivot - half;
        const auto last = node.pivot | (half - 1);
        if (right <= first || last < left)
        {
            return identity_;
        }
        if (left <= first && last < right)
        {
            return node.aggregate;
        }
        if (right <= node.pivot)
        {
            return Fold(node.left, left, right);
        }
        if (node.pivot <= left)
        {
            return Fold(node.right, left, right);
        }
        return Product(Fold(node.left, left, right), Fold(node.right, left, right));
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

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;

    constexpr bool operator==(const Affine &) const noexcept = default;
};

constexpr Affine Compose(Affine left, Affine right) noexcept
{
    return {
        static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
        static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
    };
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}
}

int main()
{
    std::uint64_t n = 0;
    int q = 0;
    verify_in >> n >> q;
    SparseSegmentTree<Affine, Compose, Identity> data(n);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0;
        verify_in >> type >> left;
        if (type == 0)
        {
            Affine value{};
            verify_in >> value.a >> value.b;
            data.set(left, value);
        }
        else
        {
            std::uint64_t right = 0;
            std::uint32_t x = 0;
            verify_in >> right >> x;
            const auto fold = data.prod(left, right);
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
        }
    }
}
