#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/dirichlet_inverse_and_prefix_sums

// Library: Math/Dirichlet_Prefix_Sums.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Dirichlet_Prefix_Sums.cpp
// 参考（積xyz<=Nの分割）: https://codeforces.com/blog/entry/117635?#comment-1041002
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dirichlet_prefix_sums.md
//
// API一覧
// [DirichletPrefixSums<T>]
// DirichletPrefixSums(n): 正整数nについて、相異なるfloor(n/i)を昇順に並べた座標を準備する。O(log log(n+2))時間・O(1)領域。
// size(): 座標数。quotient(index): 0始まりの指定座標。どちらもO(1)時間。
// convolution(left,right): 座標順の2つの累積和から、Dirichlet積の累積和を返す。
// divide(product,divisor,inverse_at_one): divisorとのDirichlet積がproductとなる数列の累積和を返す。
// inverse(divisor,inverse_at_one): Dirichlet積の逆元の累積和。inverse_at_oneはdivisorの第1項の乗法逆元。
// 累積和の入出力はsize()要素。T{}は0、T{1}は1。Tに可換環の加減乗算と等値比較が必要。
// 積・商・逆元はO(n^(2/3))回のT演算・O(sqrt(n))追加領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DIRICHLET_PREFIX_SUMS_DEFINED
#define SIRKIT_DIRICHLET_PREFIX_SUMS_DEFINED
template <class T>
class DirichletPrefixSums
{
    using u64 = std::uint64_t;

    u64 n_;
    u64 small_;
    u64 large_;

    static u64 square_root(u64 value) noexcept
    {
        u64 upper = u64{1} << ((std::bit_width(value) + 1) / 2);
        for (u64 next = (upper + value / upper) / 2; next < upper; next = (upper + value / upper) / 2)
        {
            upper = next;
        }
        return upper;
    }

    std::size_t table_size() const
    {
        if constexpr (sizeof(std::size_t) < sizeof(u64))
        {
            if (size() >= std::numeric_limits<std::size_t>::max())
            {
                throw std::length_error("DirichletPrefixSums: table size");
            }
        }
        return static_cast<std::size_t>(size());
    }

    void check_shape([[maybe_unused]] std::span<const T> values) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (values.size() != size())
        {
            throw std::invalid_argument("DirichletPrefixSums: prefix table size");
        }
#endif
    }

    static const T &prefix(std::span<const T> values, u64 one_based)
    {
        return values[static_cast<std::size_t>(one_based - 1)];
    }

    std::vector<T> coefficients(std::span<const T> values) const
    {
        std::vector<T> result(static_cast<std::size_t>(small_) + 1);
        result[1] = values[0];
        for (u64 i = 2; i <= small_; ++i)
        {
            result[i] = prefix(values, i) - prefix(values, i - 1);
        }
        return result;
    }

    std::vector<T> diagonal_prefix(const std::vector<T> &left, const std::vector<T> &right) const
    {
        std::vector<T> result(static_cast<std::size_t>(small_) + 1);
        for (u64 i = 1; i <= small_; ++i)
        {
            result[i] = result[i - 1] + left[i] * right[i];
        }
        return result;
    }

    // 出力座標がx=n/zのとき、max(a,b)<=zとなる項を区間加算する。
    std::vector<T> small_pairs(const std::vector<T> &left, const std::vector<T> &right) const
    {
        std::vector<T> differences(static_cast<std::size_t>(large_) + 1);
        for (u64 b = 1; b <= large_; ++b)
        {
            const u64 after = large_ - b + 1;
            const u64 limit = std::min(b, n_ / b / b);
            T total{};
            for (u64 a = 1; a <= limit; ++a)
            {
                const auto product = a * b;
                const auto first = product <= small_ ? 0 : large_ - n_ / product;
                const auto value = a == b ? left[a] * right[b] : left[a] * right[b] + left[b] * right[a];
                differences[first] += value;
                total += value;
            }
            differences[after] -= total;
        }
        return differences;
    }

    // 残りのmax(a,b)>zの項を、小さい方の添字ごとに区間和でまとめる。
    T large_pairs(u64 z, u64 x, u64 first, std::span<const T> left, std::span<const T> right,
                  const std::vector<T> &left_coefficients, const std::vector<T> &right_coefficients,
                  const std::vector<T> &diagonal) const
    {
        const u64 root = square_root(x);
        const u64 limit = std::min(root, x / (z + 1));
        const u64 large_limit = large_ / z;
        T value = root > z ? diagonal[root] - diagonal[z] : T{};
        for (u64 a = first; a <= limit; ++a)
        {
            const auto low = std::max(a, z);
            const auto high = a <= large_limit ? size() - a * z + 1 : x / a;
            value += left_coefficients[a] * (prefix(right, high) - prefix(right, low)) +
                     right_coefficients[a] * (prefix(left, high) - prefix(left, low));
        }
        return value;
    }

    template <class Product>
    std::vector<T> solve(std::span<const T> divisor, T inverse_at_one, const Product &product) const
    {
        const auto count = table_size();
        check_shape(divisor);
#ifndef SKIP_RUNTIME_CHECKS
        if (!(divisor[0] * inverse_at_one == T{1}))
        {
            throw std::invalid_argument("DirichletPrefixSums: invalid inverse_at_one");
        }
#endif
        const auto divisor_coefficients = coefficients(divisor);
        const auto dense_size = static_cast<std::size_t>(small_) + 1;
        std::vector<T> answer(count), solved(dense_size), pending(dense_size);
        T previous_product{}, total{};
        for (u64 i = 1; i <= small_; ++i)
        {
            const T next_product = product(i);
            solved[i] = (next_product - previous_product - pending[i]) * inverse_at_one;
            previous_product = next_product;
            total += solved[i];
            answer[i - 1] = total;
            for (u64 j = 2; j <= small_ / i; ++j)
            {
                pending[i * j] += solved[i] * divisor_coefficients[j];
            }
        }
        const auto diagonal = diagonal_prefix(divisor_coefficients, solved);
        const auto differences = small_pairs(divisor_coefficients, solved);
        T fixed{};
        for (u64 offset = 0; offset < large_; ++offset)
        {
            const auto z = large_ - offset;
            const auto rank = small_ + offset + 1;
            fixed += differences[offset];
            const auto tail = large_pairs(z, n_ / z, 2, divisor, answer, divisor_coefficients, solved, diagonal);
            // 大きい累積和で未知なのは、divisor(1)と掛かるanswer(rank)だけ。
            const auto cross = solved[1] * (prefix(divisor, rank) - prefix(divisor, z)) - divisor_coefficients[1] * prefix(answer, z);
            answer[rank - 1] = (product(rank) - fixed - tail - cross) * inverse_at_one;
        }
        return answer;
    }

