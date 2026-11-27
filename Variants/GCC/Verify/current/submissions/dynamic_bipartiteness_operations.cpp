// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: ローカル入力例回帰

// Library: Graph/Dynamic_Bipartiteness.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Dynamic_Bipartiteness.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_bipartiteness.md
//
// API一覧
// DynamicBipartiteness(n=0): n頂点、辺なしで初期化する。O(1)。
// add_edge(u,v): 無向辺を追加し辺IDを返す。自己ループと多重辺も可。償却O(1)。
// remove_edge(id): 有効な辺IDに対応する1本を削除する。O(1)。
// query_bipartite(): グラフ全体の二部性を照会し、回答添字を返す。O(1)。
// solve(): 照会順のvector<bool>を返す。全操作の記録後に呼ぶ。記録を保つ。
// 頂点数n・追加辺数e・照会数qについて、solveはO(n+q+e log(q+1) log(n+2))時間、
// O(n+q+e log(q+1))追加領域。初期頂点番号は0..n-1。
// size()/edge_count()/query_count(): 頂点数、追加辺数、照会数。各O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_BIPARTITENESS_DEFINED
#define SIRKIT_DYNAMIC_BIPARTITENESS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckDynamicBipartitenessLimit(std::size_t count, std::size_t limit)
{
    if (count > limit)
    {
        throw std::length_error("DynamicBipartiteness: too many events");
    }
}
}

class DynamicBipartiteness
{
public:
    explicit DynamicBipartiteness(int n = 0) : vertices_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0)
        {
            throw std::invalid_argument("DynamicBipartiteness: negative vertex count");
        }
#endif
    }

    std::size_t add_edge(int u, int v)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= vertices_ || v < 0 || v >= vertices_)
        {
            throw std::out_of_range("DynamicBipartiteness: invalid vertex");
        }
#endif
        const auto id = edges_.size();
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(id + 1, std::numeric_limits<EdgeIndex>::max());
        edges_.push_back({u, v, queries_, kNoEnd});
        return id;
    }

    void remove_edge(std::size_t id)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edges_.size())
        {
            throw std::out_of_range("DynamicBipartiteness: invalid edge ID");
        }
        if (edges_[id].end != kNoEnd)
        {
            throw std::invalid_argument("DynamicBipartiteness: edge already removed");
        }
#endif
        edges_[id].end = queries_;
    }

    std::size_t query_bipartite()
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(queries_ + 1, kNoEnd / 4 + 1);
        return queries_++;
    }

    [[nodiscard]] int size() const noexcept
    {
        return vertices_;
    }

    [[nodiscard]] std::size_t edge_count() const noexcept
    {
        return edges_.size();
    }

    [[nodiscard]] std::size_t query_count() const noexcept
    {
        return queries_;
    }

    [[nodiscard]] std::vector<bool> solve() const
    {
        if (queries_ == 0 || edges_.empty())
        {
            return std::vector<bool>(queries_, true);
        }

        std::vector<std::size_t> offsets;
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(queries_, std::bit_floor((offsets.max_size() - 1) / 2));
        const auto base = std::bit_ceil(queries_);
        offsets.resize(2 * base + 1);

        std::vector<EdgeIndex> slots;
        const auto nodes_per_edge = 2 * std::bit_width(base);
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(edges_.size(), slots.max_size() / nodes_per_edge);
        const auto visit_interval = [&](const Edge &edge, auto &&visit)
        {
            const auto end = std::min(edge.end, queries_);
            if (edge.begin == end)
            {
                return;
            }

            // 最後の照会まで有効な辺は、照会のない右端まで延長できる。
            auto left = edge.begin + base;
            auto right = (end == queries_ ? base : end) + base;
            for (; left < right; left >>= 1, right >>= 1)
            {
                if (left & 1)
                {
                    visit(left++);
                }
                if (right & 1)
                {
                    visit(--right);
                }
            }
        };

        for (const auto &edge : edges_)
        {
            visit_interval(edge, [&](std::size_t node)
                           {
                               ++offsets[node];
                           });
        }
        std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        if (offsets.back() == 0)
        {
            return std::vector<bool>(queries_, true);
        }
        slots.resize(offsets.back());
        for (std::size_t id = 0; id < edges_.size(); ++id)
        {
            visit_interval(edges_[id], [&](std::size_t node)
                           {
                               slots[--offsets[node]] = static_cast<EdgeIndex>(id);
                           });
        }

        std::vector<bool> answers(base);
        ParityState state(vertices_);
        const auto visit_tree = [&](auto &&self, std::size_t node) -> void
        {
            const auto saved = state.snapshot();
            for (auto position = offsets[node]; position < offsets[node + 1]; ++position)
            {
                const auto &edge = edges_[slots[position]];
                if (!state.add(edge.u, edge.v))
                {
                    // ここで矛盾するなら、この部分木に属するすべての照会の答えはfalse。
                    state.rollback(saved);
                    return;
                }
            }
            if (node >= base)
            {
                answers[node - base] = true;
            }
            else
            {
                self(self, 2 * node);
                self(self, 2 * node + 1);
            }
            state.rollback(saved);
        };
        visit_tree(visit_tree, 1);
        answers.resize(queries_);
        return answers;
    }

