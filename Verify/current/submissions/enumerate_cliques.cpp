#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/enumerate_cliques
// Submission: https://judge.yosupo.jp/submission/419843

// Library: Graph/Enumerate_Cliques.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（線形時間のコア分解）: https://arxiv.org/abs/cs/0310049
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/enumerate_cliques.md
//
// API一覧
// enumerate_cliques(n,edges,visit): 単純無向グラフの非空クリークごとにvisit(span<const int>)を1回呼ぶ。
// 頂点番号は[0,n)、自己辺・重複辺は不可。孤立頂点も1頂点のクリークとして列挙する。
// 列挙順と頂点順は任意。spanは呼び出し中だけ有効。
// 退化度d、クリーク数Cに対し、コールバックを除きO(n+m+(d+1)C)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ENUMERATE_CLIQUES_DEFINED
#define SIRKIT_ENUMERATE_CLIQUES_DEFINED

namespace detail
{
struct CliqueOrder
{
    std::vector<int> vertices;
    std::vector<std::vector<int>> forward;
    std::size_t max_forward = 0;
};

inline CliqueOrder make_clique_order(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("enumerate_cliques: negative vertex count");
    }
    const auto count = static_cast<std::size_t>(n);
    CliqueOrder result{std::vector<int>(count), std::vector<std::vector<int>>(count)};
    auto &order = result.vertices;
    std::vector<int> position(count);
    std::vector<std::vector<int>> adjacency(count);
    for (const auto &[from, to] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("enumerate_cliques: endpoint out of range");
        }
        if (from == to)
        {
            throw std::invalid_argument("enumerate_cliques: self loop");
        }
#endif
        adjacency[from].push_back(to);
        adjacency[to].push_back(from);
    }

    std::vector<std::size_t> degree(count), bucket(count + 1);
    for (int v = 0; v < n; ++v)
    {
        const auto size = adjacency[v].size();
#ifndef SKIP_RUNTIME_CHECKS
        if (size >= count)
        {
            throw std::invalid_argument("enumerate_cliques: degree exceeds simple graph bound");
        }
#endif
        degree[v] = size;
        ++bucket[size];
    }
    std::size_t start = 0;
    for (auto &slot : bucket)
    {
        const auto frequency = slot;
        slot = start;
        start += frequency;
    }
    for (int v = 0; v < n; ++v)
    {
        const auto rank = bucket[degree[v]]++;
        position[v] = static_cast<int>(rank);
        order[rank] = v;
    }
    for (std::size_t d = count; d != 0; --d)
    {
        bucket[d] = bucket[d - 1];
    }
    bucket[0] = 0;

#ifndef SKIP_RUNTIME_CHECKS
    std::vector<int> last_neighbor_of(count, -1);
#endif
    // 次数の大きい隣接頂点を、一つ前のバケットへ移す。
    for (int v : order)
    {
        for (int to : adjacency[v])
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (last_neighbor_of[to] == v)
            {
                throw std::invalid_argument("enumerate_cliques: duplicate edge");
            }
            last_neighbor_of[to] = v;
#endif
            const auto d = degree[to];
            if (d > degree[v])
            {
                const auto first = bucket[d]++;
                const int other = order[first];
                std::swap(order[position[to]], order[first]);
                std::swap(position[to], position[other]);
                --degree[to];
            }
        }
    }

    // 行き先の順位が昇順になるよう、後ろ側の頂点から辺を追加する。
    for (int rank = 0; rank < n; ++rank)
    {
        for (int from : adjacency[order[rank]])
        {
            const int earlier = position[from];
            if (earlier < rank)
            {
                result.forward[earlier].push_back(rank);
                result.max_forward = std::max(result.max_forward, result.forward[earlier].size());
            }
        }
    }
    return result;
}
} // namespace detail

template <class Visit>
void enumerate_cliques(int n, std::span<const std::pair<int, int>> edges, Visit &&visit)
{
    const auto [order, forward, max_forward] = detail::make_clique_order(n, edges);
    std::vector<std::vector<int>> buffers(max_forward + 1);
    std::vector<int> clique;
    clique.reserve(max_forward + 1);
    const auto search = [&](auto &&self, int rank, std::span<const int> candidates) -> void
    {
        clique.push_back(order[rank]);
        visit(std::span<const int>(clique));
        auto &next = buffers[clique.size() - 1];
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            const int to = candidates[i];
            next.clear();
            std::ranges::set_intersection(candidates.subspan(i + 1), forward[to], std::back_inserter(next));
            self(self, to, next);
        }
        clique.pop_back();
    };
    for (int rank = 0; rank < n; ++rank)
    {
        search(search, rank, forward[rank]);
    }
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
#include <span>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::uint32_t> weight(n);
    for (auto &x : weight)
    {
        verify_in >> x;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[from, to] : edges)
    {
        verify_in >> from >> to;
    }

    constexpr std::uint64_t kMod = 998244353;
    std::uint64_t answer = 0;
    const auto visit = [&](std::span<const int> clique)
    {
        std::uint64_t product = 1;
        for (int v : clique)
        {
            product = product * weight[v] % kMod;
        }
        answer += product;
        if (answer >= kMod)
        {
            answer -= kMod;
        }
    };
    enumerate_cliques(n, edges, visit);
    verify_out << answer << '\n';
}
