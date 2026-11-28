#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc246/tasks/abc246_g

// Library: DataStructure/Meldable_Heap.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Meldable_Heap.cpp
// 参考（左偏木）: C. A. Crane, Linear Lists and Priority Queues as Balanced Binary Trees, 1972.
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/meldable_heap.md
//
// API一覧
// MeldableHeap<T,Compare=std::less<T>>()/MeldableHeap(compare): 空の優先度付きキュー。既定は最大ヒープ。
// size()/empty()/top(): 要素数・空かどうか・最優先の要素へのconst参照。O(1)。
// push(value)/emplace(args...)/pop(): 要素の追加・構築して追加・最優先の要素の削除。O(log(n+1))。
// meld(other): 要素を取り込みotherを空にする。自分自身なら何もしない。O(log(n+m+1))。
// Compareはconstで呼べる狭義弱順序。結合する両方で同じ順序を使う。比較が例外を送出しても要素は保つ。
// clear()/デストラクタ: 全要素を再帰なしで破棄。O(n+1)時間・追加O(1)領域。全体の領域O(n)。
// コピー不可。Compareの対応するムーブ操作がnoexceptなら移動可。ムーブ元は空。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MELDABLE_HEAP_DEFINED
#define SIRKIT_MELDABLE_HEAP_DEFINED

template<class T, class Compare = std::less<T>>
class MeldableHeap
{
public:
    MeldableHeap() = default;

    explicit MeldableHeap(Compare compare) noexcept(std::is_nothrow_move_constructible_v<Compare>)
        : compare_(std::move(compare))
    {
    }

    MeldableHeap(const MeldableHeap &) = delete;
    MeldableHeap &operator=(const MeldableHeap &) = delete;

    MeldableHeap(MeldableHeap &&other) noexcept
        requires std::is_nothrow_move_constructible_v<Compare>
        : compare_(std::move(other.compare_)), root_(std::move(other.root_)), size_(std::exchange(other.size_, 0))
    {
    }

    MeldableHeap &operator=(MeldableHeap &&other) noexcept
        requires std::is_nothrow_move_assignable_v<Compare>
    {
        if (this != &other)
        {
            clear();
            compare_ = std::move(other.compare_);
            root_ = std::move(other.root_);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    ~MeldableHeap()
    {
        clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]] const T &top() const
    {
        CheckNonEmpty();
        return root_->value;
    }

    void push(const T &value)
    {
        emplace(value);
    }

    void push(T &&value)
    {
        emplace(std::move(value));
    }

    template<class... Args>
    void emplace(Args &&...args)
    {
        auto node = std::make_unique<Node>(std::forward<Args>(args)...);
        MeldLinks(root_, node);
        ++size_;
    }

    void pop()
    {
        CheckNonEmpty();
        MeldLinks(root_->left, root_->right);
        Link old = std::move(root_);
        root_ = std::move(old->left);
        --size_;
    }

    void meld(MeldableHeap &other)
    {
        if (this == &other)
        {
            return;
        }
        MeldLinks(root_, other.root_);
        size_ += std::exchange(other.size_, 0);
    }

    void clear() noexcept
    {
        while (root_)
        {
            Link old = std::move(root_);
            if (old->left)
            {
                root_ = std::move(old->left);
                old->left = std::move(root_->right);
                root_->right = std::move(old);
            }
            else
            {
                root_ = std::move(old->right);
            }
        }
        size_ = 0;
    }

private:
    struct Node;
    using Link = std::unique_ptr<Node>;

    struct Node
    {
        T value;
        Link left, right;
        unsigned rank = 1;

        template<class... Args>
        explicit Node(Args &&...args) : value(std::forward<Args>(args)...)
        {
        }
    };

    [[no_unique_address]] Compare compare_{};
    Link root_;
    std::size_t size_ = 0;

    void CheckNonEmpty() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (empty())
        {
            throw std::out_of_range("MeldableHeap: empty heap");
        }
#endif
    }

    static unsigned Rank(const Link &link) noexcept
    {
        return link ? link->rank : 0;
    }

    void MeldLinks(Link &first, Link &second)
    {
        if (!second)
        {
            return;
        }
        if (!first)
        {
            first = std::move(second);
            return;
        }
        // 比較が終わってから再帰の戻り順に所有権を移す。
        if (std::invoke(std::as_const(compare_), std::as_const(first->value), std::as_const(second->value)))
        {
            MeldLinks(second->right, first);
            first = std::move(second);
        }
        else
        {
            MeldLinks(first->right, second);
        }
        if (Rank(first->left) < Rank(first->right))
        {
            first->left.swap(first->right);
        }
        first->rank = Rank(first->right) + 1;
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


#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<int> values(n);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> values[v];
    }
    std::vector<std::vector<int>> graph(n);
    for (int i = 1; i < n; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> parent(n, -1), order{0};
    order.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        const int u = order[i];
        for (const int v : graph[u])
        {
            if (v != parent[u])
            {
                parent[v] = u;
                order.push_back(v);
            }
        }
    }

    std::vector<sirkit::MeldableHeap<int>> heaps(n);
    for (auto at = order.rbegin(); at != order.rend(); ++at)
    {
        const int v = *at;
        if (!heaps[v].empty())
        {
            heaps[v].pop();
        }
        if (v != 0)
        {
            heaps[v].push(values[v]);
            heaps[parent[v]].meld(heaps[v]);
        }
    }
    verify_out << (heaps[0].empty() ? 0 : heaps[0].top()) << '\n';
}
