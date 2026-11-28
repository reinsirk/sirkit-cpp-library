#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1303

// Library: Math/Matrix_Pencil_Determinant.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Matrix_Pencil_Determinant.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix_pencil_determinant.md
//
// API一覧
// matrix_pencil_determinant<Mod=998244353>(constant,linear): det(constant+x*linear)の係数を定数項から返す。
// 両入力は同じサイズnの正方行列。負のint64_t係数も指定可。Modはuint32_tの素数。
// 返り値はn+1個のuint32_t。行列式が恒等的に0なら全項0、空行列なら{1}。
// 線形係数行列が特異でも可。形状不一致はinvalid_argument。O(n³+n log Mod+1)時間・O(n²+1)領域。
//
// [同梱の特性多項式]
// characteristic_polynomial<Mod=998244353>(matrix): det(xI-A)の係数を定数項から返す。
// Modは32ビット符号なしの素数。符号付き64ビットの正方行列を受け取る。空行列は{1}。
// 入力は変更しない。n×n行列でO(1+n³+n log Mod)時間・O(1+n²)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CHARACTERISTIC_POLYNOMIAL_DEFINED
#define SIRKIT_CHARACTERISTIC_POLYNOMIAL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
consteval bool characteristic_prime(std::uint32_t n)
{
    if (n % 2 == 0 || n < 2)
    {
        return n == 2;
    }
    for (std::uint32_t d = 3; d <= n / d; d += 2)
    {
        if (n % d == 0)
        {
            return false;
        }
    }
    return true;
}
} // namespace detail

template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> characteristic_polynomial(std::span<const std::vector<std::int64_t>> matrix)
{
    static_assert(detail::characteristic_prime(Mod), "characteristic_polynomial: please specify a prime modulus");
    const std::size_t n = matrix.size();
    const auto multiply = [](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    };
    const auto add = [](std::uint32_t a, std::uint32_t b)
    {
        return a < Mod - b ? a + b : a - (Mod - b);
    };
    const auto subtract = [](std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : Mod - (b - a);
    };
    const auto inverse = [&](std::uint32_t base)
    {
        std::uint32_t value = 1;
        for (std::uint32_t exp = Mod - 2; exp != 0; exp >>= 1)
        {
            if (exp & 1)
            {
                value = multiply(value, base);
            }
            base = multiply(base, base);
        }
        return value;
    };
    std::vector<std::vector<std::uint32_t>> h;
    h.reserve(n);
    for (const auto &row : matrix)
    {
        auto &dest = h.emplace_back();
        dest.reserve(n);
        for (std::int64_t x : row)
        {
            const std::int64_t residue = x % static_cast<std::int64_t>(Mod);
            dest.push_back(static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue));
        }
    }

    // 行操作の逆変換を列に施して相似を保ち、下Hessenberg部分を消去する。
    for (std::size_t col = 0; col + 2 < n; ++col)
    {
        const std::size_t next = col + 1;
        std::size_t pivot = next;
        while (pivot < n && h[pivot][col] == 0)
        {
            ++pivot;
        }
        if (pivot == n)
        {
            continue;
        }
        if (pivot != next)
        {
            h[pivot].swap(h[next]);
            for (auto &row : h)
            {
                std::swap(row[pivot], row[next]);
            }
        }
        const std::uint32_t inv = inverse(h[next][col]);
        for (std::size_t row = next + 1; row < n; ++row)
        {
            if (h[row][col] == 0)
            {
                continue;
            }
            const auto scale = multiply(h[row][col], inv);
            h[row][col] = 0;
            for (std::size_t j = next; j < n; ++j)
            {
                h[row][j] = subtract(h[row][j], multiply(scale, h[next][j]));
            }
            for (auto &values : h)
            {
                values[next] = add(values[next], multiply(scale, values[row]));
            }
        }
    }

    // 先頭主小行列の det(xI-H) を最終列で展開する。
    std::vector<std::vector<std::uint32_t>> polynomials(n + 1);
    polynomials[0] = {1};
    for (std::size_t last = 0; last < n; ++last)
    {
        const auto &previous = polynomials[last];
        auto &current = polynomials[last + 1];
        current.resize(last + 2);
        for (std::size_t k = 0; k <= last; ++k)
        {
            current[k] = subtract(current[k], multiply(h[last][last], previous[k]));
            current[k + 1] = previous[k];
        }
        std::uint32_t lower_product = 1;
        for (std::size_t i = last; i > 0; --i)
        {
            lower_product = multiply(lower_product, h[i][i - 1]);
            if (lower_product == 0)
            {
                break;
            }
            const auto scale = multiply(lower_product, h[i - 1][last]);
            for (std::size_t k = 0; k < i; ++k)
            {
                current[k] = subtract(current[k], multiply(scale, polynomials[i - 1][k]));
            }
        }
    }
    return std::move(polynomials.back());
}
#endif

