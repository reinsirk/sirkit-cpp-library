#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/3291

// Library: Graph/Shortest_Odd_Cycle.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Shortest_Odd_Cycle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_odd_cycle.md
//
// API一覧
// shortest_odd_cycle(n,edges):
//   無向グラフで最も短い奇数長の単純閉路を、巡回順の頂点列で返す。なければ空列。
//   頂点番号は0始まり。始点を末尾に重ねない。自己辺は1頂点の閉路、多重辺も可。
//   O(n(n+m))時間、O(n+m)領域。二部グラフの連結成分では全頂点からのBFSを省く。
//   非二部成分の始点には奇数歩で戻れるため、BFSは目標か深さ上限で終わる。
//   最短の奇数長閉歩道に頂点の重複があれば、分割してさらに短い奇数長閉歩道が得られる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_ODD_CYCLE_DEFINED
#define SIRKIT_SHORTEST_ODD_CYCLE_DEFINED

[[nodiscard]] inline std::vector<int> shortest_odd_cycle(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("shortest_odd_cycle: negative vertex count");
    }
    std::vector<std::vector<int>> adjacent(n);
    int self_loop = -1;
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(u) >= adjacent.size() || static_cast<std::size_t>(v) >= adjacent.size())
        {
            throw std::out_of_range("shortest_odd_cycle: invalid vertex");
        }
#endif
        if (u == v)
        {
            self_loop = u;
        }
        else
        {
            adjacent[u].push_back(v);
            adjacent[v].push_back(u);
        }
    }
    if (self_loop != -1)
    {
        return {self_loop};
    }

    std::vector<int> color(n, -1);
    std::vector<int> nonbipartite;
    std::vector<int> component;
    for (int root = 0; root < n; ++root)
    {
        if (color[root] != -1)
        {
            continue;
        }
        component.clear();
        component.push_back(root);
        color[root] = 0;
        bool odd = false;
        for (std::size_t i = 0; i < component.size(); ++i)
        {
            const int u = component[i];
            for (const int v : adjacent[u])
            {
                if (color[v] == -1)
                {
                    color[v] = color[u] ^ 1;
                    component.push_back(v);
                }
                else
                {
                    odd |= color[v] == color[u];
                }
            }
        }
        if (odd)
        {
            nonbipartite.insert(nonbipartite.end(), component.begin(), component.end());
        }
    }
    if (nonbipartite.empty())
    {
        return {};
    }

    const auto states = static_cast<std::size_t>(n) * 2;
    constexpr auto kUnseen = std::numeric_limits<std::size_t>::max();
    std::vector<std::size_t> previous(states), distance(states), pending;
    std::vector<int> cycle;
    auto limit = static_cast<std::size_t>(n) + 1;
    for (const int source : nonbipartite)
    {
        const auto first = static_cast<std::size_t>(source) * 2;
        const auto last = first + 1;
        std::fill(previous.begin(), previous.end(), kUnseen);
        pending.clear();
        pending.push_back(first);
        previous[first] = first;
        distance[first] = 0;

        for (std::size_t i = 0;; ++i)
        {
            const auto state = pending[i];
            const auto depth = distance[state] + 1;
            if (depth >= limit)
            {
                break;
            }
            const auto parity = (state & 1U) ^ 1U;
            for (const int v : adjacent[state / 2])
            {
                const auto next = static_cast<std::size_t>(v) * 2 + parity;
                if (previous[next] == kUnseen)
                {
                    previous[next] = state;
                    distance[next] = depth;
                    pending.push_back(next);
                    if (next == last)
                    {
                        break;
                    }
                }
            }
            if (previous[last] != kUnseen)
            {
                break;
            }
        }
        if (previous[last] == kUnseen)
        {
            continue;
        }
        limit = distance[last];
        cycle.clear();
        for (auto state = previous[last]; state != first; state = previous[state])
        {
            cycle.push_back(static_cast<int>(state / 2));
        }
        cycle.push_back(source);
        std::ranges::reverse(cycle);
        if (limit == 3)
        {
            break;
        }
    }
    return cycle;
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
    int n{}, m{}, source{}, target{};
    std::uint64_t steps{};
    verify_in >> n >> m >> steps >> source >> target;
    --source;
    --target;
    std::vector<std::pair<int, int>> edges(m);
    bool adjacent = false;
    bool endpoint_has_other_neighbor = false;
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
        if ((u == source && v == target) || (u == target && v == source))
        {
            adjacent = true;
        }
        else if (u == source || v == source || u == target || v == target)
        {
            endpoint_has_other_neighbor = true;
        }
    }
    if ((steps & 1U) != 0 || endpoint_has_other_neighbor)
    {
        verify_out << "Yes\n";
        return 0;
    }
    if (!adjacent || steps < 6)
    {
        verify_out << "No\n";
        return 0;
    }
    // The edge between the endpoints contributes one move; the new bridge is used twice.
    const auto cycle = sirkit::shortest_odd_cycle(n, edges);
    verify_out << (!cycle.empty() && cycle.size() <= steps - 3 ? "Yes\n" : "No\n");
}
