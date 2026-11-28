#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc219/tasks/abc219_g

// Library: Graph/Neighborhood_Assignment.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Neighborhood_Assignment.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/neighborhood_assignment.md
//
// API一覧
// NeighborhoodAssignment<T>(initial, edges, threshold=0): 無向グラフに初期値を設定。
//   多重辺・自己辺を扱う。threshold=0ならfloor(sqrt(2m))+1。O(n+m)時間・領域。
// size(): 頂点数。O(1)。
// get(v): 頂点vの現在値。しきい値Bに対しO(1+m/B)。
// assign(v, value): 頂点vの値を上書き。O(1)。
// assign_neighbors(v, value, include_self=false): vと隣接する頂点を一括で上書き。
//   include_self=trueならvにも代入。O(1+B)。値のコピーはO(1)と仮定。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NEIGHBORHOOD_ASSIGNMENT_DEFINED
#define SIRKIT_NEIGHBORHOOD_ASSIGNMENT_DEFINED

namespace detail
{
inline int neighborhood_assignment_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("NeighborhoodAssignment: too many vertices");
    }
    return static_cast<int>(size);
}

inline std::uint64_t next_neighborhood_assignment_stamp(std::uint64_t stamp)
{
    if (stamp == std::numeric_limits<std::uint64_t>::max())
    {
        throw std::overflow_error("NeighborhoodAssignment: too many assignments");
    }
    return stamp + 1;
}
} // namespace detail

template <class T>
class NeighborhoodAssignment
{
    static constexpr bool kStoreDirect =
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>;
    using Storage = std::conditional_t<kStoreDirect, T, std::shared_ptr<const T>>;

    struct Assignment
    {
        std::uint64_t stamp;
        Storage value;
    };

public:
    NeighborhoodAssignment(std::span<const T> initial, std::span<const std::pair<int, int>> edges,
                           std::size_t threshold = 0)
        : adjacent_(detail::neighborhood_assignment_size(initial.size())),
          heavy_neighbors_(initial.size()), pending_index_(initial.size(), -1)
    {
        direct_.reserve(initial.size());
        for (const T &value : initial)
        {
            direct_.push_back({0, store(value)});
        }
        for (const auto &[u, v] : edges)
        {
            check_vertex(u);
            check_vertex(v);
            adjacent_[u].push_back(v);
            adjacent_[v].push_back(u);
        }
        if (threshold == 0)
        {
            threshold = static_cast<std::size_t>(std::sqrt(2.0 * static_cast<double>(edges.size()))) + 1;
        }

        std::vector<int> seen(initial.size(), -1);
        for (int v = 0; v < size(); ++v)
        {
            if (adjacent_[v].size() <= threshold)
            {
                continue;
            }
            pending_index_[v] = static_cast<int>(pending_.size());
            pending_.push_back(direct_[v]);
            for (const int u : adjacent_[v])
            {
                if (seen[u] != v)
                {
                    seen[u] = v;
                    heavy_neighbors_[u].push_back(pending_index_[v]);
                }
            }
            std::vector<int>{}.swap(adjacent_[v]);
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(direct_.size());
    }

    [[nodiscard]] T get(int v) const
    {
        check_vertex(v);
        const Assignment *newest = &direct_[v];
        for (const int index : heavy_neighbors_[v])
        {
            const auto &pending = pending_[index];
            if (pending.stamp > newest->stamp)
            {
                newest = &pending;
            }
        }
        if constexpr (kStoreDirect)
        {
            return newest->value;
        }
        else
        {
            return *newest->value;
        }
    }

    void assign(int v, const T &value)
    {
        check_vertex(v);
        const auto change = new_assignment(value);
        direct_[v] = change;
    }

    void assign_neighbors(int v, const T &value, bool include_self = false)
    {
        check_vertex(v);
        const auto change = new_assignment(value);
        if (include_self)
        {
            direct_[v] = change;
        }
        const int index = pending_index_[v];
        if (index != -1)
        {
            pending_[index] = change;
        }
        else
        {
            for (const int neighbor : adjacent_[v])
            {
                direct_[neighbor] = change;
            }
        }
    }

private:
    void check_vertex(int v) const
    {
        static_cast<void>(v);
    }

    static Storage store(const T &value)
    {
        if constexpr (kStoreDirect)
        {
            return value;
        }
        else
        {
            return std::make_shared<const T>(value);
        }
    }

    Assignment new_assignment(const T &value)
    {
        const Assignment change{detail::next_neighborhood_assignment_stamp(stamp_), store(value)};
        stamp_ = change.stamp;
        return change;
    }

    std::vector<std::vector<int>> adjacent_;
    std::vector<std::vector<int>> heavy_neighbors_;
    std::vector<int> pending_index_;
    std::vector<Assignment> direct_;
    std::vector<Assignment> pending_;
    std::uint64_t stamp_ = 0;
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


#include <numeric>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    std::vector<int> initial(n);
    std::iota(initial.begin(), initial.end(), 1);
    sirkit::NeighborhoodAssignment<int> colors(initial, edges);
    while (q-- > 0)
    {
        int v{};
        verify_in >> v;
        --v;
        colors.assign_neighbors(v, colors.get(v));
    }
    for (int v = 0; v < n; ++v)
    {
        verify_out << colors.get(v) << (v + 1 == n ? '\n' : ' ');
    }
}
