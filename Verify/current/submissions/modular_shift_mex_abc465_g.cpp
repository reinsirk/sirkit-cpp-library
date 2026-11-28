#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 0f5940bb6c2fff28e7a60266cac08df8ce603918141fd51caed74766f950a2b6
// Verify: https://atcoder.jp/contests/abc465/tasks/abc465_g

// Library: Math/Modular_Shift_Mex.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_mex.md
// 参考（円環上の連続区間と床関数和による差分）: https://atcoder.jp/contests/abc465/editorial/22561
//
// API一覧
// ModularShiftMex(count,modulus,step,start=0): 空集合で初期化。uint32_tのcount/modulus、負のint64_tのstep/startも可。
// insert(value)/erase(value): [0,modulus)の値を追加/削除。集合が変わったときtrue。重複は保持しない。
// sum(): k=0..count-1で、集合の全要素をstart+step*kだけ法modulusでずらしたmexの総和。uint64_tで厳密。
// size()/clear(): 要素数O(1)/集合を空にするO(r+1)。移動元は同じ設定の空集合。自己代入は内容を保つ。
// 一直線に並べたときの占有区間数をrとして、追加削除はO(log(r+2)+log(modulus+1))時間・合計O(r+1)領域。
// 構築とsumはO(1)、コピーはO(r+1)。法0はinvalid_argument、範囲外の値はout_of_range。
//
// [同梱のFloor Sum]
// floor_sum(n,m,a,b): i=0..n-1 の floor((a*i+b)/m) の総和。a,bは負でもよい。
// 0<=n<2^32、1<=m<2^32。ACLと同じく結果は2^64を法とする符号付き64ビット値。
// min_of_mod_of_linear(n,m,a,b): i=0..n-1 の (a*i+b) mod m の最小値。1<=n<2^32。
// 両方ともO(log(m+1))時間・O(1)領域。検査有効時、n,mが範囲外ならstd::invalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <map>
#include <type_traits>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_DEFINED
#define SIRKIT_FLOOR_SUM_DEFINED

inline std::int64_t floor_sum(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n < 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("floor_sum: n or m");
    }
#endif
    if (n == 0)
    {
        return 0;
    }
    const auto count = static_cast<std::uint64_t>(n);
    const auto triangular = count * (count - 1) / 2;
    std::uint64_t sum = 0;
    if (a < 0)
    {
        const auto remainder = a % m;
        const auto quotient = a / m;
        sum += triangular * static_cast<std::uint64_t>(quotient - (remainder != 0));
        a = remainder == 0 ? 0 : remainder + m;
    }
    if (b < 0)
    {
        const auto remainder = b % m;
        const auto quotient = b / m;
        sum += count * static_cast<std::uint64_t>(quotient - (remainder != 0));
        b = remainder == 0 ? 0 : remainder + m;
    }
    auto length = count;
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = static_cast<std::uint64_t>(a);
    auto offset = static_cast<std::uint64_t>(b);
    while (true)
    {
        if (slope >= modulus)
        {
            sum += (length * (length - 1) / 2) * (slope / modulus);
            slope %= modulus;
        }
        if (offset >= modulus)
        {
            sum += length * (offset / modulus);
            offset %= modulus;
        }
        const auto limit = slope * length + offset;
        if (limit < modulus)
        {
            break;
        }
        length = limit / modulus; // 格子点を横方向から数え直す。
        offset = limit % modulus;
        std::swap(modulus, slope);
    }
    return std::bit_cast<std::int64_t>(sum);
}

inline std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n <= 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("min_of_mod_of_linear: n or m");
    }
#endif
    const auto normalize = [](std::int64_t x, std::int64_t modulus)
    {
        x %= modulus;
        return static_cast<std::uint64_t>(x < 0 ? x + modulus : x);
    };
    auto length = static_cast<std::uint64_t>(n);
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = normalize(a, m);
    auto offset = normalize(b, m);
    auto minimum = offset;
    while (minimum != 0 && slope != 0)
    {
        if (slope * 2 > modulus) // 逆から走査して次の法を半分以下にする。
        {
            offset = (slope * (length - 1) + offset) % modulus;
            slope = modulus - slope;
        }
        minimum = std::min(minimum, offset);
        const auto wraps = (slope * (length - 1) + offset) / modulus;
        if (wraps == 0)
        {
            break;
        }
        const auto reverse_step = (slope - modulus % slope) % slope; // 次の法は slope。
        length = wraps;
        offset = (offset % slope + reverse_step) % slope; // y回目の折り返しは (offset - modulus*y) mod slope。
        modulus = slope;
        slope = reverse_step;
        minimum = std::min(minimum, offset);
    }
    return static_cast<std::int64_t>(minimum);
}

#endif

#ifndef SIRKIT_MODULAR_SHIFT_MEX_DEFINED
#define SIRKIT_MODULAR_SHIFT_MEX_DEFINED

class ModularShiftMex
{
    struct Progression
    {
        std::uint32_t count;
        std::uint32_t modulus;
        std::uint32_t slope;
        std::uint32_t phase;
    };
    using Runs = std::map<std::uint32_t, std::uint32_t>;

public:
    ModularShiftMex(std::uint32_t count, std::uint32_t modulus, std::int64_t step, std::int64_t start = 0)
        : progression_{count, modulus, Normalize(step, modulus), Normalize(start, modulus)}
    {
    }

    ModularShiftMex(const ModularShiftMex &) = default;

    ModularShiftMex(ModularShiftMex &&other) noexcept(std::is_nothrow_default_constructible_v<Runs>)
        : progression_(other.progression_)
    {
        Swap(other);
    }

