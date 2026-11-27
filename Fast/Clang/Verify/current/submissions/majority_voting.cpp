#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/majority_voting

// Library: DataStructure/Range_Majority.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Majority.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_majority.md
//
// API一覧
// RangeMajoritySet<T>{position,value}: 指定位置の値を置き換える。
// RangeMajorityQuery{left,right}: 半開区間[left,right)の過半数を問い合わせる。
// RangeMajorityOperation<T>: 上の2操作を保持するstd::variant。
// range_majority_queries(values,operations): 操作を順に適用し、問い合わせの答えだけを順番に返す。
// 区間長の半分を超える値を返し、空区間や該当する値がなければnullopt。元のvaluesは変更しない。
// 要素数n、操作数q、更新数u。n+u≤2^32−1。時間O((n+q)log(n+u+2))、領域O(n+q)。
// 計算量は値のコピー・比較をO(1)と数える。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_MAJORITY_DEFINED
#define SIRKIT_RANGE_MAJORITY_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckRangeMajorityCapacity(std::size_t n, std::size_t updates, std::size_t location_limit,
                                       std::size_t candidate_limit)
{
    if (n > std::min(location_limit, candidate_limit / 2))
    {
        throw std::length_error("range_majority_queries: sequence too large");
    }
    if (updates > location_limit - n)
    {
        throw std::length_error("range_majority_queries: too many updates");
    }
}
} // namespace detail

template <class T>
struct RangeMajoritySet
{
    std::size_t position;
    T value;
};

struct RangeMajorityQuery
{
    std::size_t left, right;
};

template <class T>
using RangeMajorityOperation = std::variant<RangeMajoritySet<T>, RangeMajorityQuery>;

