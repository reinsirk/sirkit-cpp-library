// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: ローカル入力例回帰

// Library: Graph/Global_Min_Cut.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Global_Min_Cut.cpp
// 参考: Stoer & Wagner, A Simple Min-Cut Algorithm, https://doi.org/10.1145/263867.263872
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/global_min_cut.md
//
// API一覧
// GlobalMinCutEdge {u,v,weight=1}: 無向辺と非負のuint64_t重み。
// GlobalMinCutResult {weight,side}: 最小カットの重みと片側の頂点一覧（順不同）。
// global_min_cut(n,edges): 全頂点を二つの空でない集合に分けるときの最小カット。n<2ではnullopt。
// 平行辺は加算し、自己辺は無視する。自己辺以外の全重みの合計はuint64_tに収める。
// Stoer–Wagner法。n頂点m辺でO(n³+m)時間・O(n²)領域。入力は変更しない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GLOBAL_MIN_CUT_DEFINED
#define SIRKIT_GLOBAL_MIN_CUT_DEFINED
struct GlobalMinCutEdge
{
    int u = 0;
    int v = 0;
    std::uint64_t weight = 1;
};

struct GlobalMinCutResult
{
    std::uint64_t weight = 0;
    std::vector<int> side;
};

namespace detail
{
inline std::size_t GlobalCutMatrixSize(std::size_t n, std::size_t max_cells)
{
    if (n != 0 && n > max_cells / n)
    {
        throw std::length_error("global_min_cut: graph exceeds matrix capacity");
    }
    return n * n;
}
}

inline std::optional<GlobalMinCutResult> global_min_cut(int n, std::span<const GlobalMinCutEdge> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("global_min_cut: negative vertex count");
    }
#endif
    std::vector<std::uint64_t> matrix;
    matrix.resize(detail::GlobalCutMatrixSize(n, matrix.max_size()));
    const auto row = [&](int vertex)
    {
        return matrix.data() + static_cast<std::size_t>(vertex) * n;
    };
    std::uint64_t total = 0;
    for (const auto &[u, v, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("global_min_cut: invalid endpoint");
        }
#endif
        if (u == v)
        {
            continue;
        }
        if (weight > std::numeric_limits<std::uint64_t>::max() - total)
        {
            throw std::overflow_error("global_min_cut: total edge weight does not fit");
        }
        total += weight;
        row(u)[v] += weight;
        row(v)[u] += weight;
    }
    if (n < 2)
    {
        return std::nullopt;
    }

    std::vector<int> active(n), tail(n), next(n, -1);
    std::iota(active.begin(), active.end(), 0);
    std::iota(tail.begin(), tail.end(), 0);
    std::vector<std::uint64_t> adjacent_weight(n);
    std::optional<GlobalMinCutResult> best;
    while (active.size() >= 2)
    {
        std::fill(adjacent_weight.begin(), adjacent_weight.end(), std::uint64_t{0});
        // すでに選んだ頂点への重み和が最大の頂点を選ぶ。
        for (std::size_t i = 0; i + 1 < active.size(); ++i)
        {
            auto max_index = i;
            for (auto j = i + 1; j < active.size(); ++j)
            {
                if (adjacent_weight[active[j]] > adjacent_weight[active[max_index]])
                {
                    max_index = j;
                }
            }
            std::swap(active[i], active[max_index]);
            const auto *from = row(active[i]);
            for (auto j = i + 1; j < active.size(); ++j)
            {
                const auto to = active[j];
                adjacent_weight[to] += from[to];
            }
        }
        const auto last = active.back();
        active.pop_back();
        const auto previous = active.back();
        const auto cut = adjacent_weight[last];
        if (!best || cut < best->weight)
        {
            best.emplace();
            best->weight = cut;
            for (int v = last; v >= 0; v = next[v])
            {
                best->side.push_back(v);
            }
            if (cut == 0)
            {
                return best;
            }
        }

        // 最後の2頂点を縮約し、元の頂点を連結リストで追跡する。
        const auto *removed = row(last);
        auto *merged = row(previous);
        for (std::size_t i = 0; i + 1 < active.size(); ++i)
        {
            const auto v = active[i];
            merged[v] += removed[v];
            row(v)[previous] = merged[v];
        }
        next[tail[previous]] = last;
        tail[previous] = tail[last];
    }
    return best;
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
#include <vector>

int main()
{
    int n;
    std::size_t m;
    verify_in >> n >> m;
    std::vector<GlobalMinCutEdge> edges(m);
    for (auto &[u, v, w] : edges)
    {
        verify_in >> u >> v >> w;
    }
    if (const auto cut = global_min_cut(n, edges))
    {
        verify_out << cut->weight << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