    ModularShiftMex &operator=(ModularShiftMex other) noexcept
    {
        Swap(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t sum() const noexcept
    {
        return total_;
    }

    [[nodiscard]] std::uint32_t size() const noexcept
    {
        return size_;
    }

    void clear() noexcept
    {
        runs_.clear();
        size_ = 0;
        total_ = 0;
    }

    bool insert(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before != runs_.end() && before->second > value)
        {
            return false;
        }
        const auto modulus = progression_.modulus;
        const auto next_total = size_ + 1 == modulus
                                    ? std::uint64_t{modulus} * progression_.count
                                    : total_ + AddedContribution(value, before, after);
        const auto next_value = value + 1;
        const bool touch_left = before != runs_.end() && before->second == value;
        const bool touch_right = after != runs_.end() && after->first == next_value;
        if (touch_left)
        {
            if (touch_right)
            {
                before->second = after->second;
                runs_.erase(after);
            }
            else
            {
                before->second = next_value;
            }
        }
        else if (touch_right)
        {
            MoveStart(after, value);
        }
        else
        {
            runs_.emplace_hint(after, value, next_value);
        }
        ++size_;
        total_ = next_total;
        return true;
    }

    bool erase(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before == runs_.end() || before->second <= value)
        {
            return false;
        }
        const auto next_value = value + 1;
        if (before->first == value)
        {
            if (before->second == next_value)
            {
                runs_.erase(before);
            }
            else
            {
                MoveStart(before, next_value);
            }
        }
        else
        {
            if (before->second != next_value)
            {
                runs_.emplace_hint(after, next_value, before->second);
            }
            before->second = value;
        }
        --size_;
        if (size_ + 1 == progression_.modulus)
        {
            total_ = MissingOneSum(value);
        }
        else
        {
            const auto next = runs_.upper_bound(value);
            total_ -= AddedContribution(value, Previous(next), next);
        }
        return true;
    }

private:
    void Swap(ModularShiftMex &other) noexcept
    {
        std::swap(progression_, other.progression_);
        runs_.swap(other.runs_);
        std::swap(size_, other.size_);
        std::swap(total_, other.total_);
    }

    static std::uint32_t Normalize(std::int64_t value, std::uint32_t modulus)
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("ModularShiftMex: zero modulus");
        }
        const auto remainder = value % static_cast<std::int64_t>(modulus);
        return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
    }

    void Check(std::uint32_t value) const
    {
        if (value >= progression_.modulus)
        {
            throw std::out_of_range("ModularShiftMex: residue outside modulus");
        }
    }

    Runs::iterator Previous(Runs::iterator after) noexcept
    {
        return after == runs_.begin() ? runs_.end() : std::prev(after);
    }

    void MoveStart(Runs::iterator run, std::uint32_t start)
    {
        auto node = runs_.extract(run);
        node.key() = start;
        runs_.insert(std::move(node));
    }

    [[nodiscard]] std::uint64_t FloorWithOffset(std::int64_t offset) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        return static_cast<std::uint64_t>(floor_sum(count, modulus, slope, offset + phase));
    }

    [[nodiscard]] std::uint64_t MissingOneSum(std::uint32_t missing) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        const std::uint64_t length = count;
        const auto linear = (length * (length - 1) / 2) * slope + length * (std::uint64_t{phase} + missing);
        return linear - modulus * FloorWithOffset(missing);
    }

    [[nodiscard]] std::uint64_t AddedContribution(
        std::uint32_t value, Runs::const_iterator before, Runs::const_iterator after) const
    {
        auto left = static_cast<std::int64_t>(value);
        auto right = left + 1;
        if (before != runs_.end() && before->second == value)
        {
            left = before->first;
        }
        if (after != runs_.end() && after->first == value + 1)
        {
            right = after->second;
        }
        const auto modulus = progression_.modulus;
        if (!runs_.empty())
        {
            if (left == 0 && runs_.rbegin()->second == modulus)
            {
                left -= modulus - runs_.rbegin()->first;
            }
            if (right == modulus && runs_.begin()->first == 0)
            {
                right += runs_.begin()->second;
            }
        }

        // 左端からvalueの間に0が移動した回だけ、mexの終点がvalueからrightへ動く。
        const auto occurrences = FloorWithOffset(value) - FloorWithOffset(left - 1);
        return occurrences * static_cast<std::uint64_t>(right - value);
    }

    Progression progression_;
    Runs runs_; // [first,second)の占有区間。円環をまたぐときは端の2区間に分ける。
    std::uint32_t size_ = 0;
    std::uint64_t total_ = 0;
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


#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint32_t modulus{}, count{}, step{};
    verify_in >> n >> modulus >> step >> count;
    sirkit::ModularShiftMex mex(count, modulus, step);
    std::vector<std::uint32_t> offsets(n);
    std::map<std::uint32_t, std::uint32_t> frequency;
    for (auto &offset : offsets)
    {
        verify_in >> offset;
        if (frequency[offset]++ == 0)
        {
            mex.insert(offset);
        }
    }
    int queries{};
    verify_in >> queries;
    for (int q = 0; q < queries; ++q)
    {
        std::size_t index{};
        std::uint32_t value{};
        verify_in >> index >> value;
        auto &offset = offsets[index - 1];
        if (offset != value)
        {
            const auto previous = frequency.find(offset);
            if (--previous->second == 0)
            {
                frequency.erase(previous);
                mex.erase(offset);
            }
            offset = value;
            if (frequency[offset]++ == 0)
            {
                mex.insert(offset);
            }
        }
        verify_out << mex.sum() << '\n';
    }
}
