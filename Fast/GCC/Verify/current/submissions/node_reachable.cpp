#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc228/tasks/abc228_b

// Library: DataStructure/Node.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Fast/Source/DataStructure/Node.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// [速度優先版 Node・単方向連結列]
// Node::nと各関数のノード引数/戻り値はNode*。自動所有・参照カウント・自動解放はない。
// make_node(next=nullptr): 共有bump poolから1ノード確保。償却O(1)、deleteしない。
// make_chain(count,tail=nullptr): 4096個以下は共有pool、大きい列はnew Node[count]で連続確保しtailへ接続。O(count+1)。
// 単一スレッド専用。共有poolは4096ノードずつ拡張し、既存アドレスを保つ。個別deleteは禁止、領域はプロセス終了まで保持。
// chain_size/chain_tail/clone_chain/reverse_chainは循環なしが前提。サイズはsize_t以内。
// chain_size(head): ノード数。chain_tail(head): 末尾（空ならnullptr）。ともにO(k)。
// chain_nth(head,index): index番目、末尾を超えればnullptr。O(min(index+1,k))、循環時はO(index+1)。
// push_front_node(head): make_nodeで先頭追加、O(1)。pop_front_node(head): 次へ進み、空ならfalse、O(1)。
// reverse_chain(head): リンクを逆転して新先頭を返す。O(k)、共有参照にも変更が見える。
// clone_chain(head): 独立した同長の連続領域を返す。2回走査でO(k+1)、追加確保は高々1回。
// cycle_entry(head)/has_cycle(head): 循環入口/有無、O(k)。break_cycle(head): 循環の末尾を切り、切ればtrue、O(k)。
// clear_chain(head): head=nullptrとするだけ、O(1)。全APIで例外後の回復・入力検査は省く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NODE_DEFINED
#define SIRKIT_NODE_DEFINED
struct Node {
    Node* n;
    explicit Node(Node* next = nullptr) noexcept : n(next) {}
};
#endif

#ifndef SIRKIT_FAST_NODE_POOL_DEFINED
#define SIRKIT_FAST_NODE_POOL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline Node* node_pool_next = nullptr;
inline Node* node_pool_end = nullptr;
inline Node* allocate_node_chunk(std::size_t count) {
    if (!node_pool_next || static_cast<std::size_t>(node_pool_end - node_pool_next) < count) {
        node_pool_next = new Node[4096];
        node_pool_end = node_pool_next + 4096;
    }
    Node* first = node_pool_next;
    node_pool_next += count;
    return first;
}
}
#endif

#ifndef SIRKIT_MAKE_NODE_DEFINED
#define SIRKIT_MAKE_NODE_DEFINED
inline Node* make_node(Node* next = nullptr) {
    Node* node = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::allocate_node_chunk(1);
    node->n = next;
    return node;
}
#endif

#ifndef SIRKIT_MAKE_CHAIN_DEFINED
#define SIRKIT_MAKE_CHAIN_DEFINED
inline Node* make_chain(std::size_t count, Node* tail = nullptr) {
    if (count == 0) return tail;
    Node* first = count <= 4096 ? /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::allocate_node_chunk(count) : new Node[count];
    for (std::size_t i = 1; i < count; ++i) first[i - 1].n = first + i;
    first[count - 1].n = tail;
    return first;
}
#endif

#ifndef SIRKIT_CHAIN_SIZE_DEFINED
#define SIRKIT_CHAIN_SIZE_DEFINED
inline std::size_t chain_size(const Node* head) noexcept {
    std::size_t count = 0;
    for (; head; head = head->n) ++count;
    return count;
}
#endif

#ifndef SIRKIT_CHAIN_TAIL_DEFINED
#define SIRKIT_CHAIN_TAIL_DEFINED
inline Node* chain_tail(Node* head) noexcept {
    if (head) while (head->n) head = head->n;
    return head;
}
#endif

#ifndef SIRKIT_CHAIN_NTH_DEFINED
#define SIRKIT_CHAIN_NTH_DEFINED
inline Node* chain_nth(Node* head, std::size_t index) noexcept {
    for (; head && index; --index) head = head->n;
    return head;
}
#endif

#ifndef SIRKIT_PUSH_FRONT_NODE_DEFINED
#define SIRKIT_PUSH_FRONT_NODE_DEFINED
inline void push_front_node(Node*& head) { head = make_node(head); }
#endif

#ifndef SIRKIT_POP_FRONT_NODE_DEFINED
#define SIRKIT_POP_FRONT_NODE_DEFINED
inline bool pop_front_node(Node*& head) noexcept {
    if (!head) return false;
    head = head->n;
    return true;
}
#endif

#ifndef SIRKIT_REVERSE_CHAIN_DEFINED
#define SIRKIT_REVERSE_CHAIN_DEFINED
inline Node* reverse_chain(Node* head) noexcept {
    Node* previous = nullptr;
    while (head) {
        Node* next = head->n;
        head->n = previous;
        previous = head;
        head = next;
    }
    return previous;
}
#endif

#ifndef SIRKIT_CLONE_CHAIN_DEFINED
#define SIRKIT_CLONE_CHAIN_DEFINED
inline Node* clone_chain(const Node* head) { return make_chain(chain_size(head)); }
#endif

#ifndef SIRKIT_CYCLE_ENTRY_DEFINED
#define SIRKIT_CYCLE_ENTRY_DEFINED
inline Node* cycle_entry(Node* head) noexcept {
    Node* slow = head;
    Node* fast = head;
    do {
        if (!fast || !fast->n) return nullptr;
        slow = slow->n;
        fast = fast->n->n;
    } while (slow != fast);
    for (slow = head; slow != fast; slow = slow->n, fast = fast->n) {}
    return slow;
}
#endif

#ifndef SIRKIT_HAS_CYCLE_DEFINED
#define SIRKIT_HAS_CYCLE_DEFINED
inline bool has_cycle(Node* head) noexcept {
    Node* slow = head;
    Node* fast = head;
    while (fast && fast->n) {
        slow = slow->n; fast = fast->n->n;
        if (slow == fast) return true;
    }
    return false;
}
#endif

#ifndef SIRKIT_BREAK_CYCLE_DEFINED
#define SIRKIT_BREAK_CYCLE_DEFINED
inline bool break_cycle(Node* head) noexcept {
    Node* entry = cycle_entry(head);
    if (!entry) return false;
    Node* last = entry;
    while (last->n != entry) last = last->n;
    last->n = nullptr;
    return true;
}
#endif

#ifndef SIRKIT_CLEAR_CHAIN_DEFINED
#define SIRKIT_CLEAR_CHAIN_DEFINED
inline void clear_chain(Node*& head) noexcept { head = nullptr; }
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

int main() {
    int n{}, start{};
    if (!(verify_in >> n >> start)) return 0;
    std::vector<Node*> nodes(n);
    for (auto& node : nodes) node = make_node();
    for (int i = 0; i < n; ++i) {
        int next{};
        verify_in >> next;
        nodes[i]->n = nodes[next - 1];
    }
    auto head = nodes[start - 1];
    break_cycle(head);
    verify_out << chain_size(head) << '\n';
    for (auto& node : nodes) node->n = nullptr;
}
