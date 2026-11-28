#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/jsc2024-final/tasks/jsc2024_final_e

// Library: Graph/Tree_Backedge_Absorption.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Backedge_Absorption.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_backedge_absorption.md
//
// API一覧
// TreeBackedgeAbsorption(parents, backward_edges): 親から子へ進む根付き木に、葉以外から真の祖先への辺を加える。
//   parents[v]=-1 は唯一の根。頂点は0始まり。重複辺は選択肢に別々に数える。構築 O(n+m)。
// expected_steps<Field>(): 根から各出辺を等確率で選び、葉に着くまでの移動回数の期待値。
// expected_cost<Field>(costs): 頂点vを出発する度に払う costs[v] の合計の期待値。
//   Field は0,1と辺本数を表せる体。消去中の係数が0なら std::nullopt を返す。
//   n頂点、追加m辺で O((n+m) log²(n+m+1)) 回の体演算、うち除算は O(n)。領域 O(n+m)。
// 部分木内の頂点を消去し、祖先に戻るときの係数と部分木滞在中の期待コストをまとめる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBTREE_INTERVALS_DEFINED
#define SIRKIT_SUBTREE_INTERVALS_DEFINED

namespace detail
{
inline int checked_subtree_interval_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("SubtreeIntervals: too many vertices");
    }
    return static_cast<int>(size);
}
}

class SubtreeIntervals
{
public:
    explicit SubtreeIntervals(std::span<const int> parents)
        : data_(std::make_shared<const Data>(parents))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(order().size());
    }

    [[nodiscard]] std::span<const int> order() const noexcept
    {
        return data_ ? std::span<const int>(data_->order) : std::span<const int>{};
    }

    [[nodiscard]] int index(int vertex) const
    {
        CheckIndex(vertex);
        return data_->first[vertex];
    }

    [[nodiscard]] std::pair<int, int> range(int vertex) const
    {
        const auto start = index(vertex);
        return {start, start + data_->count[vertex]};
    }

    [[nodiscard]] bool contains(int ancestor, int vertex) const
    {
        const auto [left, right] = range(ancestor);
        const auto at = index(vertex);
        return left <= at && at < right;
    }

    [[nodiscard]] int vertex_at(int position) const
    {
        CheckIndex(position);
        return data_->order[position];
    }

private:
    struct Data
    {
        std::vector<int> order;
        std::vector<int> first;
        std::vector<int> count;

        explicit Data(std::span<const int> parents)
        {
            const auto n = detail::checked_subtree_interval_size(parents.size());
            std::vector<int> child(n, -1), next(n, -1), work;
            work.reserve(n);
            for (int v = 0; v < n; ++v)
            {
                const int p = parents[v];
                if (p == -1)
                {
                    work.push_back(v);
                }
                else
                {
                    if (p < 0 || p >= n)
                    {
                        throw std::out_of_range("SubtreeIntervals: parent");
                    }
                    next[v] = child[p];
                    child[p] = v;
                }
            }

            first.resize(n);
            order.reserve(n);
            std::reverse(work.begin(), work.end());
            while (!work.empty())
            {
                const int v = work.back();
                work.pop_back();
                first[v] = static_cast<int>(order.size());
                order.push_back(v);
                for (int w = child[v]; w != -1; w = next[w])
                {
                    work.push_back(w);
                }
            }
            if (order.size() != parents.size())
            {
                throw std::invalid_argument("SubtreeIntervals: cycle");
            }

            count.assign(n, 1);
            for (auto current = order.rbegin(); current != order.rend(); ++current)
            {
                const int parent = parents[*current];
                if (parent != -1)
                {
                    count[parent] += count[*current];
                }
            }
        }
    };

    std::shared_ptr<const Data> data_;

    void CheckIndex([[maybe_unused]] int index) const
    {
    }
};
#endif

#ifndef SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED
#define SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED

class TreeBackedgeAbsorption
{
public:
    explicit TreeBackedgeAbsorption(
        std::span<const int> parents,
        std::span<const std::pair<int, int>> backward_edges = {})
        : layout_(parents), children_(parents.size()), backward_edges_(parents.size())
    {
        if (std::ranges::count(parents, -1) != 1)
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: expected one root");
        }
        for (const int vertex : layout_.order())
        {
            const int parent = parents[vertex];
            if (parent != -1)
            {
                children_[parent].push_back(vertex);
            }
        }
        for (const auto &[from, to] : backward_edges)
        {
            if (from == to || !layout_.contains(to, from) || children_[from].empty())
            {
                throw std::invalid_argument("TreeBackedgeAbsorption: invalid backward edge");
            }
            backward_edges_[from].push_back(to);
        }
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_steps() const
    {
        return Solve<Field>([](int)
                            {
                                return Field{1};
                            });
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_cost(std::span<const Field> costs) const
    {
        if (costs.size() != static_cast<std::size_t>(layout_.size()))
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: wrong number of costs");
        }
        return Solve<Field>([costs](int vertex)
                            {
                                return costs[vertex];
                            });
    }

private:
    SubtreeIntervals layout_;
    std::vector<std::vector<int>> children_;
    std::vector<std::vector<int>> backward_edges_;

