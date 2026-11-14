#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/practice2/tasks/practice2_d

// Library: Graph/Max_Flow.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Max_Flow.cpp
// 参考（最大流の理論・ACLのAPI）: https://atcoder.github.io/ac-library/production/document_ja/maxflow.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/max_flow.md
//
// API一覧
// mf_graph<Cap=std::int64_t>()/mf_graph(n): 空/n頂点の有向グラフ。Capはbool以外の整数型。O(1)/O(n)。
// ACLと同名・同じ呼び出し形式。自己辺・多重辺・逆向きの元辺を追加できる。
// add_edge(from,to,cap): 非負容量の元辺を追加し、追加順のint番号を返す。償却O(1)。
// edgeは{int from,to;Cap cap,flow;}。get_edge(int id)/edges(): 元辺のコピー/追加順の一覧。O(1)/O(m)。
// flow(s,t)/flow(s,t,flow_limit): 型の最大値/非負の指定上限まで追加で流し、追加量を返す。O(n²m)、作業領域O(n)。
// 同じ始点・終点で継続可能。総量がCap上限を越す場合は分けて呼び、累積値を十分広い型で保持する。
// min_cut(s): 残余グラフでsから到達可能な頂点のbool配列。O(n+m)時間・O(n)領域。最後まで流した後は最小カット。
// get_edge/edges/min_cutはACL形式の非const版に加えて、constオブジェクトからも同名で呼べる。
// change_edge(int id,new_cap,new_flow): 指定辺を0<=new_flow<=new_capへ変更。O(1)。流量保存は利用側で保証する。
// 追加機能: size()/edge_count()は頂点数int/元辺数size_tをO(1)で返す。reserve_edges(m)は位置表を予約。
// コピー構築/代入: 残余を独立所有しO(n+m)。移動構築はO(1)、移動元は空。代入は自身への参照を返す。
// reset(n=0): O(n+旧領域の破棄)で空のn頂点へ戻す。swap(other)はO(1)。代入にも旧領域の破棄費用が加わる。
// ACLのs!=tという制約外では、s==tを追加仕様として0にする。flow_limit==0も0。
// 反復Dinic法で深い経路も再帰しない。入力検査はSKIP_RUNTIME_CHECKSで除くが、内部添字上限は維持。
// get/changeのidは[0,edge_count())。辺数はINT_MAX以下。継続flowで異なる端点を使う場合も流量保存が前提。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAX_FLOW_DEFINED
#define SIRKIT_MAX_FLOW_DEFINED

template<class Cap = std::int64_t>
class mf_graph {
    static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool>, "capacity must be a non-bool integer");
public:
    struct edge { int from; int to; Cap cap; Cap flow; };
private:
    struct Arc { int to; int reverse; Cap residual; };
    struct Frame { int vertex; Cap limit; Cap sent; };
    std::vector<std::vector<Arc>> graph_;
    std::vector<std::pair<int, int>> positions_;
    std::vector<int> level_, next_, queue_;
    std::vector<Frame> frames_;
    static std::size_t checked_size(int n) {
        if (n < 0) throw std::length_error("mf_graph: negative vertex count");
        return static_cast<std::size_t>(n);
    }
#ifndef SKIP_RUNTIME_CHECKS
    void vertex(int v) const {
        if (v < 0 || v >= size()) throw std::out_of_range("mf_graph: vertex out of range");
    }
    void edge_id(int id) const {
        if (id < 0 || static_cast<std::size_t>(id) >= positions_.size()) throw std::out_of_range("mf_graph: edge out of range");
    }
    static void nonnegative(Cap value) {
        if constexpr (std::is_signed_v<Cap>) if (value < 0) throw std::invalid_argument("mf_graph: negative capacity");
    }
#endif
    bool levels(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, end = 0; queue_[end++] = source; level_[source] = 0;
        while (front != end) {
            const int v = queue_[front++];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && level_[edge.to] == -1) {
                level_[edge.to] = level_[v] + 1;
                // sinkの直前層までは発見済みなので、最短路の辺はDFSで列挙できる。
                if (edge.to == sink) return true;
                queue_[end++] = edge.to;
            }
        }
        return level_[sink] != -1;
    }
    Cap blocking_flow(int source, int sink, Cap limit) {
        frames_.clear(); frames_.push_back({source, limit, 0});
        while (!frames_.empty()) {
            auto& frame = frames_.back(); const int v = frame.vertex;
            if (v != sink && frame.sent != frame.limit) {
                auto& cursor = next_[v];
                while (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    if (edge.residual != 0 && level_[edge.to] == level_[v] + 1) break;
                    ++cursor;
                }
                if (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    frames_.push_back({edge.to, std::min(static_cast<Cap>(frame.limit - frame.sent), edge.residual), 0});
                    continue;
                }
                level_[v] = -1;
            }
            const Cap sent = v == sink ? frame.limit : frame.sent;
            frames_.pop_back();
            if (frames_.empty()) return sent;
            auto& parent = frames_.back(); auto& edge = graph_[parent.vertex][next_[parent.vertex]];
            edge.residual -= sent; graph_[edge.to][edge.reverse].residual += sent;
            parent.sent += sent;
            if (edge.residual == 0 || level_[edge.to] == -1) ++next_[parent.vertex];
        }
        return 0;
    }
