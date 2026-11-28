#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/line_add_get_min

// Library: DataStructure/Persistent_Li_Chao_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Persistent_Li_Chao_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_li_chao_tree.md
//
// API一覧
// PersistentLiChaoTree<T>(coordinates): 照会するx座標を登録。重複はまとめる。Tは符号付き整数。
// Version{}: 直線を含まない初期版。各版は作成した木で使う。
// add_line(version,line): 直線を追加した版を作る。元の版も使える。償却O(log(n+1))時間・追加領域。
// add_segment(version,l,r,line): l<=x<rに直線を追加した版。l<=r。償却O(log²(n+1))時間・追加領域。
// get(version,x): xでの最小値をO(log(n+1))で返す。登録外のx、または有効な直線がない場合はnullopt。
// Line{slope,intercept}: y=slope*x+intercept。計算途中もTに収まること。nは登録した異なる座標数。
// 構築は座標の入力数をmとしてO(m log(m+1))時間、O(m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_LI_CHAO_TREE_DEFINED
#define SIRKIT_PERSISTENT_LI_CHAO_TREE_DEFINED

template <std::signed_integral T = std::int64_t>
class PersistentLiChaoTree
{
    using Index = std::size_t;

public:
    struct Line
    {
        T slope;
        T intercept;
    };

    class Version
    {
        friend class PersistentLiChaoTree;

    public:
        Version() = default;

    private:
        explicit Version(Index root) : root_(root)
        {
        }

        Index root_ = 0;
    };

    explicit PersistentLiChaoTree(std::vector<T> coordinates) : xs_(std::move(coordinates))
    {
        std::sort(xs_.begin(), xs_.end());
        xs_.erase(std::unique(xs_.begin(), xs_.end()), xs_.end());
    }

    PersistentLiChaoTree(const PersistentLiChaoTree &) = default;
    PersistentLiChaoTree(PersistentLiChaoTree &&) noexcept = default;

    PersistentLiChaoTree &operator=(PersistentLiChaoTree other) noexcept
    {
        xs_.swap(other.xs_);
        nodes_.swap(other.nodes_);
        lines_.swap(other.lines_);
        return *this;
    }

    Version add_line(Version version, Line line)
    {
        CheckVersion(version);
        return Update(version, 0, xs_.size(), line);
    }

    Version add_segment(Version version, T left, T right, Line line)
    {
        CheckVersion(version);
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right)
        {
            throw std::invalid_argument("PersistentLiChaoTree: reversed segment");
        }
#endif
        return Update(version, LowerBound(left), LowerBound(right), line);
    }

    std::optional<T> get(Version version, T x) const
    {
        CheckVersion(version);
        const Index position = LowerBound(x);
        if (position == xs_.size() || xs_[position] != x)
        {
            return std::nullopt;
        }
        Index from = 0;
        Index to = xs_.size();
        Index node = version.root_;
        std::optional<T> best;
        while (node)
        {
            const auto &current = Get(node);
            if (current.line)
            {
                const T y = Evaluate(current.line, x);
                if (!best || y < *best)
                {
                    best = y;
                }
            }
            const Index mid = from + (to - from) / 2;
            if (position < mid)
            {
                node = current.left;
                to = mid;
            }
            else
            {
                node = current.right;
                from = mid;
            }
        }
        return best;
    }

