#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/common_interval_decomposition_tree

// Library: Algorithm/Common_Interval_Decomposition.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Common_Interval_Decomposition.cpp
// 参考: I. Belghiti, M. Habib, A general method for common intervals
// (https://arxiv.org/abs/1309.7141) の商と分解木。区間探索を含むコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/common_interval_decomposition.md
//
// API一覧
// common_interval_decomposition(permutation): 0..N-1の順列の強区間木を、親が先の順序で返す。O(N log(N+1))時間、O(N)領域。
// CommonIntervalNode: 位置[left,right)、値[value_begin,value_end)、親parent（根は-1）、種類kind。
// 順列の部分区間の値が連続し、他の連続値区間と包含・互いに素であるとき強区間という。
// CommonIntervalKind: leaf（1要素）、increasing/decreasing（子の値区間が順/逆順）、prime（それ以外）。
// 空列は空。通常版で範囲外や重複値はinvalid_argument。節の添字はint。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMMON_INTERVAL_DECOMPOSITION_DEFINED
#define SIRKIT_COMMON_INTERVAL_DECOMPOSITION_DEFINED

enum class CommonIntervalKind
{
    leaf,
    increasing,
    decreasing,
    prime,
};

struct CommonIntervalNode
{
    int left;
    int right;
    int value_begin;
    int value_end;
    int parent;
    CommonIntervalKind kind;
};

namespace detail
{
inline int CommonIntervalSize(std::size_t count)
{
    constexpr auto kMaximum = static_cast<std::size_t>(std::numeric_limits<int>::max() / 2) + 1;
    if (count > kMaximum)
    {
        throw std::length_error("common_interval_decomposition: node indices do not fit in int");
    }
    return static_cast<int>(count);
}

class CommonIntervalScoreTree
{
    struct Entry
    {
        std::int64_t minimum;
        std::int64_t pending;
    };

public:
    CommonIntervalScoreTree(int n, std::int64_t inactive)
        : leaves_(static_cast<int>(std::bit_ceil(static_cast<unsigned>(n)))),
          scores_(2 * static_cast<std::size_t>(leaves_), Entry{inactive, 0})
    {
    }

    void add(int first, int last, std::int64_t delta)
    {
        if (first < last)
        {
            Add(1, 0, leaves_, first, last, delta);
        }
    }

    void set(int position, std::int64_t value)
    {
        Set(1, 0, leaves_, position, value);
    }

    int last_zero_before(int limit)
    {
        return LastZero(1, 0, leaves_, limit);
    }

private:
    int leaves_;
    std::vector<Entry> scores_;

    void Apply(int node, std::int64_t delta)
    {
        scores_[node].minimum += delta;
        scores_[node].pending += delta;
    }

    void Push(int node)
    {
        if (const auto delta = scores_[node].pending; delta != 0)
        {
            Apply(2 * node, delta);
            Apply(2 * node + 1, delta);
            scores_[node].pending = 0;
        }
    }

    void Pull(int node)
    {
        scores_[node].minimum = std::min(scores_[2 * node].minimum, scores_[2 * node + 1].minimum);
    }