public:
    explicit DirichletPrefixSums(u64 n) : n_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n == 0)
        {
            throw std::invalid_argument("DirichletPrefixSums: positive n required");
        }
#endif
        small_ = square_root(n);
        large_ = n / (small_ + 1);
    }

    [[nodiscard]] u64 size() const noexcept
    {
        return small_ + large_;
    }

    [[nodiscard]] u64 quotient(u64 index) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= size())
        {
            throw std::out_of_range("DirichletPrefixSums: quotient index");
        }
#endif
        return index < small_ ? index + 1 : n_ / (size() - index);
    }

    [[nodiscard]] std::vector<T> convolution(std::span<const T> left, std::span<const T> right) const
    {
        const auto count = table_size();
        check_shape(left);
        check_shape(right);
        const auto left_coefficients = coefficients(left);
        const auto right_coefficients = coefficients(right);
        std::vector<T> answer(count);
        for (u64 a = 1; a <= small_; ++a)
        {
            for (u64 b = 1; b <= small_ / a; ++b)
            {
                answer[a * b - 1] += left_coefficients[a] * right_coefficients[b];
            }
        }
        for (u64 i = 1; i < small_; ++i)
        {
            answer[i] += answer[i - 1];
        }
        const auto diagonal = diagonal_prefix(left_coefficients, right_coefficients);
        const auto differences = small_pairs(left_coefficients, right_coefficients);
        T fixed{};
        for (u64 offset = 0; offset < large_; ++offset)
        {
            const auto z = large_ - offset;
            fixed += differences[offset];
            answer[small_ + offset] = fixed + large_pairs(z, n_ / z, 1, left, right, left_coefficients,
                                                          right_coefficients, diagonal);
        }
        return answer;
    }

    [[nodiscard]] std::vector<T> divide(std::span<const T> product, std::span<const T> divisor, T inverse_at_one) const
    {
        check_shape(product);
        return solve(divisor, inverse_at_one, [product](u64 rank)
                     {
                         return prefix(product, rank);
                     });
    }

    [[nodiscard]] std::vector<T> inverse(std::span<const T> divisor, T inverse_at_one) const
    {
        return solve(divisor, inverse_at_one, [](u64)
                     {
                         return T{1};
                     });
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

// Verify support: Verify/current/common/matrix_field.hpp
// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <stdexcept>
#ifndef SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
#define SIRKIT_VERIFY_MATRIX_FIELD_DEFINED
struct VerifyMatrixField {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    VerifyMatrixField()=default;
    VerifyMatrixField(std::uint64_t input):value(static_cast<std::uint32_t>(input%modulus)){}
    std::uint32_t get()const noexcept{return value;}
    VerifyMatrixField& operator+=(VerifyMatrixField b)noexcept{value+=b.value;if(value>=modulus)value-=modulus;return *this;}
    VerifyMatrixField& operator-=(VerifyMatrixField b)noexcept{value=value>=b.value?value-b.value:value+modulus-b.value;return *this;}
    VerifyMatrixField& operator*=(VerifyMatrixField b)noexcept{value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;}
    friend VerifyMatrixField operator+(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a+=b;}
    friend VerifyMatrixField operator-(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a-=b;}
    friend VerifyMatrixField operator*(VerifyMatrixField a,VerifyMatrixField b)noexcept{return a*=b;}
    friend bool operator==(VerifyMatrixField,VerifyMatrixField)=default;
    friend VerifyMatrixField operator/(VerifyMatrixField a,VerifyMatrixField b){
        if(b.value==0)throw std::domain_error("verification field: zero divisor");
        VerifyMatrixField inverse{1};std::uint32_t exponent=modulus-2;
        while(exponent){if(exponent&1)inverse*=b;b*=b;exponent>>=1;}return a*inverse;
    }
};
#endif

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int cases{};
    verify_in >> cases;
    while (cases-- > 0)
    {
        std::uint64_t n{};
        verify_in >> n;
        const sirkit::DirichletPrefixSums<VerifyMatrixField> layout(n);
        const auto size = static_cast<std::size_t>(layout.size());
        std::vector<VerifyMatrixField> divisor(size);
        for (auto &value : divisor)
        {
            std::uint32_t residue{};
            verify_in >> residue;
            value = residue;
        }
        const auto result = layout.inverse(divisor, VerifyMatrixField{1} / divisor.front());
        for (std::size_t i = 0; i < size; ++i)
        {
            verify_out << result[i].get() << (i + 1 == size ? '\n' : ' ');
        }
    }
}
