#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/manhattanmst

// Library: Geometry/Manhattan_MST.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Manhattan_MST.cpp
// 参考: https://ei1333.github.io/library/graph/mst/manhattan-mst.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/manhattan_mst.md
//
// API一覧
// manhattan_mst(points): 格子点のマンハッタン距離による最小全域木。O(N log(N+1))時間・O(N)領域。
// pointsは(x,y)の列。同じ座標の頂点も可。各軸の絶対値はINT64_MAX/4以下。
// ManhattanMSTResult: costは距離の総和。edgesは元の0始まりの頂点番号の組。
// 頂点が0個か1個なら{0,{}}。総和がint64_tを超えるとstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MANHATTAN_MST_DEFINED
#define SIRKIT_MANHATTAN_MST_DEFINED

struct ManhattanMSTResult
{
    std::int64_t cost = 0;
    std::vector<std::pair<int, int>> edges;
};

namespace detail
{
inline int CheckedManhattanMSTSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("manhattan_mst: vertex count");
    }
    return static_cast<int>(size);
}

struct ManhattanRanks
{
    std::vector<int> position;
    int count = 0;
};

inline ManhattanRanks GetManhattanRanks(std::span<const std::pair<std::int64_t, std::int64_t>> points,
                                        std::vector<int> &order, bool use_x)
{
    const auto value = [&](int i)
    {
        return use_x ? points[i].first : points[i].second;
    };
    std::sort(order.begin(), order.end(), [&](int a, int b)
              {
                  return value(a) > value(b);
              });
    ManhattanRanks result{std::vector<int>(points.size()), 0};
    auto previous = value(order.front());
    for (int i : order)
    {
        const auto key = value(i);
        result.count += (key != previous);
        result.position[i] = result.count;
        previous = key;
    }
    ++result.count;
    return result;
}

// 1段上のビットは、下の段の64ビットに候補が残っていることを表す。
class ManhattanRankSet
{
    std::vector<std::vector<std::uint64_t>> words_;
    std::size_t end_;

public:
    explicit ManhattanRankSet(std::size_t size) : end_(size)
    {
        for (auto blocks = std::max(std::size_t{1}, (size + 63) / 64);; blocks = (blocks + 63) / 64)
        {
            words_.emplace_back(blocks);
            if (blocks == 1)
            {
                break;
            }
        }
    }

    std::size_t lower_bound(std::size_t index) const
    {
        std::size_t level = 0;
        while (true)
        {
            const auto block = index / 64;
            if (block >= words_[level].size())
            {
                return end_;
            }
            const auto bits = words_[level][block] & (~std::uint64_t{0} << (index % 64));
            if (bits != 0)
            {
                index = block * 64 + std::countr_zero(bits);
                break;
            }
            ++level;
            if (level == words_.size())
            {
                return end_;
            }
            index = block + 1;
        }
        while (level != 0)
        {
            --level;
            index = index * 64 + std::countr_zero(words_[level][index]);
        }
        return index;
    }

    void insert(std::size_t index)
    {
        for (auto &level : words_)
        {
            auto &word = level[index / 64];
            const bool nonempty = word != 0;
            word |= std::uint64_t{1} << (index % 64);
            if (nonempty)
            {
                break;
            }
            index /= 64;
        }
    }

    void erase(std::size_t index)
    {
        for (auto &level : words_)
        {
            auto &word = level[index / 64];
            word &= ~(std::uint64_t{1} << (index % 64));
            if (word != 0)
            {
                break;
            }
            index /= 64;
        }
    }
};
} // namespace detail

