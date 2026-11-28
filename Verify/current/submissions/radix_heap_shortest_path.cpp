#define SKIP_RUNTIME_CHECKS
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/shortest_path

// Library: DataStructure/Radix_Heap.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/radix_heap.md
//
// API一覧
// RadixHeap<Key,Value>(lower_bound=0): 空の単調最小キュー。Keyはbool以外の64ビット以下の符号なし整数型。
// Valueは非送出でムーブ構築できる型。キーの幅をWとしてO(W)。
// emplace(key,args...): keyに対応する値を構築して追加する。key>=lower_bound()が必要。償却O(1)。
// top(): 最小キーの要素をconst参照で返す。下限を変えない。走査するバケットの大きさをbとしてO(1+b)。
// pop(): 最小キーの要素を取り出し、下限をそのキーへ進める。挿入件数あたり償却O(W)、最悪O(W+b)。
// lower_bound()/size()/empty(): 挿入できる最小キー・要素数・空判定。O(1)。
// clear(lower_bound=0): 全要素を除き、下限を指定値に戻す。容量は再利用。O(W+n)。
// swap(other): キューを交換する。O(W)。同じキーの取り出し順は未規定。
// top/popは非空が前提。入力検査有効時に空ならout_of_range、下限未満の追加はinvalid_argument。
// topの参照は追加、削除、交換で無効になり得る。最大要素数をMとして領域O(W(M+1))。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RADIX_HEAP_DEFINED
#define SIRKIT_RADIX_HEAP_DEFINED

template <std::unsigned_integral Key, class Value>
    requires(!std::same_as<Key, bool> && std::numeric_limits<Key>::digits <= 64 &&
             std::is_nothrow_move_constructible_v<Value>)
class RadixHeap
{
public:
    using value_type = std::pair<Key, Value>;

    explicit RadixHeap(Key lower_bound = 0) noexcept : lower_bound_(lower_bound)
    {
    }

    RadixHeap(const RadixHeap &) = default;

    RadixHeap(RadixHeap &&other) noexcept
    {
        swap(other);
    }

    RadixHeap &operator=(RadixHeap other) noexcept
    {
        swap(other);
        return *this;
    }

    template <class... Args>
    void emplace(Key key, Args &&...args)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (key < lower_bound_)
        {
            throw std::invalid_argument("RadixHeap: key is below the lower bound");
        }
#endif
        const auto index = Bucket(key, lower_bound_);
        buckets_[index].emplace_back(std::piecewise_construct, std::forward_as_tuple(key),
                                     std::forward_as_tuple(std::forward<Args>(args)...));
        MarkOccupied(index);
        ++size_;
    }

    [[nodiscard]] const value_type &top() const
    {
        RequireNonempty();
        if (!buckets_[0].empty())
        {
            return buckets_[0].back();
        }
        const auto &bucket = buckets_[FirstOutsideLowerBound()];
        return bucket[MinIndex(bucket)];
    }

    [[nodiscard]] value_type pop()
    {
        RequireNonempty();
        if (buckets_[0].empty())
        {
            Refill();
        }
        auto &bucket = buckets_[0];
        value_type result = std::move(bucket.back());
        bucket.pop_back();
        --size_;
        return result;
    }

    [[nodiscard]] Key lower_bound() const noexcept
    {
        return lower_bound_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    void clear(Key lower_bound = 0) noexcept
    {
        for (auto &bucket : buckets_)
        {
            bucket.clear();
        }
        lower_bound_ = lower_bound;
        occupied_ = 0;
        size_ = 0;
    }

    void swap(RadixHeap &other) noexcept
    {
        buckets_.swap(other.buckets_);
        std::swap(lower_bound_, other.lower_bound_);
        std::swap(occupied_, other.occupied_);
        std::swap(size_, other.size_);
    }

private:
    static constexpr std::size_t kBucketCount = std::numeric_limits<Key>::digits + 1;

    [[nodiscard]] static std::size_t Bucket(Key key, Key lower_bound) noexcept
    {
        return static_cast<std::size_t>(std::bit_width(static_cast<std::uint64_t>(key) ^
                                                       static_cast<std::uint64_t>(lower_bound)));
    }

    void MarkOccupied(std::size_t index) noexcept
    {
        if (index != 0)
        {
            occupied_ |= std::uint64_t{1} << (index - 1);
        }
    }

    [[nodiscard]] std::size_t FirstOutsideLowerBound() const noexcept
    {
        return static_cast<std::size_t>(std::countr_zero(occupied_)) + 1;
    }

    [[nodiscard]] static std::size_t MinIndex(const std::vector<value_type> &bucket) noexcept
    {
        std::size_t best = 0;
        for (std::size_t index = 1; index < bucket.size(); ++index)
        {
            if (bucket[index].first < bucket[best].first)
            {
                best = index;
            }
        }
        return best;
    }

    void RequireNonempty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("RadixHeap: empty queue");
        }
