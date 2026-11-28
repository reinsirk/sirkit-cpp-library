#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/lca

// Library: Graph/Linear_LCA.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（Euler Tourと±1 RMQ）: https://doi.org/10.1007/10719839_9
// 参考（AtCoder Algorithm Lectures）: https://info.atcoder.jp/entry/algorithm_lectures/linear_time_lca
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_lca.md
//
// API一覧
// LinearLCA(parents): 親配列から森を構築。根の親は-1。頂点は0始まり、親と子の番号順は任意。
//   Euler Tourの深さの差が±1になることを利用し、O(n+1) 時間・領域で前処理。再帰を使わない。
// get(u,v)/operator()(u,v): 最小共通祖先。別成分なら-1。
// depth_of(v): 成分根からの辺数。get_distance(u,v): 辺数の距離。別成分なら-1。
// size(): 頂点数。構築以外はすべて O(1)。n<=(INT_MAX-1)/2。空の森可。親番号の範囲外と閉路は例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LINEAR_LCA_DEFINED
#define SIRKIT_LINEAR_LCA_DEFINED

namespace detail
{
inline int checked_linear_lca_count(std::size_t count)
{
    constexpr auto kMaximum = static_cast<std::size_t>((std::numeric_limits<int>::max() - 1) / 2);
    if (count > kMaximum)
    {
        throw std::length_error("LinearLCA: too many vertices");
    }
    return static_cast<int>(count);
}

class LinearLcaRmq
{
public:
    void build(std::vector<int> depths)
    {
        depths_ = std::move(depths);
        const auto log = std::bit_width(static_cast<unsigned int>(depths_.size())) - 1U;
        block_size_ = std::max(1U, log / 2);
        const auto block_count = (depths_.size() - 1) / block_size_ + 1;
        signatures_.assign(block_count, 0);
        BuildMicroTable();

        std::vector<int> minima(block_count);
        for (std::size_t block = 0; block < block_count; ++block)
        {
            const auto start = block * block_size_;
            const auto limit = std::min(start + block_size_, depths_.size());
            unsigned int shape = 0;
            for (std::size_t i = start + 1; i < limit; ++i)
            {
                if (depths_[i - 1] < depths_[i])
                {
                    shape |= 1U << (i - start - 1);
                }
            }
            signatures_[block] = shape;
            minima[block] = WithinBlock(start, limit - 1);
        }
        sparse_.clear();
        sparse_.push_back(std::move(minima));
        for (std::size_t width = 2; width <= block_count; width *= 2)
        {
            const auto &previous = sparse_.back();
            const auto half = width / 2;
            std::vector<int> row(block_count - width + 1);
            for (std::size_t i = 0; i < row.size(); ++i)
            {
                row[i] = EarlierMinimum(previous[i], previous[i + half]);
            }
            sparse_.push_back(std::move(row));
        }
    }

    [[nodiscard]] int depth(std::size_t index) const
    {
        return depths_[index];
    }

    [[nodiscard]] int index(std::size_t left, std::size_t right) const
    {
        const auto first = left / block_size_, last = right / block_size_;
        if (first == last)
        {
            return WithinBlock(left, right);
        }
        const int tail = WithinBlock(left, (first + 1) * block_size_ - 1);
        const int head = WithinBlock(last * block_size_, right);
        const int border = EarlierMinimum(tail, head);
        if (last - first == 1)
        {
            return border;
        }
        const auto level = std::bit_width(last - first - 1) - 1U;
        const auto span = std::size_t{1} << level;
        const int inner = EarlierMinimum(sparse_[level][first + 1], sparse_[level][last - span]);
        return EarlierMinimum(border, inner);
    }

private:
    unsigned int block_size_ = 1;
    std::vector<int> depths_;
    std::vector<unsigned int> signatures_;
    std::vector<std::uint8_t> micro_;
    std::vector<std::vector<int>> sparse_;

    [[nodiscard]] std::size_t Offset(unsigned int signature, std::size_t left, std::size_t right) const
    {
        return (static_cast<std::size_t>(signature) * block_size_ + left) * block_size_ + right;
    }

