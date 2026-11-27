#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_affine_range_sum_large_array

// Library: DataStructure/Sparse_Affine_Range_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Sparse_Affine_Range_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_affine_range_sum.md
//
// API一覧
// SparseAffineRangeSum<Mod>(n=0,initial=0): 長さnをinitialで初期化。O(1)、nはuint64_t。
// range_affine(l,r,a,b): 半開区間[l,r)の各値をa*x+bに変換。償却O(log(n+1))。
// range_sum(l,r): 半開区間の和の非負剰余。空区間は0。O(log(n+1))。照会時に節点を確保しない。
// size()/all_sum()/stored_nodes(): 長さ・全体和・確保した節点数。各O(1)。
// 重みと係数はint64_tで受け取り縮約。Modは正のuint32_t、既定998244353。
// U回の更新後の領域はO(1+U log(n+1))。未分割区間は一様値だけで保持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_AFFINE_RANGE_SUM_DEFINED
#define SIRKIT_SPARSE_AFFINE_RANGE_SUM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t NextSparseAffineNodeCapacity(std::size_t used, std::size_t capacity, std::size_t growth,
                                                std::size_t limit)
{
    if (used > limit || growth > limit - used)
    {
        throw std::length_error("SparseAffineRangeSum: too many nodes");
    }
    const auto need = used + growth;
    if (need <= capacity)
    {
        return capacity;
    }
    const auto doubled = capacity <= limit / 2 ? 2 * capacity : limit;
    return std::max(need, doubled);
}
}

template <std::uint32_t Mod = 998244353>
class SparseAffineRangeSum
{
    static_assert(Mod > 0);

public:
    explicit SparseAffineRangeSum(std::uint64_t n = 0, std::int64_t initial = 0)
        : length_(n), root_(Normalize(initial))
    {
    }

    SparseAffineRangeSum(const SparseAffineRangeSum &) = default;

    SparseAffineRangeSum &operator=(const SparseAffineRangeSum &other)
    {
        if (this != &other)
        {
            SparseAffineRangeSum copy(other);
            Swap(copy);
        }
        return *this;
    }

    SparseAffineRangeSum(SparseAffineRangeSum &&other) noexcept
        : nodes_(std::move(other.nodes_)), length_(std::exchange(other.length_, 0)),
          root_(std::exchange(other.root_, 0))
    {
        other.nodes_.clear();
    }

    SparseAffineRangeSum &operator=(SparseAffineRangeSum &&other) noexcept
    {
        if (this != &other)
        {
            SparseAffineRangeSum copy(std::move(other));
            Swap(copy);
        }
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t stored_nodes() const noexcept
    {
        return nodes_.size();
    }

    [[nodiscard]] std::uint32_t all_sum() const noexcept
    {
        return CurrentSum(root_, length_);
    }

    void range_affine(std::uint64_t left, std::uint64_t right, std::int64_t a, std::int64_t b)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }
        const Affine change{Normalize(a), Normalize(b)};
        if (change.identity())
        {
            return;
        }
        if (left == 0 && right == length_)
        {
            root_ = Apply(root_, length_, change);
            return;
        }

        // 1層の未確定境界は二つ以下。対数個の新規節点を先に確保する。
        ReserveForUpdate();
        root_ = Update(root_, 0, length_, left, right, change);
    }

    [[nodiscard]] std::uint32_t range_sum(std::uint64_t left, std::uint64_t right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return 0;
        }
        return Query(root_, 0, length_, left, right, {});
    }

