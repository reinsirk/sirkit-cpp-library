#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1467

// Library: DataStructure/Weighted_Slope_Trick.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Weighted_Slope_Trick.cpp
// 参考（重み付き折点による表現）: https://nyaannyaan.github.io/library/data-structure/slope-trick-weighted.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_slope_trick.md
//
// API一覧
// WeightedSlopeTrick(): 凸区分線形関数f=0。折点数をKとする。
// add_left_hinge(a,w=1), add_right_hinge(a,w=1): w*max(a-x,0), w*max(x-a,0)を加える。
// add_abs(a,w=1): w*|x-a|を加える。移す折点がC個なら償却O((C+1)log(K+2))時間、一時領域O(C+1)。
// add_constant(c), min_value(), argmin(): 定数加算、最小値、最小点の閉区間。端がnulloptなら、その側は無限。
// shift(a), shift(a,b): f(x-a)、またはmin_{x-b<=y<=x-a} f(y)へ変換。a<=b。いずれもO(1)。
// clear_left(), clear_right(): min_{x<=y} f(y)、min_{y<=x} f(y)。消す折点数に比例。
// eval(x): f(x)をO(K+1)で求める。格納領域は折点数の過去最大に比例。重みはuint64_t、座標と値はint64_t。
// 整数範囲や確保に失敗した更新では関数を保つ。shiftの順序検査のみSKIP_RUNTIME_CHECKSで省略。
// コピーはO(K+1)で独立。moveはO(1)で移動元はf=0。swapはO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <limits>
#include <optional>
#include <vector>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_WEIGHTED_SLOPE_TRICK_DEFINED
#define SIRKIT_WEIGHTED_SLOPE_TRICK_DEFINED

class WeightedSlopeTrick
{
    using Value = std::int64_t;
    using Weight = std::uint64_t;
    struct Knot
    {
        Value position;
        Weight weight;
    };

    struct Knots
    {
        std::vector<Knot> values;
        Value opposite = 0;
    };
    static constexpr Value kMin = std::numeric_limits<Value>::min();
    static constexpr Value kMax = std::numeric_limits<Value>::max();

public:
    struct Argmin
    {
        std::optional<Value> lower, upper;
    };

    WeightedSlopeTrick() = default;
    WeightedSlopeTrick(const WeightedSlopeTrick &) = default;
    WeightedSlopeTrick(WeightedSlopeTrick &&other) noexcept
    {
        swap(other);
    }

    WeightedSlopeTrick &operator=(WeightedSlopeTrick other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(WeightedSlopeTrick &other) noexcept
    {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(left_shift_, other.left_shift_);
        std::swap(right_shift_, other.right_shift_);
        std::swap(minimum_, other.minimum_);
    }

    void add_left_hinge(Value x, Weight weight = 1)
    {
        AddHinges(x, weight, 0);
    }

    void add_right_hinge(Value x, Weight weight = 1)
    {
        AddHinges(x, 0, weight);
    }

    void add_abs(Value x, Weight weight = 1)
    {
        AddHinges(x, weight, weight);
    }

    void add_constant(Value value)
    {
        minimum_ = Add(minimum_, value);
    }

    [[nodiscard]] Value min_value() const noexcept
    {
        return minimum_;
    }

    [[nodiscard]] Argmin argmin() const noexcept
    {
        Argmin range;
        if (!left_.values.empty())
        {
            range.lower = left_.values.front().position + left_shift_;
        }
        if (!right_.values.empty())
        {
            range.upper = right_.values.front().position + right_shift_;
        }
        return range;
    }

    void shift(Value amount)
    {
        shift(amount, amount);
    }

    void shift(Value lower, Value upper)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (lower > upper)
        {
            throw std::invalid_argument("WeightedSlopeTrick: invalid shift interval");
        }
#endif
        const auto next_left = ShiftedOffset(left_, left_shift_, lower);
        const auto next_right = ShiftedOffset(right_, right_shift_, upper);
        left_shift_ = next_left;
        right_shift_ = next_right;
    }

