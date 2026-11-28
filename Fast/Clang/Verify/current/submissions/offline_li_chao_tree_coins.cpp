#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/iroha2019-day4/tasks/iroha2019_day4_l

// Library: DataStructure/Offline_Li_Chao_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Offline_Li_Chao_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_li_chao_tree.md
//
// API一覧
// OfflineLiChaoTree<X, Function=OfflineLiChaoLine<X>>(): 空のクエリ列を準備する。
// OfflineLiChaoLine<T>{slope,intercept}: slope*x+interceptを返す既定の関数型。
// add_line(function): 全域で有効な関数を追加し、削除用IDを返す。償却O(1)。
// add_segment(l,r,function): l<=x<rで有効な関数を追加し、削除用IDを返す。償却O(1)。
// erase(id): 有効なIDを削除する。O(1)。IDは追加したオブジェクトの履歴内で使用する。
// query(x): 現在の最小値照会を登録し、その回答の添字を返す。償却O(1)。
// solve(): 照会順に最小値（候補なしはnullopt）を返す。履歴は保持し、追記・再実行可。
// 関数の比較は任意の2関数について、両方が有効な区間で高々1回だけ真偽が変わること。
// 既定の整数型では登録済み座標への評価の中間・最終結果がすべて型内に収まること。
// Q=照会数、X=異なる照会座標数、A=全域追加数、S=区間追加数とする。
// 時間O(Q log(Q+1)+(A+S)log(X+1)+log(Q+1)*(A log(X+1)+S log^2(X+1)))。
// 追加領域O(Q+X+(A+S)log(Q+1)+A log(X+1)+S log^2(X+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_LI_CHAO_TREE_DEFINED
#define SIRKIT_OFFLINE_LI_CHAO_TREE_DEFINED

namespace detail
{
inline std::size_t offline_li_chao_time_base(std::size_t queries, std::size_t capacity)
{
    if (queries > std::bit_floor((capacity - 1) / 2))
    {
        throw std::length_error("OfflineLiChaoTree: too many queries");
    }
    return std::bit_ceil(queries);
}

inline void offline_li_chao_add_count(std::size_t &count, std::size_t increment, std::size_t capacity)
{
    if (increment > capacity - count)
    {
        throw std::length_error("OfflineLiChaoTree: too many active intervals");
    }
    count += increment;
}
}

template <class T>
struct OfflineLiChaoLine
{
    T slope;
    T intercept;

    [[nodiscard]] T operator()(const T &x) const
    {
        return slope * x + intercept;
    }
};

template <class X, class Function = OfflineLiChaoLine<X>>
class OfflineLiChaoTree
{
public:
    using value_type = std::remove_cvref_t<std::invoke_result_t<const Function &, const X &>>;
    using id_type = std::size_t;

    OfflineLiChaoTree() = default;
    OfflineLiChaoTree(const OfflineLiChaoTree &)
        requires std::is_copy_constructible_v<Function>
    = default;
    OfflineLiChaoTree(OfflineLiChaoTree &&) noexcept = default;

    OfflineLiChaoTree &operator=(OfflineLiChaoTree other) noexcept
    {
        functions_.swap(other.functions_);
        queries_.swap(other.queries_);
        return *this;
    }

    id_type add_line(Function function)
    {
        return add(std::move(function), std::nullopt);
    }

    id_type add_segment(X left, X right, Function function)
    {
        return add(std::move(function), std::pair{std::move(left), std::move(right)});
    }

    void erase(id_type id)
    {
        functions_[id].finish = queries_.size();
    }

    std::size_t query(X x)
    {
        queries_.push_back(std::move(x));
        return queries_.size() - 1;
    }

