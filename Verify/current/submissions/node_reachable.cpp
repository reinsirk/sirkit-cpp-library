#define SKIP_RUNTIME_CHECKS
// Verify: https://atcoder.jp/contests/abc228/tasks/abc228_b
// Submission: https://atcoder.jp/contests/abc228/submissions/80395895

// Library: DataStructure/Node.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/node.md
//
// API一覧
// [Node・単方向連結列]
// Node(next={}): 値を持たない単方向ノード。nはNode自体を所有するshared_ptrを接続する。
// コピーはnを共有し、ムーブはnを移す。循環の自動回収は行わない。
// 非共有の後続k個をO(k)時間・O(1)追加領域で解放。長い連結は反復、2ノード以下は通常破棄。
// make_node(next={}): nextを後続に持つ新しいノードを返す。O(1)時間・1回確保。
// make_chain(count, tail={}): tailの前にcount個追加して先頭を返す。0個ならtail。O(count)時間・count回確保。
// chain_size(head): ノード数を返す。循環は既定でinvalid_argument、検査省略時は循環なしが前提。O(k)時間・O(1)領域。
// chain_tail(head): 末尾（空ならnullptr）を返す。循環は既定でinvalid_argument、検査省略時は循環なしが前提。O(k)時間・O(1)領域。
// chain_nth(head, index): 0始まりのindex番目（不足ならnullptr）を返す。循環も可。非循環O(min(index+1,k))、循環O(index+1)時間・O(1)領域。
// push_front_node(head): 新しい先頭を追加してheadを更新する。O(1)時間・1回確保。
// pop_front_node(head): headを次へ移し、空ならfalseを返す。共有された元先頭のnは保持する。O(1)時間・領域。
// reverse_chain(head): 循環なしのnを逆転し新しい先頭を返す。共有者にも変更が見える。O(k)時間・O(1)領域。
// clone_chain(head): 独立した同長の列を返す。循環は既定でinvalid_argument、検査省略時は循環なしが前提。O(k)時間・k回確保。
// cycle_entry(head): 循環入口の非所有ポインタ（循環なしならnullptr）を返す。O(k)時間・O(1)領域。
// has_cycle(head): 循環の有無を返す。O(k)時間・O(1)領域。
// break_cycle(head): 循環の末尾から入口への辺を切り、切ったらtrueを返す。共有者にも変更が見える。O(k)時間・O(1)領域。
// clear_chain(head): headを空にし、共有されていない後続k個を反復解放する。循環回収には先にbreak_cycle。O(k)時間・O(1)領域。

#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#ifndef SIRKIT_NODE_DEFINED
#define SIRKIT_NODE_DEFINED
struct Node {
    std::shared_ptr<Node> n;

    explicit Node(std::shared_ptr<Node> next = {}) noexcept : n(std::move(next)) {}
    Node(const Node&) noexcept = default;
    Node(Node&&) noexcept = default;
    Node& operator=(const Node&) noexcept = default;
    Node& operator=(Node&&) noexcept = default;

    ~Node() {
        if (!n || !n->n) return;
        if (n.use_count() != 1) return;
        auto current = std::move(n);
        do {
            auto next = std::move(current->n);
            current.swap(next); // nextが解放するノードのnは空なので、連結に沿って再帰しない。
        } while (current.use_count() == 1 && current);
    }
};
#endif

#ifndef SIRKIT_MAKE_NODE_DEFINED
#define SIRKIT_MAKE_NODE_DEFINED
inline std::shared_ptr<Node> make_node(std::shared_ptr<Node> next = {}) {
    return std::make_shared<Node>(std::move(next));
}
#endif

#ifndef SIRKIT_MAKE_CHAIN_DEFINED
#define SIRKIT_MAKE_CHAIN_DEFINED
inline std::shared_ptr<Node> make_chain(std::size_t count, std::shared_ptr<Node> tail = {}) {
    while (count != 0) {
        tail = make_node(std::move(tail));
        --count;
    }
    return tail;
}
#endif

