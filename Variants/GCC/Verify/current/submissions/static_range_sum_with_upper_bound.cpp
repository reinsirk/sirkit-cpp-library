#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound

// Library: DataStructure/Weighted_Wavelet_Matrix.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Weighted_Wavelet_Matrix.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_wavelet_matrix.md
//
// API一覧
// WeightedWaveletMatrix<Key,Sum>(keys,weights): 同じ長さのキー列と重み列。サイズはINT_MAX以下。
// sum(l,r): 元の位置区間[l,r)の重み和。O(1)。
// count_sum_less/less_equal(l,r,x): キーがx未満／以下の個数と重み和。O(1+B)。
// count_sum(l,r,lo,hi): lo<=key<hiの個数と重み和。lo>=hiはゼロ。O(1+B)。
// size()/empty(): 要素数／空か。O(1)。Bは異なるキーの個数σに対しceil(log2(σ))、空では0。
// 構築O(1+n log(n+1))時間、格納O(1+n(1+B))領域。キーは全順序、Sum{}は加法の単位元。
// 整数の和は型の幅で剰余演算。その他のSumは可換な加法と減法を備える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_WEIGHTED_WAVELET_MATRIX_DEFINED
#define SIRKIT_WEIGHTED_WAVELET_MATRIX_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class S, bool Integral = std::is_integral_v<S> && !std::is_same_v<S, bool>>
struct WeightedWaveletStorage
{
    using Type = S;
};

template <class S>
struct WeightedWaveletStorage<S, true>
{
    using Type = std::make_unsigned_t<S>;
};

inline std::size_t CheckedWeightedWaveletSize(std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("WeightedWaveletMatrix: too many values");
    }
    return count;
}
}

template <std::totally_ordered Key = std::int64_t, class Sum = std::int64_t>
    requires std::copyable<Key> && std::copyable<Sum> && std::default_initializable<Sum>
class WeightedWaveletMatrix
{
    static_assert(!std::is_same_v<Sum, bool>);

public:
    using CountSum = std::pair<std::size_t, Sum>;

    WeightedWaveletMatrix() = default;

    WeightedWaveletMatrix(std::span<const Key> keys, std::span<const Sum> weights)
    {
        if (keys.size() != weights.size())
        {
            throw std::invalid_argument("WeightedWaveletMatrix: mismatched sizes");
        }
        const auto n = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedWeightedWaveletSize(keys.size());
        if (n == 0)
        {
            return;
        }

        alphabet_.assign(keys.begin(), keys.end());
        std::sort(alphabet_.begin(), alphabet_.end());
        alphabet_.erase(std::unique(alphabet_.begin(), alphabet_.end()), alphabet_.end());

        std::vector<Stored> ordered(n);
        total_prefix_.reserve(n + 1);
        total_prefix_.emplace_back();
        for (std::size_t i = 0; i < n; ++i)
        {
            ordered[i] = Pack(weights[i]);
            total_prefix_.push_back(Add(total_prefix_.back(), ordered[i]));
        }
        const int height = std::bit_width(alphabet_.size() - 1);
        if (height == 0)
        {
            return;
        }

        std::vector<Index> ranks(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            ranks[i] = static_cast<Index>(std::lower_bound(alphabet_.begin(), alphabet_.end(), keys[i]) - alphabet_.begin());
        }
        std::vector<Index> next_ranks(n);
        std::vector<Stored> next_weights(n);
        levels_.reserve(height);
        for (int shift = height; shift-- > 0;)
        {
            levels_.emplace_back(n);
            auto &level = levels_.back();
            for (std::size_t i = 0; i < n; ++i)
            {
                if ((ranks[i] >> shift) & 1)
                {
                    level.words[i / 64] |= std::uint64_t{1} << (i % 64);
                }
            }
            for (std::size_t i = 0; i + 1 < level.words.size(); ++i)
            {
                level.ones_before[i + 1] = level.ones_before[i] + std::popcount(level.words[i]);
            }
            const std::size_t zeros = n - level.ones_before.back();
            level.zero_prefix.reserve(zeros + 1);
            level.zero_prefix.emplace_back();
            std::size_t zero = 0, one = zeros;
            for (std::size_t i = 0; i < n; ++i)
            {
                if ((ranks[i] >> shift) & 1)
                {
                    next_ranks[one] = ranks[i];
                    next_weights[one++] = ordered[i];
                }
                else
                {
                    level.zero_prefix.push_back(Add(level.zero_prefix.back(), ordered[i]));
                    next_ranks[zero] = ranks[i];
                    next_weights[zero++] = ordered[i];
                }
            }
            ranks.swap(next_ranks);
            ordered.swap(next_weights);
        }
    }

    WeightedWaveletMatrix(const WeightedWaveletMatrix &) = default;

    WeightedWaveletMatrix &operator=(const WeightedWaveletMatrix &other)
    {
        if (this != &other)
        {
            WeightedWaveletMatrix next(other);
            Swap(next);
        }
        return *this;
    }

