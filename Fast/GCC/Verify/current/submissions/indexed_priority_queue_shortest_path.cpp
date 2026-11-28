#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/shortest_path

// Library: DataStructure/Indexed_Priority_Queue.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Indexed_Priority_Queue.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/indexed_priority_queue.md
//
// API一覧
// IndexedPriorityQueue<Key,Largest>(id_limit): IDが0以上id_limit未満の空のキュー。O(id_limit+1)。
// Keyは整数型。Largestがfalseなら最小、trueなら最大を先に取り出す。同じ優先度は小さいIDを先に扱う。
// set(id,priority): 未登録なら追加、登録済みなら優先度を更新。O(log(n+1))。
// erase(id): 削除できたかを返す。O(log(n+1))。
// top()/pop(): {ID,優先度}の取得・取り出し。O(1)/O(log(n+1))。
// priority(id)/contains(id): 登録済みの優先度・登録済みか。各O(1)。
// id_limit()/size()/empty(): ID上限・登録数・空判定。O(1)。clear(): 全削除。O(n)。
// nは登録数。領域O(id_limit+1)。入力検査有効時は範囲外のID、空のtop/pop、未登録のpriorityでout_of_range。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INDEXED_PRIORITY_QUEUE_DEFINED
#define SIRKIT_INDEXED_PRIORITY_QUEUE_DEFINED

template <std::integral Key = int, bool Largest = false>
class IndexedPriorityQueue
{
public:
    explicit IndexedPriorityQueue(std::size_t id_limit)
        : positions_(id_limit, kMissing), keys_(id_limit)
    {
        heap_.reserve(id_limit);
    }

    IndexedPriorityQueue(const IndexedPriorityQueue &other)
        : positions_(other.positions_), keys_(other.keys_), heap_(other.heap_)
    {
        heap_.reserve(id_limit());
    }

    IndexedPriorityQueue(IndexedPriorityQueue &&) noexcept = default;

    IndexedPriorityQueue &operator=(IndexedPriorityQueue other) noexcept
    {
        positions_.swap(other.positions_);
        keys_.swap(other.keys_);
        heap_.swap(other.heap_);
        return *this;
    }

    void set(std::size_t id, Key value)
    {
        CheckId(id);
        auto position = positions_[id];
        if (position == kMissing)
        {
            position = heap_.size();
            heap_.push_back(id);
            positions_[id] = position;
            keys_[id] = value;
            Swim(position);
            return;
        }
        const Key previous = keys_[id];
        keys_[id] = value;
        if (Precedes(value, previous))
        {
            Swim(position);
        }
        else
        {
            Sink(position);
        }
    }

    bool erase(std::size_t id)
    {
        CheckId(id);
        if (positions_[id] == kMissing)
        {
            return false;
        }
        Remove(id);
        return true;
    }

    [[nodiscard]] std::pair<std::size_t, Key> top() const
    {
        const auto id = heap_.front();
        return {id, static_cast<Key>(keys_[id])};
    }

    [[nodiscard]] std::pair<std::size_t, Key> pop()
    {
        const auto result = top();
        Remove(result.first);
        return result;
    }

    [[nodiscard]] Key priority(std::size_t id) const
    {
        CheckId(id);
        return keys_[id];
    }

    [[nodiscard]] bool contains(std::size_t id) const
    {
        CheckId(id);
        return positions_[id] != kMissing;
    }

    [[nodiscard]] std::size_t id_limit() const noexcept
    {
        return positions_.size();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return heap_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return heap_.empty();
    }

    void clear() noexcept
    {
        for (const auto id : heap_)
        {
            positions_[id] = kMissing;
        }
        heap_.clear();
    }

private:
    static constexpr std::size_t kMissing = std::numeric_limits<std::size_t>::max();

    [[nodiscard]] static bool Precedes(Key lhs, Key rhs) noexcept
    {
        if constexpr (Largest)
        {
            return lhs > rhs;
        }
        else
        {
            return lhs < rhs;
        }
    }

    [[nodiscard]] bool Earlier(std::size_t lhs, std::size_t rhs) const noexcept
    {
        const Key left = keys_[lhs], right = keys_[rhs];
        return left == right ? lhs < rhs : Precedes(left, right);
    }

    void CheckId([[maybe_unused]] std::size_t id) const
    {
    }

    void SwapEntries(std::size_t a, std::size_t b) noexcept
    {
        std::swap(heap_[a], heap_[b]);
        positions_[heap_[a]] = a;
        positions_[heap_[b]] = b;
    }

    void Swim(std::size_t at) noexcept
    {
        while (at != 0)
        {
            const auto parent = (at - 1) / 2;
            if (!Earlier(heap_[at], heap_[parent]))
            {
                break;
            }
            SwapEntries(at, parent);
            at = parent;
        }
    }

    void Sink(std::size_t at) noexcept
    {
        while (at < heap_.size() / 2)
        {
            auto child = 2 * at + 1;
            if (child + 1 < heap_.size() && Earlier(heap_[child + 1], heap_[child]))
            {
                ++child;
            }
            if (!Earlier(heap_[child], heap_[at]))
            {
                break;
            }
            SwapEntries(at, child);
            at = child;
        }
    }

    void Remove(std::size_t id) noexcept
    {
        const auto at = positions_[id];
        const auto last = heap_.back();
        heap_.pop_back();
        positions_[id] = kMissing;
        if (at == heap_.size())
        {
            return;
        }
        heap_[at] = last;
        positions_[last] = at;
        if (at != 0 && Earlier(last, heap_[(at - 1) / 2]))
        {
            Swim(at);
        }
        else
        {
            Sink(at);
        }
    }

    std::vector<std::size_t> positions_;
    std::vector<Key> keys_;
    std::vector<std::size_t> heap_;
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
#include <limits>
#include <utility>
#include <vector>

int main()
{
    std::size_t n, m, start, goal;
    verify_in >> n >> m >> start >> goal;
    std::vector<std::vector<std::pair<std::size_t, std::uint32_t>>> graph(n);
    for (std::size_t i = 0; i < m; ++i)
    {
        std::size_t from, to;
        std::uint32_t weight;
        verify_in >> from >> to >> weight;
        graph[from].emplace_back(to, weight);
    }
    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(n, kUnreachable);
    std::vector<std::size_t> parent(n);
    sirkit::IndexedPriorityQueue<std::uint64_t> queue(n);
    queue.set(start, 0);
    distance[start] = 0;
    while (!queue.empty())
    {
        const auto [vertex, key] = queue.pop();
        if (vertex == goal)
        {
            break;
        }
        for (const auto &[to, weight] : graph[vertex])
        {
            const auto candidate = key + weight;
            if (candidate < distance[to])
            {
                distance[to] = candidate;
                parent[to] = vertex;
                queue.set(to, candidate);
            }
        }
    }
    if (distance[goal] == kUnreachable)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    std::vector<std::size_t> path;
    for (std::size_t vertex = goal; vertex != start; vertex = parent[vertex])
    {
        path.push_back(vertex);
    }
    verify_out << distance[goal] << ' ' << path.size() << '\n';
    auto previous = start;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        verify_out << previous << ' ' << *it << '\n';
        previous = *it;
    }
}
