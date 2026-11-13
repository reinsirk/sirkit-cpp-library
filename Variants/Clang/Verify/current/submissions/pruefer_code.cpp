#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc239/tasks/abc239_f

// Library: Graph/Pruefer_Code.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Pruefer_Code.cpp
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/prufer_code
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pruefer_code.md
//
// API一覧
// complete_tree_with_degrees以外はすべてO(n)時間・領域。
// 頂点は0..n-1、n>=1。辺はpair<int,int>。入力を変更せず、結果を所有するvectorで返す。
// pruefer_encode(n,edges): 単純無向木の辺n-1本から、最小の葉を順に除くPrüfer列を返す。
// pruefer_decode(n,code): 長さmax(0,n-2)の列を辺リストへ戻す。各要素は0..n-1。
// pruefer_decode(code): n=code.size()+2として復元する。空列なら2頂点。
// pruefer_degrees(n,code): 復元される木の次数列を返す。
// pruefer_from_degrees(degrees): 指定次数を持つ木のPrüfer列。存在しなければnullopt。
// tree_from_degrees(degrees): 指定次数を持つ木の辺。存在しなければnullopt。
// complete_tree_with_degrees(degrees,edges): 指定次数の木にする追加辺。存在しなければnullopt。
// 既存辺は森が前提。循環・次数不足はnullopt。O((n+m)α(n))時間、O(n+m)領域。
// n=1では辺/符号とも空、次数は{0}。n=2では符号は空、辺は{0,1}、次数は{1,1}。
// n=0の次数列は木を持たずnullopt。符号だけでは1頂点と2頂点を区別できない。
// SKIP_RUNTIME_CHECKSでn/入力長/辺端点/符号値/除去中に分かる非木の検査を除く。
// encodeの検査は次数構築・葉除去中に行う。未参照の入力を検査できるとは保証しない。
// 次数列の実現可能性と件数の表現上限は検査省略時も判定する。

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_PRUEFER_DEGREES_DEFINED
#define SIRKIT_PRUEFER_DEGREES_DEFINED
inline std::vector<int> pruefer_degrees(int n, std::span<const int> code) {
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 1 || code.size() != (n < 2 ? 0 : static_cast<std::size_t>(n - 2)))
        throw std::invalid_argument("Pruefer code length");
#endif
    std::vector<int> degrees(static_cast<std::size_t>(n), n == 1 ? 0 : 1);
    for (const int vertex : code) {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n) throw std::out_of_range("Pruefer vertex");
#endif
        ++degrees[static_cast<std::size_t>(vertex)];
    }
    return degrees;
}
#endif

#ifndef SIRKIT_PRUEFER_DECODE_DEFINED
#define SIRKIT_PRUEFER_DECODE_DEFINED
inline std::vector<std::pair<int, int>> pruefer_decode(int n, std::span<const int> code) {
    auto degrees = pruefer_degrees(n, code);
    std::vector<std::pair<int, int>> edges;
    if (n == 1) return edges;
    edges.reserve(static_cast<std::size_t>(n - 1));
    int scan = 0;
    while (degrees[static_cast<std::size_t>(scan)] != 1) ++scan;
    int leaf = scan;
    for (const int vertex : code) {
        edges.emplace_back(leaf, vertex);
        degrees[static_cast<std::size_t>(leaf)] = 0;
        if (--degrees[static_cast<std::size_t>(vertex)] == 1 && vertex < scan) leaf = vertex;
        else {
            do { ++scan; } while (degrees[static_cast<std::size_t>(scan)] != 1);
            leaf = scan;
        }
    }
    edges.emplace_back(leaf, n - 1);
    return edges;
}
inline std::vector<std::pair<int, int>> pruefer_decode(std::span<const int> code) {
    if (code.size() > static_cast<std::size_t>(std::numeric_limits<int>::max() - 2))
        throw std::length_error("Pruefer vertex count");
    return pruefer_decode(static_cast<int>(code.size()) + 2, code);
}
#endif

