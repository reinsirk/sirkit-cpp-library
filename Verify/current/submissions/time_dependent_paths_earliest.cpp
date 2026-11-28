#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 6c06775293a8fc121095184c6b6cd484dec4f714834083109bb3d4b8fcd591c6
// Verify: https://atcoder.jp/contests/abc192/tasks/abc192_e

// Library: Graph/Time_Dependent_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（時刻を距離として扱うダイクストラ法）: https://atcoder.jp/contests/ABC192/editorial/764
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/time_dependent_paths.md
//
// API一覧
// earliest_arrival_times(n,edges,source,start_time,arrival): 始点から全頂点への最早到着時刻。
//   arrival(edge_id,time)はtime以降に乗るときの最早到着時刻、移動できなければnullopt。
// latest_departure_times(n,edges,target,deadline,departure): 期限までに終点へ着く全頂点の最遅出発時刻。
//   departure(edge_id,time)はtimeまでに着く最遅出発時刻、移動できなければnullopt。
//   edgesは0始まりの(from,to)、移動関数と結果の時刻はoptional<int64_t>。
//   早く到着すれば待てる。時刻を早めると最早到着が遅くならず、期限を延ばすと最遅出発が早まらないこと。
//   コールバック1回の費用をTとし、O(n+m log(m+1)+mT)時間、O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TIME_DEPENDENT_PATHS_DEFINED
#define SIRKIT_TIME_DEPENDENT_PATHS_DEFINED

namespace detail
{
template <bool Reverse, class Transition>
std::vector<std::optional<std::int64_t>> search_timetable(
    int n, std::span<const std::pair<int, int>> edges, int start, std::int64_t start_time, Transition transition)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (start < 0 || start >= n)
    {
        throw std::out_of_range("timetable: start vertex out of range");
    }
#endif
    std::vector<std::size_t> offsets(static_cast<std::size_t>(n) + 1);
    for (const auto &[from, to] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("timetable: endpoint out of range");
        }
#endif
        ++offsets[static_cast<std::size_t>(Reverse ? to : from) + 1];
    }
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    std::vector<std::size_t> ids(edges.size());
    {
        auto write = offsets;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [from, to] = edges[i];
            ids[write[Reverse ? to : from]++] = i;
        }
    }

    using Entry = std::pair<std::int64_t, int>;
    using QueueOrder = std::conditional_t<Reverse, std::less<Entry>, std::greater<Entry>>;
    const std::conditional_t<Reverse, std::greater<std::int64_t>, std::less<std::int64_t>> better;
    std::priority_queue<Entry, std::vector<Entry>, QueueOrder> pending;
    std::vector<std::optional<std::int64_t>> times(n);
    times[start] = start_time;
    pending.emplace(start_time, start);
    while (!pending.empty())
    {
        const auto [time, u] = pending.top();
        pending.pop();
        if (time != *times[u])
        {
            continue;
        }
        const auto until = offsets[static_cast<std::size_t>(u) + 1];
        for (auto i = offsets[u]; i < until; ++i)
        {
            const auto id = ids[i];
            const std::optional<std::int64_t> next = std::invoke(transition, id, time);
            if (!next)
            {
                continue;
            }
#ifndef SKIP_RUNTIME_CHECKS
            if (better(*next, time))
            {
                throw std::invalid_argument("timetable: transition reverses time");
            }
#endif
            const int v = Reverse ? edges[id].first : edges[id].second;
            if (!times[v] || better(*next, *times[v]))
            {
                times[v] = *next;
                pending.emplace(*next, v);
            }
        }
    }
    return times;
}
} // namespace detail

template <class Arrival>
std::vector<std::optional<std::int64_t>> earliest_arrival_times(
    int n, std::span<const std::pair<int, int>> edges, int source, std::int64_t start_time, Arrival arrival)
{
    return detail::search_timetable<false>(n, edges, source, start_time, std::move(arrival));
}

template <class Departure>
std::vector<std::optional<std::int64_t>> latest_departure_times(
    int n, std::span<const std::pair<int, int>> edges, int target, std::int64_t deadline, Departure departure)
{
    return detail::search_timetable<true>(n, edges, target, deadline, std::move(departure));
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
#include <optional>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    verify_in >> n >> m >> source >> target;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(2 * m);
    std::vector<std::pair<std::int64_t, std::int64_t>> schedule(m);
    for (auto &[length, interval] : schedule)
    {
        int u{}, v{};
        verify_in >> u >> v >> length >> interval;
        edges.emplace_back(u - 1, v - 1);
        edges.emplace_back(v - 1, u - 1);
    }
    const auto arrival = [&](std::size_t edge, std::int64_t ready) -> std::optional<std::int64_t>
    {
        const auto [length, interval] = schedule[edge / 2];
        const auto remainder = ready % interval;
        return ready + (remainder == 0 ? 0 : interval - remainder) + length;
    };
    const auto times = sirkit::earliest_arrival_times(n, edges, source - 1, 0, arrival);
    verify_out << times[target - 1].value_or(-1) << '\n';
}