#ifndef SKIP_RUNTIME_CHECKS
#ifndef SIRKIT_INTERNAL_CHAIN_CYCLE_CHECK_DEFINED
#define SIRKIT_INTERNAL_CHAIN_CYCLE_CHECK_DEFINED
namespace reinsirk::detail {
// 通常走査で進んだ先を受け取り、循環を検出する。
struct ChainCycleCheck {
    const Node* checkpoint;
    std::size_t power = 1;
    std::size_t distance = 0;

    explicit ChainCycleCheck(const Node* head) noexcept : checkpoint(head) {}

    void advance(const Node* current) {
        if (current == nullptr) return;
        ++distance;
        if (current == checkpoint) throw std::invalid_argument("chain contains a cycle");
        if (distance == power) {
            checkpoint = current;
            distance = 0;
            constexpr auto max = std::numeric_limits<std::size_t>::max();
            power = power > max / 2 ? max : power * 2;
        }
    }
};
}  // namespace reinsirk::detail
#endif
#endif

#ifndef SIRKIT_CHAIN_SIZE_DEFINED
#define SIRKIT_CHAIN_SIZE_DEFINED
inline std::size_t chain_size(const std::shared_ptr<Node>& head) {
    std::size_t size = 0;
    const Node* current = head.get();
#ifdef SKIP_RUNTIME_CHECKS
    constexpr auto max = std::numeric_limits<std::size_t>::max();
    while (current != nullptr && size <= max - 2) {
        current = current->n.get();
        if (current == nullptr) return size + 1;
        current = current->n.get();
        size += 2;
    }
#endif
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::ChainCycleCheck cycle_check(current);
#endif
    while (current != nullptr) {
        if (size == std::numeric_limits<std::size_t>::max()) {
            throw std::length_error("chain size is not representable");
        }
        ++size;
        current = current->n.get();
#ifndef SKIP_RUNTIME_CHECKS
        cycle_check.advance(current);
#endif
    }
    return size;
}
#endif

#ifndef SIRKIT_CHAIN_TAIL_DEFINED
#define SIRKIT_CHAIN_TAIL_DEFINED
inline std::shared_ptr<Node> chain_tail(const std::shared_ptr<Node>& head) {
    const std::shared_ptr<Node>* link = &head;
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::ChainCycleCheck cycle_check(head.get());
#endif
    while (*link && (*link)->n) {
        link = &(*link)->n;
#ifndef SKIP_RUNTIME_CHECKS
        cycle_check.advance(link->get());
#endif
    }
    return *link;
}
#endif

#ifndef SIRKIT_CHAIN_NTH_DEFINED
#define SIRKIT_CHAIN_NTH_DEFINED
inline std::shared_ptr<Node> chain_nth(const std::shared_ptr<Node>& head, std::size_t index) {
    const std::shared_ptr<Node>* link = &head;
#if defined(__GNUC__) && !defined(__clang__)
    if (!head) return head;
    while (*link && index >= 2) {
        link = &(*link)->n;
        --index;
        if (!*link) return *link;
        link = &(*link)->n;
        --index;
    }
#endif
    while (*link && index != 0) {
        link = &(*link)->n;
        --index;
    }
    return *link;
}
#endif

#ifndef SIRKIT_PUSH_FRONT_NODE_DEFINED
#define SIRKIT_PUSH_FRONT_NODE_DEFINED
inline void push_front_node(std::shared_ptr<Node>& head) {
    auto next = make_node();
    next->n = std::move(head);
    head = std::move(next);
}
#endif

#ifndef SIRKIT_POP_FRONT_NODE_DEFINED
#define SIRKIT_POP_FRONT_NODE_DEFINED
inline bool pop_front_node(std::shared_ptr<Node>& head) noexcept {
    if (!head) return false;
    auto next = head->n;
    head = std::move(next);
    return true;
}
#endif

#ifndef SIRKIT_REVERSE_CHAIN_DEFINED
#define SIRKIT_REVERSE_CHAIN_DEFINED
inline std::shared_ptr<Node> reverse_chain(std::shared_ptr<Node> head) noexcept {
    std::shared_ptr<Node> previous;
    while (head) {
        auto next = std::move(head->n);
        head->n = std::move(previous);
        previous = std::move(head);
        head = std::move(next);
    }
    return previous;
}
#endif

