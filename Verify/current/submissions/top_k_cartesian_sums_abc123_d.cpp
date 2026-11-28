#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: e616a582a0ad3c9f333520c83e280f0bffa48cef09138c2274eaf93d927ad3db
// Verify: https://atcoder.jp/contests/abc123/tasks/abc123_d

// Library: Algorithm/Top_K_Cartesian_Sums.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/top_k_cartesian_sums.md
// 参考（上位K個を順に残す考え方）: https://img.atcoder.jp/abc123/editorial.pdf
//
// API一覧
// top_k_cartesian_sums(groups,k[,order]): 各組から1個ずつ選んだ和の上位k個を返す。同値も選び方の個数分残す。
// CartesianSumOrder::largestが既定（大きい順）。smallestは小さい順。全体がk通り未満なら得られる分だけ返す。
// 要素はint64_t。返す和がint64_tを超えるとoverflow_error。途中の部分和は広い型で扱う。
// 組数をD、総要素数をS、最長組をMとして期待O(S+D*k*log(k+1))時間、O(M+k+1)領域。
// k=0または空の組があれば空列。組が0個でk>=1なら{0}。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TOP_K_CARTESIAN_SUMS_DEFINED
#define SIRKIT_TOP_K_CARTESIAN_SUMS_DEFINED
enum class CartesianSumOrder
{
    largest,
    smallest,
};

namespace detail
{
struct CartesianWideSum
{
    static constexpr auto kSign = std::uint64_t{1} << 63;
    static constexpr auto kAllBits = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t high = 0;
    std::uint64_t low = 0;

    [[nodiscard]] static CartesianWideSum From(std::int64_t weight) noexcept
    {
        return {weight < 0 ? kAllBits : 0, static_cast<std::uint64_t>(weight)};
    }

    [[nodiscard]] CartesianWideSum add(CartesianWideSum other) const noexcept
    {
        const auto combined = low + other.low;
        return {high + other.high + static_cast<std::uint64_t>(combined < low), combined};
    }

    [[nodiscard]] bool less(CartesianWideSum other) const noexcept
    {
        if (high != other.high)
        {
            return (high ^ kSign) < (other.high ^ kSign);
        }
        return low < other.low;
    }

    [[nodiscard]] std::int64_t to_signed() const
    {
        const auto extension = (low & kSign) != 0 ? kAllBits : 0;
        if (high != extension)
        {
            throw std::overflow_error("top_k_cartesian_sums: result exceeds int64_t");
        }
        return std::bit_cast<std::int64_t>(low);
    }
};

template <bool Largest>
bool BetterCartesianSum(CartesianWideSum first, CartesianWideSum second) noexcept
{
    if constexpr (Largest)
    {
        return second.less(first);
    }
    else
    {
        return first.less(second);
    }
}

template <bool Largest>
std::vector<CartesianWideSum> MergeCartesianSums(std::span<const CartesianWideSum> first,
                                                 std::span<const CartesianWideSum> second, std::size_t k)
{
    if (first.size() > second.size())
    {
        std::swap(first, second);
    }
    const auto length = second.size() > k / first.size() ? k : first.size() * second.size();
    struct Candidate
    {
        CartesianWideSum value;
        std::size_t row = 0, column = 0;
    };
    const auto lower_priority = [](const Candidate &a, const Candidate &b)
    {
        return BetterCartesianSum<Largest>(b.value, a.value);
    };
    std::vector<Candidate> heap;
    heap.reserve(first.size());
    for (std::size_t row = 0; row < first.size(); ++row)
    {
        heap.push_back({first[row].add(second.front()), row, 0});
    }
    std::priority_queue queue(lower_priority, std::move(heap));
    std::vector<CartesianWideSum> result;
    result.reserve(length);
    // 各行の未使用の先頭だけをヒープに置く。
    while (result.size() < length)
    {
        auto next = queue.top();
        queue.pop();
        result.push_back(next.value);
        ++next.column;
        if (next.column < second.size())
        {
            queue.push({first[next.row].add(second[next.column]), next.row, next.column});
        }
    }
    return result;
}

template <bool Largest>
std::vector<std::int64_t> TopKCartesianSums(std::span<const std::vector<std::int64_t>> groups, std::size_t k)
{
    if (k == 0)
    {
        return {};
    }
    std::vector<CartesianWideSum> prefix(1);
    for (const auto &group : groups)
    {
        if (group.empty())
        {
            return {};
        }
        std::vector<CartesianWideSum> sorted;
        sorted.reserve(group.size());
        for (const auto value : group)
        {
            sorted.push_back(CartesianWideSum::From(value));
        }
        if (sorted.size() > k)
        {
            std::nth_element(sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(k), sorted.end(), BetterCartesianSum<Largest>);
            sorted.resize(k);
        }
        std::sort(sorted.begin(), sorted.end(), BetterCartesianSum<Largest>);
        prefix = MergeCartesianSums<Largest>(prefix, sorted, k);
    }
    std::vector<std::int64_t> result;
    result.reserve(prefix.size());
    for (const auto value : prefix)
    {
        result.push_back(value.to_signed());
    }
    return result;
}
}

inline std::vector<std::int64_t> top_k_cartesian_sums(std::span<const std::vector<std::int64_t>> groups,
                                                      std::size_t k, CartesianSumOrder order = CartesianSumOrder::largest)
{
    if (order == CartesianSumOrder::largest)
    {
        return detail::TopKCartesianSums<true>(groups, k);
    }
    return detail::TopKCartesianSums<false>(groups, k);
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

int main()
{
    std::size_t x{}, y{}, z{}, k{};
    verify_in >> x >> y >> z >> k;
    std::vector<std::vector<std::int64_t>> groups(3);
    groups[0].resize(x);
    groups[1].resize(y);
    groups[2].resize(z);
    for (auto &group : groups)
    {
        for (auto &score : group)
        {
            verify_in >> score;
        }
    }
    for (const auto score : sirkit::top_k_cartesian_sums(groups, k))
    {
        verify_out << score << '\n';
    }
}
