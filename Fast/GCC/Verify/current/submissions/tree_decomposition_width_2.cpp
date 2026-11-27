#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/tree_decomposition_width_2

// Library: Graph/Tree_Decomposition_Width_Two.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Decomposition_Width_Two.cpp
// 木分解の定義: https://webspace.science.uu.nl/~bodla101/dyntw2.pdf
// ハッシュの64ビット変換: https://prng.di.unimi.it/splitmix64.c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_decomposition_width_two.md
//
// API一覧
// tree_decomposition_width_two(n, edges): 0始まりの無向グラフを幅2以下で木分解する。木幅3以上ならnullopt。
// WidthTwoTreeDecomposition: bagsは袋の列、tree_edgesは袋番号（0始まり）による木の辺、widthは得られた分解の幅。
// WidthTwoBag: verticesの先頭size個（1～3個）が袋の頂点。空グラフでは袋も木の辺も空でwidthは0。
// 多重辺と自己辺を許す。非連結でも袋は1本の木にまとめる。平均O(n+m)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_DECOMPOSITION_WIDTH_TWO_DEFINED
#define SIRKIT_TREE_DECOMPOSITION_WIDTH_TWO_DEFINED

struct WidthTwoBag
{
    std::array<int, 3> vertices{};
    int size = 0;
};

struct WidthTwoTreeDecomposition
{
    std::vector<WidthTwoBag> bags;
    std::vector<std::pair<int, int>> tree_edges;
    int width = 0;
};

namespace detail
{
inline constexpr std::size_t kWidthTwoNoArc = std::numeric_limits<std::size_t>::max();

struct WidthTwoArc
{
    std::size_t next;
    int to;
};

struct WidthTwoEdgeHash
{
    std::size_t operator()(std::uint64_t key) const noexcept
    {
        key ^= key >> 30;
        key *= 0xbf58476d1ce4e5b9ULL;
        key ^= key >> 27;
        key *= 0x94d049bb133111ebULL;
        return static_cast<std::size_t>(key ^ (key >> 31));
    }
};

class WidthTwoGraph
{
    std::vector<std::size_t> heads_;
    std::vector<WidthTwoArc> arcs_;
    std::unordered_set<std::uint64_t, WidthTwoEdgeHash> edges_;

    void link(int from, int to)
    {
        const auto next = heads_[from];
        heads_[from] = arcs_.size();
        arcs_.push_back({next, to});
    }

public:
    WidthTwoGraph(int n, std::size_t m) : heads_(n, kWidthTwoNoArc)
    {
        arcs_.reserve(2 * m);
        edges_.reserve(m);
    }

    bool add(int u, int v)
    {
        const auto [first, second] = std::minmax(u, v);
        const auto key = (static_cast<std::uint64_t>(first) << 32) | static_cast<std::uint32_t>(second);
        if (!edges_.insert(key).second)
        {
            return false;
        }
        link(u, v);
        link(v, u);
        return true;
    }

    WidthTwoBag bag(int v, std::span<const int> degree) const
    {
        WidthTwoBag result{{v, 0, 0}, 1};
        for (auto i = heads_[v]; i != kWidthTwoNoArc; i = arcs_[i].next)
        {
            const int to = arcs_[i].to;
            if (degree[to] >= 0)
            {
                result.vertices[result.size++] = to;
            }
        }
        return result;
    }
};
} // namespace detail

inline std::optional<WidthTwoTreeDecomposition> tree_decomposition_width_two(
    int n, std::span<const std::pair<int, int>> edges)
{
    detail::WidthTwoGraph graph(n, edges.size());
    std::vector<int> degree(n);
    for (const auto &[u, v] : edges)
    {
        if (u != v && graph.add(u, v))
        {
            ++degree[u];
            ++degree[v];
        }
    }

    std::vector<int> ready;
    ready.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (degree[v] <= 2)
        {
            ready.push_back(v);
        }
    }

    WidthTwoTreeDecomposition result;
    result.bags.reserve(n);
    std::vector<int> position(n);
    for (std::size_t i = 0; i < ready.size(); ++i)
    {
        const int v = ready[i];
        const auto bag = graph.bag(v, degree);
        position[v] = static_cast<int>(i);
        degree[v] = -1;
        result.bags.push_back(bag);
        result.width = std::max(result.width, bag.size - 1);

        // 次数2の頂点を消すときに隣接2頂点を結ぶと、新しい補辺がある場合は両端の次数が変わらない。
        if (bag.size == 3 && graph.add(bag.vertices[1], bag.vertices[2]))
        {
            continue;
        }
        for (int j = 1; j < bag.size; ++j)
        {
            const int to = bag.vertices[j];
            if (--degree[to] == 2)
            {
                ready.push_back(to);
            }
        }
    }
    if (ready.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }

    if (n > 0)
    {
        result.tree_edges.reserve(n - 1);
    }
    for (int i = 0; i < n - 1; ++i)
    {
        const auto &bag = result.bags[i];
        int parent = n - 1;
        // 残りの頂点を最初に消す袋に結ぶ。成分内で最後の袋は木全体の根に結ぶ。
        for (int j = 1; j < bag.size; ++j)
        {
            parent = std::min(parent, position[bag.vertices[j]]);
        }
        result.tree_edges.emplace_back(i, parent);
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


#include <utility>
#include <vector>

int main()
{
    char p, t, w;
    int n, m;
    verify_in >> p >> t >> w >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }

    const auto result = tree_decomposition_width_two(n, edges);
    if (!result)
    {
        verify_out << "-1\n";
        return 0;
    }

    verify_out << "s td " << result->bags.size() << ' ' << result->width << ' ' << n << '\n';
    for (int i = 0; i < n; ++i)
    {
        const auto &bag = result->bags[i];
        verify_out << "b " << i + 1;
        for (int j = 0; j < bag.size; ++j)
        {
            verify_out << ' ' << bag.vertices[j] + 1;
        }
        verify_out << '\n';
    }
    for (const auto &[u, v] : result->tree_edges)
    {
        verify_out << u + 1 << ' ' << v + 1 << '\n';
    }
}