    WeightedWaveletMatrix(WeightedWaveletMatrix &&other) noexcept
        : alphabet_(std::move(other.alphabet_)), total_prefix_(std::move(other.total_prefix_)),
          levels_(std::move(other.levels_))
    {
        other.alphabet_.clear();
        other.total_prefix_.clear();
        other.levels_.clear();
    }

    WeightedWaveletMatrix &operator=(WeightedWaveletMatrix &&other) noexcept
    {
        if (this != &other)
        {
            WeightedWaveletMatrix next(std::move(other));
            Swap(next);
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return total_prefix_.empty() ? 0 : total_prefix_.size() - 1;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] Sum sum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? Sum{} : Unpack(Subtract(total_prefix_[right], total_prefix_[left]));
    }

    [[nodiscard]] CountSum count_sum_less(std::size_t left, std::size_t right, const Key &upper) const
    {
        CheckRange(left, right);
        return Export(Below(left, right, LowerRank(upper)));
    }

    [[nodiscard]] CountSum count_sum_less_equal(std::size_t left, std::size_t right, const Key &upper) const
    {
        CheckRange(left, right);
        const auto limit = std::upper_bound(alphabet_.begin(), alphabet_.end(), upper) - alphabet_.begin();
        return Export(Below(left, right, static_cast<std::size_t>(limit)));
    }

    [[nodiscard]] CountSum count_sum(std::size_t left, std::size_t right, const Key &lower, const Key &upper) const
    {
        CheckRange(left, right);
        if (!(lower < upper))
        {
            return {};
        }
        const auto low = Below(left, right, LowerRank(lower));
        const auto high = Below(left, right, LowerRank(upper));
        return {high.first - low.first, Unpack(Subtract(high.second, low.second))};
    }

private:
    using Stored = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WeightedWaveletStorage<Sum>::Type;
    using Index = std::uint32_t;
    using Partial = std::pair<std::size_t, Stored>;

    struct Level
    {
        // nが64の倍数の場合にもrank(n)で読めるように末尾に0語を置く。
        explicit Level(std::size_t n) : words((n + 63) / 64 + 1), ones_before(words.size())
        {
        }

        [[nodiscard]] std::size_t rank(std::size_t index) const noexcept
        {
            const auto word = index / 64;
            const auto mask = (std::uint64_t{1} << (index % 64)) - 1;
            return ones_before[word] + std::popcount(words[word] & mask);
        }

        std::vector<std::uint64_t> words;
        std::vector<Index> ones_before;
        std::vector<Stored> zero_prefix;
    };

    std::vector<Key> alphabet_;
    std::vector<Stored> total_prefix_;
    std::vector<Level> levels_;

    void Swap(WeightedWaveletMatrix &other) noexcept
    {
        alphabet_.swap(other.alphabet_);
        total_prefix_.swap(other.total_prefix_);
        levels_.swap(other.levels_);
    }

    void CheckRange(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("WeightedWaveletMatrix: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    static Stored Pack(const Sum &value)
    {
        return static_cast<Stored>(value);
    }

    static Sum Unpack(const Stored &value)
    {
        if constexpr (std::is_integral_v<Sum> && std::is_signed_v<Sum>)
        {
            return std::bit_cast<Sum>(value);
        }
        else
        {
            return value;
        }
    }

    static Stored Add(const Stored &a, const Stored &b)
    {
        return static_cast<Stored>(a + b);
    }

    static Stored Subtract(const Stored &a, const Stored &b)
    {
        return static_cast<Stored>(a - b);
    }

    [[nodiscard]] std::size_t LowerRank(const Key &key) const
    {
        return static_cast<std::size_t>(std::lower_bound(alphabet_.begin(), alphabet_.end(), key) - alphabet_.begin());
    }

    static CountSum Export(const Partial &value)
    {
        return {value.first, Unpack(value.second)};
    }

    Partial Below(std::size_t left, std::size_t right, std::size_t limit) const
    {
        if (left == right || limit == 0)
        {
            return {};
        }
        if (limit == alphabet_.size())
        {
            return {right - left, Subtract(total_prefix_[right], total_prefix_[left])};
        }

        Partial result{};
        auto level_index = levels_.size();
        for (const auto &level : levels_)
        {
            const auto shift = --level_index;
            const auto left_one = level.rank(left);
            const auto right_one = level.rank(right);
            const auto left_zero = left - left_one;
            const auto right_zero = right - right_one;
            if ((limit >> shift) & 1)
            {
                result.first += right_zero - left_zero;
                result.second = Add(result.second, Subtract(level.zero_prefix[right_zero], level.zero_prefix[left_zero]));
                const auto zeros = level.zero_prefix.size() - 1;
                left = zeros + left_one;
                right = zeros + right_one;
            }
            else
            {
                left = left_zero;
                right = right_zero;
            }
            if (left == right)
            {
                break;
            }
        }
        return result;
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


#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const WeightedWaveletMatrix<> matrix(values, values);
    while (q-- > 0)
    {
        int left = 0, right = 0;
        std::int64_t limit = 0;
        verify_in >> left >> right >> limit;
        const auto [count, sum] = matrix.count_sum_less_equal(left, right, limit);
        verify_out << count << ' ' << sum << '\n';
    }
}
