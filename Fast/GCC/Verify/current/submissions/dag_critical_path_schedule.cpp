#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/468

// Library: Graph/DAG_Critical_Path_Schedule.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/DAG_Critical_Path_Schedule.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dag_critical_path_schedule.md
//
// API一覧
// DagScheduleEdge{from,to,duration}: 0始まりの端点と非負の uint64_t 所要時間。
// dag_critical_path_schedule(n,edges,start,finish): DAG上の最短完了時刻と余裕を求める。
//   閉路があるか始点から終点に届かなければ nullopt。最短完了時刻が uint64_t を超えれば overflow_error。
//   結果の duration は完了時刻、earliest/latest は頂点の最早・最遅時刻。両者が同じ頂点はクリティカル。
//   edge_slack[i] は辺 ID i の余裕。始点から終点へのいずれの経路にも含まれない頂点・辺は nullopt。
//   critical_path はクリティカルパス一つの辺 ID 列。時間・領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
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
#ifndef SIRKIT_DAG_CRITICAL_PATH_SCHEDULE_DEFINED
#define SIRKIT_DAG_CRITICAL_PATH_SCHEDULE_DEFINED

struct DagScheduleEdge
{
    int from;
    int to;
    std::uint64_t duration;
};

struct DagCriticalPathSchedule
{
    std::uint64_t duration;
    std::vector<std::optional<std::uint64_t>> earliest;
    std::vector<std::optional<std::uint64_t>> latest;
    std::vector<std::optional<std::uint64_t>> edge_slack;
    std::vector<std::size_t> critical_path;
};

inline std::optional<DagCriticalPathSchedule> dag_critical_path_schedule(
    int n, std::span<const DagScheduleEdge> edges, int start, int finish)
{
    std::vector<std::vector<std::size_t>> outgoing(n);
    std::vector<std::size_t> indegree(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v, duration] = edges[id];
        outgoing[u].push_back(id);
        ++indegree[v];
    }
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(n));
    for (int u = 0; u < n; ++u)
    {
        if (indegree[u] == 0)
        {
            order.push_back(u);
        }
    }
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        for (const auto id : outgoing[order[i]])
        {
            const int v = edges[id].to;
            if (--indegree[v] == 0)
            {
                order.push_back(v);
            }
        }
    }
    if (order.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }

    std::vector<bool> can_finish(n);
    can_finish[finish] = true;
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int u = *it;
        for (const auto id : outgoing[u])
        {
            if (can_finish[edges[id].to])
            {
                can_finish[u] = true;
                break;
            }
        }
    }
    if (!can_finish[start])
    {
        return std::nullopt;
    }

    DagCriticalPathSchedule result{0, std::vector<std::optional<std::uint64_t>>(n), std::vector<std::optional<std::uint64_t>>(n), std::vector<std::optional<std::uint64_t>>(edges.size()), {}};
    result.earliest[start] = 0;
    for (int u : order)
    {
        if (!result.earliest[u])
        {
            continue;
        }
        const auto prefix = *result.earliest[u];
        for (const auto id : outgoing[u])
        {
            const auto [from, to, duration] = edges[id];
            if (!can_finish[to])
            {
                continue;
            }
            if (duration > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                throw std::overflow_error("dag_critical_path_schedule: duration overflows uint64_t");
            }
            const auto arrival = prefix + duration;
            auto &earliest = result.earliest[to];
            if (!earliest || arrival > *earliest)
            {
                earliest = arrival;
            }
        }
    }

    result.duration = result.earliest[finish].value();
    result.latest[finish] = result.duration;
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int u = *it;
        if (!result.earliest[u])
        {
            continue;
        }
        for (const auto id : outgoing[u])
        {
            const auto [from, to, duration] = edges[id];
            if (!result.latest[to])
            {
                continue;
            }
            const auto deadline = *result.latest[to] - duration;
            auto &latest = result.latest[u];
            if (!latest || deadline < *latest)
            {
                latest = deadline;
            }
            result.edge_slack[id] = deadline - *result.earliest[u];
        }
    }

    for (int u = start; u != finish;)
    {
        const auto &arcs = outgoing[u];
        const auto id = *std::ranges::find_if(arcs, [&](std::size_t i)
                                              {
                                                  return result.edge_slack[i] == 0;
                                              });
        result.critical_path.push_back(id);
        u = edges[id].to;
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


#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::DagScheduleEdge> edges(m);
    for (auto &[u, v, duration] : edges)
    {
        verify_in >> u >> v >> duration;
    }
    const auto schedule = sirkit::dag_critical_path_schedule(n, edges, 0, n - 1).value();
    int flexible = 0;
    for (int u = 0; u < n; ++u)
    {
        if (!schedule.earliest[u] || schedule.earliest[u] != schedule.latest[u])
        {
            ++flexible;
        }
    }
    verify_out << schedule.duration << ' ' << flexible << '/' << n << '\n';
}