private:
    static constexpr auto kOne = std::uint32_t{1} % Mod;
    using Handle = std::conditional_t<Mod <= (std::uint64_t{1} << 31), std::uint32_t, std::uint64_t>;
    static constexpr Handle kNodeBit = Handle{1} << (std::numeric_limits<Handle>::digits - 1);

    struct Affine
    {
        std::uint32_t a = kOne, b = 0;

        bool identity() const noexcept
        {
            return a == kOne && b == 0;
        }
    };

    struct Node
    {
        Handle left, right;
        std::uint32_t sum;
        Affine pending{};
    };

    std::vector<Node> nodes_;
    std::uint64_t length_;
    Handle root_;

    void Swap(SparseAffineRangeSum &other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(length_, other.length_);
        std::swap(root_, other.root_);
    }

    void CheckRange(std::uint64_t left, std::uint64_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > length_)
        {
            throw std::out_of_range("SparseAffineRangeSum: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    void ReserveForUpdate()
    {
        const auto extra = static_cast<std::size_t>(2 * std::bit_width(length_));
        const auto limit = static_cast<std::size_t>(std::min<std::uint64_t>(kNodeBit, nodes_.max_size()));
        nodes_.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NextSparseAffineNodeCapacity(
            nodes_.size(), nodes_.capacity(), extra, limit));
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto reduced = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(reduced < 0 ? reduced + Mod : reduced);
    }

    static std::uint32_t Add(std::uint32_t left, std::uint32_t right) noexcept
    {
        const auto sum = std::uint64_t{left} + right;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b) noexcept
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }

    static std::uint32_t Scale(std::uint32_t value, std::uint64_t length) noexcept
    {
        return Multiply(value, static_cast<std::uint32_t>(length % Mod));
    }

    static std::uint32_t Transform(Affine f, std::uint32_t sum, std::uint64_t length) noexcept
    {
        const auto product = std::uint64_t{f.a} * sum;
        const auto offset = std::uint64_t{f.b} * (length % Mod);
        if constexpr (Mod <= (std::uint64_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    static Affine Compose(Affine after, Affine before) noexcept
    {
        return {Multiply(after.a, before.a), Transform(after, before.b, 1)};
    }

    static bool IsNode(Handle handle) noexcept
    {
        return (handle & kNodeBit) != 0;
    }

    static std::size_t Index(Handle handle) noexcept
    {
        return static_cast<std::size_t>(handle & (kNodeBit - 1));
    }

    std::uint32_t CurrentSum(Handle handle, std::uint64_t length) const noexcept
    {
        if (IsNode(handle))
        {
            return nodes_[Index(handle)].sum;
        }
        return Scale(static_cast<std::uint32_t>(handle), length);
    }

    Handle Apply(Handle handle, std::uint64_t length, Affine change) noexcept
    {
        if (!IsNode(handle))
        {
            return Transform(change, static_cast<std::uint32_t>(handle), 1);
        }
        auto &node = nodes_[Index(handle)];
        node.sum = Transform(change, node.sum, length);
        node.pending = Compose(change, node.pending);
        return handle;
    }

    Handle Update(Handle handle, std::uint64_t start, std::uint64_t end,
                  std::uint64_t left, std::uint64_t right, Affine change) noexcept
    {
        if (left <= start && end <= right)
        {
            return Apply(handle, end - start, change);
        }

        if (!IsNode(handle))
        {
            if (Transform(change, static_cast<std::uint32_t>(handle), 1) == handle)
            {
                return handle;
            }
            // 片側の子を更新するときも、反対側は一様値のまま保持する。
            const auto new_handle = kNodeBit | static_cast<Handle>(nodes_.size());
            nodes_.push_back({handle, handle, CurrentSum(handle, end - start), {}});
            handle = new_handle;
        }

        auto &node = nodes_[Index(handle)];
        const auto middle = start + (end - start) / 2;
        if (!node.pending.identity())
        {
            node.left = Apply(node.left, middle - start, node.pending);
            node.right = Apply(node.right, end - middle, node.pending);
            node.pending = {};
        }
        if (left < middle)
        {
            node.left = Update(node.left, start, middle, left, right, change);
        }
        if (middle < right)
        {
            node.right = Update(node.right, middle, end, left, right, change);
        }
        node.sum = Add(CurrentSum(node.left, middle - start), CurrentSum(node.right, end - middle));
        return handle;
    }

    std::uint32_t Query(Handle handle, std::uint64_t start, std::uint64_t end,
                        std::uint64_t left, std::uint64_t right, Affine inherited) const noexcept
    {
        if (!IsNode(handle))
        {
            const auto length = std::min(end, right) - std::max(start, left);
            return Scale(Transform(inherited, static_cast<std::uint32_t>(handle), 1), length);
        }

        const auto &node = nodes_[Index(handle)];
        if (left <= start && end <= right)
        {
            return Transform(inherited, node.sum, end - start);
        }
        inherited = Compose(inherited, node.pending);
        const auto middle = start + (end - start) / 2;
        std::uint32_t result = 0;
        if (left < middle)
        {
            result = Query(node.left, start, middle, left, right, inherited);
        }
        if (middle < right)
        {
            result = Add(result, Query(node.right, middle, end, left, right, inherited));
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

int main()
{
    std::uint64_t length = 0;
    int q = 0;
    verify_in >> length >> q;
    SparseAffineRangeSum<> data(length);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::uint32_t a = 0, b = 0;
            verify_in >> a >> b;
            data.range_affine(left, right, a, b);
        }
        else
        {
            verify_out << data.range_sum(left, right) << '\n';
        }
    }
}
