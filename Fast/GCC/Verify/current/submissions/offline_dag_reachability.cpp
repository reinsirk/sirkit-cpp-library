#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/typical90/tasks/typical90_bg

// Library: Graph/Offline_DAG_Reachability.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Offline_DAG_Reachability.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_dag_reachability.md
//
// API一覧
// offline_dag_reachability(n,edges,queries): DAGで始点から終点に到達できるか、入力順のvector<bool>で返す。
// edgesとqueriesは(始点,終点)の列。頂点番号は[0,n)。平行辺を許す。同じ頂点への照会はtrue。
// トポロジカル順に逆らう照会はfalse。残りの始点数・終点数の小さい方をpとする。
// O((n+m)(1+ceil(p/64))+q)時間・O(n+m+q)領域。mは辺数、qは照会数。
// 通常版で有向閉路を含む入力はinvalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_DAG_REACHABILITY_DEFINED
#define SIRKIT_OFFLINE_DAG_REACHABILITY_DEFINED

namespace detail
{
inline std::vector<int> reachability_topological_ranks(int n, std::span<const std::pair<int, int>> edges,
                                                        bool already_sorted)
{
    std::vector<int> ranks(n);
    if (already_sorted)
    {
        std::iota(ranks.begin(), ranks.end(), 0);
        return ranks;
    }
    std::vector<std::vector<int>> outgoing(n);
    std::vector<std::size_t> inbound(n);
    for (const auto &[u, v] : edges)
    {
        outgoing[u].push_back(v);
        ++inbound[v];
    }
    std::vector<int> order;
    order.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (inbound[v] == 0)
        {
            order.push_back(v);
        }
    }
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        ranks[v] = static_cast<int>(i);
        for (const int u : outgoing[v])
        {
            if (--inbound[u] == 0)
            {
                order.push_back(u);
            }
        }
    }
    return ranks;
}
}

inline std::vector<bool> offline_dag_reachability(int n, std::span<const std::pair<int, int>> edges,
                                                    std::span<const std::pair<int, int>> queries)
{
    if (n < 0)
    {
        throw std::length_error("offline_dag_reachability: negative vertex count");
    }
    bool already_sorted = true;
    for (const auto &[u, v] : edges)
    {
        already_sorted &= u < v;
    }
    const auto ranks = detail::reachability_topological_ranks(n, edges, already_sorted);
    struct Query
    {
        int from;
        int to;
        std::size_t answer_index;
    };
    std::vector<bool> answer(queries.size());
    std::vector<Query> pending;
    pending.reserve(queries.size());
    std::vector<std::uint8_t> sources(n), targets(n);
    int unique_sources = 0, unique_targets = 0;
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [u, v] = queries[i];
        const int a = ranks[u], b = ranks[v];
        if (a == b)
        {
            answer[i] = true;
        }
        else if (a < b)
        {
            pending.push_back({a, b, i});
            unique_sources += sources[a] == 0;
            unique_targets += targets[b] == 0;
            sources[a] = targets[b] = 1;
        }
    }
    if (pending.empty())
    {
        return answer;
    }

    // グラフと照会を逆向きに読み替えれば、少ない側の端点だけをビットに割り当てられる。
    const bool reversed = unique_targets < unique_sources;
    const auto orient = [reversed, n](int from, int to)
    {
        return reversed ? std::pair{n - 1 - to, n - 1 - from} : std::pair{from, to};
    };
    std::vector<std::size_t> edge_begin(static_cast<std::size_t>(n) + 1), query_begin(edge_begin.size());
    for (const auto &[u, v] : edges)
    {
        const auto [from, to] = orient(ranks[u], ranks[v]);
        ++edge_begin[from + 1];
    }
    for (auto &[from, to, index] : pending)
    {
        const auto oriented = orient(from, to);
        from = oriented.first;
        to = oriented.second;
        ++query_begin[from + 1];
    }
    std::partial_sum(edge_begin.begin(), edge_begin.end(), edge_begin.begin());
    std::partial_sum(query_begin.begin(), query_begin.end(), query_begin.begin());
    auto edge_cursor = edge_begin, query_cursor = query_begin;
    std::vector<int> destinations(edges.size());
    for (const auto &[u, v] : edges)
    {
        const auto [from, to] = orient(ranks[u], ranks[v]);
        destinations[edge_cursor[from]++] = to;
    }
    std::vector<Query> grouped(pending.size());
    for (const auto &query : pending)
    {
        grouped[query_cursor[query.from]++] = query;
    }

    std::vector<std::uint64_t> reach(n);
    for (int first = 0; first < n; )
    {
        std::array<int, 64> seeds;
        std::size_t size = 0;
        for (; first < n && size < seeds.size(); ++first)
        {
            if (query_begin[first] != query_begin[first + 1])
            {
                seeds[size++] = first;
            }
        }
        if (size == 0)
        {
            break;
        }
        int last_target = seeds[0];
        std::fill(reach.begin(), reach.end(), 0);
        for (std::size_t bit = 0; bit < size; ++bit)
        {
            const int start = seeds[bit];
            reach[start] = std::uint64_t{1} << bit;
            for (auto i = query_begin[start]; i != query_begin[start + 1]; ++i)
            {
                last_target = std::max(last_target, grouped[i].to);
            }
        }
        for (int v = seeds[0]; v < last_target; ++v)
        {
            if (const auto bits = reach[v]; bits != 0)
            {
                for (auto i = edge_begin[v]; i != edge_begin[v + 1]; ++i)
                {
                    reach[destinations[i]] |= bits;
                }
            }
        }
        for (std::size_t bit = 0; bit < size; ++bit)
        {
            const int start = seeds[bit];
            for (auto i = query_begin[start]; i != query_begin[start + 1]; ++i)
            {
                const auto &query = grouped[i];
                answer[query.answer_index] = (reach[query.to] >> bit) & 1;
            }
        }
    }
    return answer;
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
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::pair<int, int>> edges(m), queries(q);
    for (auto &list : {&edges, &queries})
    {
        for (auto &[u, v] : *list)
        {
            verify_in >> u >> v;
            --u;
            --v;
        }
    }
    for (const bool reachable : sirkit::offline_dag_reachability(n, edges, queries))
    {
        verify_out << (reachable ? "Yes\n" : "No\n");
    }
}
