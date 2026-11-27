#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/deque_operate_all_composite

// Library: DataStructure/Deque_Aggregation.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Deque_Aggregation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/deque_aggregation.md
//
// API一覧
// DequeAggregation<T,Op>(identity,op=Op{}): 空。Opはconst呼び出し可能な結合演算。O(1)。
// push_front(value)/push_back(value): 両端への追加。償却O(1)。
// pop_front()/pop_back(): 両端の削除。片側が空なら半分を移す。償却O(1)、最悪O(n)。
// front()/back(): 両端をconst参照で返す。all_prod(): 列順の積。空ならidentity。各O(1)。
// size()/empty(): 要素数と空判定。O(1)。clear(): 全削除。O(n)。
// swap(other): 演算と列の交換。O(1)。代入・交換にはTとOpの非送出swapが必要。
// front/back/popは非空が前提。入力検査有効時は空でout_of_range。
// 要素と演算はコピー可能。領域は過去最大要素数Mに対してO(M+1)。
// 両端の参照は更新・clear・交換・移動で無効になり得る。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DEQUE_AGGREGATION_DEFINED
#define SIRKIT_DEQUE_AGGREGATION_DEFINED

template <class T, class Op>
class DequeAggregation
{
    static_assert(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<Op>);
    static_assert(std::is_invocable_r_v<T, const Op &, const T &, const T &>);
    static constexpr bool kInPlaceRebalance =
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_invocable_r_v<T, const Op &, const T &, const T &>;

    struct Entry
    {
        T value, product;
    };

public:
    explicit DequeAggregation(const T &identity, Op op = Op{})
        : op_(std::move_if_noexcept(op)), identity_(identity)
    {
    }

    DequeAggregation(const DequeAggregation &) = default;

    DequeAggregation(DequeAggregation &&other) noexcept(
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<Op>)
        : op_(other.op_), identity_(other.identity_), left_(std::move(other.left_)), right_(std::move(other.right_))
    {
        other.clear();
    }

    DequeAggregation &operator=(DequeAggregation other) noexcept
        requires std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>
    {
        swap(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        // EntryはTを2個含むため、左右の合計もsize_tに収まる。
        return left_.size() + right_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return left_.empty() && right_.empty();
    }

    void push_front(const T &value)
    {
        Append<true>(left_, value);
    }

    void push_back(const T &value)
    {
        Append<false>(right_, value);
    }

    void pop_front()
    {
        Pop<true>(left_, right_);
    }

    void pop_back()
    {
        Pop<false>(right_, left_);
    }

    [[nodiscard]] const T &front() const
    {
        CheckNonempty();
        return left_.empty() ? right_.front().value : left_.back().value;
    }

    [[nodiscard]] const T &back() const
    {
        CheckNonempty();
        return right_.empty() ? left_.front().value : right_.back().value;
    }

    [[nodiscard]] T all_prod() const
    {
        if (left_.empty())
        {
            return right_.empty() ? identity_ : right_.back().product;
        }
        if (right_.empty())
        {
            return left_.back().product;
        }
        return std::invoke(op_, left_.back().product, right_.back().product);
    }

    void clear() noexcept
    {
        left_.clear();
        right_.clear();
    }

    void swap(DequeAggregation &other) noexcept
        requires std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>
    {
        if (this == &other)
        {
            return;
        }
        using std::swap;
        swap(op_, other.op_);
        swap(identity_, other.identity_);
        left_.swap(other.left_);
        right_.swap(other.right_);
    }

private:
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<Entry> left_, right_;

    void CheckNonempty() const
    {
    }

    template <bool Front>
    T Extend(const T &product, const T &value) const
    {
        if constexpr (Front)
        {
            return std::invoke(op_, value, product);
        }
        else
        {
            return std::invoke(op_, product, value);
        }
    }

    template <bool Front>
    void Append(std::vector<Entry> &stack, const T &value)
    {
        T product = stack.empty() ? value : Extend<Front>(stack.back().product, value);
        stack.push_back(Entry{value, std::move_if_noexcept(product)});
    }

    template <bool Front>
    void Pop(std::vector<Entry> &target, std::vector<Entry> &source)
    {
        CheckNonempty();
        if (target.empty())
        {
            if (source.size() == 1)
            {
                source.pop_back();
                return;
            }
            Rebalance<Front>(target, source);
        }
        target.pop_back();
    }

    template <bool Front>
    void Rebalance(std::vector<Entry> &target, std::vector<Entry> &source)
    {
        const auto count = source.size();
        const auto remaining = count / 2;
        const auto transferred = count - remaining;
        if constexpr (kInPlaceRebalance)
        {
            // 確保が終われば、値・集約の構築と代入は例外を送出しない。
            target.reserve(transferred);
            for (auto i = transferred; i != 0; --i)
            {
                Append<Front>(target, source[i - 1].value);
            }
            for (std::size_t i = 0; i < remaining; ++i)
            {
                auto &entry = source[i];
                entry.value = source[i + transferred].value;
                entry.product = i == 0 ? entry.value : Extend<!Front>(source[i - 1].product, entry.value);
            }
            source.erase(source.begin() + static_cast<typename std::vector<Entry>::difference_type>(remaining), source.end());
        }
        else
        {
            // 両方の集約を作り直せた時点で差し替える。途中で失敗しても元の列を保つ。
            std::vector<Entry> new_target, new_source;
            new_target.reserve(transferred);
            new_source.reserve(remaining);
            for (auto i = transferred; i != 0; --i)
            {
                Append<Front>(new_target, source[i - 1].value);
            }
            for (auto i = transferred; i < count; ++i)
            {
                Append<!Front>(new_source, source[i].value);
            }
            target.swap(new_target);
            source.swap(new_source);
        }
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

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;
};

struct Compose
{
    constexpr Affine operator()(Affine left, Affine right) const noexcept
    {
        return {
            static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
            static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
        };
    }
};
}

int main()
{
    int q = 0;
    verify_in >> q;
    DequeAggregation deque(Affine{1, 0}, Compose{});
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        switch (type)
        {
        case 0:
        {
            Affine f{};
            verify_in >> f.a >> f.b;
            deque.push_front(f);
            break;
        }
        case 1:
        {
            Affine f{};
            verify_in >> f.a >> f.b;
            deque.push_back(f);
            break;
        }
        case 2:
            deque.pop_front();
            break;
        case 3:
            deque.pop_back();
            break;
        case 4:
        {
            std::uint32_t x = 0;
            verify_in >> x;
            const auto fold = deque.all_prod();
            verify_out << (std::uint64_t{fold.a} * x + fold.b) % kMod << '\n';
            break;
        }
        }
    }
}