    void clear_left() noexcept
    {
        left_ = Knots{};
        left_shift_ = 0;
    }

    void clear_right() noexcept
    {
        right_ = Knots{};
        right_shift_ = 0;
    }

    [[nodiscard]] Value eval(Value x) const
    {
        auto value = minimum_;
        for (const auto &[key, weight] : left_.values)
        {
            const auto position = key + left_shift_;
            if (position > x)
            {
                value = Raise(value, Distance(position, x), weight);
            }
        }
        for (const auto &[key, weight] : right_.values)
        {
            const auto position = key + right_shift_;
            if (position < x)
            {
                value = Raise(value, Distance(x, position), weight);
            }
        }
        return value;
    }

private:
    Knots left_, right_;
    Value left_shift_ = 0, right_shift_ = 0, minimum_ = 0;

    [[nodiscard]] static Value Add(Value a, Value b)
    {
        if ((b > 0 && a > kMax - b) || (b < 0 && a < kMin - b))
        {
            throw std::overflow_error("WeightedSlopeTrick: addition out of range");
        }
        return a + b;
    }

    [[nodiscard]] static Value ToKey(Value position, Value shift)
    {
        if ((shift > 0 && position < kMin + shift) || (shift < 0 && position > kMax + shift))
        {
            throw std::overflow_error("WeightedSlopeTrick: key out of range");
        }
        return position - shift;
    }

    [[nodiscard]] static Weight Distance(Value high, Value low) noexcept
    {
        return static_cast<Weight>(high) - static_cast<Weight>(low);
    }

    [[nodiscard]] static Value Raise(Value base, Weight distance, Weight weight)
    {
        const Weight room = Distance(kMax, base);
        if (distance > room / weight)
        {
            throw std::overflow_error("WeightedSlopeTrick: value out of range");
        }
        return static_cast<Value>(static_cast<Weight>(base) + distance * weight);
    }

    [[nodiscard]] static Value ShiftedOffset(const Knots &knots, Value shift, Value amount)
    {
        if (knots.values.empty())
        {
            return 0;
        }
        const auto next = Add(shift, amount);
        (void)Add(knots.values.front().position, next);
        (void)Add(knots.opposite, next);
        return next;
    }

    static void Reserve(Knots &knots, std::size_t more)
    {
        const auto needed = knots.values.size() + more;
        const auto capacity = knots.values.capacity();
        if (needed > capacity)
        {
            knots.values.reserve(std::max(needed, capacity + capacity / 2 + 1));
        }
    }

    template <bool Maximum>
    static constexpr auto Compare = [](const Knot &a, const Knot &b)
    {
        return Maximum ? a.position < b.position : a.position > b.position;
    };

    template <bool Maximum>
    static void Push(Knots &knots, Knot knot) noexcept
    {
        if (knots.values.empty() || Compare<Maximum>(knot, {knots.opposite, 0}))
        {
            knots.opposite = knot.position;
        }
        knots.values.push_back(knot);
        std::push_heap(knots.values.begin(), knots.values.end(), Compare<Maximum>);
    }

    void AddHinges(Value x, Weight left_weight, Weight right_weight)
    {
        if (left_weight == 0 && right_weight == 0)
        {
            return;
        }
        if (right_weight != 0 && !left_.values.empty() && left_.values.front().position + left_shift_ > x)
        {
            Exchange<true>(x, right_weight, left_weight);
        }
        else if (left_weight != 0 && !right_.values.empty() && right_.values.front().position + right_shift_ < x)
        {
            Exchange<false>(x, left_weight, right_weight);
        }
        else
        {
            const Value left_key = left_weight == 0 ? 0 : ToKey(x, left_shift_);
            const Value right_key = right_weight == 0 ? 0 : ToKey(x, right_shift_);
            Reserve(left_, left_weight != 0);
            Reserve(right_, right_weight != 0);
            if (left_weight != 0)
            {
                Push<true>(left_, {left_key, left_weight});
            }
            if (right_weight != 0)
            {
                Push<false>(right_, {right_key, right_weight});
            }
        }
    }

