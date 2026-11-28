#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc202/tasks/abc202_e

// Library: Graph/DSU_On_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/DSU_On_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dsu_on_tree.md
//
// API一覧
// DsuOnTree(parents): 親配列の森を前処理。頂点は0始まり。根の親は-1。O(n)時間・領域。
// DsuOnTree(intervals): 構築済みのSubtreeIntervalsを共有して前処理。O(n)時間・追加領域。
// size(): 頂点数。O(1)。
// run(add,remove,answer): answer(v)でvの部分木全体が登録されるようにadd(u), remove(u)を呼ぶ。
//   呼び出し前後の集合は空。頂点ごとにanswerを1回呼ぶ。走査順は規定しない。
//   add/removeは逆操作。コールバックがO(1)なら O(n log(n+1)) 時間、O(1) 追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBTREE_INTERVALS_DEFINED
#define SIRKIT_SUBTREE_INTERVALS_DEFINED

namespace detail
{
inline int checked_subtree_interval_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("SubtreeIntervals: too many vertices");
    }
    return static_cast<int>(size);
}
}

class SubtreeIntervals
{
public:
    explicit SubtreeIntervals(std::span<const int> parents)
        : data_(std::make_shared<const Data>(parents))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(order().size());
    }

    [[nodiscard]] std::span<const int> order() const noexcept
    {
        return data_ ? std::span<const int>(data_->order) : std::span<const int>{};
    }

    [[nodiscard]] int index(int vertex) const
    {
        CheckIndex(vertex);
        return data_->first[vertex];
    }

    [[nodiscard]] std::pair<int, int> range(int vertex) const
    {
        const auto start = index(vertex);
        return {start, start + data_->count[vertex]};
    }

    [[nodiscard]] bool contains(int ancestor, int vertex) const
    {
        const auto [left, right] = range(ancestor);
        const auto at = index(vertex);
        return left <= at && at < right;
    }

    [[nodiscard]] int vertex_at(int position) const
    {
        CheckIndex(position);
        return data_->order[position];
    }

private:
    struct Data
    {
        std::vector<int> order;
        std::vector<int> first;
        std::vector<int> count;

        explicit Data(std::span<const int> parents)
        {
            const auto n = detail::checked_subtree_interval_size(parents.size());
            std::vector<int> child(n, -1), next(n, -1), work;
            work.reserve(n);
            for (int v = 0; v < n; ++v)
            {
                const int p = parents[v];
                if (p == -1)
                {
                    work.push_back(v);
                }
                else
                {
                    if (p < 0 || p >= n)
                    {
                        throw std::out_of_range("SubtreeIntervals: parent");
                    }
                    next[v] = child[p];
                    child[p] = v;
                }
            }

            first.resize(n);
            order.reserve(n);
            std::reverse(work.begin(), work.end());
            while (!work.empty())
            {
                const int v = work.back();
                work.pop_back();
                first[v] = static_cast<int>(order.size());
                order.push_back(v);
                for (int w = child[v]; w != -1; w = next[w])
                {
                    work.push_back(w);
                }
            }
            if (order.size() != parents.size())
            {
                throw std::invalid_argument("SubtreeIntervals: cycle");
            }

            count.assign(n, 1);
            for (auto current = order.rbegin(); current != order.rend(); ++current)
            {
                const int parent = parents[*current];
                if (parent != -1)
                {
                    count[parent] += count[*current];
                }
            }
        }
    };

    std::shared_ptr<const Data> data_;

    void CheckIndex([[maybe_unused]] int index) const
    {
    }
};
#endif

#ifndef SIRKIT_DSU_ON_TREE_DEFINED
#define SIRKIT_DSU_ON_TREE_DEFINED

class DsuOnTree
{
public:
    explicit DsuOnTree(std::span<const int> parents)
        : DsuOnTree(SubtreeIntervals(parents))
    {
    }

    explicit DsuOnTree(SubtreeIntervals intervals)
        : intervals_(std::move(intervals)),
          heavy_(intervals_.size(), -1),
          keep_(intervals_.size())
    {
        const auto vertices = intervals_.order();
        schedule_.reserve(vertices.size());
        std::vector<int> pending;
        for (int i = 0; i < size();)
        {
            const int root = vertices[i];
            pending.push_back(root);
            i = intervals_.range(root).second;
        }
        while (!pending.empty())
        {
            const int v = pending.back();
            pending.pop_back();
            schedule_.push_back(v);
            const auto [first, last] = intervals_.range(v);
            int largest = 0;
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                const int end = intervals_.range(child).second;
                if (end - i > largest)
                {
                    largest = end - i;
                    heavy_[v] = child;
                }
                i = end;
            }
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                if (child != heavy_[v])
                {
                    pending.push_back(child);
                }
                i = intervals_.range(child).second;
            }
            if (const int child = heavy_[v]; child != -1)
            {
                keep_[child] = true;
                pending.push_back(child);
            }
        }
        std::ranges::reverse(schedule_);
    }

    [[nodiscard]] int size() const noexcept
    {
        return intervals_.size();
    }

    template <typename Add, typename Remove, typename Answer>
    void run(Add &&add, Remove &&remove, Answer &&answer) const
    {
        const auto vertices = intervals_.order();
        for (int vertex : schedule_)
        {
            const auto [first, last] = intervals_.range(vertex);
            if (const int heavy = heavy_[vertex]; heavy != -1)
            {
                const auto [begin, end] = intervals_.range(heavy);
                for (int i = first; i < begin; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
                for (int i = end; i < last; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
            }
            else
            {
                std::invoke(add, vertex);
            }
            std::invoke(answer, vertex);
            if (!keep_[vertex])
            {
                for (int i = first; i < last; ++i)
                {
                    std::invoke(remove, vertices[i]);
                }
            }
        }
    }

private:
    SubtreeIntervals intervals_;
    std::vector<int> heavy_;
    std::vector<bool> keep_;
    std::vector<int> schedule_;
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


#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> parent(n, -1), depth(n);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parent[v];
        --parent[v];
        depth[v] = 1 + depth[parent[v]];
    }
    int q{};
    verify_in >> q;
    std::vector<std::vector<std::pair<int, int>>> queries(n);
    for (int i = 0; i < q; ++i)
    {
        int vertex{}, target_depth{};
        verify_in >> vertex >> target_depth;
        queries[vertex - 1].emplace_back(target_depth, i);
    }
    const sirkit::DsuOnTree tree(parent);
    std::vector<int> counts(n), answers(q);
    tree.run(
        [&](int v) { ++counts[depth[v]]; },
        [&](int v) { --counts[depth[v]]; },
        [&](int v)
        {
            for (const auto &[d, id] : queries[v])
            {
                answers[id] = counts[d];
            }
        });
    for (const int answer : answers)
    {
        verify_out << answer << '\n';
    }
}