template <class T>
std::vector<std::optional<T>> range_majority_queries(
    const std::vector<T> &values, std::span<const std::type_identity_t<RangeMajorityOperation<T>>> operations)
{
    using Index = std::uint32_t;
    using Location = std::pair<T, Index>;
    struct Vote
    {
        Index group = 0;
        Index balance = 0;
    };
    const auto n = values.size();
    constexpr auto kMaxIndex = std::numeric_limits<Index>::max();
    std::vector<Location> locations;
    std::vector<Vote> candidates;
    const auto capacity = std::min<std::size_t>(kMaxIndex, locations.max_size());

    std::size_t updates = 0;
    for (const auto &operation : operations)
    {
        updates += std::holds_alternative<RangeMajoritySet<T>>(operation);
    }
    detail::CheckRangeMajorityCapacity(n, updates, capacity, candidates.max_size());
    locations.reserve(n + updates);
    for (std::size_t i = 0; i < n; ++i)
    {
        locations.emplace_back(values[i], static_cast<Index>(i));
    }
    for (const auto &operation : operations)
    {
        if (const auto *set = std::get_if<RangeMajoritySet<T>>(&operation))
        {
            locations.emplace_back(set->value, static_cast<Index>(set->position));
        }
    }
    std::sort(locations.begin(), locations.end());
    locations.erase(std::unique(locations.begin(), locations.end()), locations.end());

    const auto combine = [](Vote a, Vote b) -> Vote
    {
        if (a.group == b.group)
        {
            return {a.group, a.balance + b.balance};
        }
        return a.balance > b.balance ? Vote{a.group, a.balance - b.balance} : Vote{b.group, b.balance - a.balance};
    };

    std::vector<Index> current(n), group_of(locations.size()), counts(locations.size());
    std::vector<Index> group_begin;
    candidates.resize(2 * n);
    for (std::size_t i = 0; i < locations.size(); ++i)
    {
        const auto &[value, position] = locations[i];
        if (i == 0 || locations[i - 1].first != value)
        {
            group_begin.push_back(static_cast<Index>(i));
        }
        const auto group = static_cast<Index>(group_begin.size() - 1);
        group_of[i] = group;
        if (value == values[position])
        {
            current[position] = static_cast<Index>(i);
            candidates[n + position] = {group, 1};
            counts[i] = 1;
        }
    }
    group_begin.push_back(static_cast<Index>(locations.size()));

    for (auto i = n; i > 1;)
    {
        --i;
        candidates[i] = combine(candidates[2 * i], candidates[2 * i + 1]);
    }
    for (std::size_t i = 0; i < counts.size(); ++i)
    {
        const auto parent = i | (i + 1);
        if (parent < counts.size())
        {
            counts[parent] += counts[i];
        }
    }

    const auto modify = [&]<bool Add>(std::size_t index)
    {
        for (; index < counts.size(); index |= index + 1)
        {
            if constexpr (Add)
            {
                ++counts[index];
            }
            else
            {
                --counts[index];
            }
        }
    };
    const auto prefix_count = [&](std::size_t end)
    {
        Index count = 0;
        for (; end != 0; end &= end - 1)
        {
            count += counts[end - 1];
        }
        return count;
    };

    std::vector<std::optional<T>> result;
    result.reserve(operations.size() - updates);
    for (const auto &operation : operations)
    {
        if (const auto *set = std::get_if<RangeMajoritySet<T>>(&operation))
        {
            auto &slot = current[set->position];
            if (locations[slot].first == set->value)
            {
                continue;
            }
            // 同じ値・位置の座標を使い回し、現在の値の座標だけを有効にする。
            const Location key{set->value, static_cast<Index>(set->position)};
            const auto replacement = static_cast<Index>(std::lower_bound(locations.begin(), locations.end(), key) -
                                                        locations.begin());
            modify.template operator()<false>(slot);
            modify.template operator()<true>(replacement);
            slot = replacement;

            auto i = n + set->position;
            candidates[i] = {group_of[slot], 1};
            for (i /= 2; i != 0; i /= 2)
            {
                candidates[i] = combine(candidates[2 * i], candidates[2 * i + 1]);
            }
            continue;
        }

        const auto [left, right] = std::get<RangeMajorityQuery>(operation);
        Vote candidate;
        // 異なる値を対にして消しても、過半数の値は消え切らない。
        for (auto a = left + n, b = right + n; a < b; a /= 2, b /= 2)
        {
            if (a % 2 != 0)
            {
                candidate = combine(candidate, candidates[a++]);
            }
            if (b % 2 != 0)
            {
                candidate = combine(candidate, candidates[--b]);
            }
        }
        if (candidate.balance == 0)
        {
            result.emplace_back(std::nullopt);
            continue;
        }

        const auto begin = locations.begin() + group_begin[candidate.group];
        const auto end = locations.begin() + group_begin[static_cast<std::size_t>(candidate.group) + 1];
        const auto half = (right - left) / 2;
        if (candidate.balance <= half)
        {
            const auto position_less = [](const Location &coordinate, std::size_t position)
            {
                return coordinate.second < position;
            };
            const auto a = static_cast<std::size_t>(std::lower_bound(begin, end, left, position_less) - locations.begin());
            const auto b = static_cast<std::size_t>(std::lower_bound(begin, end, right, position_less) - locations.begin());
            if (prefix_count(b) - prefix_count(a) <= half)
            {
                result.emplace_back(std::nullopt);
                continue;
            }
        }
        result.emplace_back(begin->first);
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


#include <cstddef>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::vector<RangeMajorityOperation<int>> operations;
    operations.reserve(q);
    for (int i = 0; i < q; ++i)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            std::size_t position = 0;
            int value = 0;
            verify_in >> position >> value;
            operations.emplace_back(RangeMajoritySet<int>{position, value});
        }
        else
        {
            std::size_t left = 0, right = 0;
            verify_in >> left >> right;
            operations.emplace_back(RangeMajorityQuery{left, right});
        }
    }
    for (const auto &answer : range_majority_queries(values, operations))
    {
        verify_out << answer.value_or(-1) << '\n';
    }
}
