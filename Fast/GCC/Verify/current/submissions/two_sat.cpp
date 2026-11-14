#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3", "unroll-loops")
#endif
// Verify: https://judge.yosupo.jp/problem/two_sat

// Library: Graph/Two_SAT.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Two_SAT.cpp
// 参考（2-SATの問題分類）: https://nyaannyaan.github.io/library/math/two-sat.hpp
// 参考（API仕様）: https://atcoder.github.io/ac-library/production/document_ja/twosat.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/two_sat.md
//
// API一覧
// two_sat() / two_sat(n): 空／n個のbool変数。変数番号[0,n)。O(1)準備。n<=INT_MAX/2。
// add_clause(i,a,j,b): (x_i==a)または(x_j==b)という節を追加。償却O(1)。解を無効化する。
// imply(i,a,j,b): (x_i==a)なら(x_j==b)。force(i,a): x_i==aを強制。償却O(1)。
// add_xor(i,j,different=true): 2変数の値が異なる/等しいという制約。償却O(1)。
// reserve_clauses(m): m節分の領域を確保。size()/clause_count(): 変数数/内部2リテラル節数。O(1)。
// satisfiable(): 充足可能ならtrue。反復SCCでO(n+m)時間・追加領域。複数回実行できる。
// answer(): 最後の判定の解をvector<bool>の値で返す。O(n)時間・領域。未実行/UNSATでも長さn、内容は未規定。
// value(i): 最後のsatisfiable()成功時の変数iのbool値。O(1)。制約追加後は再判定する。
// reset(n=0): 制約を捨てn変数へ。swap(other): O(1)。コピーは独立所有、ムーブ元は空の未解決状態。
// 入力検査はSKIP_RUNTIME_CHECKSで除く。answer()の返り値は独立したコピー。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TWO_SAT_SCC_DETAIL_DEFINED
#define SIRKIT_TWO_SAT_SCC_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
// nは偶数、相補リテラルはvと(v^1)。Indexは総辺数を表現できる符号なし型。
// 矛盾を検出したら{-1,{}}、成功時はトポロジカル順の成分番号を返す。
template<class Index>
inline std::pair<int, std::vector<int>> two_sat_component_ids_index(
        int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<Index> begin(static_cast<std::size_t>(n) + 1);
    for (auto [u, v] : edges) { (void)v; ++begin[static_cast<std::size_t>(u) + 1]; }
    for (int v = 0; v < n; ++v) begin[v + 1] += begin[v];
    std::vector<Index> next(begin.begin() + 1, begin.end());
    // CSRの全要素へ書き込んでから参照するため、先行初期化は不要。
    auto destination = std::make_unique_for_overwrite<int[]>(edges.size());
    // 辺順を保って逆向きに埋め、走査位置の再初期化を省く。
    for (auto it = edges.rbegin(); it != edges.rend(); ++it)
        destination[--next[it->first]] = it->second;
    // idsは中断中のlow-linkの退避先を兼ね、確定時に成分番号を書く。
    std::vector<int> entered(n, -1), ids(n), dfs, pending;
    dfs.reserve(n); pending.reserve(n);
    int clock = 0, count = 0;
    for (int root = 0; root < n; ++root) {
        if (entered[root] != -1) continue;
        int v = root;
        Index cursor = next[v], end = begin[v + 1];
        // 出辺0の頂点は即確定し、DFS番号の割当ても省く。
        if (cursor == end) {
            entered[v] = n; ids[v] = count++;
            continue;
        }
        int low = entered[v] = clock++;
        pending.push_back(v);
        for (;;) {
            if (cursor != end) {
                const int u = destination[cursor++];
                if (entered[u] == -1) {
                    const Index child_cursor = next[u], child_end = begin[u + 1];
                    if (child_cursor == child_end) {
                        entered[u] = n; ids[u] = count++;
                        continue;
                    }
                    // 子へ降りるときだけ、親の走査位置とlow-linkを退避する。
                    next[v] = cursor;
                    ids[v] = low;
                    dfs.push_back(v);
                    v = u;
                    low = entered[v] = clock++;
                    cursor = child_cursor; end = child_end;
                    pending.push_back(v);
                } else low = std::min(low, entered[u]);
                continue;
            }
            // 成分番号への上書き後も親へ渡せるよう、low-linkを退避する。
            const int child_low = low;
            if (child_low == entered[v]) {
                // 1頂点の成分には相補的な2頂点が同居しない。
                if (pending.back() == v) {
                    pending.pop_back();
                    entered[v] = n; ids[v] = count;
                } else {
                    for (;;) {
                        const int u = pending.back(); pending.pop_back();
                        entered[u] = n; ids[u] = count;
                        // 確定済みかつ今回の成分番号なら、同じ成分内の矛盾。
                        if (entered[u ^ 1] == n && ids[u ^ 1] == count)
                            return {-1, {}};
                        if (u == v) break;
                    }
                }
                ++count;
            }
            if (dfs.empty()) break;
            v = dfs.back(); dfs.pop_back();
            low = std::min(ids[v], child_low);
            cursor = next[v]; end = begin[v + 1];
        }
    }
    for (auto& id : ids) id = count - 1 - id;
    return {count, std::move(ids)};
}
inline std::pair<int, std::vector<int>> two_sat_component_ids(
        int n, const std::vector<std::pair<int, int>>& edges) {
    // 無辺では各頂点が1成分。従来と同じ逆番号で返す。
    if (edges.empty()) {
        std::vector<int> ids(n);
        for (int v = 0; v < n; ++v) ids[v] = n - 1 - v;
        return {n, std::move(ids)};
    }
    // 辺数が収まる場合だけCSRの添字を32bitにする。
    if (edges.size() <= std::numeric_limits<std::uint32_t>::max())
        return two_sat_component_ids_index<std::uint32_t>(n, edges);
    return two_sat_component_ids_index<std::size_t>(n, edges);
}
}
#endif

