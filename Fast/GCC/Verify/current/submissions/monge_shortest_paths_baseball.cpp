#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc355/tasks/abc355_g

// Library: Algorithm/Monge_Shortest_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Monge_Shortest_Paths.cpp
// 参考（Larmore・Schieberのオンライン行列探索）: https://web.cs.unlv.edu/larmore/Research/ltol.02.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monge_shortest_paths.md
//
// API一覧
// monge_shortest_paths(n,cost,initial={}): 0..n-1の頂点でj<iの辺j→iを持つDAGの頂点0からの最短路。
// cost(a,c)+cost(b,d)<=cost(a,d)+cost(b,c)（a<b<c<d）が必要。
// cost(j,i)はj<iかつjの距離を確定したあとだけ呼ぶ。iの呼び出し順は単調とは限らない。
// distances[i]は最短距離、predecessors[i]は同値のとき最小の直前頂点。始点の直前頂点は-1。
// cost・加算・比較がO(1)なら時間・領域ともO(n+1)。加算と比較は正確に行えること。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONGE_SHORTEST_PATHS_DEFINED
#define SIRKIT_MONGE_SHORTEST_PATHS_DEFINED

template<class T>
struct MongeShortestPathResult
{
    std::vector<T> distances;
    std::vector<int> predecessors;
};

namespace detail
{
template<class T, class Cost>
class OnlineMongeRows
{
public:
    OnlineMongeRows(int rows, const std::vector<T> &distance, Cost &cost)
        : distance_(distance), cost_(cost)
    {
        int stride = 1;
        for (;;)
        {
            levels_.emplace_back(rows, stride);
            if (rows < 2)
            {
                break;
            }
            rows /= 2;
            stride *= 2;
        }
    }

    T evaluate(int from, int to) const
    {
        return distance_[from] + std::invoke(cost_, from, to);
    }

    int next()
    {
        return NextRow(0);
    }

private:
    struct Column
    {
        int local;
        int original;
    };

    struct Level
    {
        int rows;
        int stride;
        int next_row = 0;
        int previous = 0;
        int pending = 0;
        std::vector<Column> reduced;

        Level(int row_count, int row_stride) : rows(row_count), stride(row_stride)
        {
            reduced.reserve(rows / 2);
        }
    };

    const std::vector<T> &distance_;
    Cost &cost_;
    std::vector<Level> levels_;

    int OriginalColumn(int level, int column) const
    {
        return level == 0 ? column : levels_[level - 1].reduced[column].original;
    }

    bool Better(int level, int row, int current, int candidate) const
    {
        const int to = (row + 1) * levels_[level].stride;
        return evaluate(OriginalColumn(level, candidate), to) <
               evaluate(OriginalColumn(level, current), to);
    }

    void ReduceColumns(int level, int row)
    {
        auto &state = levels_[level];
        const int first = row == 0 ? 0 : row - 1;
        for (int candidate = first; candidate <= row; ++candidate)
        {
            while (!state.reduced.empty())
            {
                const int k = static_cast<int>(state.reduced.size()) - 1;
                // 下位のk行は元の2k+1行で、2k列までを使う。
                // 新しい列が範囲外なら、それまでに確定した列を取り除かない。
                if (candidate > 2 * k || !Better(level, 2 * k + 1, state.reduced.back().local, candidate))
                {
                    break;
                }
                state.reduced.pop_back();
            }
            if (state.reduced.size() < static_cast<std::size_t>(state.rows / 2))
            {
                state.reduced.push_back({candidate, OriginalColumn(level, candidate)});
            }
        }
    }

    int NextRow(int level)
    {
        auto &state = levels_[level];
        const int row = state.next_row++;
        if (row % 2 != 0)
        {
            // 直前の処理では未確定だった最後の列だけを比べる。
            state.previous = Better(level, row, state.pending, row) ? row : state.pending;
            return state.previous;
        }

        int upper = row;
        if (row + 1 < state.rows)
        {
            ReduceColumns(level, row);
            const int reduced = NextRow(level + 1);
            upper = state.reduced[reduced].local;
        }
        state.pending = upper;
        // 前の行と、現在の列だけを使った次の行の答えが探索範囲を挟む。
        // 各段でこの範囲を左から右へ詰めるため、走査の合計は行数に比例する。
        int best = state.previous;
        for (int column = best + 1; column <= upper; ++column)
        {
            if (Better(level, row, best, column))
            {
                best = column;
            }
        }
        state.previous = best;
        return best;
    }
};
} // namespace detail

template<class Cost, class T = std::remove_cvref_t<std::invoke_result_t<Cost &, int, int>>>
MongeShortestPathResult<T> monge_shortest_paths(int n, Cost cost, std::type_identity_t<T> initial = {})
{
    MongeShortestPathResult<T> result;
    if (n == 0)
    {
        return result;
    }
    result.distances.reserve(n);
    result.predecessors.reserve(n);
    result.distances.push_back(std::move(initial));
    result.predecessors.push_back(-1);
    detail::OnlineMongeRows<T, Cost> rows(n - 1, result.distances, cost);
    for (int to = 1; to < n; ++to)
    {
        const int from = rows.next();
        result.predecessors.push_back(from);
        result.distances.push_back(rows.evaluate(from, to));
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


#include <compare>
#include <cstdint>
#include <numeric>
#include <vector>

namespace
{
struct State
{
    std::int64_t value;
    int edges;

    State operator+(State other) const
    {
        return {value + other.value, edges + other.edges};
    }

    auto operator<=>(const State &) const = default;
};
}

int main()
{
    int n{}, k{};
    verify_in >> n >> k;
    std::vector<std::int64_t> sum(n + 1), moment(n + 1);
    for (int i = 1; i <= n; ++i)
    {
        std::int64_t p{};
        verify_in >> p;
        sum[i] = sum[i - 1] + p;
        moment[i] = moment[i - 1] + p * i;
    }

    const auto evaluate = [&](std::int64_t penalty) {
        const auto result = sirkit::monge_shortest_paths(n + 1, [&](int from, int to) -> State {
            if (from == 0)
            {
                return {penalty + to * sum[to - 1] - moment[to - 1], 1};
            }
            const int mid = (from + to) / 2;
            const auto left = (moment[mid] - moment[from]) - from * (sum[mid] - sum[from]);
            const auto right = to * (sum[to - 1] - sum[mid]) - (moment[to - 1] - moment[mid]);
            return {penalty + left + right, 1};
        });
        // 番兵の終点へは、選んだ位置を1個以上通ってから到達する。
        const auto suffix = [&](int from) -> State {
            return {penalty + moment[n] - moment[from] - from * (sum[n] - sum[from]), 1};
        };
        State best = result.distances[1] + suffix(1);
        for (int from = 2; from <= n; ++from)
        {
            const State value = result.distances[from] + suffix(from);
            if (value < best)
            {
                best = value;
            }
        }
        return best;
    };

    std::int64_t low = -1, high = sum[n] * n + 1;
    while (high - low > 1)
    {
        const auto mid = std::midpoint(low, high);
        if (evaluate(mid).edges <= k + 1)
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }
    verify_out << evaluate(high).value - (k + 1) * high << '\n';
}
