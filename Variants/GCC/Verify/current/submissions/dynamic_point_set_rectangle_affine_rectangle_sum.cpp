#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/dynamic_point_set_rectangle_affine_rectangle_sum

// Library: DataStructure/Point_Rectangle_Affine_KD_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Point_Rectangle_Affine_KD_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_rectangle_affine_kd_tree.md
//
// API一覧
// PointRectangleAffineKDTree<Mod>(points): 点の座標を登録する。各点は非活性で始まる。
// set(id,w): 点を有効化して値をwにする。再代入可。O(log(M+1))。
// range_affine(rect,a,b): 半開長方形内の有効な点だけw←a*w+b。O(√M+1)。
// range_sum(rect): 半開長方形内の有効な点の和。O(√M+1)。空長方形は0。
// size()/active_count()/is_active(id)/all_sum(): 登録数、有効数、点の状態、全体和。各O(1)。
// IDは登録時の0始まり添字。同一座標も点ごとに扱う。後から有効化した点は過去の更新を継がない。
// 座標・引数の重みと係数はint64_t。Modは正のuint32_tで既定998244353。
// M点の登録は平均O(1+M log(M+1))時間、O(M+1)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_POINT_RECTANGLE_AFFINE_KD_TREE_DEFINED
#define SIRKIT_POINT_RECTANGLE_AFFINE_KD_TREE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPointRectangleAffineNodeCount(std::size_t count, std::size_t capacity)
{
    if (count == 0)
    {
        return 0;
    }
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        count > capacity / 2 + capacity % 2)
    {
        throw std::length_error("PointRectangleAffineKDTree: too many points");
    }
    return 2 * count - 1;
}
}

template <std::uint32_t Mod = 998244353>
class PointRectangleAffineKDTree
{
    static_assert(Mod > 0);

public:
    struct Point
    {
        std::int64_t x, y;
    };

    struct Rectangle
    {
        std::int64_t left, bottom, right, top;
    };

    explicit PointRectangleAffineKDTree(std::span<const Point> points = {})
    {
        nodes_.resize(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPointRectangleAffineNodeCount(points.size(), nodes_.max_size()));
        if (points.empty())
        {
            return;
        }
        leaf_.resize(points.size());
        std::vector<Candidate> candidates;
        candidates.reserve(points.size());
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            candidates.push_back({points[i], static_cast<Index>(i)});
        }
        Build(0, candidates, true);
    }

    PointRectangleAffineKDTree(const PointRectangleAffineKDTree &) = default;

    PointRectangleAffineKDTree &operator=(const PointRectangleAffineKDTree &other)
    {
        if (this != &other)
        {
            PointRectangleAffineKDTree next(other);
            nodes_.swap(next.nodes_);
            leaf_.swap(next.leaf_);
        }
        return *this;
    }

    PointRectangleAffineKDTree(PointRectangleAffineKDTree &&other) noexcept
        : nodes_(std::move(other.nodes_)), leaf_(std::move(other.leaf_))
    {
        other.nodes_.clear();
        other.leaf_.clear();
    }

    PointRectangleAffineKDTree &operator=(PointRectangleAffineKDTree &&other) noexcept
    {
        if (this != &other)
        {
            nodes_ = std::move(other.nodes_);
            leaf_ = std::move(other.leaf_);
            other.nodes_.clear();
            other.leaf_.clear();
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(leaf_.size());
    }

    [[nodiscard]] int active_count() const noexcept
    {
        return nodes_.empty() ? 0 : static_cast<int>(nodes_[0].count);
    }

    [[nodiscard]] std::uint32_t all_sum() const noexcept
    {
        return nodes_.empty() ? 0 : nodes_[0].sum;
    }

    [[nodiscard]] bool is_active(int id) const
    {
        CheckPoint(id);
        return nodes_[leaf_[id]].count != 0;
    }

    void set(int id, std::int64_t value)
    {
        CheckPoint(id);
        Set(0, leaf_[id], Normalize(value));
    }

    void range_affine(Rectangle rectangle, std::int64_t a, std::int64_t b)
    {
        if (!Nonempty(rectangle) || nodes_.empty())
        {
            return;
        }
        const auto multiplier = Normalize(a);
        const auto addition = Normalize(b);
        if (multiplier == kOne && addition == 0)
        {
            return;
        }
        Update(0, rectangle, multiplier, addition);
    }

    [[nodiscard]] std::uint32_t range_sum(Rectangle rectangle)
    {
        if (!Nonempty(rectangle) || nodes_.empty())
        {
            return 0;
        }
        return Sum(0, rectangle);
    }

private:
    using Index = std::uint32_t;
    static constexpr std::uint32_t kOne = 1 % Mod;

    struct Node
    {
        std::int64_t low_x = 0, high_x = 0, low_y = 0, high_y = 0;
        Index right = 0;
        std::uint32_t count = 0, sum = 0;
        std::uint32_t multiplier = kOne, addition = 0;
    };

    struct Candidate
    {
        Point point;
        Index id;
    };

    std::vector<Node> nodes_;
    std::vector<Index> leaf_;

    void CheckPoint(int id) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(id) >= leaf_.size())
        {
            throw std::out_of_range("PointRectangleAffineKDTree: invalid point ID");
        }
