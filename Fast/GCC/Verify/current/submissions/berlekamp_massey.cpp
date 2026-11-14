#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/find_linear_recurrence

// Library: Math/Berlekamp_Massey.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Berlekamp_Massey.cpp
// 数学的参考: J. L. Massey, Shift-Register Synthesis and BCH Decoding (1969).
// https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI411.pdf
// 数学的参考（講座）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_recurrence.md
//
// API一覧
// BerlekampMassey<T>(s): 体T上の有限列sに合う最短線形漸化式の係数cを返す。O(n²)時間・O(n)領域。
// 返却長dとし、d<=i<nでs[i]=sum(c[j]*s[i-j-1])。同じ最短次数の解は一意とは限らない。
// Tは正確な体の四則演算・0/1からの構築・等値比較を提供する。Mintは利用側が用意する。
// BerlekampMassey(vector<uint32_t>): 法998244353で同じ係数を返す。入力は自動で正規化する。
// 空列・全零列は{}。有限列の適合だけを保証し、未知の後続項や本来の漸化式の特定は保証しない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
#define SIRKIT_RECURRENCE_COEFFICIENT_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
struct RecurrenceMod998 {
    static constexpr std::uint32_t modulus=998244353;
    std::uint32_t value=0;
    RecurrenceMod998()=default;
    RecurrenceMod998(std::uint64_t x):value(static_cast<std::uint32_t>(x%modulus)){}
    RecurrenceMod998& operator+=(RecurrenceMod998 b)noexcept{
        value+=b.value;if(value>=modulus)value-=modulus;return *this;
    }
    RecurrenceMod998& operator-=(RecurrenceMod998 b)noexcept{
        value=value>=b.value?value-b.value:value+modulus-b.value;return *this;
    }
    RecurrenceMod998& operator*=(RecurrenceMod998 b)noexcept{
        value=static_cast<std::uint32_t>(std::uint64_t{value}*b.value%modulus);return *this;
    }
    friend RecurrenceMod998 operator+(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a+=b;}
    friend RecurrenceMod998 operator-(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a-=b;}
    friend RecurrenceMod998 operator*(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{return a*=b;}
    friend bool operator==(RecurrenceMod998,RecurrenceMod998)=default;
    friend RecurrenceMod998 operator/(RecurrenceMod998 a,RecurrenceMod998 b)noexcept{
        RecurrenceMod998 inverse{1};std::uint32_t power=modulus-2;
        while(power){if(power&1)inverse*=b;b*=b;power>>=1;}return a*inverse;
    }
};
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BERLEKAMP_MASSEY_DEFINED
#define SIRKIT_BERLEKAMP_MASSEY_DEFINED
template<class T>
std::vector<T> BerlekampMassey(const std::vector<T>& sequence) {
    static_assert(!std::is_integral_v<T>,"BerlekampMassey<T>: use an exact field type, or the uint32_t overload");
    if(sequence.empty())return {};
    const auto n=sequence.size();
    if(n>=std::vector<T>{}.max_size())throw std::length_error("BerlekampMassey: size overflow");
    std::vector<T> connection{T{1}},previous{T{1}};
    std::size_t order=0,shift=1;
    T previous_inverse{1};
    for(std::size_t position=0;position<n;++position) {
        T discrepancy=sequence[position];
        for(std::size_t j=1;j<=order;++j)discrepancy+=connection[j]*sequence[position-j];
        if(discrepancy==T{0}){++shift;continue;}
        const T ratio=discrepancy*previous_inverse;
        const bool extend=order<=position/2;
        std::vector<T> saved;
        if(extend)saved=connection;
        if(previous.size()>connection.max_size()-shift)
            throw std::length_error("BerlekampMassey: size overflow");
        if(connection.size()<previous.size()+shift)connection.resize(previous.size()+shift,T{0});
        for(std::size_t j=0;j<previous.size();++j)connection[j+shift]-=ratio*previous[j];
        if(extend) {
            order=position+1-order;
            previous=std::move(saved);
            previous_inverse=T{1}/discrepancy;
            shift=1;
        } else ++shift;
    }
    std::vector<T> result(order,T{0});
    for(std::size_t j=0;j<order;++j)result[j]=T{0}-connection[j+1];
    return result;
}
inline std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence) {
    using Field=/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RecurrenceMod998;
    std::vector<Field> input;input.reserve(sequence.size());
    for(const auto value:sequence)input.emplace_back(value);
    const auto coefficients=BerlekampMassey(input);
    std::vector<std::uint32_t> result;result.reserve(coefficients.size());
    for(const auto value:coefficients)result.push_back(value.value);
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

#include <cstddef>
#include <cstdint>
#include <vector>
int main() {
    std::size_t n{};if(!(verify_in>>n))return 0;
    std::vector<std::uint32_t> sequence(n);
    for(auto&value:sequence)verify_in>>value;
    const auto coefficients=BerlekampMassey(sequence);
    verify_out<<coefficients.size()<<'\n';
    for(std::size_t i=0;i<coefficients.size();++i)verify_out<<coefficients[i]<<(i+1==coefficients.size()?'\n':' ');
    if(coefficients.empty())verify_out<<'\n';
}