    template <bool ToRight>
    void Exchange(Value x, Weight requested, Weight extra)
    {
        auto &donor = ToRight ? left_ : right_;
        auto &receiver = ToRight ? right_ : left_;
        const Value donor_shift = ToRight ? left_shift_ : right_shift_;
        const Value receiver_shift = ToRight ? right_shift_ : left_shift_;

        // 失敗時に取り出した折点を戻せるよう、移す前と後の値を控える。
        struct Transfer
        {
            Knot original, relocated;
        };
        std::array<Transfer, 4> local;
        std::vector<Transfer> overflow;
        std::size_t local_count = 0;
        const auto for_each = [&](auto process)
        {
            for (std::size_t i = 0; i < local_count; ++i)
            {
                process(local[i]);
            }
            for (const auto &entry : overflow)
            {
                process(entry);
            }
        };
        Weight remaining = requested, partial = 0;
        Value next_minimum = minimum_, partial_key = 0, donor_key = 0, receiver_key = 0;
        try
        {
            while (remaining != 0 && !donor.values.empty())
            {
                const Knot current = donor.values.front();
                const auto position = current.position + donor_shift;
                if (ToRight ? position <= x : position >= x)
                {
                    break;
                }
                const Weight moved = std::min(remaining, current.weight);
                const Weight gap = ToRight ? Distance(position, x) : Distance(x, position);
                next_minimum = Raise(next_minimum, gap, moved);
                const auto key = ToKey(position, receiver_shift);
                remaining -= moved;
                if (moved < current.weight)
                {
                    partial = moved;
                    partial_key = key;
                    break;
                }
                const Transfer entry{current, {key, moved}};
                if (local_count < local.size())
                {
                    local[local_count++] = entry;
                }
                else
                {
                    overflow.push_back(entry);
                }
                std::pop_heap(donor.values.begin(), donor.values.end(), Compare<ToRight>);
                donor.values.pop_back();
            }
            donor_key = ToKey(x, donor_shift);
            if (remaining != 0)
            {
                receiver_key = ToKey(x, receiver_shift);
            }
            // ここで両側を確保し、以降の処理では割り当てが起きないようにする。
            Reserve(donor, 1 + (extra != 0));
            Reserve(receiver, local_count + overflow.size() + (partial != 0) + (remaining != 0));
        }
        catch (...)
        {
            for_each([&](const auto &entry)
                     {
                         Push<ToRight>(donor, entry.original);
                     });
            throw;
        }

        for_each([&](const auto &entry)
                 {
                     Push<!ToRight>(receiver, entry.relocated);
                 });
        if (partial != 0)
        {
            donor.values.front().weight -= partial;
            Push<!ToRight>(receiver, {partial_key, partial});
        }
        Push<ToRight>(donor, {donor_key, requested - remaining});
        if (extra != 0)
        {
            Push<ToRight>(donor, {donor_key, extra});
        }
        if (remaining != 0)
        {
            Push<!ToRight>(receiver, {receiver_key, remaining});
        }
        minimum_ = next_minimum;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
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


int main()
{
    int buyers = 0, colors = 0;
    verify_in >> buyers >> colors;
    std::vector<std::pair<std::int64_t, bool>> events(buyers + colors);
    for (int i = 0; i < buyers + colors; ++i)
    {
        verify_in >> events[i].first;
        events[i].second = i < buyers;
    }
    std::sort(events.begin(), events.end());
    const auto span = events.back().first - events.front().first;
    for (int capacity = 1; capacity <= buyers; ++capacity)
    {
        WeightedSlopeTrick cost;
        // 始点の余分な在庫・不足を1個減らす費用は、色の全幅以下で補正できる。
        cost.add_abs(0, span + 1);
        auto previous = events.front().first;
        for (const auto &[position, demand] : events)
        {
            cost.add_abs(0, position - previous);
            if (demand)
            {
                cost.shift(1);
            }
            else
            {
                cost.shift(-capacity, 0);
            }
            previous = position;
        }
        verify_out << cost.eval(0) << '\n';
    }
}