private:
    using EdgeIndex = std::uint32_t;
    static constexpr auto kNoEnd = std::numeric_limits<std::size_t>::max();

    struct Edge
    {
        int u, v;
        std::size_t begin, end;
    };

    class ParityState
    {
    public:
        explicit ParityState(int n) : parent_(n, -1), parity_(n)
        {
        }

        std::size_t snapshot() const noexcept
        {
            return history_.size();
        }

        bool add(int u, int v)
        {
            auto [root_u, parity_u] = Root(u);
            auto [root_v, parity_v] = Root(v);
            if (root_u == root_v)
            {
                return parity_u != parity_v;
            }
            if (parent_[root_u] > parent_[root_v])
            {
                std::swap(root_u, root_v);
            }
            history_.push_back({root_u, root_v, parent_[root_v]});
            parent_[root_u] += parent_[root_v];
            parent_[root_v] = root_u;
            parity_[root_v] = static_cast<std::uint8_t>(parity_u == parity_v);
            return true;
        }

        void rollback(std::size_t saved) noexcept
        {
            while (history_.size() > saved)
            {
                const auto [root, child, child_size] = history_.back();
                history_.pop_back();
                parent_[root] -= child_size;
                parent_[child] = child_size;
                parity_[child] = 0;
            }
        }

    private:
        struct Change
        {
            int root, child, child_size;
        };

        std::vector<int> parent_;
        std::vector<std::uint8_t> parity_;
        std::vector<Change> history_;

        std::pair<int, bool> Root(int v) const noexcept
        {
            bool parity = false;
            while (parent_[v] >= 0)
            {
                parity = parity != static_cast<bool>(parity_[v]);
                v = parent_[v];
            }
            return {v, parity};
        }
    };

    int vertices_;
    std::size_t queries_ = 0;
    std::vector<Edge> edges_;
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

#include <cstddef>

int main()
{
    int n;
    std::size_t operation_count;
    verify_in >> n >> operation_count;
    DynamicBipartiteness graph(n);
    for (std::size_t i = 0; i < operation_count; ++i)
    {
        int type;
        verify_in >> type;
        if (type == 0)
        {
            int u, v;
            verify_in >> u >> v;
            graph.add_edge(u, v);
        }
        else if (type == 1)
        {
            std::size_t edge_id;
            verify_in >> edge_id;
            graph.remove_edge(edge_id);
        }
        else
        {
            graph.query_bipartite();
        }
    }
    for (const bool bipartite : graph.solve())
    {
        verify_out << (bipartite ? "Yes\n" : "No\n");
    }
}
