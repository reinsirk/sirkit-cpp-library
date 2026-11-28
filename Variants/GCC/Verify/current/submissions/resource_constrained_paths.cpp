#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc164/tasks/abc164_e

// Library: Graph/Resource_Constrained_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Resource_Constrained_Paths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/resource_constrained_paths.md
//
// API一覧
// ResourcePathArc{from,to,spend,gain,time}: 資源をspend消費しgain得る有向辺。timeは所要時間。
// ResourceConstrainedPaths(n,edges,source,capacity,initial): 所持量が上限capacityの最短路を構築。
//   初期値と補充は上限で止まる。資源がspend未満なら辺を通れない。頂点は0始まり。
//   資源量・時間はuint64_t。時間が表現可能な経路だけ扱う。
// minimum_time(v,required=0): 資源をrequired以上持って到着する最短時間。なければnullopt。O(log(k+2))。
// labels(v): (残量,時間)のうち他の組に支配されないものを残量昇順で参照する。kはその組数。
//   上限をCとし、最悪 O(n+(m+1)(C+1) log((m+1)(C+1)+2)) 時間、O((n+m)(C+1)) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RESOURCE_CONSTRAINED_PATHS_DEFINED
#define SIRKIT_RESOURCE_CONSTRAINED_PATHS_DEFINED

struct ResourcePathArc
{
    int from;
    int to;
    std::uint64_t spend;
    std::uint64_t gain;
    std::uint64_t time;
};

struct ResourcePathLabel
{
    std::uint64_t remaining;
    std::uint64_t time;
};

class ResourceConstrainedPaths
{
public:
    ResourceConstrainedPaths(
        int n, std::span<const ResourcePathArc> edges, int source, std::uint64_t capacity, std::uint64_t initial)
    {
        if (n < 0)
        {
            throw std::length_error("ResourceConstrainedPaths: negative vertex count");
        }
        labels_.resize(n);
        check_vertex(source);

        std::vector<std::vector<std::size_t>> outgoing(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            check_vertex(edges[i].from);
            check_vertex(edges[i].to);
            outgoing[edges[i].from].push_back(i);
        }

        using Key = std::tuple<std::uint64_t, std::uint64_t, int>;
        std::priority_queue<Key, std::vector<Key>, std::greater<Key>> pending;
        std::vector<std::map<std::uint64_t, std::uint64_t>> frontier(n);
        auto enqueue = [&](int v, std::uint64_t remaining, std::uint64_t time)
        {
            auto &labels = frontier[v];
            auto after = labels.lower_bound(remaining);
            if (after != labels.end())
            {
                if (after->second <= time)
                {
                    return;
                }
                if (after->first == remaining)
                {
                    after = labels.erase(after);
                }
            }
            auto first = after;
            while (first != labels.begin())
            {
                const auto before = std::prev(first);
                if (before->second < time)
                {
                    break;
                }
                first = before;
            }
            const auto hint = labels.erase(first, after);
            labels.emplace_hint(hint, remaining, time);
            pending.emplace(time, capacity - remaining, v);
        };
        enqueue(source, std::min(capacity, initial), 0);
        while (!pending.empty())
        {
            const auto [time, used, v] = pending.top();
            pending.pop();
            const auto remaining = capacity - used;
            const auto found = frontier[v].find(remaining);
            if (found == frontier[v].end() || found->second != time)
            {
                continue;
            }
            for (const auto id : outgoing[v])
            {
                const auto &edge = edges[id];
                if (remaining < edge.spend || edge.time > std::numeric_limits<std::uint64_t>::max() - time)
                {
                    continue;
                }
                const auto rest = remaining - edge.spend;
                const auto next = rest + std::min(edge.gain, capacity - rest);
                enqueue(edge.to, next, time + edge.time);
            }
        }

        for (int v = 0; v < n; ++v)
        {
            labels_[v].reserve(frontier[v].size());
            for (const auto &[remaining, time] : frontier[v])
            {
                labels_[v].push_back({remaining, time});
            }
        }
    }

    [[nodiscard]] std::span<const ResourcePathLabel> labels(int v) const
    {
        check_vertex(v);
        return labels_[v];
    }

    [[nodiscard]] std::optional<std::uint64_t> minimum_time(int v, std::uint64_t required = 0) const
    {
        const auto options = labels(v);
        const auto found = std::ranges::lower_bound(options, required, {}, &ResourcePathLabel::remaining);
        if (found == options.end())
        {
            return std::nullopt;
        }
        return found->time;
    }

private:
    void check_vertex([[maybe_unused]] int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(v) >= labels_.size())
        {
            throw std::out_of_range("ResourceConstrainedPaths: invalid vertex");
        }
#endif
    }

    std::vector<std::vector<ResourcePathLabel>> labels_;
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


#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t initial{};
    verify_in >> n >> m >> initial;
    std::vector<sirkit::ResourcePathArc> edges;
    edges.reserve(2 * m + n);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        std::uint64_t fare{}, time{};
        verify_in >> u >> v >> fare >> time;
        --u;
        --v;
        edges.push_back({u, v, fare, 0, time});
        edges.push_back({v, u, fare, 0, time});
    }
    for (int v = 0; v < n; ++v)
    {
        std::uint64_t coins{}, time{};
        verify_in >> coins >> time;
        edges.push_back({v, v, 0, coins, time});
    }
    // 両替の間に通る単純路は最大 n - 1 区間分の運賃で足りる。
    const sirkit::ResourceConstrainedPaths paths(n, edges, 0, 50 * n, initial);
    for (int v = 1; v < n; ++v)
    {
        verify_out << paths.minimum_time(v).value() << '\n';
    }
}