#ifndef SIRKIT_PRUEFER_ENCODE_DEFINED
#define SIRKIT_PRUEFER_ENCODE_DEFINED
inline std::vector<int> pruefer_encode(int n, std::span<const std::pair<int, int>> edges) {
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 1 || edges.size() != static_cast<std::size_t>(n - 1))
        throw std::invalid_argument("Pruefer tree edge count");
#endif
    std::vector<int> degrees(static_cast<std::size_t>(n)), neighbors(static_cast<std::size_t>(n));
    for (const auto& [u, v] : edges) {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n || u == v)
            throw std::invalid_argument("Pruefer tree edge");
#endif
        ++degrees[static_cast<std::size_t>(u)]; ++degrees[static_cast<std::size_t>(v)];
        neighbors[static_cast<std::size_t>(u)] ^= v;
        neighbors[static_cast<std::size_t>(v)] ^= u;
    }
    std::vector<int> code;
    if (n <= 2) return code;
    code.reserve(static_cast<std::size_t>(n - 2));
    int scan = 0;
    while (scan < n && degrees[static_cast<std::size_t>(scan)] != 1) ++scan;
    int leaf = scan;
    for (int step = 0; step < n - 2; ++step) {
#ifndef SKIP_RUNTIME_CHECKS
        if (leaf == n || degrees[static_cast<std::size_t>(leaf)] != 1)
            throw std::invalid_argument("Pruefer input is not a tree");
#endif
        const int vertex = neighbors[static_cast<std::size_t>(leaf)];
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n || degrees[static_cast<std::size_t>(vertex)] <= 0)
            throw std::invalid_argument("Pruefer input is not a tree");
#endif
        code.push_back(vertex);
        degrees[static_cast<std::size_t>(leaf)] = 0;
        neighbors[static_cast<std::size_t>(vertex)] ^= leaf;
        if (--degrees[static_cast<std::size_t>(vertex)] == 1 && vertex < scan) leaf = vertex;
        else {
            do { ++scan; } while (scan < n && degrees[static_cast<std::size_t>(scan)] != 1);
            leaf = scan;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (leaf == n || leaf == n - 1 || degrees[static_cast<std::size_t>(leaf)] != 1 ||
        degrees[static_cast<std::size_t>(n - 1)] != 1 ||
        neighbors[static_cast<std::size_t>(leaf)] != n - 1 ||
        neighbors[static_cast<std::size_t>(n - 1)] != leaf)
        throw std::invalid_argument("Pruefer input is not a tree");
#endif
    return code;
}
#endif

#ifndef SIRKIT_PRUEFER_FROM_DEGREES_DEFINED
#define SIRKIT_PRUEFER_FROM_DEGREES_DEFINED
inline std::optional<std::vector<int>> pruefer_from_degrees(std::span<const int> degrees) {
    const auto n = degrees.size();
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("Pruefer vertex count");
    if (n == 0) return std::nullopt;
    if (n == 1) {
        if (degrees[0] == 0) return std::vector<int>{};
        return std::nullopt;
    }
    std::size_t remaining = n - 2;
    std::vector<int> code;
    code.reserve(remaining);
    for (std::size_t vertex = 0; vertex < n; ++vertex) {
        if (degrees[vertex] < 1) return std::nullopt;
        const auto copies = static_cast<std::size_t>(degrees[vertex] - 1);
        if (copies > remaining) return std::nullopt;
        remaining -= copies;
        code.insert(code.end(), copies, static_cast<int>(vertex));
    }
    if (remaining != 0) return std::nullopt;
    return code;
}
#endif

#ifndef SIRKIT_TREE_FROM_DEGREES_DEFINED
#define SIRKIT_TREE_FROM_DEGREES_DEFINED
inline std::optional<std::vector<std::pair<int, int>>> tree_from_degrees(std::span<const int> degrees) {
    auto code = pruefer_from_degrees(degrees);
    if (!code) return std::nullopt;
    return pruefer_decode(static_cast<int>(degrees.size()), *code);
}
#endif

