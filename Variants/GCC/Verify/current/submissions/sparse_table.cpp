#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/staticrmq

// Library: DataStructure/Sparse_Table.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Sparse_Table.cpp
// 参考（数学的構成）: https://info.atcoder.jp/entry/algorithm_lectures/sparse_table
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_table.md
//
// API一覧
// [SparseTable<T, Op>]
// SparseTable(vector<T>={}, identity=T{}, op=Op{}): 静的列を構築。O(n log(n+1)+1)時間・領域。
// Op(a,b)はconst呼出可能かつ結合的かつべき等、identityは左右の単位元。可換性は不要。 演算/コピーをO(1)とする。
// size()/empty(): 要素数/空か。get(i): const T&（T=boolは値）。prod(l,r): 半開区間[l,r)の積。all_prod(): 全積。O(1)。
// prodの空区間・空列の全積はidentity。参照はassign/clear/交換/移動で無効になり得る。添字は0始まり。
// max_right(l,pred)/min_left(r,pred): predが真の最大右端/最小左端。O(log(n+1))。pred(identity)==trueと境界の単調性が前提。
// values(): 元の列をコピー、O(n+1)。assign(vector<T>): 再構築。O(旧n log(旧n+1)+n log(n+1)+1)。点更新は行わない。
// clear()/swap(other): 空へ戻す/内容交換。O(旧n log(旧n+1)+1)/O(1)。コピーは独立、移動元は空。演算等の例外時は内容の強い保証なし。
// SIMD: 32bit以下の整数（bool以外）の構築に条件付きAVX2経路。SIRKIT_DISABLE_SIMDで専用経路とCPU判定を除く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_TABLE_DEFINED
#define SIRKIT_SPARSE_TABLE_DEFINED
template<class T, class Op>
class SparseTable {
    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<std::vector<T>> table_;
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
    [[gnu::target("avx2"), gnu::optimize("O3")]]
    // 入出力は別々のvectorが所有する領域への非所有ポインタ。
    static void fill_row_avx2(T* target, const T* left, const T* right, std::size_t count, Op& op) {
        for (std::size_t i = 0; i < count; ++i) target[i] = T(op(left[i], right[i]));
    }
    static bool supports_avx2() noexcept {
        static const bool available = [] {
            __builtin_cpu_init();
            return __builtin_cpu_supports("avx2") != 0;
        }();
        return available;
    }
#endif
    void check_range(std::size_t l, std::size_t r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l > r || r > size()) throw std::out_of_range("SparseTable: range");
#else
        (void)l; (void)r;
#endif
    }
    T range_unchecked(std::size_t l, std::size_t r) const {
        if (l == r) return identity_;
        const auto k = std::bit_width(r - l) - 1;
        const auto width = std::size_t{1} << k;
        return op_(table_[k][l], table_[k][r - width]);
    }
public:
    using const_reference = typename std::vector<T>::const_reference;
    explicit SparseTable(const std::vector<T>& values = {}, const T& identity = T{}, Op op = Op{})
        : op_(std::move(op)), identity_(identity) {
        const auto n = values.size();
        if (!n) return;
        table_.reserve(std::bit_width(n));
        table_.push_back(values);
        for (std::size_t width = 2; width <= n;) {
            const auto half = width / 2;
            const auto count = n - width + 1;
            std::vector<T> row;
            bool filled = false;
#if (!defined(SIRKIT_DISABLE_SIMD)) && ((defined(__i386__)) || (defined(__x86_64__)))
            using Result = decltype(op_(std::declval<const std::vector<T>&>()[std::size_t{}],
                                        std::declval<const std::vector<T>&>()[std::size_t{}]));
            if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 4 &&
                          std::is_same_v<std::remove_cvref_t<Result>, T>) {
                if (count >= 256 && supports_avx2()) {
                    row = std::vector<T>(count, identity_);
                    const auto& previous = table_.back();
                    fill_row_avx2(row.data(), previous.data(), previous.data() + half, count, op_);
                    filled = true;
                }
            }
#endif
            if (!filled) {
                row.reserve(count);
                const auto& previous = table_.back();
                for (std::size_t i = 0; i < count; ++i) row.emplace_back(op_(previous[i], previous[i + half]));
            }
            table_.push_back(std::move(row));
            if (width > n / 2) break;
            width *= 2;
        }
    }
    SparseTable(const SparseTable&) = default;
    SparseTable& operator=(const SparseTable& other) {
        if (this != &other) { SparseTable replacement(other); swap(replacement); }
        return *this;
    }
    SparseTable(SparseTable&& other)
        : op_(other.op_), identity_(other.identity_), table_(std::move(other.table_)) {
        other.table_.clear();
    }
    SparseTable& operator=(SparseTable&& other) {
        if (this != &other) { SparseTable replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return (table_.empty() ? 0 : table_.front().size()); }
    bool empty() const noexcept { return size() == 0; }
    const_reference get(std::size_t i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i >= size()) throw std::out_of_range("SparseTable: index");
#endif
        return table_.front()[i];
    }
    T prod(std::size_t l, std::size_t r) const { check_range(l, r); return range_unchecked(l, r); }
    T all_prod() const { return range_unchecked(0, size()); }
    template<class Pred> std::size_t max_right(std::size_t l, Pred pred) const {
        check_range(l, l);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(identity_)) throw std::invalid_argument("SparseTable: predicate(identity)");
#endif
        auto low = l, high = size();
        while (low < high) {
            const auto distance = high - low;
            const auto middle = low + distance / 2 + distance % 2;
            if (pred(range_unchecked(l, middle))) low = middle;
            else high = middle - 1;
        }
        return low;
    }
    template<class Pred> std::size_t min_left(std::size_t r, Pred pred) const {
        check_range(r, r);
#ifndef SKIP_RUNTIME_CHECKS
        if (!pred(identity_)) throw std::invalid_argument("SparseTable: predicate(identity)");
#endif
        std::size_t low = 0, high = r;
        while (low < high) {
            const auto middle = low + (high - low) / 2;
            if (pred(range_unchecked(middle, r))) high = middle;
            else low = middle + 1;
        }
        return low;
    }
    std::vector<T> values() const { return (table_.empty() ? std::vector<T>{} : table_.front()); }
    void assign(const std::vector<T>& values) { SparseTable replacement(values, identity_, op_); swap(replacement); }
    void clear() noexcept { table_.clear(); }
    void swap(SparseTable& other) noexcept(std::is_nothrow_swappable_v<Op> && std::is_nothrow_swappable_v<T>) {
        using std::swap;
        swap(op_, other.op_); swap(identity_, other.identity_);
        table_.swap(other.table_);
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

#include <algorithm>
#include <limits>
#include <vector>
struct StaticRangeMinimum { int operator()(int a, int b) const { return std::min(a, b); } };
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<int> a(n);
    for (auto& x : a) verify_in >> x;
    SparseTable<int, StaticRangeMinimum> table(a, std::numeric_limits<int>::max());
    while (q--) {
        int l{}, r{}; verify_in >> l >> r;
        verify_out << table.prod(l, r) << '\n';
    }
}