public:
    mf_graph() = default;
    explicit mf_graph(int n) : graph_(checked_size(n)), level_(graph_.size()), next_(graph_.size()), queue_(graph_.size()) {}
    mf_graph(const mf_graph&) = default;
    mf_graph(mf_graph&& other) noexcept { swap(other); }
    mf_graph& operator=(mf_graph other) noexcept { swap(other); return *this; }
    void swap(mf_graph& other) noexcept {
        graph_.swap(other.graph_); positions_.swap(other.positions_); level_.swap(other.level_);
        next_.swap(other.next_); queue_.swap(other.queue_); frames_.swap(other.frames_);
    }
    void reset(int n = 0) { mf_graph replacement(n); swap(replacement); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    std::size_t edge_count() const noexcept { return positions_.size(); }
    void reserve_edges(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("mf_graph: too many edges");
        positions_.reserve(count);
    }
    int add_edge(int from, int to, Cap capacity) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(from); vertex(to); nonnegative(capacity);
#endif
        constexpr auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
        if (positions_.size() >= maximum) throw std::length_error("mf_graph: too many edges");
        if (graph_[from].size() >= maximum || graph_[to].size() >= maximum ||
            (from == to && graph_[from].size() >= maximum - 1)) throw std::length_error("mf_graph: too many incident edges");
        const int forward = static_cast<int>(graph_[from].size());
        const int backward = static_cast<int>(graph_[to].size()) + static_cast<int>(from == to);
        graph_[from].push_back({to, backward, capacity});
        try { graph_[to].push_back({from, forward, 0}); }
        catch (...) { graph_[from].pop_back(); throw; }
        try { positions_.emplace_back(from, forward); }
        catch (...) { graph_[to].pop_back(); graph_[from].pop_back(); throw; }
        return static_cast<int>(positions_.size()) - 1;
    }
    edge get_edge(int id) { return std::as_const(*this).get_edge(id); }
    edge get_edge(int id) const {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id);
#endif
        const auto [from, position] = positions_[id]; const auto& edge = graph_[from][position];
        const Cap sent = graph_[edge.to][edge.reverse].residual;
        return {from, edge.to, static_cast<Cap>(edge.residual + sent), sent};
    }
    std::vector<edge> edges() { return std::as_const(*this).edges(); }
    std::vector<edge> edges() const {
        std::vector<edge> result; result.reserve(positions_.size());
        for (int id = 0; id < static_cast<int>(positions_.size()); ++id) result.push_back(get_edge(id));
        return result;
    }
    void change_edge(int id, Cap capacity, Cap sent) {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id); nonnegative(capacity); nonnegative(sent);
        if (sent > capacity) throw std::invalid_argument("mf_graph: flow exceeds capacity");
#endif
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.residual = capacity - sent; graph_[edge.to][edge.reverse].residual = sent;
    }
    Cap flow(int source, int sink) { return flow(source, sink, std::numeric_limits<Cap>::max()); }
    Cap flow(int source, int sink, Cap limit) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source); vertex(sink); nonnegative(limit);
#endif
        if (source == sink || limit == 0) return 0;
        frames_.reserve(graph_.size());
        Cap result = 0;
        while (result != limit && levels(source, sink)) {
            std::fill(next_.begin(), next_.end(), 0);
            result += blocking_flow(source, sink, limit - result);
        }
        return result;
    }
    std::vector<bool> min_cut(int source) { return std::as_const(*this).min_cut(source); }
    std::vector<bool> min_cut(int source) const {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source);
#endif
        std::vector<bool> reachable(graph_.size()); std::vector<int> queue;
        queue.reserve(graph_.size()); queue.push_back(source); reachable[source] = true;
        for (std::size_t front = 0; front < queue.size(); ++front) {
            const int v = queue[front];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && !reachable[edge.to]) {
                reachable[edge.to] = true; queue.push_back(edge.to);
            }
        }
        return reachable;
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

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
int main(){
 int height{},width{};verify_in>>height>>width;std::vector<std::string> board(height);for(auto& row:board)verify_in>>row;
 const int source=height*width,sink=source+1;mf_graph<int> network(sink+1);
 const int di[4]={1,0,-1,0},dj[4]={0,1,0,-1};
 for(int i=0;i<height;++i)for(int j=0;j<width;++j){
  if(board[i][j]!='.')continue;const int v=i*width+j;
  if((i+j)&1){network.add_edge(v,sink,1);continue;}
  network.add_edge(source,v,1);
  for(int direction=0;direction<4;++direction){int ni=i+di[direction],nj=j+dj[direction];if(ni>=0&&ni<height&&nj>=0&&nj<width&&board[ni][nj]=='.')network.add_edge(v,ni*width+nj,1);}
 }
 verify_out<<network.flow(source,sink)<<'\n';
 for(const auto edge:network.edges()){
  if(edge.from==source||edge.to==sink||edge.flow==0)continue;
  int a=edge.from,b=edge.to;if(a>b)std::swap(a,b);const int ai=a/width,aj=a%width,bi=b/width,bj=b%width;
  if(ai==bi){board[ai][aj]='>';board[bi][bj]='<';}else{board[ai][aj]='v';board[bi][bj]='^';}
 }
 for(const auto& row:board)verify_out<<row<<'\n';
}