    template <class Field>
    struct Row
    {
        Field cost{};
        Field scale{1}, inverse_scale{1};
        std::map<int, Field> coefficients;
    };

    template <class Field, class Reward>
    std::optional<Field> Solve(Reward reward) const
    {
        auto order = layout_.order();
        if (order.empty())
        {
            return Field{};
        }
        std::vector<Row<Field>> rows(order.size());
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int vertex = *it;
            const auto &children = children_[vertex];
            if (children.empty())
            {
                continue;
            }
            const auto &backward = backward_edges_[vertex];
            const Field degree(children.size() + backward.size());
            Field value = degree * reward(vertex);
            int largest = children.front();
            for (const int child : children)
            {
                value += rows[child].cost;
                if (rows[child].coefficients.size() > rows[largest].coefficients.size())
                {
                    largest = child;
                }
            }
            auto &row = rows[vertex];
            std::swap(row, rows[largest]);
            for (const int child : children)
            {
                auto &part = rows[child];
                const Field conversion = part.scale * row.inverse_scale;
                for (const auto &[ancestor, coefficient] : part.coefficients)
                {
                    row.coefficients[ancestor] += conversion * coefficient;
                }
                part.coefficients.clear();
            }
            for (const int ancestor : backward)
            {
                row.coefficients[ancestor] += row.inverse_scale;
            }
            Field pivot = degree;
            const auto self = row.coefficients.find(vertex);
            if (self != row.coefficients.end())
            {
                pivot = pivot - row.scale * self->second;
                row.coefficients.erase(self);
            }
            if (pivot == Field{})
            {
                return std::nullopt;
            }
            const Field reciprocal = pivot == Field{1} ? Field{1} : Field{1} / pivot;
            row.cost = value * reciprocal;
            row.scale = row.scale * reciprocal;
            row.inverse_scale = row.inverse_scale * pivot;
        }
        return rows[order.front()].cost;
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
#include <random>
#include <utility>
#include <vector>

namespace
{
class Residue
{
public:
    static inline std::uint32_t modulus = 998244353;

    Residue() = default;
    explicit Residue(std::uint64_t value) : value_(static_cast<std::uint32_t>(value % modulus))
    {
    }

    std::uint32_t value() const
    {
        return value_;
    }

    Residue &operator+=(Residue other)
    {
        value_ += other.value_;
        if (value_ >= modulus)
        {
            value_ -= modulus;
        }
        return *this;
    }

    friend Residue operator-(Residue a, Residue b)
    {
        if (a.value_ < b.value_)
        {
            a.value_ += modulus;
        }
        a.value_ -= b.value_;
        return a;
    }

    friend Residue operator*(Residue a, Residue b)
    {
        return Residue(std::uint64_t{a.value_} * b.value_);
    }

    friend Residue operator/(Residue a, Residue b)
    {
        for (std::uint32_t exponent = modulus - 2; exponent != 0; exponent /= 2)
        {
            if (exponent % 2 == 1)
            {
                a = a * b;
            }
            b = b * b;
        }
        return a;
    }

    friend bool operator==(Residue, Residue) = default;

private:
    std::uint32_t value_{};
};

bool is_prime(std::uint32_t value)
{
    for (std::uint32_t divisor = 3; divisor * divisor <= value; divisor += 2)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return true;
}
} // namespace

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<int> parents(n, -1);
    for (int vertex = 1; vertex < n; ++vertex)
    {
        verify_in >> parents[vertex];
        --parents[vertex];
    }
    std::vector<std::pair<int, int>> backward(m);
    for (auto &[from, to] : backward)
    {
        verify_in >> from >> to;
        --from;
        --to;
    }
    const sirkit::TreeBackedgeAbsorption walk(parents, backward);
    const auto try_prime = [&](std::uint32_t prime)
    {
        Residue::modulus = prime;
        const auto answer = walk.expected_steps<Residue>();
        if (answer)
        {
            verify_out << answer->value() << ' ' << prime << '\n';
        }
        return answer.has_value();
    };
    if (try_prime(998244353) || try_prime(1000000007))
    {
        return 0;
    }
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<std::uint32_t> indices(0, 500877823);
    while (true)
    {
        const std::uint32_t candidate = 998244353 + 2 * indices(random);
        if (is_prime(candidate) && try_prime(candidate))
        {
            return 0;
        }
    }
}
