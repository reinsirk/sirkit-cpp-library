#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc366/tasks/abc366_d

// Library: Algorithm/Prefix_Sum_ND.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Prefix_Sum_ND.cpp
// 参考: https://nyaannyaan.github.io/library/data-structure-2d/2d-cumulative-sum.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_sum_nd.md
//
// API一覧
// CumulativeSum2D<T>(h,w) / CumulativeSum3D<T>(h,w,d): 各軸の長さを指定。
// CumulativeSumND<T,D>(shape): D次元版（D>=1）。Index=std::array<std::size_t,D>。
// add(point,value): 1点に加算。O(D)。
// build(): 加算がすべて済んでから一度だけ実行。各軸で累積和を取る。O(DM)。
// query(left,right): 点加算後の半開領域の和。O(2^D)。
// imos(left,right,value=1): 構築前に半開領域へ一括加算。O(2^D)。
// imos_get(point): 構築後の1点の値。O(D)。
// 2次元・3次元では座標を並べて渡せる。例 query(i1,j1,i2,j2)。
// 0始まり。点は各軸で0以上・長さ未満。領域は0<=left[d]<=right[d]<=shape()[d]。
// shape(): 各軸の長さ。O(1)。Mは各軸の長さ+1の積。初期化O(M+D)、領域O(M+D)。
// 長さ0の軸があれば初期化O(D)、build O(1)、保存領域O(D)。空領域の和はT{}。
// T{}は加法の単位元。加算・減算の中間結果をTで表現できること。
// 検査有効時は容量超過にstd::length_error、範囲外にstd::out_of_range、操作順違反にstd::logic_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_SUM_ND_DEFINED
#define SIRKIT_PREFIX_SUM_ND_DEFINED

template <class T, std::size_t Dimensions>
class CumulativeSumND
{
    static_assert(Dimensions > 0);

public:
    using Index = std::array<std::size_t, Dimensions>;

    explicit CumulativeSumND(const Index &shape) : shape_(shape)
    {
        if (std::ranges::find(shape_, std::size_t{0}) != shape_.end())
        {
            return;
        }
        std::size_t cells = 1;
        for (std::size_t axis = Dimensions; axis > 0; --axis)
        {
            const std::size_t length = shape_[axis - 1];
            strides_[axis - 1] = cells;
            cells *= length + 1;
        }
        data_.resize(cells);
    }

    explicit CumulativeSumND(std::size_t h, std::size_t w)
        requires(Dimensions == 2)
        : CumulativeSumND(Index{h, w})
    {
    }

    explicit CumulativeSumND(std::size_t h, std::size_t w, std::size_t d)
        requires(Dimensions == 3)
        : CumulativeSumND(Index{h, w, d})
    {
    }

    void add(const Index &point, const T &value)
    {
        check_build(false);
        T &cell = data_[point_index(point)];
        cell = cell + value;
    }

    void add(std::size_t i, std::size_t j, const T &value)
        requires(Dimensions == 2)
    {
        add({i, j}, value);
    }

    void add(std::size_t i, std::size_t j, std::size_t k, const T &value)
        requires(Dimensions == 3)
    {
        add({i, j, k}, value);
    }

    void build()
    {
        check_build(false);
        const std::size_t cells = data_.size();
        if (cells != 0)
        {
            for (std::size_t axis = 0; axis < Dimensions; ++axis)
            {
                const std::size_t step = strides_[axis];
                const std::size_t width = step * (shape_[axis] + 1);
                for (std::size_t start = 0; start < cells; start += width)
                {
                    for (std::size_t pos = start + step; pos < start + width; ++pos)
                    {
                        data_[pos] = data_[pos] + data_[pos - step];
                    }
                }
            }
        }
    }

    T query(const Index &left, const Index &right) const
    {
        check_build(true);
        check_box(left, right);
        return data_.empty() ? T{} : box_sum<0>(left, right, 0);
    }

    T query(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2) const
        requires(Dimensions == 2)
    {
        return query({i1, j1}, {i2, j2});
    }