inline ManhattanMSTResult manhattan_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    const int n = detail::CheckedManhattanMSTSize(points.size());
    ManhattanMSTResult result;
    std::vector<std::pair<std::int64_t, std::int64_t>> transformed(points.begin(), points.end());
    if (n < 2)
    {
        return result;
    }
    struct Candidate
    {
        std::int64_t distance;
        int from;
        int to;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(points.size());
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    const auto x_rank = detail::GetManhattanRanks(points, order, true);
    const auto y_rank = detail::GetManhattanRanks(points, order, false);
    std::vector<int> pending(std::max(x_rank.count, y_rank.count));
    for (int direction = 0; direction < 4; ++direction)
    {
        // 座標を交換しても x+y は同じなので、直前の整列を再利用する。
        if (direction % 2 == 0)
        {
            std::sort(order.begin(), order.end(), [&](int lhs, int rhs)
                      {
                          const auto &[lx, ly] = transformed[lhs];
                          const auto &[rx, ry] = transformed[rhs];
                          return lx + ly < rx + ry;
                      });
        }
        // 4回の走査で第2座標は順にy、x、x、-y。-yの順位は逆順で再利用する。
        const auto &ranks = direction == 0 || direction == 3 ? y_rank : x_rank;
        const auto end = static_cast<std::size_t>(ranks.count);
        detail::ManhattanRankSet active(end);
        for (int current : order)
        {
            const auto &[x, y] = transformed[current];
            const auto rank = static_cast<std::size_t>(direction == 3
                                                           ? ranks.count - 1 - ranks.position[current]
                                                           : ranks.position[current]);
            for (auto next = active.lower_bound(rank); next != end; next = active.lower_bound(next))
            {
                const int previous = pending[next];
                const auto &[px, py] = transformed[previous];
                const auto dx = x - px, dy = y - py;
                if (dx < dy)
                {
                    break;
                }
                candidates.push_back({dx + dy, previous, current});
                active.erase(next);
            }
            pending[rank] = current;
            active.insert(rank);
        }
        for (auto &[x, y] : transformed)
        {
            if (direction % 2 == 0)
            {
                std::swap(x, y);
            }
            else
            {
                x = -x;
            }
        }
    }

    constexpr std::size_t kRadixThreshold = 1024;
    constexpr int kDigitBits = 8;
    constexpr std::size_t kBuckets = 1U << kDigitBits;
    constexpr auto kDigitMask = kBuckets - 1;
    if (candidates.size() < kRadixThreshold)
    {
        std::sort(candidates.begin(), candidates.end(), [](const Candidate &lhs, const Candidate &rhs)
                  {
                      return lhs.distance < rhs.distance;
                  });
    }
    else
    {
        std::uint64_t highest = 0;
        for (const auto &candidate : candidates)
        {
            highest = std::max(highest, static_cast<std::uint64_t>(candidate.distance));
        }
        if (highest != 0)
        {
            std::vector<Candidate> buffer(candidates.size());
            const int weight_bits = std::bit_width(highest);
            for (int shift = 0; shift < weight_bits; shift += kDigitBits)
            {
                std::array<std::size_t, kBuckets> count{};
                for (const auto &edge : candidates)
                {
                    ++count[(static_cast<std::uint64_t>(edge.distance) >> shift) & kDigitMask];
                }
                std::size_t seen = 0;
                for (auto &bucket : count)
                {
                    const auto frequency = bucket;
                    bucket = seen;
                    seen += frequency;
                }
                for (const auto &edge : candidates)
                {
                    buffer[count[(static_cast<std::uint64_t>(edge.distance) >> shift) & kDigitMask]++] = edge;
                }
                candidates.swap(buffer);
            }
        }
    }
    std::vector<int> parent(n, -1);
    const auto root = [&parent](int vertex)
    {
        int leader = vertex;
        while (parent[leader] >= 0)
        {
            leader = parent[leader];
        }
        while (vertex != leader)
        {
            const int next = parent[vertex];
            parent[vertex] = leader;
            vertex = next;
        }
        return leader;
    };
    result.edges.reserve(n - 1);
    const std::size_t needed = points.size() - 1;
    for (std::size_t i = 0; i < candidates.size() && result.edges.size() < needed; ++i)
    {
        const auto &[distance, from, to] = candidates[i];
        int left = root(from), right = root(to);
        if (left == right)
        {
            continue;
        }
        if (parent[left] > parent[right])
        {
            std::swap(left, right);
        }
        parent[left] += parent[right];
        parent[right] = left;
        if (distance > std::numeric_limits<std::int64_t>::max() - result.cost)
        {
            throw std::overflow_error("manhattan_mst: total weight");
        }
        result.cost += distance;
        result.edges.emplace_back(from, to);
    }
    return result;
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

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<std::int64_t, std::int64_t>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto tree = manhattan_mst(points);
    verify_out << tree.cost << '\n';
    for (const auto &[u, v] : tree.edges)
    {
        verify_out << u << ' ' << v << '\n';
    }
}