private:
    struct Node
    {
        Index line = 0;
        Index left = 0;
        Index right = 0;
    };

    static constexpr Node kEmpty{};
    std::vector<T> xs_;
    std::vector<Node> nodes_;
    std::vector<Line> lines_;

    const Node &Get(Index id) const
    {
        return id == 0 ? kEmpty : nodes_[id - 1];
    }

    T Evaluate(Index line, T x) const
    {
        const auto &[slope, intercept] = lines_[line - 1];
        return static_cast<T>(slope * x + intercept);
    }

    Index LowerBound(T x) const
    {
        return static_cast<Index>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
    }

    Index Store(Node node)
    {
        nodes_.push_back(node);
        return nodes_.size();
    }

    Index InsertLine(Index id, Index candidate, Index from, Index to)
    {
        Node next = Get(id);
        if (next.line == 0)
        {
            next.line = candidate;
            return Store(next);
        }
        const auto strictly_better = [&](Index at)
        {
            return Evaluate(candidate, xs_[at]) < Evaluate(next.line, xs_[at]);
        };
        const bool wins_left = strictly_better(from);
        const bool wins_right = strictly_better(to - 1);
        if (!wins_left && !wins_right)
        {
            return id;
        }
        if (wins_left && wins_right)
        {
            next.line = candidate;
            return Store(next);
        }
        const Index mid = from + (to - from) / 2;
        const bool wins_mid = strictly_better(mid);
        if (wins_mid)
        {
            std::swap(next.line, candidate);
        }
        if (wins_left != wins_mid)
        {
            next.left = InsertLine(next.left, candidate, from, mid);
        }
        else
        {
            next.right = InsertLine(next.right, candidate, mid, to);
        }
        return Store(next);
    }

    Index InsertSegment(Index id, Index line, Index from, Index to, Index first, Index last)
    {
        if (first <= from && to <= last)
        {
            return InsertLine(id, line, from, to);
        }
        const Node old = Get(id);
        Node next = old;
        const Index mid = from + (to - from) / 2;
        if (first < mid)
        {
            next.left = InsertSegment(old.left, line, from, mid, first, last);
        }
        if (last > mid)
        {
            next.right = InsertSegment(old.right, line, mid, to, first, last);
        }
        if (next.left == old.left && next.right == old.right)
        {
            return id;
        }
        return Store(next);
    }

    Version Update(Version version, Index first, Index last, Line line)
    {
        if (first == last)
        {
            return version;
        }
        const Index depth = std::bit_width(xs_.size() - 1) + 1;
        const Index maximum_new_nodes = first == 0 && last == xs_.size() ? depth : 2 * depth * (depth + 1);
        const Index required = nodes_.size() + maximum_new_nodes;
        const Index capacity = nodes_.capacity();
        if (required > capacity)
        {
            nodes_.reserve(std::max(required, std::min(nodes_.max_size(), capacity + capacity / 2)));
        }
        lines_.push_back(line);
        const Index root = InsertSegment(version.root_, lines_.size(), 0, xs_.size(), first, last);
        if (root == version.root_)
        {
            lines_.pop_back();
        }
        return Version(root);
    }

    void CheckVersion([[maybe_unused]] Version version) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (version.root_ > nodes_.size())
        {
            throw std::out_of_range("PersistentLiChaoTree: invalid version");
        }
#endif
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

// Verify support: Verify/current/common/persistent_li_chao_verify.hpp
// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <utility>
#include <vector>

template <bool Segments>
int VerifyPersistentLiChao()
{
    using T = std::int64_t;
    using Tree = sirkit::PersistentLiChaoTree<T>;
    struct Operation
    {
        Tree::Line line{};
        T first{};
        T last{};
        bool query = false;
    };
    const auto read_line = [](Operation &op)
    {
        if constexpr (Segments)
        {
            verify_in >> op.first >> op.last;
        }
        verify_in >> op.line.slope >> op.line.intercept;
    };

    int n, q;
    verify_in >> n >> q;
    std::vector<Operation> operations(static_cast<std::size_t>(n) + q);
    for (int i = 0; i < n; ++i)
    {
        read_line(operations[i]);
    }
    std::vector<T> coordinates;
    coordinates.reserve(q);
    for (std::size_t i = n; i < operations.size(); ++i)
    {
        int type;
        verify_in >> type;
        auto &op = operations[i];
        if (type == 0)
        {
            read_line(op);
        }
        else
        {
            op.query = true;
            verify_in >> op.first;
            coordinates.push_back(op.first);
        }
    }

    Tree tree(std::move(coordinates));
    Tree::Version current;
    for (const auto &op : operations)
    {
        if (!op.query)
        {
            if constexpr (Segments)
            {
                current = tree.add_segment(current, op.first, op.last, op.line);
            }
            else
            {
                current = tree.add_line(current, op.line);
            }
        }
        else if (const auto result = tree.get(current, op.first))
        {
            verify_out << *result << '\n';
        }
        else
        {
            verify_out << "INFINITY\n";
        }
    }
    return 0;
}


int main()
{
    return VerifyPersistentLiChao<false>();
}