    [[nodiscard]] std::vector<std::optional<value_type>> solve() const
    {
        if (queries_.empty())
        {
            return {};
        }
        const std::size_t capacity = std::vector<std::size_t>().max_size();
        const std::size_t base = detail::offline_li_chao_time_base(queries_.size(), capacity);
        std::vector<std::size_t> offsets(2 * base + 1);
        visit_intervals(base, [&](std::size_t node, id_type)
                        {
                            detail::offline_li_chao_add_count(offsets[node + 1], 1, capacity);
                        });
        for (std::size_t node = 1; node < offsets.size(); ++node)
        {
            detail::offline_li_chao_add_count(offsets[node], offsets[node - 1], capacity);
        }
        std::vector<id_type> ids(offsets.back());
        {
            auto cursor = offsets;
            visit_intervals(base, [&](std::size_t node, id_type id)
                            {
                                ids[cursor[node]++] = id;
                            });
        }

        auto coordinates = queries_;
        std::ranges::sort(coordinates);
        coordinates.erase(std::unique(coordinates.begin(), coordinates.end()), coordinates.end());
        const auto index = [&](const X &x)
        {
            return static_cast<std::size_t>(std::ranges::lower_bound(coordinates, x) - coordinates.begin());
        };
        std::vector<std::pair<std::size_t, std::size_t>> domains;
        domains.reserve(functions_.size());
        for (const auto &record : functions_)
        {
            domains.push_back(record.domain ? std::pair{index(record.domain->first), index(record.domain->second)}
                                            : std::pair{std::size_t{0}, coordinates.size()});
        }
        SpatialTree storage(functions_, coordinates);
        // 時間木の葉を左からたどり、照会順に結果を追加する。
        std::vector<std::optional<value_type>> answers;
        answers.reserve(queries_.size());
        const auto search = [&](const auto &visit, std::size_t node) -> void
        {
            if (node >= base && node - base >= queries_.size())
            {
                return;
            }
            const auto before = storage.snapshot();
            for (auto position = offsets[node]; position < offsets[node + 1]; ++position)
            {
                const auto id = ids[position];
                const auto [left, right] = domains[id];
                if (left < right)
                {
                    storage.add(left, right, id);
                }
            }
            if (node >= base)
            {
                const auto time = node - base;
                answers.push_back(storage.get_min(index(queries_[time])));
            }
            else
            {
                visit(visit, 2 * node);
                visit(visit, 2 * node + 1);
            }
            storage.roll_back(before);
        };
        search(search, 1);
        return answers;
    }

private:
    static constexpr std::size_t kOngoing = std::numeric_limits<std::size_t>::max();
    static constexpr id_type kEmpty = std::numeric_limits<id_type>::max();

    struct Record
    {
        Function function;
        std::optional<std::pair<X, X>> domain;
        std::size_t start;
        std::size_t finish;
    };

    static_assert(std::is_copy_constructible_v<X>);
    static_assert(std::is_copy_constructible_v<Record> || std::is_nothrow_move_constructible_v<Record>);

    class SpatialTree
    {
    public:
        SpatialTree(const std::vector<Record> &records, const std::vector<X> &coordinates)
            : records_(records), x_(coordinates), nodes_(4 * coordinates.size(), kEmpty)
        {
        }

        [[nodiscard]] std::size_t snapshot() const noexcept
        {
            return journal_.size();
        }

        void roll_back(std::size_t before) noexcept
        {
            while (journal_.size() > before)
            {
                const auto [node, old] = journal_.back();
                nodes_[node] = old;
                journal_.pop_back();
            }
        }

        void add(std::size_t first, std::size_t last, id_type id)
        {
            add_segment(1, 0, x_.size(), first, last, id);
        }

        [[nodiscard]] std::optional<value_type> get_min(std::size_t point) const
        {
            std::optional<value_type> best;
            std::size_t node = 1, left = 0, right = x_.size();
            for (;;)
            {
                if (nodes_[node] != kEmpty)
                {
                    auto candidate = eval(nodes_[node], point);
                    if (!best || candidate < *best)
                    {
                        best.emplace(std::move(candidate));
                    }
                }
                if (right - left == 1)
                {
                    return best;
                }
                const auto middle = left + (right - left) / 2;
                node *= 2;
                if (point < middle)
                {
                    right = middle;
                }
                else
                {
                    left = middle;
                    ++node;
                }
            }
        }

    private:
        [[nodiscard]] value_type eval(id_type id, std::size_t point) const
        {
            return records_[id].function(x_[point]);
        }

        void write(std::size_t node, id_type id)
        {
            journal_.emplace_back(node, nodes_[node]);
            nodes_[node] = id;
        }

