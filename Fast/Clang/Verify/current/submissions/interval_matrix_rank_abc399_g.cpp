#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc399/tasks/abc399_g

// Library: Math/Interval_Matrix_Rank.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Interval_Matrix_Rank.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_matrix_rank.md
// 参考（区間の階数）: https://atcoder.jp/contests/abc399/editorial/12546
//
// API一覧
// interval_matrix_rank_ends<Mod=998244353>(rows,rank):
// 各左端lについて行区間[l,r)の階数がrank以上になる最小のrを返す。なければnullopt。
// rowsは長方形のuint32_t行列。各値を素数Modで割った余りとして扱う。
// rank=0なら各lの答えはl。空の行列に対する答えは空。行幅が違えばinvalid_argument。
// 行数H、列数Wに対しO(HW(W+log Mod)+H)時間、O(W^2+H)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTERVAL_MATRIX_RANK_DEFINED
#define SIRKIT_INTERVAL_MATRIX_RANK_DEFINED

namespace detail
{
consteval bool IntervalMatrixRankPrime(std::uint32_t mod)
{
    if (mod < 2)
    {
        return false;
    }
    for (std::uint32_t factor = 2; factor <= mod / factor; ++factor)
    {
        if (mod % factor == 0)
        {
            return false;
        }
    }
    return true;
}

template <std::uint32_t Mod>
std::uint32_t IntervalMatrixRankInverse(std::uint32_t value)
{
    std::uint32_t result = 1;
    for (std::uint32_t exponent = Mod - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = static_cast<std::uint32_t>(std::uint64_t{result} * value % Mod);
        }
        value = static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }
    return result;
}

template <std::uint32_t Mod>
void NormalizeIntervalMatrixRankPivot(std::vector<std::uint32_t> &row, std::size_t column)
{
    const auto inverse = IntervalMatrixRankInverse<Mod>(row[column]);
    row[column] = 1;
    for (std::size_t i = column + 1; i < row.size(); ++i)
    {
        row[i] = static_cast<std::uint32_t>(std::uint64_t{row[i]} * inverse % Mod);
    }
}

struct TaggedIntervalMatrixPivot
{
    std::vector<std::uint32_t> row;
    std::size_t index{};
};
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::optional<std::size_t>> interval_matrix_rank_ends(
    std::span<const std::vector<std::uint32_t>> rows, std::size_t rank)
{
    static_assert(detail::IntervalMatrixRankPrime(Mod), "interval_matrix_rank_ends: Mod must be prime");
    if (rows.empty())
    {
        return {};
    }
    const auto width = rows.front().size();
    for (const auto &row : rows)
    {
        if (row.size() != width)
        {
            throw std::invalid_argument("interval_matrix_rank_ends: inconsistent row widths");
        }
    }
    std::vector<std::optional<std::size_t>> ends(rows.size());
    if (rank == 0)
    {
        for (std::size_t left = 0; left < rows.size(); ++left)
        {
            ends[left] = left;
        }
        return ends;
    }
    if (rank > width)
    {
        return ends;
    }

    std::vector<detail::TaggedIntervalMatrixPivot> basis(width);
    std::vector<std::size_t> indexes;
    indexes.reserve(std::min(rows.size(), width));
    for (std::size_t next = rows.size(); next != 0; --next)
    {
        const auto left = next - 1;
        auto index = left;
        auto row = rows[left];
        for (auto &value : row)
        {
            value %= Mod;
        }
        for (std::size_t column = 0; column < width; ++column)
        {
            if (row[column] == 0)
            {
                continue;
            }
            auto &pivot = basis[column];
            if (pivot.row.empty())
            {
                detail::NormalizeIntervalMatrixRankPivot<Mod>(row, column);
                pivot = {std::move(row), index};
                break;
            }
            // 同じ列のピボットは右端が小さい方を残す。
            if (index < pivot.index)
            {
                detail::NormalizeIntervalMatrixRankPivot<Mod>(row, column);
                row.swap(pivot.row);
                std::swap(index, pivot.index);
            }
            const auto factor = row[column];
            row[column] = 0;
            for (std::size_t i = column + 1; i < width; ++i)
            {
                const auto sub = static_cast<std::uint32_t>(std::uint64_t{factor} * pivot.row[i] % Mod);
                row[i] = row[i] >= sub ? row[i] - sub : row[i] + (Mod - sub);
            }
        }
        indexes.clear();
        for (const auto &pivot : basis)
        {
            if (!pivot.row.empty())
            {
                indexes.push_back(pivot.index);
            }
        }
        if (indexes.size() >= rank)
        {
            std::sort(indexes.begin(), indexes.end());
            ends[left] = indexes[rank - 1] + 1;
        }
    }
    return ends;
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


#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Edge
{
    std::size_t u, v, color;
};

std::vector<std::vector<std::uint32_t>> SampleIntersectionMatrix(
    std::size_t rank, const std::vector<std::size_t> &offsets, const std::vector<Edge> &edges, std::mt19937_64 &random)
{
    std::vector<std::vector<std::uint32_t>> rows(offsets.back(), std::vector<std::uint32_t>(rank));
    std::uniform_int_distribution<std::uint32_t> coefficients(0, kMod - 1);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[u, v, color] = edges[i];
        const auto point = i + 1;
        auto term = coefficients(random);
        for (std::size_t row = offsets[color]; row < offsets[color + 1]; ++row)
        {
            rows[row][u] = (rows[row][u] + term) % kMod;
            if (v < rank)
            {
                rows[row][v] = (rows[row][v] + kMod - term) % kMod;
            }
            term = static_cast<std::uint32_t>(std::uint64_t{term} * point % kMod);
        }
    }
    return rows;
}
} // namespace

int main()
{
    std::size_t n{}, m{}, colors{};
    verify_in >> n >> m >> colors;
    std::vector<std::size_t> offsets(colors + 1);
    for (std::size_t color = 0; color < colors; ++color)
    {
        std::size_t count{};
        verify_in >> count;
        offsets[color + 1] = offsets[color] + count;
    }
    std::vector<Edge> edges(m);
    for (auto &[u, v, color] : edges)
    {
        verify_in >> u >> v >> color;
        --u;
        --v;
        --color;
    }
    const auto rank = n - 1;
    std::vector<std::size_t> right(colors, offsets.back() + 1);
    std::mt19937_64 random(std::random_device{}());
    // 独立した3組の乱数で共通独立集合の最大サイズの下界を調べる。
    for (int trial = 0; trial < 3; ++trial)
    {
        const auto rows = SampleIntersectionMatrix(rank, offsets, edges, random);
        const auto ends = sirkit::interval_matrix_rank_ends(rows, rank);
        for (std::size_t color = 0; color < colors; ++color)
        {
            if (const auto end = ends[offsets[color]])
            {
                right[color] = std::min(right[color], end.value());
            }
        }
    }
    std::size_t answer = 0;
    for (const auto end : right)
    {
        const auto first = std::lower_bound(offsets.begin() + 1, offsets.end(), end);
        answer += static_cast<std::size_t>(offsets.end() - first);
    }
    verify_out << answer << '\n';
}
