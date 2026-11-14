#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/queue_operate_all_composite

// Library: DataStructure/Sliding_Window_Aggregation.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sliding_Window_Aggregation.cpp
// 参考（構成・問題選定）: https://nyaannyaan.github.io/library/data-structure/slide-window-aggregation.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sliding_window_aggregation.md
//
// API一覧
// [SlidingWindowAggregation<T, Op>]
// SlidingWindowAggregation(identity, op=Op{}): 空のキュー。identityは左右の単位元。O(1)。
// Op(a,b)はconst呼出可能な結合演算。非可換でも可。T/Opはコピー可能、演算に外部副作用がないこと。
// push_back(value): 末尾にコピーを追加。pop_front(): 先頭を削除。演算/コピーをO(1)として償却O(1)、最悪O(n)。
// front()/back(): 先頭/末尾のconst T&。all_prod(): 先頭から順に合成した値、空ならidentity。O(1)。
// front/back/pop_frontは非空が前提。既定で空を検査しout_of_range。SKIP_RUNTIME_CHECKSで検査を除く。
// size()/empty(): 要素数/空か。O(1)。clear(): 全削除、O(n)。swap(other): 演算/単位元/列を交換、O(1)。
// 代入/swapではT/Opをswapで交換可能とする。コピーは独立、移動元は空。保持領域O(過去最大n+1)。
// front/backの参照は更新/clear/交換/移動で無効になり得る。
// push/pop中のコピー・演算・確保の例外では列を保つ。代入の強い保証はT/Opのswapがnoexceptの場合。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SLIDING_WINDOW_AGGREGATION_DEFINED
#define SIRKIT_SLIDING_WINDOW_AGGREGATION_DEFINED
template<class T, class Op>
class SlidingWindowAggregation {
    struct Entry { T value, product; };
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<Entry> incoming_, outgoing_;

    void check_nonempty() const {
    }
    void transfer() {
        // 構築が完了するまで元のキューを変更しない。
        std::vector<Entry> replacement;
        replacement.reserve(incoming_.size());
        for (auto i = incoming_.size(); i != 0; --i) {
            const T& value = incoming_[i - 1].value;
            T product = replacement.empty() ? value : std::as_const(op_)(value, replacement.back().product);
            replacement.push_back(Entry{value, std::move_if_noexcept(product)});
        }
        outgoing_.swap(replacement);
        incoming_.clear();
    }
public:
    explicit SlidingWindowAggregation(const T& identity, Op op = Op{})
        : op_(std::move_if_noexcept(op)), identity_(identity) {}
    SlidingWindowAggregation(const SlidingWindowAggregation&) = default;
    SlidingWindowAggregation& operator=(const SlidingWindowAggregation& other) {
        if (this != &other) { SlidingWindowAggregation replacement(other); swap(replacement); }
        return *this;
    }
    SlidingWindowAggregation(SlidingWindowAggregation&& other)
        : op_(other.op_), identity_(other.identity_), incoming_(std::move(other.incoming_)), outgoing_(std::move(other.outgoing_)) {
        other.incoming_.clear(); other.outgoing_.clear();
    }
    SlidingWindowAggregation& operator=(SlidingWindowAggregation&& other) {
        if (this != &other) { SlidingWindowAggregation replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return incoming_.size() + outgoing_.size(); }
    bool empty() const noexcept { return incoming_.empty() && outgoing_.empty(); }
    void push_back(const T& value) {
        if (size() == std::numeric_limits<std::size_t>::max())
            throw std::length_error("SlidingWindowAggregation: size");
        T product = incoming_.empty() ? value : std::as_const(op_)(incoming_.back().product, value);
        incoming_.push_back(Entry{value, std::move_if_noexcept(product)});
    }
    void pop_front() {
        check_nonempty();
        if (outgoing_.empty()) transfer();
        outgoing_.pop_back();
    }
    const T& front() const {
        check_nonempty();
        return outgoing_.empty() ? incoming_.front().value : outgoing_.back().value;
    }
    const T& back() const {
        check_nonempty();
        return incoming_.empty() ? outgoing_.front().value : incoming_.back().value;
    }
    T all_prod() const {
        if (outgoing_.empty()) return incoming_.empty() ? identity_ : incoming_.back().product;
        if (incoming_.empty()) return outgoing_.back().product;
        return op_(outgoing_.back().product, incoming_.back().product);
    }
    void clear() noexcept { incoming_.clear(); outgoing_.clear(); }
    void swap(SlidingWindowAggregation& other) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>) {
        using std::swap;
        swap(op_, other.op_); swap(identity_, other.identity_);
        incoming_.swap(other.incoming_); outgoing_.swap(other.outgoing_);
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

#include <cstddef>
#include <cstdint>
struct Affine {
    std::uint64_t a, b;
};
struct Compose {
    static constexpr std::uint64_t mod = 998244353;
    Affine operator()(const Affine& first, const Affine& second) const {
        return {first.a * second.a % mod, (first.b * second.a + second.b) % mod};
    }
};
int main() {
    std::size_t q{}; verify_in >> q;
    SlidingWindowAggregation<Affine, Compose> queue(Affine{1, 0});
    while (q--) {
        unsigned type{}; verify_in >> type;
        if (type == 0) {
            Affine value{}; verify_in >> value.a >> value.b; queue.push_back(value);
        } else if (type == 1) queue.pop_front();
        else {
            std::uint64_t x{}; verify_in >> x;
            const auto value = queue.all_prod();
            verify_out << (value.a * x + value.b) % Compose::mod << '\n';
        }
    }
}
