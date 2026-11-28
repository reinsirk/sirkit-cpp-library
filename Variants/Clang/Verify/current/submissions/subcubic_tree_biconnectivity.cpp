#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/arc160/tasks/arc160_e

// Library: Graph/Subcubic_Tree_Biconnectivity.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Subcubic_Tree_Biconnectivity.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subcubic_tree_biconnectivity.md
//
// API一覧
// augment_subcubic_tree_biconnectivity(weights, edges):
//   各頂点の次数が3以下の木に、追加後にどの1頂点を削除しても連結な辺集合を構成する。
//   n>=3、頂点番号は0始まり。辺(u,v)の加算費用weights[u]+weights[v]の総和を最小化。
//   O(n)時間・領域。頂点重みは非負のuint64_t。
//   各枝の葉は巡回順で連続し、半周先への辺で枝どうしをつなぐ。
//   奇数葉では追加頂点を根とし、反対側が単独葉の枝にならない枝の境界に根を挿す。
// BiconnectivityAugmentation{edges,cost}: 追加する辺と最小費用。費用が表現範囲外ならcostはnullopt。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBCUBIC_TREE_BICONNECTIVITY_DEFINED
#define SIRKIT_SUBCUBIC_TREE_BICONNECTIVITY_DEFINED

struct BiconnectivityAugmentation
{
    std::vector<std::pair<int, int>> edges;
    std::optional<std::uint64_t> cost;
};

namespace detail
{
inline void check_subcubic_tree_size(std::size_t vertices, std::size_t edges)
{
    if (vertices < 3 || vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        edges != vertices - 1)
    {
        throw std::length_error("augment_subcubic_tree_biconnectivity: invalid tree size");
    }
}
} // namespace detail

[[nodiscard]] inline BiconnectivityAugmentation augment_subcubic_tree_biconnectivity(
    std::span<const std::uint64_t> weights, std::span<const std::pair<int, int>> edges)
{
    detail::check_subcubic_tree_size(weights.size(), edges.size());
    const int n = static_cast<int>(weights.size());
    std::vector<std::array<int, 3>> neighbors(n);
    std::vector<int> degree(n);
    auto add = [&](int u, int v)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("augment_subcubic_tree_biconnectivity: invalid vertex");
        }
#endif
        if (degree[u] == 3)
        {
            throw std::invalid_argument("augment_subcubic_tree_biconnectivity: degree above three");
        }
        neighbors[u][degree[u]++] = v;
    };
    for (const auto &[u, v] : edges)
    {
        add(u, v);
        add(v, u);
    }

    int leaf_count = 0;
    for (const int d : degree)
    {
        leaf_count += d == 1;
    }
    int extra = -1;
    if (leaf_count % 2 != 0)
    {
        for (int v = 0; v < n; ++v)
        {
            // 三叉の木は中心を失うと、中心に接続した辺を使わず3枝をつなぐ必要がある。
            if (leaf_count == 3 && degree[v] == 3)
            {
                continue;
            }
            if (extra == -1 || weights[v] < weights[extra])
            {
                extra = v;
            }
        }
    }

    const int root = extra == -1 ? 0 : extra;
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(leaf_count) + 1);
    if (degree[root] == 1)
    {
        order.push_back(root);
    }
    std::array<std::size_t, 4> begin{};
    std::vector<std::pair<int, int>> pending;
#ifndef SKIP_RUNTIME_CHECKS
    std::vector<bool> seen(n);
    seen[root] = true;
    int visited = 1;
#endif
    for (int branch = 0; branch < degree[root]; ++branch)
    {
        begin[branch] = order.size();
        pending.emplace_back(neighbors[root][branch], root);
        while (!pending.empty())
        {
            const auto [v, previous] = pending.back();
            pending.pop_back();
#ifndef SKIP_RUNTIME_CHECKS
            if (seen[v])
            {
                throw std::invalid_argument("augment_subcubic_tree_biconnectivity: cycle");
            }
            seen[v] = true;
            ++visited;
#endif
            if (degree[v] == 1)
            {
                order.push_back(v);
            }
            for (int i = 0; i < degree[v]; ++i)
            {
                const int u = neighbors[v][i];
                if (u != previous)
                {
                    pending.emplace_back(u, v);
                }
            }
        }
    }
    begin[degree[root]] = order.size();
#ifndef SKIP_RUNTIME_CHECKS
    if (visited != n)
    {
        throw std::invalid_argument("augment_subcubic_tree_biconnectivity: disconnected");
    }
#endif
    if (extra != -1)
    {
        std::size_t insertion = 0;
        if (degree[root] != 1)
        {
            for (int corner = 0; corner < degree[root]; ++corner)
            {
                const auto at = begin[corner];
                const auto opposite = (at + order.size() / 2) % order.size();
                bool singleton = false;
                for (int branch = 0; branch < degree[root]; ++branch)
                {
                    if (begin[branch] == opposite && begin[branch + 1] == opposite + 1)
                    {
                        singleton = true;
                        break;
                    }
                }
                if (!singleton)
                {
                    insertion = at;
                }
            }
        }
        order.insert(order.begin() + static_cast<std::ptrdiff_t>(insertion), extra);
    }

    BiconnectivityAugmentation result{{}, 0};
    const auto half = order.size() / 2;
    result.edges.reserve(half);
    for (std::size_t i = 0; i < half; ++i)
    {
        result.edges.emplace_back(order[i], order[i + half]);
    }
    for (const int v : order)
    {
        if (!result.cost)
        {
            break;
        }
        const auto weight = weights[v];
        if (weight > std::numeric_limits<std::uint64_t>::max() - *result.cost)
        {
            result.cost.reset();
        }
        else
        {
            *result.cost += weight;
        }
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
#include <utility>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    while (tests-- != 0)
    {
        int n{};
        verify_in >> n;
        std::vector<std::uint64_t> weights(n);
        for (auto &weight : weights)
        {
            verify_in >> weight;
        }
        std::vector<std::pair<int, int>> edges(n - 1);
        for (auto &[u, v] : edges)
        {
            verify_in >> u >> v;
            --u;
            --v;
        }
        const auto extra = sirkit::augment_subcubic_tree_biconnectivity(weights, edges);
        verify_out << extra.edges.size() << '\n';
        for (const auto &[u, v] : extra.edges)
        {
            verify_out << u + 1 << ' ' << v + 1 << '\n';
        }
    }
}