#ifndef SIRKIT_MATRIX_PENCIL_DETERMINANT_DEFINED
#define SIRKIT_MATRIX_PENCIL_DETERMINANT_DEFINED

namespace detail
{
template <std::uint32_t Mod>
class MatrixPencilReducer
{
    using Matrix = std::vector<std::vector<std::uint32_t>>;
    using Input = std::span<const std::vector<std::int64_t>>;

public:
    MatrixPencilReducer(Input constant, Input linear) : constant_(Normalize(constant)), linear_(Normalize(linear))
    {
    }

    [[nodiscard]] std::vector<std::uint32_t> Determinant()
    {
        const auto side = constant_.size();
        std::uint32_t scaling = 1;
        std::size_t degree_shift = 0;
        for (std::size_t i = 0; i < side; ++i)
        {
            auto pivot = Pivot(i);
            while (pivot.first == side)
            {
                if (degree_shift == side)
                {
                    return std::vector<std::uint32_t>(side + 1);
                }
                // 余った行は定数行。この行をx倍して、既に処理した列を消す。
                ++degree_shift;
                constant_[i].swap(linear_[i]);
                for (std::size_t column = 0; column < i; ++column)
                {
                    SubtractRow(i, column, linear_[i][column]);
                }
                pivot = Pivot(i);
            }
            if (pivot.first != i)
            {
                constant_[i].swap(constant_[pivot.first]);
                linear_[i].swap(linear_[pivot.first]);
                scaling = Subtract(0, scaling);
            }
            if (pivot.second != i)
            {
                for (std::size_t row = 0; row < side; ++row)
                {
                    std::swap(constant_[row][i], constant_[row][pivot.second]);
                    std::swap(linear_[row][i], linear_[row][pivot.second]);
                }
                scaling = Subtract(0, scaling);
            }
            const auto diagonal = linear_[i][i];
            const auto inverse = Inverse(diagonal);
            scaling = Multiply(scaling, diagonal);
            for (auto &value : constant_[i])
            {
                value = Multiply(value, inverse);
            }
            for (std::size_t j = i; j < side; ++j)
            {
                linear_[i][j] = Multiply(linear_[i][j], inverse);
            }
            for (std::size_t row = i + 1; row < side; ++row)
            {
                SubtractRow(row, i, linear_[row][i]);
            }
        }

        std::vector<std::vector<std::int64_t>> converted(side, std::vector<std::int64_t>(side));
        for (std::size_t reverse = side; reverse != 0; --reverse)
        {
            const auto i = reverse - 1;
            for (std::size_t j = 0; j < side; ++j)
            {
                auto value = Subtract(0, constant_[i][j]);
                for (std::size_t row = i + 1; row < side; ++row)
                {
                    value = Subtract(value, Multiply(linear_[i][row], static_cast<std::uint32_t>(converted[row][j])));
                }
                converted[i][j] = value;
            }
        }
        auto coefficients = characteristic_polynomial<Mod>(converted);
        coefficients.erase(coefficients.begin(), coefficients.begin() + static_cast<std::ptrdiff_t>(degree_shift));
        for (auto &coefficient : coefficients)
        {
            coefficient = Multiply(coefficient, scaling);
        }
        coefficients.resize(side + 1);
        return coefficients;
    }

private:
    [[nodiscard]] static Matrix Normalize(Input matrix)
    {
        Matrix result;
        result.reserve(matrix.size());
        for (const auto &row : matrix)
        {
            if (row.size() != matrix.size())
            {
                throw std::invalid_argument("matrix_pencil_determinant: matrix must be square");
            }
            auto &values = result.emplace_back();
            values.reserve(row.size());
            for (const auto value : row)
            {
                const auto remainder = value % static_cast<std::int64_t>(Mod);
                values.push_back(static_cast<std::uint32_t>(remainder < 0 ? remainder + Mod : remainder));
            }
        }
        return result;
    }

    [[nodiscard]] static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }

    [[nodiscard]] static std::uint32_t Subtract(std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : Mod - (b - a);
    }

