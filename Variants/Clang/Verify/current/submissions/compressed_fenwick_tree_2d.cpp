#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/point_add_rectangle_sum

// Library: DataStructure/Compressed_Fenwick_Tree_2D.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Compressed_Fenwick_Tree_2D.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/compressed_fenwick_tree_2d.md
//
// API一覧
// CompressedFenwickTree2D<T=int64_t>(coordinates): 加算する点を登録して零で初期化。座標はint64_tのpair。
// size()/empty(): 登録した異なる座標の数・空判定、O(1)。
// point_id(point): 座標からIDを得る。未登録ならnullopt。IDは座標辞書順の0始まり。O(log(n+1))。
// add(id,delta): 登録点を加算。add(point,delta): 座標で加算し、未登録ならfalse。
// prefix_sum(x,y): X<xかつY<yの重み和。sum(l,b,r,t): [l,r)×[b,t)の重み和。
// 構築O(n log(n+1)+1)時間・領域。更新と照会はO(log²(n+1)+1)。nは構築前の座標数。
// 整数の和は重み型Tのビット幅を法とする。Tはbool以外の64ビット以下の整数型。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPRESSED_FENWICK_TREE_TWO_D_DEFINED
#define SIRKIT_COMPRESSED_FENWICK_TREE_TWO_D_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class CompressedFenwickTree2D
{
    using Storage = std::uint64_t;

public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    explicit CompressedFenwickTree2D(std::span<const Point> coordinates = {})
        : points_(coordinates.begin(), coordinates.end())
    {
        std::sort(points_.begin(), points_.end());
        points_.erase(std::unique(points_.begin(), points_.end()), points_.end());
        // points_ is ordered by (x,y), so x ranks can be assigned in one pass.
        x_ranks_.reserve(points_.size());
        for (const auto &[x, y] : points_)
        {
            if (xs_.empty() || xs_.back() != x)
            {
                xs_.push_back(x);
            }
            x_ranks_.push_back(xs_.size());
        }

        std::vector<std::size_t> y_order(points_.size());
        std::iota(y_order.begin(), y_order.end(), std::size_t{0});
        std::sort(y_order.begin(), y_order.end(),
                  [&](std::size_t lhs, std::size_t rhs)
                  {
                      return points_[lhs].second < points_[rhs].second;
                  });

        std::vector<std::vector<std::int64_t>> rows(xs_.size() + 1);
        for (std::size_t id : y_order)
        {
            const auto y = points_[id].second;
            for (std::size_t index = x_ranks_[id]; index <= xs_.size(); index += Lowbit(index))
            {
                if (rows[index].empty() || rows[index].back() != y)
                {
                    rows[index].push_back(y);
                }
            }
        }

        offsets_.reserve(xs_.size() + 2);
        std::size_t total = 0;
        for (const auto &row : rows)
        {
            offsets_.push_back(total);
            total += row.size();
        }
        offsets_.push_back(total);
        ys_.reserve(total);
        for (const auto &row : rows)
        {
            ys_.insert(ys_.end(), row.begin(), row.end());
        }
        values_.resize(total);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return points_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return points_.empty();
    }

    [[nodiscard]] std::optional<std::size_t> point_id(Point point) const
    {
        auto it = std::lower_bound(points_.begin(), points_.end(), point);
        if (it == points_.end() || *it != point)
        {
            return std::nullopt;
        }
        return static_cast<std::size_t>(it - points_.begin());
    }

    void add(std::size_t id, T delta)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= size())
        {
            throw std::out_of_range("CompressedFenwickTree2D: point ID");
        }
