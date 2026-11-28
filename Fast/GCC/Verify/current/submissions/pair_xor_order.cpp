#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2977

// Library: Algorithm/Pair_Xor_Order.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Pair_Xor_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_xor_order.md
//
// API一覧
// PairXorOrder<T=uint64_t>(values): 符号なし整数列のコピーを整列する。O(n log(n+1))時間、O(n)領域。
// kth(k): i<jを満たす全組のvalues[i]^values[j]を重複込みで昇順に並べ、0始まりのk番目を返す。
// count_less(x): XOR値がx未満の組の個数。pair_count(): 全組の数、O(1)。
// 照会はO(n(1+B))時間・追加O(n)領域。Bは列の最小値と最大値のXORのビット幅。
// 要素数nはUINT32_MAX以下。型Tはbool以外の64ビット以下の符号なし整数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PAIR_XOR_ORDER_DEFINED
#define SIRKIT_PAIR_XOR_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{

inline void CheckPairXorSize(std::size_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairXorOrder: too many values");
    }
}

}

template <std::unsigned_integral T = std::uint64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class PairXorOrder
{
    struct Range
    {
        std::uint32_t begin, end;

        [[nodiscard]] std::uint64_t size() const
        {
            return end - begin;
        }
    };

    struct Pair
    {
        Range first, second;
    };

    struct Bucket
    {
        std::vector<Pair> pairs;
        std::uint64_t count = 0;

        void add(Range first, Range second)
        {
            if (first.size() == 0 || second.size() == 0)
            {
                return;
            }
            const auto ways = first.begin == second.begin
                                  ? PairCount(first.size())
                                  : first.size() * second.size();
            if (ways != 0)
            {
                pairs.push_back({first, second});
                count += ways;
            }
        }

        void clear()
        {
            pairs.clear();
            count = 0;
        }
    };

public:
    explicit PairXorOrder(std::vector<T> values) : values_(std::move(values))
    {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckPairXorSize(values_.size());
        std::sort(values_.begin(), values_.end());
    }

    [[nodiscard]] std::uint64_t pair_count() const noexcept
    {
        return PairCount(values_.size());
    }

    [[nodiscard]] T kth(std::uint64_t k) const
    {
        T value = 0;
        Search(BitWidth(), [&](T bit, std::uint64_t same)
               {
                   if (k < same)
                   {
                       return false;
                   }
                   k -= same;
                   value |= bit;
                   return true;
               });
        return value;
    }

    [[nodiscard]] std::uint64_t count_less(T bound) const
    {
        if (pair_count() == 0 || bound == 0)
        {
            return 0;
        }
        const int bits = BitWidth();
        if (std::bit_width(bound) > bits)
        {
            return pair_count();
        }
        std::uint64_t count = 0;
        Search(bits, [&](T bit, std::uint64_t same)
               {
                   if ((bound & bit) == 0)
                   {
                       return false;
                   }
                   count += same;
                   return true;
               });
        return count;
    }

private:
    [[nodiscard]] static std::uint64_t PairCount(std::uint64_t size) noexcept
    {
        return size * (size - 1) / 2;
    }

    [[nodiscard]] int BitWidth() const
    {
        return std::bit_width(static_cast<T>(values_.front() ^ values_.back()));
    }

    [[nodiscard]] std::pair<Range, Range> Split(Range range, T bit) const
    {
        auto it = std::partition_point(values_.begin() + range.begin, values_.begin() + range.end,
                                       [&](T value)
                                       {
                                           return (value & bit) == 0;
                                       });
        const auto middle = static_cast<std::uint32_t>(it - values_.begin());
        return {{range.begin, middle}, {middle, range.end}};
    }

    template <class ChooseOne>
    void Search(int bits, ChooseOne choose_one) const
    {
        const Range all{0, static_cast<std::uint32_t>(values_.size())};
        Bucket current, same, different;
        current.add(all, all);
        for (int level = bits - 1; level >= 0 && !current.pairs.empty(); --level)
        {
            const auto bit = static_cast<T>(T{1} << level);
            same.clear();
            different.clear();
            for (const auto &[first, second] : current.pairs)
            {
                const auto [a0, a1] = Split(first, bit);
                if (first.begin == second.begin)
                {
                    same.add(a0, a0);
                    same.add(a1, a1);
                    different.add(a0, a1);
                }
                else
                {
                    const auto [b0, b1] = Split(second, bit);
                    same.add(a0, b0);
                    same.add(a1, b1);
                    different.add(a0, b1);
                    different.add(a1, b0);
                }
            }
            Bucket &next = choose_one(bit, same.count) ? different : same;
            current.pairs.swap(next.pairs);
        }
    }

    std::vector<T> values_;
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


#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n;
    std::uint64_t k;
    verify_in >> n >> k;
    std::vector<std::uint32_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairXorOrder order(std::move(values));
    verify_out << order.kth(k - 1) << '\n';
}