    void Add(int node, int low, int high, int first, int last, std::int64_t delta)
    {
        if (first <= low && high <= last)
        {
            Apply(node, delta);
            return;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        if (first < middle)
        {
            Add(2 * node, low, middle, first, last, delta);
        }
        if (middle < last)
        {
            Add(2 * node + 1, middle, high, first, last, delta);
        }
        Pull(node);
    }

    void Set(int node, int low, int high, int position, std::int64_t value)
    {
        if (high - low == 1)
        {
            scores_[node] = {value, 0};
            return;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        if (position < middle)
        {
            Set(2 * node, low, middle, position, value);
        }
        else
        {
            Set(2 * node + 1, middle, high, position, value);
        }
        Pull(node);
    }

    int LastZero(int node, int low, int high, int limit)
    {
        if (low >= limit || scores_[node].minimum != 0)
        {
            return -1;
        }
        if (high - low == 1)
        {
            return low;
        }
        Push(node);
        const int middle = low + (high - low) / 2;
        const int right = LastZero(2 * node + 1, middle, high, limit);
        return right >= 0 ? right : LastZero(2 * node, low, middle, limit);
    }
};

struct CommonIntervalRawNode
{
    CommonIntervalNode interval;
    int first_child = -1;
    int next_sibling = -1;
};

inline std::vector<CommonIntervalNode> NormalizeCommonIntervals(
    const std::vector<CommonIntervalRawNode> &raw, int root)
{
    struct Task
    {
        int node;
        int parent;
    };
    std::vector<Task> pending{{root, -1}};
    std::vector<CommonIntervalNode> result;
    result.reserve(raw.size());
    while (!pending.empty())
    {
        auto [index, parent] = pending.back();
        pending.pop_back();
        const auto &node = raw[index];
        if (node.next_sibling >= 0)
        {
            pending.push_back({node.next_sibling, parent});
        }
        const auto kind = node.interval.kind;
        const bool monotone = kind == CommonIntervalKind::increasing || kind == CommonIntervalKind::decreasing;
        // 同じ向きの線形の親子を縮約する。残る節が強区間になる。
        if (!monotone || parent < 0 || result[parent].kind != kind)
        {
            const int id = static_cast<int>(result.size());
            result.push_back(node.interval);
            result.back().parent = parent;
            parent = id;
        }
        if (node.first_child >= 0)
        {
            pending.push_back({node.first_child, parent});
        }
    }
    return result;
}
} // namespace detail

inline std::vector<CommonIntervalNode> common_interval_decomposition(std::span<const int> permutation)
{
    if (permutation.empty())
    {
        return {};
    }
    const int n = detail::CommonIntervalSize(permutation.size());
#ifndef SKIP_RUNTIME_CHECKS
    {
        std::vector<bool> seen(n);
        for (const auto x : permutation)
        {
            if (x < 0 || x >= n || seen[x])
            {
                throw std::invalid_argument("common_interval_decomposition: expected a permutation of 0..N-1");
            }
            seen[x] = true;
        }
    }
#endif

    const auto inactive = static_cast<std::int64_t>(n) + 1;
    detail::CommonIntervalScoreTree score(n, inactive);
    std::vector<detail::CommonIntervalRawNode> raw;
    raw.reserve(n + (n - 1));
    std::vector<int> forest;
    forest.reserve(n);

    struct Extremum
    {
        int first;
        int value;
    };
    std::vector<Extremum> maxima, minima;
    for (int r = 0; r < n; ++r)
    {
        // 左端lの値は max(P[l..r])-min(P[l..r])-(r-l)。0が連続値区間。
        score.add(0, r, -1);
        const auto replace_extrema = [&](std::vector<Extremum> &ranges, bool maximum)
        {
            int l = r;
            while (!ranges.empty() &&
                   (maximum ? ranges.back().value < permutation[r] : ranges.back().value > permutation[r]))
            {
                const auto [first, before] = ranges.back();
                ranges.pop_back();
                const auto delta = maximum ? permutation[r] - before : before - permutation[r];
                score.add(first, l, delta);
                l = first;
            }
            ranges.push_back({l, permutation[r]});
        };
        replace_extrema(maxima, true);
        replace_extrema(minima, false);
        score.set(r, 0);

        forest.push_back(static_cast<int>(raw.size()));
        raw.push_back({{r, r + 1, permutation[r], permutation[r] + 1, -1, CommonIntervalKind::leaf}});
        for (;;)
        {
            const int l = score.last_zero_before(raw[forest.back()].interval.left);
            if (l < 0)
            {
                break;
            }

            // 現在の森の根を2個以上まとめる、最短の連続値区間を選ぶ。
            int first_child = -1, count = 0, value_begin = n, value_end = 0;
            for (;;)
            {
                const int child = forest.back();
                forest.pop_back();
                const auto &interval = raw[child].interval;
                raw[child].next_sibling = first_child;
                first_child = child;
                ++count;
                value_begin = std::min(value_begin, interval.value_begin);
                value_end = std::max(value_end, interval.value_end);
                if (interval.left == l)
                {
                    break;
                }
                score.set(interval.left, inactive);
            }

            // 2子より多い場合は、子の並びに非自明な連続値区間がない。
            auto kind = CommonIntervalKind::prime;
            if (count == 2)
            {
                kind = raw[first_child].interval.value_begin == value_begin
                           ? CommonIntervalKind::increasing
                           : CommonIntervalKind::decreasing;
            }
            forest.push_back(static_cast<int>(raw.size()));
            raw.push_back({{l, r + 1, value_begin, value_end, -1, kind}, first_child});
        }
    }
    return detail::NormalizeCommonIntervals(raw, forest.back());
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
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<int> permutation(n);
    for (auto &value : permutation)
    {
        verify_in >> value;
    }
    const auto tree = sirkit::common_interval_decomposition(permutation);
    verify_out << tree.size() << '\n';
    for (const auto &interval : tree)
    {
        verify_out << interval.parent << ' ' << interval.left << ' ' << interval.right - 1
                   << (interval.kind == sirkit::CommonIntervalKind::prime ? " prime\n" : " linear\n");
    }
}