#ifndef SIRKIT_TWO_SAT_DEFINED
#define SIRKIT_TWO_SAT_DEFINED
class two_sat {
    int n_ = 0;
    std::vector<std::pair<int, int>> implications_;
    std::vector<unsigned char> answer_;
    void room(std::size_t extra) {
        const auto maximum = implications_.max_size(), current = implications_.size();
        if (extra > maximum - current) throw std::length_error("two_sat: too many clauses");
        if (implications_.capacity() - current >= extra) return;
        const auto capacity = implications_.capacity();
        const auto grown = capacity <= maximum / 2 ? capacity * 2 : maximum;
        implications_.reserve(std::max(current + extra, std::max(std::min(std::size_t{4}, maximum), grown)));
    }
    void invalidate() noexcept {
    }
public:
    two_sat() = default;
    explicit two_sat(int n) : n_(n) {
        if (n < 0 || n > std::numeric_limits<int>::max() / 2) throw std::length_error("two_sat: invalid variable count");
    }
    two_sat(const two_sat&) = default;
    two_sat(two_sat&& other) noexcept { swap(other); }
    two_sat& operator=(two_sat other) noexcept { swap(other); return *this; }
    void swap(two_sat& other) noexcept {
        std::swap(n_, other.n_); implications_.swap(other.implications_); answer_.swap(other.answer_);
    }
    void reset(int n = 0) { two_sat replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t clause_count() const noexcept { return implications_.size() / 2; }
    void reserve_clauses(std::size_t count) {
        if (count > implications_.max_size() / 2) throw std::length_error("two_sat: too many clauses");
        implications_.reserve(count * 2);
    }
    void add_clause(int i, bool a, int j, bool b) {
        room(2);
        implications_.emplace_back(2 * i + !a, 2 * j + b);
        implications_.emplace_back(2 * j + !b, 2 * i + a);
        invalidate();
    }
    void imply(int i, bool a, int j, bool b) { add_clause(i, !a, j, b); }
    void force(int i, bool a) { add_clause(i, a, i, a); }
    void add_xor(int i, int j, bool different = true) {
        room(4);
        add_clause(i, true, j, different);
        add_clause(i, false, j, !different);
    }
    bool satisfiable() {
        if (implications_.empty()) {
            // 制約なしで保持済みの解は全false。再判定ではそのまま使える。
            if (answer_.size() != static_cast<std::size_t>(n_)) {
                std::vector<unsigned char> result(n_, 0);
                answer_.swap(result);
            }
            return true;
        }
        const auto components = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::two_sat_component_ids(2 * n_, implications_);
        if (components.first < 0) {
            answer_.clear();
            return false;
        }
        const auto& ids = components.second;
        std::vector<unsigned char> result(n_);
        for (int i = 0; i < n_; ++i) result[i] = ids[2 * i] < ids[2 * i + 1];
        answer_.swap(result);
        return true;
    }
    std::vector<bool> answer() {
        if (answer_.size() == static_cast<std::size_t>(n_))
            return std::vector<bool>(answer_.begin(), answer_.end());
        return std::vector<bool>(static_cast<std::size_t>(n_));
    }
    bool value(int i) const {
        return answer_[i] != 0;
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

#include <string>
int main(){
 std::string prefix,format;int n{},m{};verify_in>>prefix>>format>>n>>m;two_sat constraints(n);constraints.reserve_clauses(m);
 for(int i=0;i<m;++i){int a{},b{},zero{};verify_in>>a>>b>>zero;constraints.add_clause((a>0?a:-a)-1,a>0,(b>0?b:-b)-1,b>0);}
 if(!constraints.satisfiable()){verify_out<<"s UNSATISFIABLE\n";return 0;}
 verify_out<<"s SATISFIABLE\nv";for(int i=0;i<n;++i)verify_out<<' '<<(constraints.value(i)?i+1:-i-1);verify_out<<" 0\n";
}