    void BuildMicroTable()
    {
        const auto shapes = 1U << (block_size_ - 1);
        micro_.resize(static_cast<std::size_t>(shapes) * block_size_ * block_size_);
        for (unsigned int shape = 0; shape < shapes; ++shape)
        {
            for (unsigned int left = 0; left < block_size_; ++left)
            {
                int current_depth = 0, minimum_depth = 0;
                auto best = left;
                micro_[Offset(shape, left, left)] = static_cast<std::uint8_t>(best);
                for (unsigned int right = left + 1; right < block_size_; ++right)
                {
                    current_depth += ((shape >> (right - 1)) & 1U) != 0 ? 1 : -1;
                    if (current_depth < minimum_depth)
                    {
                        minimum_depth = current_depth;
                        best = right;
                    }
                    micro_[Offset(shape, left, right)] = static_cast<std::uint8_t>(best);
                }
            }
        }
    }

    [[nodiscard]] int WithinBlock(std::size_t left, std::size_t right) const
    {
        const auto first = left / block_size_;
        const auto offset = Offset(signatures_[first], left % block_size_, right % block_size_);
        return static_cast<int>(first * block_size_ + micro_[offset]);
    }

    [[nodiscard]] int EarlierMinimum(int first, int second) const
    {
        return std::pair{depths_[first], first} <= std::pair{depths_[second], second} ? first : second;
    }
};
} // namespace detail

class LinearLCA
{
public:
    explicit LinearLCA(std::span<const int> parents)
    {
        const int n = detail::checked_linear_lca_count(parents.size());
        std::vector<int> child(static_cast<std::size_t>(n) + 1, -1), sibling(n, -1);
        for (int vertex = n; vertex > 0;)
        {
            --vertex;
            const int parent = parents[vertex];
            if (parent < -1 || parent >= n)
            {
                throw std::out_of_range("LinearLCA: invalid parent");
            }
            const int owner = parent == -1 ? n : parent;
            sibling[vertex] = child[owner];
            child[owner] = vertex;
        }
        const auto expected = static_cast<std::size_t>(2 * n + 1);
        std::vector<int> pending{n}, depths{0};
        pending.reserve(parents.size() + 1);
        depths.reserve(expected);
        tour_.reserve(expected);
        tour_.push_back(-1);
        first_.resize(n);

        while (!pending.empty())
        {
            const int vertex = pending.back();
            const int next = child[vertex];
            if (next != -1)
            {
                child[vertex] = sibling[next];
                first_[next] = static_cast<int>(tour_.size());
                tour_.push_back(next);
                depths.push_back(static_cast<int>(pending.size()));
                pending.push_back(next);
            }
            else
            {
                pending.pop_back();
                if (!pending.empty())
                {
                    const int previous = pending.back();
                    tour_.push_back(previous == n ? -1 : previous);
                    depths.push_back(static_cast<int>(pending.size() - 1));
                }
            }
        }
        if (tour_.size() != expected)
        {
            throw std::invalid_argument("LinearLCA: cycle in parent input");
        }
        rmq_.build(std::move(depths));
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(first_.size());
    }

    [[nodiscard]] int get(int u, int v) const
    {
        CheckVertex(u);
        CheckVertex(v);
        const auto [left, right] = std::minmax(first_[u], first_[v]);
        return tour_[rmq_.index(left, right)];
    }

    [[nodiscard]] int operator()(int u, int v) const
    {
        return get(u, v);
    }

    [[nodiscard]] int depth_of(int vertex) const
    {
        CheckVertex(vertex);
        return rmq_.depth(first_[vertex]) - 1;
    }

    [[nodiscard]] int get_distance(int u, int v) const
    {
        const int ancestor = get(u, v);
        if (ancestor == -1)
        {
            return -1;
        }
        const int depth = depth_of(ancestor);
        return (depth_of(u) - depth) + (depth_of(v) - depth);
    }

private:
    detail::LinearLcaRmq rmq_;
    std::vector<int> tour_;
    std::vector<int> first_;

    void CheckVertex(int vertex) const
    {
        if (static_cast<std::size_t>(vertex) >= first_.size())
        {
            throw std::out_of_range("LinearLCA: vertex");
        }
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
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<int> parents(n, -1);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parents[v];
    }
    const sirkit::LinearLCA lca(parents);
    for (int i = 0; i < q; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << lca.get(u, v) << '\n';
    }
}