        void add_line(std::size_t node, std::size_t left, std::size_t right, id_type id)
        {
            auto current = nodes_[node];
            if (current == kEmpty)
            {
                write(node, id);
                return;
            }
            const bool left_wins = eval(id, left) < eval(current, left);
            const bool right_wins = eval(id, right - 1) < eval(current, right - 1);
            if (left_wins == right_wins)
            {
                if (left_wins)
                {
                    write(node, id);
                }
                return;
            }

            const auto middle = left + (right - left) / 2;
            const bool center_wins = eval(id, middle) < eval(current, middle);
            if (center_wins)
            {
                write(node, id);
                id = current;
            }
            if (left_wins != center_wins)
            {
                add_line(2 * node, left, middle, id);
            }
            else
            {
                add_line(2 * node + 1, middle, right, id);
            }
        }

        void add_segment(std::size_t node, std::size_t left, std::size_t right,
                         std::size_t first, std::size_t last, id_type id)
        {
            if (first <= left && right <= last)
            {
                add_line(node, left, right, id);
                return;
            }
            const auto middle = left + (right - left) / 2;
            if (first < middle)
            {
                add_segment(2 * node, left, middle, first, last, id);
            }
            if (middle < last)
            {
                add_segment(2 * node + 1, middle, right, first, last, id);
            }
        }

        const std::vector<Record> &records_;
        const std::vector<X> &x_;
        std::vector<id_type> nodes_;
        std::vector<std::pair<std::size_t, id_type>> journal_;
    };

    id_type add(Function function, std::optional<std::pair<X, X>> domain)
    {
        const auto id = functions_.size();
        functions_.push_back({std::move(function), std::move(domain), queries_.size(), kOngoing});
        return id;
    }

    template <class Visitor>
    void visit_intervals(std::size_t base, Visitor visit) const
    {
        for (id_type id = 0; id < functions_.size(); ++id)
        {
            const auto &record = functions_[id];
            auto left = base + record.start;
            auto right = base + (record.finish == kOngoing ? queries_.size() : record.finish);
            for (; left < right; left /= 2, right /= 2)
            {
                if (left & 1)
                {
                    visit(left++, id);
                }
                if (right & 1)
                {
                    visit(--right, id);
                }
            }
        }
    }

    std::vector<Record> functions_;
    std::vector<X> queries_;
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


#include <array>
#include <cstdint>
#include <map>
#include <optional>

namespace
{
struct Ratio
{
    std::uint64_t numerator;
    std::uint64_t denominator;

    friend bool operator<(const Ratio &a, const Ratio &b)
    {
        return a.numerator * b.denominator < b.numerator * a.denominator;
    }
};

struct Piece
{
    int position;
    std::uint64_t value = 0;
    bool increasing;

    Ratio operator()(int x) const
    {
        return {static_cast<std::uint64_t>(increasing ? x - position : position - x), value};
    }
};
}

int main()
{
    using Envelope = sirkit::OfflineLiChaoTree<int, Piece>;
    using Ids = std::array<Envelope::id_type, 2>;
    Envelope timeline;
    std::map<int, std::optional<Ids>> active;
    int q = 0;
    verify_in >> q;
    while (q-- > 0)
    {
        int type = 0, x = 0;
        verify_in >> type >> x;
        if (type == 1)
        {
            std::uint64_t value = 0;
            verify_in >> value;
            auto &ids = active[x];
            if (value != 0)
            {
                ids = Ids{timeline.add_segment(0, x, {x, value, false}),
                          timeline.add_segment(x + 1, 1'000'000'001, {x, value, true})};
            }
        }
        else if (type == 2)
        {
            const auto it = active.find(x);
            if (it->second)
            {
                for (const auto id : *it->second)
                {
                    timeline.erase(id);
                }
            }
            active.erase(it);
        }
        else
        {
            timeline.query(x);
        }
    }
    verify_out.fixed(12);
    for (const auto &answer : timeline.solve())
    {
        const auto efficiency = answer ? static_cast<double>(answer->denominator) / static_cast<double>(answer->numerator) : 0.0;
        verify_out << efficiency << '\n';
    }
}