    [[nodiscard]] static std::uint32_t Inverse(std::uint32_t value)
    {
        std::uint32_t inverse = 1;
        for (std::uint32_t exponent = Mod - 2; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                inverse = Multiply(inverse, value);
            }
            value = Multiply(value, value);
        }
        return inverse;
    }

    [[nodiscard]] std::pair<std::size_t, std::size_t> Pivot(std::size_t first) const
    {
        const auto side = linear_.size();
        for (std::size_t row = first; row < side; ++row)
        {
            for (std::size_t column = first; column < side; ++column)
            {
                if (linear_[row][column] != 0)
                {
                    return {row, column};
                }
            }
        }
        return {side, side};
    }

    void SubtractRow(std::size_t target, std::size_t pivot, std::uint32_t factor)
    {
        if (factor == 0)
        {
            return;
        }
        const auto side = constant_.size();
        for (std::size_t j = 0; j < side; ++j)
        {
            constant_[target][j] = Subtract(constant_[target][j], Multiply(constant_[pivot][j], factor));
        }
        for (std::size_t j = pivot; j < side; ++j)
        {
            linear_[target][j] = Subtract(linear_[target][j], Multiply(linear_[pivot][j], factor));
        }
    }

    Matrix constant_;
    Matrix linear_;
};
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::uint32_t> matrix_pencil_determinant(
    std::span<const std::vector<std::int64_t>> constant,
    std::span<const std::vector<std::int64_t>> linear)
{
    static_assert(detail::characteristic_prime(Mod), "matrix_pencil_determinant: modulus must be prime");
    if (constant.size() != linear.size())
    {
        throw std::invalid_argument("matrix_pencil_determinant: matrix dimensions differ");
    }
    return detail::MatrixPencilReducer<Mod>(constant, linear).Determinant();
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

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Matrix = std::vector<std::vector<std::int64_t>>;
using Adjacency = std::vector<std::vector<bool>>;

void AddEdge(Matrix &matrix, std::size_t u, std::size_t v)
{
    if (u < matrix.size())
    {
        ++matrix[u][u];
    }
    if (v < matrix.size())
    {
        ++matrix[v][v];
        --matrix[u][v];
        --matrix[v][u];
    }
}

std::uint32_t CountTrees(const Adjacency &graph, const std::vector<std::size_t> &vertices, bool allow_special_edge)
{
    const auto dimension = vertices.size() - 1;
    Matrix roads(dimension, std::vector<std::int64_t>(dimension));
    Matrix additions(dimension, std::vector<std::int64_t>(dimension));
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        for (std::size_t j = i + 1; j < vertices.size(); ++j)
        {
            if (graph[vertices[i]][vertices[j]])
            {
                AddEdge(roads, i, j);
            }
            else if (allow_special_edge)
            {
                AddEdge(additions, i, j);
            }
        }
    }
    const auto counts = sirkit::matrix_pencil_determinant(roads, additions);
    return allow_special_edge ? (counts[0] + counts[1]) % kMod : counts[0];
}
} // namespace

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    Adjacency graph(n, std::vector<bool>(n));
    for (std::size_t i = 0; i < m; ++i)
    {
        std::size_t u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        graph[u][v] = graph[v][u] = true;
    }

    std::vector<bool> visited(n);
    std::vector<std::vector<std::size_t>> components;
    for (std::size_t u = 0; u < n; ++u)
    {
        if (visited[u])
        {
            continue;
        }
        auto &component = components.emplace_back(1, u);
        visited[u] = true;
        for (std::size_t i = 0; i < component.size(); ++i)
        {
            for (std::size_t v = 0; v < n; ++v)
            {
                if (!visited[v] && graph[component[i]][v])
                {
                    visited[v] = true;
                    component.push_back(v);
                }
            }
        }
    }
    if (components.size() == 1)
    {
        verify_out << 0 << '\n'
                   << CountTrees(graph, components.front(), true) << '\n';
        return 0;
    }

    auto inconvenient = n * n;
    std::uint64_t ways = 1;
    std::size_t best_product = 0, bridge_count = 0;
    for (std::size_t i = 0; i < components.size(); ++i)
    {
        const auto count = components[i].size();
        inconvenient -= count * count;
        ways = ways * CountTrees(graph, components[i], false) % kMod;
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto product = components[j].size() * count;
            if (product > best_product)
            {
                best_product = product;
                bridge_count = product;
            }
            else if (product == best_product)
            {
                bridge_count += product;
            }
        }
    }
    verify_out << inconvenient - 2 * best_product << '\n'
               << ways * bridge_count % kMod << '\n';
}