#ifndef SIRKIT_CLONE_CHAIN_DEFINED
#define SIRKIT_CLONE_CHAIN_DEFINED
inline std::shared_ptr<Node> clone_chain(const std::shared_ptr<Node>& head) {
    std::shared_ptr<Node> result;
    auto* link = &result;
    const Node* current = head.get();
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::ChainCycleCheck cycle_check(current);
#endif
    while (current != nullptr) {
        *link = make_node();
        link = &(*link)->n;
        current = current->n.get();
#ifndef SKIP_RUNTIME_CHECKS
        cycle_check.advance(current);
#endif
    }
    return result;
}
#endif

#ifndef SIRKIT_CYCLE_ENTRY_DEFINED
#define SIRKIT_CYCLE_ENTRY_DEFINED
inline Node* cycle_entry(const std::shared_ptr<Node>& head) noexcept {
    Node* slow = head.get();
    Node* fast = head.get();
    do {
        if (fast == nullptr || !fast->n) return nullptr;
        slow = slow->n.get();
        fast = fast->n->n.get();
    } while (slow != fast);
    slow = head.get();
    while (slow != fast) {
        slow = slow->n.get();
        fast = fast->n.get();
    }
    return slow;
}
#endif

#ifndef SIRKIT_HAS_CYCLE_DEFINED
#define SIRKIT_HAS_CYCLE_DEFINED
inline bool has_cycle(const std::shared_ptr<Node>& head) noexcept {
    Node* slow = head.get();
    Node* fast = head.get();
    while (fast != nullptr && fast->n) {
        slow = slow->n.get();
        fast = fast->n->n.get();
        if (slow == fast) return true;
    }
    return false;
}
#endif

#ifndef SIRKIT_BREAK_CYCLE_DEFINED
#define SIRKIT_BREAK_CYCLE_DEFINED
inline bool break_cycle(const std::shared_ptr<Node>& head) noexcept {
    Node* slow = head.get();
    Node* fast = slow;
    Node* last = nullptr;
    do {
        if (fast == nullptr || !fast->n) return false;
        slow = slow->n.get();
        last = fast->n.get();
        fast = last->n.get();
    } while (slow != fast);
    slow = head.get();
    while (slow != last->n.get()) {
        slow = slow->n.get();
        last = last->n.get();
    }
    auto keep_alive = head;
    last->n.reset();
    return true;
}
#endif

#ifndef SIRKIT_CLEAR_CHAIN_DEFINED
#define SIRKIT_CLEAR_CHAIN_DEFINED
inline void clear_chain(std::shared_ptr<Node>& head) noexcept {
    head.reset();
}
#endif


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
    std::array<unsigned char, buffer_size> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_.size(), file_);
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
        while (c >= '0' && c <= '9') {
#ifndef SKIP_RUNTIME_CHECKS
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
#endif
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
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
        do {
            result.push_back(static_cast<char>(c));
            ++position_;
            c = peek();
        } while (c != EOF && c > ' ');
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
            char digits[std::numeric_limits<std::uint64_t>::digits10 + 3];
            const auto end = std::to_chars(digits, digits + sizeof(digits), value).ptr;
            write(std::string_view(digits, static_cast<std::size_t>(end - digits)));
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

#include <memory>
#include <vector>

int main() {
    int n{}, start{};
    if (!(verify_in >> n >> start)) return 0;
    std::vector<std::shared_ptr<Node>> nodes(n);
    for (auto& node : nodes) node = make_node();
    for (int i = 0; i < n; ++i) {
        int next{};
        verify_in >> next;
        nodes[i]->n = nodes[next - 1];
    }
    auto head = nodes[start - 1];
    break_cycle(head);
    verify_out << chain_size(head) << '\n';

    // 始点から到達しない成分の循環も切ってから解放する。
    for (auto& node : nodes) node->n.reset();
}