#endif
    }

    void Refill()
    {
        const auto index = FirstOutsideLowerBound();
        auto &source = buckets_[index];
        const Key next_bound = source[MinIndex(source)].first;
        std::array<std::size_t, kBucketCount> sizes{};
        for (const auto &item : source)
        {
            ++sizes[Bucket(item.first, next_bound)];
        }
        // 移動先はすべて下位の空バケット。確保が終わるまではキーの下限も変えない。
        for (std::size_t target = 0; target < index; ++target)
        {
            buckets_[target].reserve(sizes[target]);
        }
        lower_bound_ = next_bound;
        occupied_ &= ~(std::uint64_t{1} << (index - 1));
        for (auto &item : source)
        {
            const auto target = Bucket(item.first, lower_bound_);
            buckets_[target].push_back(std::move(item));
            MarkOccupied(target);
        }
        source.clear();
    }

    std::array<std::vector<value_type>, kBucketCount> buckets_;
    Key lower_bound_ = 0;
    std::uint64_t occupied_ = 0;
    std::size_t size_ = 0;
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
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

struct EdgeInput
{
    int from, to;
    std::uint32_t weight;
};

int main()
{
    int n, m, start, goal;
    verify_in >> n >> m >> start >> goal;
    std::vector<int> offset(static_cast<std::size_t>(n) + 1);
    std::vector<EdgeInput> input(static_cast<std::size_t>(m));
    for (auto &edge : input)
    {
        verify_in >> edge.from >> edge.to >> edge.weight;
        ++offset[static_cast<std::size_t>(edge.from) + 1];
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    auto cursor = offset;
    std::vector<int> to(static_cast<std::size_t>(m));
    std::vector<std::uint32_t> weight(static_cast<std::size_t>(m));
    for (const auto &edge : input)
    {
        const auto index = static_cast<std::size_t>(cursor[static_cast<std::size_t>(edge.from)]++);
        to[index] = edge.to;
        weight[index] = edge.weight;
    }

    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(static_cast<std::size_t>(n), kUnreachable);
    std::vector<int> parent(static_cast<std::size_t>(n), -1);
    sirkit::RadixHeap<std::uint64_t, int> queue;
    queue.emplace(0, start);
    distance[static_cast<std::size_t>(start)] = 0;
    while (!queue.empty())
    {
        const auto [current, vertex] = queue.pop();
        const auto index = static_cast<std::size_t>(vertex);
        if (vertex == goal)
        {
            break;
        }
        if (current != distance[index])
        {
            continue;
        }
        for (int edge = offset[index]; edge < offset[index + 1]; ++edge)
        {
            const auto edge_index = static_cast<std::size_t>(edge);
            const auto next = static_cast<std::size_t>(to[edge_index]);
            const auto candidate = current + weight[edge_index];
            if (candidate < distance[next])
            {
                distance[next] = candidate;
                parent[next] = vertex;
                queue.emplace(candidate, to[edge_index]);
            }
        }
    }

    if (distance[static_cast<std::size_t>(goal)] == kUnreachable)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    std::vector<int> path;
    for (int vertex = goal; vertex != start; vertex = parent[static_cast<std::size_t>(vertex)])
    {
        path.push_back(vertex);
    }
    verify_out << distance[static_cast<std::size_t>(goal)] << ' ' << path.size() << '\n';
    int previous = start;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        verify_out << previous << ' ' << *it << '\n';
        previous = *it;
    }
}