    T query(std::size_t i1, std::size_t j1, std::size_t k1,
            std::size_t i2, std::size_t j2, std::size_t k2) const
        requires(Dimensions == 3)
    {
        return query({i1, j1, k1}, {i2, j2, k2});
    }

    void imos(const Index &left, const Index &right, const T &value = T{1})
    {
        check_build(false);
        check_box(left, right);
        for (std::size_t axis = 0; axis < Dimensions; ++axis)
        {
            if (left[axis] == right[axis])
            {
                return;
            }
        }
        add_corners<0, false>(left, right, 0, value);
    }

    void imos(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2, const T &value = T{1})
        requires(Dimensions == 2)
    {
        imos({i1, j1}, {i2, j2}, value);
    }

    void imos(std::size_t i1, std::size_t j1, std::size_t k1,
              std::size_t i2, std::size_t j2, std::size_t k2, const T &value = T{1})
        requires(Dimensions == 3)
    {
        imos({i1, j1, k1}, {i2, j2, k2}, value);
    }

    T imos_get(const Index &point) const
    {
        check_build(true);
        return data_[point_index(point)];
    }

    T imos_get(std::size_t i, std::size_t j) const
        requires(Dimensions == 2)
    {
        return imos_get({i, j});
    }

    T imos_get(std::size_t i, std::size_t j, std::size_t k) const
        requires(Dimensions == 3)
    {
        return imos_get({i, j, k});
    }

    const Index &shape() const noexcept
    {
        return shape_;
    }

private:
    void check_build([[maybe_unused]] bool expected) const
    {
    }

    void check_box([[maybe_unused]] const Index &left, [[maybe_unused]] const Index &right) const
    {
    }

    std::size_t point_index(const Index &point) const
    {
        std::size_t pos = 0;
        for (std::size_t axis = 0; axis < Dimensions; ++axis)
        {
            pos += (point[axis] + 1) * strides_[axis];
        }
        return pos;
    }

    template <std::size_t Axis, bool Subtract>
    void add_corners(const Index &left, const Index &right, std::size_t start, const T &value)
    {
        if constexpr (Axis == Dimensions)
        {
            T &cell = data_[start];
            if constexpr (Subtract)
            {
                cell = cell - value;
            }
            else
            {
                cell = cell + value;
            }
        }
        else
        {
            const std::size_t step = strides_[Axis];
            add_corners<Axis + 1, Subtract>(left, right, start + (left[Axis] + 1) * step, value);
            // 軸の末尾の差分は配列内のどの点にも影響しない。
            if (right[Axis] < shape_[Axis])
            {
                add_corners<Axis + 1, !Subtract>(left, right, start + (right[Axis] + 1) * step, value);
            }
        }
    }

    template <std::size_t Axis>
    T box_sum(const Index &left, const Index &right, std::size_t start) const
    {
        if constexpr (Axis == Dimensions)
        {
            return data_[start];
        }
        else
        {
            const std::size_t step = strides_[Axis];
            return box_sum<Axis + 1>(left, right, start + right[Axis] * step) -
                   box_sum<Axis + 1>(left, right, start + left[Axis] * step);
        }
    }

    Index shape_;
    Index strides_{};
    std::vector<T> data_;
};

template <class T>
using CumulativeSum2D = CumulativeSumND<T, 2>;

template <class T>
using CumulativeSum3D = CumulativeSumND<T, 3>;

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

int main()
{
    std::size_t n{};
    verify_in >> n;
    CumulativeSum3D<int> sums(n, n, n);
    for (std::size_t x = 0; x < n; ++x)
    {
        for (std::size_t y = 0; y < n; ++y)
        {
            for (std::size_t z = 0; z < n; ++z)
            {
                int value{};
                verify_in >> value;
                sums.add(x, y, z, value);
            }
        }
    }
    sums.build();
    std::size_t q{};
    verify_in >> q;
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t lx{}, rx{}, ly{}, ry{}, lz{}, rz{};
        verify_in >> lx >> rx >> ly >> ry >> lz >> rz;
        verify_out << sums.query(lx - 1, ly - 1, lz - 1, rx, ry, rz) << '\n';
    }
}