#endif
        const auto y = points_[id].second;
        const auto increment = static_cast<Storage>(delta);
        for (std::size_t index = x_ranks_[id]; index <= xs_.size(); index += Lowbit(index))
        {
            const auto offset = offsets_[index];
            const auto width = offsets_[index + 1] - offset;
            for (std::size_t j = RowRank(index, y) + 1; j <= width; j += Lowbit(j))
            {
                values_[offset + j - 1] += increment;
            }
        }
    }

    bool add(Point point, T delta)
    {
        if (const auto id = point_id(point))
        {
            add(*id, delta);
            return true;
        }
        return false;
    }

    [[nodiscard]] T prefix_sum(std::int64_t x, std::int64_t y) const
    {
        Storage answer = 0;
        for (std::size_t index = XRank(x); index != 0; index -= Lowbit(index))
        {
            answer += RowPrefix(offsets_[index], RowRank(index, y));
        }
        return static_cast<T>(answer);
    }

    [[nodiscard]] T sum(std::int64_t left, std::int64_t bottom, std::int64_t right, std::int64_t top) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || bottom > top)
        {
            throw std::invalid_argument("CompressedFenwickTree2D: reversed rectangle");
        }
#endif
        if (bottom == top)
        {
            return T{};
        }
        Storage answer = 0;
        std::size_t lo = XRank(left), hi = XRank(right);
        for (; lo != hi;)
        {
            if (lo < hi)
            {
                answer += RowSum(hi, bottom, top);
                hi -= Lowbit(hi);
            }
            else
            {
                answer -= RowSum(lo, bottom, top);
                lo -= Lowbit(lo);
            }
        }
        return static_cast<T>(answer);
    }

private:
    static std::size_t Lowbit(std::size_t index) noexcept
    {
        return index & (~index + 1);
    }

    [[nodiscard]] std::size_t XRank(std::int64_t x) const
    {
        return static_cast<std::size_t>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
    }

    [[nodiscard]] std::size_t RowRank(std::size_t index, std::int64_t y) const
    {
        const auto first = ys_.begin() + offsets_[index];
        const auto last = ys_.begin() + offsets_[index + 1];
        return static_cast<std::size_t>(std::lower_bound(first, last, y) - first);
    }

    [[nodiscard]] Storage RowPrefix(std::size_t offset, std::size_t count) const
    {
        Storage sum = 0;
        for (; count != 0; count -= Lowbit(count))
        {
            sum += values_[offset + count - 1];
        }
        return sum;
    }

    [[nodiscard]] Storage RowSum(std::size_t index, std::int64_t bottom, std::int64_t top) const
    {
        const auto offset = offsets_[index];
        return RowPrefix(offset, RowRank(index, top)) - RowPrefix(offset, RowRank(index, bottom));
    }

    std::vector<Point> points_;
    std::vector<std::size_t> x_ranks_, offsets_;
    std::vector<std::int64_t> xs_, ys_;
    std::vector<Storage> values_;
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

using Fenwick = sirkit::CompressedFenwickTree2D<>;

struct Operation
{
    int type = 0;
    std::int64_t x = 0, y = 0, upper_x_or_weight = 0, upper_y = 0;
};

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<Operation> events(static_cast<std::size_t>(n) + q);
    std::vector<Fenwick::Point> points;
    points.reserve(events.size());
    for (int i = 0; i < n; ++i)
    {
        auto &event = events[i];
        verify_in >> event.x >> event.y >> event.upper_x_or_weight;
        points.emplace_back(event.x, event.y);
    }
    for (int i = 0; i < q; ++i)
    {
        auto &event = events[n + i];
        verify_in >> event.type >> event.x >> event.y >> event.upper_x_or_weight;
        if (event.type == 0)
        {
            points.emplace_back(event.x, event.y);
        }
        else
        {
            verify_in >> event.upper_y;
        }
    }

    Fenwick tree(points);
    for (const auto &event : events)
    {
        if (event.type == 0)
        {
            tree.add({event.x, event.y}, event.upper_x_or_weight);
        }
        else
        {
            verify_out << tree.sum(event.x, event.y, event.upper_x_or_weight, event.upper_y) << '\n';
        }
    }
}
