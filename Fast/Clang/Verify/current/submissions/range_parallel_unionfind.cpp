#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_parallel_unionfind

// Library: DataStructure/Range_Parallel_Union_Find.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Parallel_Union_Find.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_parallel_union_find.md
//
// API一覧
// RangeParallelUnionFind(n=0): n頂点を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// merge_range(a,b,len[,on_merge]): 各i in [0,len)で頂点a+iとb+iを併合し、新規結合数を返す。
// on_merge(keep, absorbed)は実際の成分結合の直後に1回呼ぶ。再入更新は不可。例外時は完了済みの結合を保つ。
// merge(a,b)/unite(a,b): 1組だけ結合し、新規結合かを返す／戻り値なし。
// root(i)/leader(i)、same(a,b)/issame(a,b)、size(i)は通常のUnionFindと同じ。償却O(α(n+1))。
// size()/empty()/component_count(): 全頂点数、空判定、成分数。各O(1)。
// 最大処理区間長K、操作数QでO((n log(K+2)+Q)α(n+1))時間、O(n log(K+2))領域。
// レベルhは2^h長の有効開始位置n-2^h+1個を確保する。空区間・重なった区間も可。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_PARALLEL_UNION_FIND_DEFINED
#define SIRKIT_RANGE_PARALLEL_UNION_FIND_DEFINED

class RangeParallelUnionFind
{
public:
    explicit RangeParallelUnionFind(std::size_t n = 0)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("RangeParallelUnionFind: too many vertices");
        }
        levels_.emplace_back(n, -1);
        components_ = static_cast<int>(n);
    }

    RangeParallelUnionFind(const RangeParallelUnionFind &) = default;
    RangeParallelUnionFind &operator=(const RangeParallelUnionFind &) = default;

    RangeParallelUnionFind(RangeParallelUnionFind &&other) noexcept
        : levels_(std::move(other.levels_)), components_(std::exchange(other.components_, 0))
    {
        other.levels_.clear();
    }

    RangeParallelUnionFind &operator=(RangeParallelUnionFind &&other) noexcept
    {
        if (this != &other)
        {
            levels_ = std::move(other.levels_);
            components_ = std::exchange(other.components_, 0);
            other.levels_.clear();
        }
        return *this;
    }

    template <class OnMerge>
    int merge_range(int a, int b, int length, OnMerge &&on_merge)
    {
        CheckRanges(a, b, length);
        if (length == 0 || a == b || components_ == 1)
        {
            return 0;
        }

        const auto level = static_cast<unsigned>(std::bit_width(static_cast<unsigned>(length)) - 1);
        const int n = size();
        while (levels_.size() <= level)
        {
            const auto next_length = std::size_t{1} << levels_.size();
            levels_.emplace_back(static_cast<std::size_t>(n) - next_length + 1, -1);
        }

        // 先頭と末尾のブロックで区間全体を覆う。重なる部分の再結合は起きない。
        int result = MergeBlock(level, a, b, on_merge);
        const int tail = length - (1 << level);
        if (tail != 0)
        {
            result += MergeBlock(level, a + tail, b + tail, on_merge);
        }
        return result;
    }

    int merge_range(int a, int b, int length)
    {
        return merge_range(a, b, length, [](int, int)
                           {
                           });
    }

    bool merge(int a, int b)
    {
        return merge_range(a, b, 1) != 0;
    }

    void unite(int a, int b)
    {
        (void)merge(a, b);
    }

    [[nodiscard]] int root(int i)
    {
        CheckIndex(i);
        return Find(levels_.front(), i);
    }

    [[nodiscard]] int leader(int i)
    {
        return root(i);
    }

    [[nodiscard]] bool same(int a, int b)
    {
        CheckIndex(a);
        CheckIndex(b);
        return Find(levels_.front(), a) == Find(levels_.front(), b);
    }

    [[nodiscard]] bool issame(int a, int b)
    {
        return same(a, b);
    }

    [[nodiscard]] int size(int i)
    {
        const auto representative = root(i);
        return -levels_.front()[representative];
    }

    [[nodiscard]] int size() const noexcept
    {
        return levels_.empty() ? 0 : static_cast<int>(levels_.front().size());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return components_;
    }

private:
    std::vector<std::vector<int>> levels_;
    int components_ = 0;

    void CheckIndex(int i) const
    {
        (void)i;
    }

    void CheckRanges(int a, int b, int length) const
    {
        (void)a;
        (void)b;
        (void)length;
    }

    static int Find(std::vector<int> &parents, int i) noexcept
    {
        while (parents[i] >= 0)
        {
            const int next = parents[i];
            if (parents[next] < 0)
            {
                return next;
            }
            i = parents[i] = parents[next];
        }
        return i;
    }

    template <class OnMerge>
    int MergeBlock(unsigned level, int a, int b, OnMerge &on_merge)
    {
        auto &parents = levels_[level];
        auto keep = Find(parents, a);
        auto absorbed = Find(parents, b);
        if (keep == absorbed)
        {
            return 0;
        }
        if (parents[keep] > parents[absorbed])
        {
            std::swap(keep, absorbed);
        }

        int count = 1;
        if (level != 0)
        {
            // 通知が例外を送出しても、未完了のブロックを結合済みにしない。
            const unsigned child = level - 1;
            const int half = 1 << child;
            count = MergeBlock(child, a, b, on_merge);
            count += MergeBlock(child, a + half, b + half, on_merge);
        }

        parents[keep] += parents[absorbed];
        parents[absorbed] = keep;
        if (level == 0)
        {
            --components_;
            on_merge(keep, absorbed);
        }
        return count;
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
    constexpr int kMod = 998244353;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> sums(n);
    for (auto &value : sums)
    {
        verify_in >> value;
    }
    RangeParallelUnionFind uf(n);
    int answer = 0;
    const auto joined = [&](int keep, int absorbed)
    {
        const auto product = static_cast<std::int64_t>(sums[keep]) * sums[absorbed];
        answer = static_cast<int>((answer + product) % kMod);
        sums[keep] += sums[absorbed];
        if (sums[keep] >= kMod)
        {
            sums[keep] -= kMod;
        }
    };
    while (q-- > 0)
    {
        int length = 0, a = 0, b = 0;
        verify_in >> length >> a >> b;
        uf.merge_range(a, b, length, joined);
        verify_out << answer << '\n';
    }
}
