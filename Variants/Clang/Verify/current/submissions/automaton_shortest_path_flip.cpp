#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc395/tasks/abc395_e

// Library: Graph/Automaton_Shortest_Path.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Automaton_Shortest_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/automaton_shortest_path.md
//
// API一覧
// AutomatonPathEdge{from,to,label,weight}: 0始まりの端点、任意の int ラベル、非負の uint64_t 重み。
// automaton_shortest_path(n,edges,source,target,state_count,initial,transition,accept,undirected=false):
//   ラベルを順に読んで受理状態で終点へ着く最短歩道。AutomatonPath{distance,vertices,states,edges} を返す。
//   状態は0始まり。transition(state,label) は次状態、-1で通行不可。accept(state) は受理するなら true。
//   edges は入力順の辺ID列。到達不能・距離の uint64_t 範囲超過はnullopt。
//   状態数を S とし、時間 O(S(n+m log(Sm+1)))、領域 O(S(n+m))。全辺重み1なら O(S(n+m)) 時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_AUTOMATON_SHORTEST_PATH_DEFINED
#define SIRKIT_AUTOMATON_SHORTEST_PATH_DEFINED

struct AutomatonPathEdge
{
    int from;
    int to;
    int label;
    std::uint64_t weight;
};

struct AutomatonPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<int> states;
    std::vector<std::size_t> edges;
};

template <typename Transition, typename Accept>
std::optional<AutomatonPath> automaton_shortest_path(
    int n, std::span<const AutomatonPathEdge> edges, int source, int target, int state_count,
    int initial, Transition &&transition, Accept &&accept, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("automaton_shortest_path: invalid terminal");
    }
    if (state_count <= 0 || initial < 0 || initial >= state_count)
    {
        throw std::out_of_range("automaton_shortest_path: invalid initial state");
    }
#endif
    struct VertexState
    {
        std::optional<std::uint64_t> distance;
        std::size_t previous = 0;
        std::size_t edge = 0;
    };
    const auto states = static_cast<std::size_t>(state_count);
    std::vector<VertexState> info;
    if (static_cast<std::size_t>(n) > info.max_size() / states)
    {
        throw std::length_error("automaton_shortest_path: too many vertex-state pairs");
    }
    info.resize(static_cast<std::size_t>(n) * states);
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    bool unit = true;
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const int from = edges[id].from;
        const int to = edges[id].to;
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("automaton_shortest_path: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id});
        if (undirected && from != to)
        {
            graph[to].push_back({from, id});
        }
        unit = unit && edges[id].weight == 1;
    }
    auto index = [states](int vertex, int state)
    {
        return static_cast<std::size_t>(vertex) * states + state;
    };
    const auto start = index(source, initial);
    auto make_path = [&](std::size_t goal, std::uint64_t distance)
    {
        AutomatonPath result{distance, {}, {}, {}};
        for (auto where = goal;; where = info[where].previous)
        {
            result.vertices.push_back(static_cast<int>(where / states));
            result.states.push_back(static_cast<int>(where % states));
            if (where == start)
            {
                break;
            }
            result.edges.push_back(info[where].edge);
        }
        std::ranges::reverse(result.vertices);
        std::ranges::reverse(result.states);
        std::ranges::reverse(result.edges);
        return result;
    };
    auto search = [&]<bool Unit>() -> std::optional<AutomatonPath>
    {
        using Entry = std::pair<std::uint64_t, std::size_t>;
        using Queue = std::conditional_t<Unit, std::queue<Entry>, std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>>>;
        Queue pending;
        info[start].distance = 0;
        pending.emplace(0, start);
        while (!pending.empty())
        {
            Entry current;
            if constexpr (Unit)
            {
                current = pending.front();
            }
            else
            {
                current = pending.top();
            }
            const auto [distance, where] = current;
            pending.pop();
            if constexpr (!Unit)
            {
                if (distance != *info[where].distance)
                {
                    continue;
                }
            }
            const int vertex = static_cast<int>(where / states);
            const int state = static_cast<int>(where % states);
            if (vertex == target && std::invoke(accept, state))
            {
                return make_path(where, distance);
            }
            for (const auto &[v, id] : graph[vertex])
            {
                const auto &edge = edges[id];
                const int next_state = std::invoke(transition, state, edge.label);
                if (next_state == -1)
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (next_state < 0 || next_state >= state_count)
                {
                    throw std::out_of_range("automaton_shortest_path: invalid transition result");
                }
#endif
                if constexpr (!Unit)
                {
                    if (edge.weight > std::numeric_limits<std::uint64_t>::max() - distance)
                    {
                        continue;
                    }
                }
                const auto next = index(v, next_state);
                const auto total = distance + edge.weight;
                auto &entry = info[next];
                if (!entry.distance || total < *entry.distance)
                {
                    entry = {total, where, id};
                    pending.emplace(total, next);
                }
            }
        }
        return std::nullopt;
    };
    return unit ? search.template operator()<true>() : search.template operator()<false>();
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


#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t flip{};
    verify_in >> n >> m >> flip;
    std::vector<sirkit::AutomatonPathEdge> edges;
    edges.reserve(static_cast<std::size_t>(2 * m + n));
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        edges.push_back({u, v, 0, 1});
        edges.push_back({v, u, 1, 1});
    }
    for (int v = 0; v < n; ++v)
    {
        edges.push_back({v, v, 2, flip});
    }
    auto next = [](int mode, int label) { return label == 2 ? 1 - mode : (label == mode ? mode : -1); };
    auto accept = [](int) { return true; };
    const auto path = sirkit::automaton_shortest_path(n, edges, 0, n - 1, 2, 0, next, accept);
    verify_out << path.value().distance << '\n';
}
