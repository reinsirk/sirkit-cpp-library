#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 1755b15dc0f329699403e863dc6d5dbd6747f93c83548e4f43a52ffff4f5948e
// Verify: https://atcoder.jp/contests/arc208/tasks/arc208_d

// Library: Graph/Round_Robin_Schedule.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/round_robin_schedule.md
// 参考（完全グラフの辺彩色）: https://info.atcoder.jp/entry/algorithm_lectures/complete_graph_edge_coloring
//
// API一覧
// [RoundRobinSchedule]
// RoundRobinSchedule(n): n人の総当たり戦を最小のラウンド数で配置。1ラウンドにつき1人1試合まで。
// size()/round_count(): 選手数/ラウンド数。n<=1なら0ラウンド。
// partner(r,u): ラウンドrの選手uの対戦相手。試合がなければ-1。
// round_of(u,v): 選手u,vの試合のラウンド。同じ選手なら-1。
// 番号はすべて0始まり。負のnはinvalid_argument、範囲外の照会はout_of_range。
// 全操作O(1)時間、保持する領域もO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROUND_ROBIN_SCHEDULE_DEFINED
#define SIRKIT_ROUND_ROBIN_SCHEDULE_DEFINED

class RoundRobinSchedule
{
    int players_;

    void check_player(int player) const
    {
        if (player < 0 || player >= players_)
        {
            throw std::out_of_range("round-robin schedule: player out of range");
        }
    }

public:
    explicit RoundRobinSchedule(int players) : players_(players)
    {
        if (players < 0)
        {
            throw std::invalid_argument("round-robin schedule: negative number of players");
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return players_;
    }

    [[nodiscard]] int round_count() const noexcept
    {
        if (players_ <= 1)
        {
            return 0;
        }
        return players_ - (players_ % 2 == 0);
    }

    [[nodiscard]] int partner(int round, int player) const
    {
        check_player(player);
        const int circle = round_count();
        if (round < 0 || round >= circle)
        {
            throw std::out_of_range("round-robin schedule: round out of range");
        }
        if (player == round)
        {
            return circle < players_ ? circle : -1;
        }
        if (player == circle)
        {
            return round;
        }
        // 円周上の対称位置。偶数人のとき、円外の1人はそのラウンドの中心と対戦する。
        return static_cast<int>((2 * std::int64_t{round} + circle - player) % circle);
    }

    [[nodiscard]] int round_of(int u, int v) const
    {
        check_player(u);
        check_player(v);
        if (u == v)
        {
            return -1;
        }
        const int circle = round_count();
        if (u == circle)
        {
            return v;
        }
        if (v == circle)
        {
            return u;
        }
        const auto sum = (std::int64_t{u} + v) % circle;
        return static_cast<int>((sum + (sum % 2 == 0 ? 0 : circle)) / 2);
    }
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
#include <utility>
#include <vector>

namespace
{
void solve()
{
    int n{};
    verify_in >> n;
    const auto size = static_cast<std::size_t>(n);
    std::vector<int> requested(size), fixed;
    for (int &partner : requested)
    {
        verify_in >> partner;
        --partner;
    }
    for (int i = 0; i < n; ++i)
    {
        const int partner = requested[static_cast<std::size_t>(i)];
        if (requested[static_cast<std::size_t>(partner)] != i)
        {
            verify_out << "No\n";
            return;
        }
        if (partner == i)
        {
            fixed.push_back(i);
        }
    }
    if (n % 2 != 0 && fixed.size() != 1)
    {
        verify_out << "No\n";
        return;
    }
    verify_out << "Yes\n";
    if (n == 1)
    {
        verify_out << "1\n";
        return;
    }

    const sirkit::RoundRobinSchedule schedule(n);
    std::vector<std::pair<int, int>> pairs;
    for (int i = 0; i < n; ++i)
    {
        const int j = schedule.partner(0, i);
        if (i < j)
        {
            pairs.emplace_back(i, j);
        }
    }

    std::size_t chosen = 0;
    std::vector<int> renumbered(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        const auto j = static_cast<std::size_t>(requested[i]);
        if (i < j)
        {
            const auto [a, b] = pairs[chosen++];
            renumbered[i] = a;
            renumbered[j] = b;
        }
    }
    if (n % 2 != 0)
    {
        renumbered[static_cast<std::size_t>(fixed.front())] = 0;
    }
    else
    {
        for (std::size_t i = 0; i < fixed.size(); i += 2)
        {
            const auto [a, b] = pairs[chosen++];
            renumbered[static_cast<std::size_t>(fixed[i])] = a;
            renumbered[static_cast<std::size_t>(fixed[i + 1])] = b;
        }
    }

    for (std::size_t u = 0; u < size; ++u)
    {
        for (std::size_t v = 0; v < size; ++v)
        {
            int value{};
            if (u == v)
            {
                value = n % 2 != 0 ? renumbered[u] : n - 1;
                if (requested[u] == static_cast<int>(u))
                {
                    value = 0;
                }
            }
            else
            {
                value = schedule.round_of(renumbered[u], renumbered[v]);
                if (value == 0 && requested[u] == static_cast<int>(u))
                {
                    value = n - 1;
                }
            }
            verify_out << value + 1 << (v + 1 == size ? '\n' : ' ');
        }
    }
}
} // namespace

int main()
{
    int cases{};
    verify_in >> cases;
    for (int i = 0; i < cases; ++i)
    {
        solve();
    }
}