#ifndef SIRKIT_COMPLETE_TREE_WITH_DEGREES_DEFINED
#define SIRKIT_COMPLETE_TREE_WITH_DEGREES_DEFINED
inline std::optional<std::vector<std::pair<int, int>>> complete_tree_with_degrees(
        std::span<const int> degrees, std::span<const std::pair<int, int>> edges) {
    const auto n = degrees.size();
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("Pruefer vertex count");
    if (n == 0 || edges.size() >= n) return std::nullopt;
    std::vector<int> residual, parent(n, -1);
    residual.reserve(n);
    std::uint64_t budget = std::uint64_t{2} * (n - 1);
    for (const int degree : degrees) {
        if (degree < (n == 1 ? 0 : 1) || static_cast<std::uint64_t>(degree) > budget)
            return std::nullopt;
        budget -= static_cast<std::uint64_t>(degree);
        residual.push_back(degree);
    }
    if (budget != 0) return std::nullopt;
    const auto find = [&parent](int vertex) {
        int root = vertex;
        while (parent[static_cast<std::size_t>(root)] >= 0) root = parent[static_cast<std::size_t>(root)];
        while (vertex != root) {
            const int next = parent[static_cast<std::size_t>(vertex)];
            parent[static_cast<std::size_t>(vertex)] = root;
            vertex = next;
        }
        return root;
    };
    for (const auto& [u, v] : edges) {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || v < 0 || static_cast<std::size_t>(u) >= n || static_cast<std::size_t>(v) >= n)
            throw std::out_of_range("Pruefer forest vertex");
#endif
        int a = find(u), b = find(v);
        if (a == b || residual[static_cast<std::size_t>(u)] == 0 || residual[static_cast<std::size_t>(v)] == 0)
            return std::nullopt;
        --residual[static_cast<std::size_t>(u)]; --residual[static_cast<std::size_t>(v)];
        if (parent[static_cast<std::size_t>(a)] > parent[static_cast<std::size_t>(b)]) std::swap(a, b);
        parent[static_cast<std::size_t>(a)] += parent[static_cast<std::size_t>(b)];
        parent[static_cast<std::size_t>(b)] = a;
    }
    std::vector<int> component(n, -1), component_degrees;
    std::vector<std::vector<int>> endpoints;
    for (std::size_t vertex = 0; vertex < n; ++vertex) {
        const auto root = static_cast<std::size_t>(find(static_cast<int>(vertex)));
        if (component[root] == -1) {
            component[root] = static_cast<int>(component_degrees.size());
            component_degrees.push_back(0); endpoints.emplace_back();
        }
        const auto index = static_cast<std::size_t>(component[root]);
        // 有効な残余次数の総和は2*(成分数-1)。intへの加算前に上限を確認する。
        if (residual[vertex] > std::numeric_limits<int>::max() - component_degrees[index])
            return std::nullopt;
        component_degrees[index] += residual[vertex];
        endpoints[index].insert(endpoints[index].end(), static_cast<std::size_t>(residual[vertex]),
                                static_cast<int>(vertex));
    }
    auto result = tree_from_degrees(component_degrees);
    if (!result) return std::nullopt;
    for (auto& [u, v] : *result) {
        auto& a = endpoints[static_cast<std::size_t>(u)];
        auto& b = endpoints[static_cast<std::size_t>(v)];
        u = a.back(); a.pop_back(); v = b.back(); b.pop_back();
    }
    return result;
}
#endif


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

#include <cstddef>
#include <utility>
#include <vector>
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0, m = 0; input >> n >> m;
    std::vector<int> degrees(static_cast<std::size_t>(n));
    for (auto& degree : degrees) input >> degree;
    std::vector<std::pair<int,int>> edges(static_cast<std::size_t>(m));
    for (auto& [u, v] : edges) { input >> u >> v; --u; --v; }
    auto added = complete_tree_with_degrees(degrees, edges);
    if (!added) { output << -1 << '\n'; return 0; }
    for (const auto& [u, v] : *added) output << u + 1 << ' ' << v + 1 << '\n';
}