#else
        (void)id;
#endif
    }

    static bool Nonempty(Rectangle rect)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (rect.left > rect.right || rect.bottom > rect.top)
        {
            throw std::invalid_argument("PointRectangleAffineKDTree: reversed rectangle");
        }
#endif
        return rect.left < rect.right && rect.bottom < rect.top;
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b) noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Transform(std::uint32_t a, std::uint32_t b,
                                   std::uint32_t sum, std::uint32_t count) noexcept
    {
        const auto product = std::uint64_t{a} * sum;
        const auto offset = std::uint64_t{b} * count;
        if constexpr (Mod <= (std::uint32_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    static bool Disjoint(const Node &node, const Rectangle &rect) noexcept
    {
        return node.high_x < rect.left || node.low_x >= rect.right ||
               node.high_y < rect.bottom || node.low_y >= rect.top;
    }

    static bool Contained(const Node &node, const Rectangle &rect) noexcept
    {
        return rect.left <= node.low_x && node.high_x < rect.right &&
               rect.bottom <= node.low_y && node.high_y < rect.top;
    }

    void Build(Index index, std::span<Candidate> candidates, bool x_axis)
    {
        auto &node = nodes_[index];
        if (candidates.size() == 1)
        {
            const auto &[p, id] = candidates.front();
            node.low_x = node.high_x = p.x;
            node.low_y = node.high_y = p.y;
            leaf_[id] = index;
            return;
        }

        const auto half = candidates.size() / 2;
        std::nth_element(candidates.begin(), candidates.begin() + half, candidates.end(),
                         [x_axis](const Candidate &a, const Candidate &b)
                         {
                             if (x_axis)
                             {
                                 return std::tie(a.point.x, a.point.y, a.id) < std::tie(b.point.x, b.point.y, b.id);
                             }
                             return std::tie(a.point.y, a.point.x, a.id) < std::tie(b.point.y, b.point.x, b.id);
                         });

        // 部分木を前順で連続配置し、左の根と各点への経路を添字だけで復元する。
        node.right = static_cast<Index>(index + 2 * half);
        Build(index + 1, candidates.first(half), !x_axis);
        Build(node.right, candidates.subspan(half), !x_axis);
        const auto &left = nodes_[index + 1];
        const auto &right = nodes_[node.right];
        node.low_x = std::min(left.low_x, right.low_x);
        node.high_x = std::max(left.high_x, right.high_x);
        node.low_y = std::min(left.low_y, right.low_y);
        node.high_y = std::max(left.high_y, right.high_y);
    }

    void Apply(Node &node, std::uint32_t a, std::uint32_t b) noexcept
    {
        if (node.count == 0)
        {
            return;
        }
        node.sum = Transform(a, b, node.sum, node.count);
        if (node.right != 0)
        {
            node.multiplier = static_cast<std::uint32_t>(std::uint64_t{a} * node.multiplier % Mod);
            node.addition = Transform(a, b, node.addition, 1);
        }
    }

    void Push(Index index) noexcept
    {
        auto &node = nodes_[index];
        if (node.multiplier == kOne && node.addition == 0)
        {
            return;
        }
        Apply(nodes_[index + 1], node.multiplier, node.addition);
        Apply(nodes_[node.right], node.multiplier, node.addition);
        node.multiplier = kOne;
        node.addition = 0;
    }

    void Pull(Index index) noexcept
    {
        auto &node = nodes_[index];
        const auto &left = nodes_[index + 1];
        const auto &right = nodes_[node.right];
        node.count = left.count + right.count;
        node.sum = Add(left.sum, right.sum);
    }

    void Set(Index index, Index leaf, std::uint32_t value) noexcept
    {
        auto &node = nodes_[index];
        if (index == leaf)
        {
            node.count = 1;
            node.sum = value;
            return;
        }

        // 先に伝播してから有効点を増やす。その時点までの更新は新しい点に届かない。
        Push(index);
        Set(leaf < node.right ? index + 1 : node.right, leaf, value);
        Pull(index);
    }

    void Update(Index index, const Rectangle &rect, std::uint32_t a, std::uint32_t b) noexcept
    {
        auto &node = nodes_[index];
        if (node.count == 0 || Disjoint(node, rect))
        {
            return;
        }
        if (Contained(node, rect))
        {
            Apply(node, a, b);
            return;
        }
        Push(index);
        Update(index + 1, rect, a, b);
        Update(node.right, rect, a, b);
        Pull(index);
    }

    std::uint32_t Sum(Index index, const Rectangle &rect) noexcept
    {
        const auto &node = nodes_[index];
        if (node.count == 0 || Disjoint(node, rect))
        {
            return 0;
        }
        if (Contained(node, rect))
        {
            return node.sum;
        }
        Push(index);
        const auto left = Sum(index + 1, rect);
        return Add(left, Sum(node.right, rect));
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
#include <vector>

int main()
{
    using Tree = PointRectangleAffineKDTree<>;
    struct Operation
    {
        Tree::Rectangle rectangle{};
        int type = 0, id = 0;
        std::uint32_t a = 0, b = 0;
    };

    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Tree::Point> points;
    points.reserve(n + q);
    std::vector<std::uint32_t> weights(n);
    for (auto &weight : weights)
    {
        Tree::Point point{};
        verify_in >> point.x >> point.y >> weight;
        points.push_back(point);
    }
    std::vector<Operation> operations(q);
    for (auto &op : operations)
    {
        verify_in >> op.type;
        if (op.type == 0)
        {
            Tree::Point point{};
            verify_in >> point.x >> point.y >> op.a;
            op.id = static_cast<int>(points.size());
            points.push_back(point);
        }
        else if (op.type == 1)
        {
            verify_in >> op.id >> op.a;
        }
        else
        {
            auto &rect = op.rectangle;
            verify_in >> rect.left >> rect.bottom >> rect.right >> rect.top;
            if (op.type == 3)
            {
                verify_in >> op.a >> op.b;
            }
        }
    }

    Tree tree(points);
    for (int id = 0; id < n; ++id)
    {
        tree.set(id, weights[id]);
    }
    for (const auto &op : operations)
    {
        if (op.type <= 1)
        {
            tree.set(op.id, op.a);
        }
        else if (op.type == 2)
        {
            verify_out << tree.range_sum(op.rectangle) << '\n';
        }
        else
        {
            tree.range_affine(op.rectangle, op.a, op.b);
        }
    }
}
