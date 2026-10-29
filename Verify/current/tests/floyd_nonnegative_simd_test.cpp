// SPDX-License-Identifier: CC0-1.0
// 非負Dijkstraとの独立照合。SIMDの閾値・疎なactive行・大きな整数・FP状態を確認する。
#ifndef SIRKIT_FLOYD_TEST_SOURCE
#define SIRKIT_FLOYD_TEST_SOURCE "../../../Graph/Floyd-Warshall.cpp"
#endif
#include SIRKIT_FLOYD_TEST_SOURCE

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#if !defined(SIRKIT_FLOYD_TEST_DISABLE_MXCSR) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || (defined(__i386__) && defined(__SSE2__)))
#include <xmmintrin.h>
#define SIRKIT_FLOYD_TEST_MXCSR 1
#endif

namespace {
using Matrix = std::vector<std::vector<std::int64_t>>;
struct Case {
    std::string name;
    Matrix input;
    Matrix expected;
    bool check_fp = false;
};

Matrix reference(const Matrix& input) {
    const std::size_t n = input.size();
    Matrix expected(n, std::vector<std::int64_t>(n, INF));
    using Entry = std::pair<std::int64_t, std::size_t>;
    for (std::size_t source = 0; source < n; ++source) {
        auto& row = expected[source];
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;
        row[source] = 0;
        queue.emplace(0, source);
        while (!queue.empty()) {
            const auto [distance, vertex] = queue.top();
            queue.pop();
            if (distance != row[vertex]) continue;
            for (std::size_t to = 0; to < n; ++to) {
                const auto weight = input[vertex][to];
                if (weight == INF) continue;
                // 各単純路がINF未満になる入力だけを生成する。
                // 非最短の閉路によるINF以上の候補も加算前に除く。
                if (weight >= INF - distance) continue;
                const auto candidate = distance + weight;
                if (candidate < row[to]) {
                    row[to] = candidate;
                    queue.emplace(candidate, to);
                }
            }
        }
    }
    return expected;
}

void compare(const Case& test, const Matrix& actual, bool negative) {
    if (negative) throw std::runtime_error(test.name + ": unexpected negative cycle");
    if (actual.size() != test.expected.size())
        throw std::runtime_error(test.name + ": changed matrix size");
    for (std::size_t from = 0; from < actual.size(); ++from) {
        if (actual[from].size() != test.expected[from].size())
            throw std::runtime_error(test.name + ": changed row size");
        for (std::size_t to = 0; to < actual.size(); ++to)
            if (actual[from][to] != test.expected[from][to])
                throw std::runtime_error(test.name + ": distance mismatch at " +
                    std::to_string(from) + ',' + std::to_string(to) + ", actual=" +
                    std::to_string(actual[from][to]) + ", expected=" +
                    std::to_string(test.expected[from][to]));
    }
}

Matrix make_case(int n, int kind, std::mt19937_64& random) {
    Matrix input(n, std::vector<std::int64_t>(n, INF));
    if (n == 0) return input;
    for (int i = 0; i < n; ++i) input[i][i] = 0;
    const auto bound = (INF - 1) / n;
    const auto small = [&] { return static_cast<std::int64_t>(random() % 4096); };
    const int hub = n / 2;
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), random);
    std::vector<std::int64_t> boundary{0,1,2};
    for (int exponent : {20,32,52,53,54,55,56}) {
        const auto power = std::int64_t{1} << exponent;
        for (std::int64_t delta : {-1,0,1})
            if (power + delta <= bound) boundary.push_back(power + delta);
    }
    if (bound > 0) boundary.push_back(bound - 1);
    boundary.push_back(bound);
    const auto edge = [&](int from, int to, std::int64_t weight) {
        input[from][to] = std::min(input[from][to], weight);
    };
    if (kind == 0) {
        for (int from = 0; from < n; ++from)
            for (int to = 0; to < n; ++to)
                if (from != to) edge(from, to, small());
    } else if (kind == 1) {
        for (int from = 0; from < n; ++from)
            for (int to = 0; to < n; ++to)
                if (from != to && random() % 17 == 0) edge(from, to, small());
    } else if (kind == 2) {
        // 出辺を持つ頂点は1個だけ。他はsinkまたは孤立点。
        for (int to = 0; to < n; ++to)
            if (to != hub) edge(hub, to, boundary[static_cast<std::size_t>(to) % boundary.size()]);
    } else if (kind == 3) {
        for (int from = 0; from < n; ++from)
            if (from != hub) edge(from, hub, small());
    } else if (kind == 4) {
        // 成分内の頂点番号を非連続にし、末尾の1点を孤立させる。
        for (int from = 0; from + 1 < n; ++from)
            for (int to = 0; to + 1 < n; ++to)
                if (from != to && from % 5 == to % 5) edge(from, to, small());
    } else if (kind == 5) {
        // 非連続のactive頂点から、多数のsinkへ向かう。
        for (int from = 0; from < n; ++from) if (from % 5 == 1 || from % 5 == 3)
            for (int to = 0; to < n; ++to)
                if (from != to && random() % 3 != 0)
                    edge(from, to, boundary[static_cast<std::size_t>(to + from) % boundary.size()]);
    } else if (kind == 6) {
        for (int i = 0; i + 1 < n; ++i)
            edge(order[i], order[i + 1], i % 3 == 0 ? 0 : bound);
    } else if (kind == 7) {
        for (int i = 0; i < n; ++i) edge(order[i], order[(i + 1) % n], 0);
    } else if (kind == 8) {
        for (int from = 0; from < n; ++from)
            for (int to = from + 1; to < n; ++to)
                if (random() % 5 == 0) edge(order[from], order[to], bound - small() % (bound + 1));
    } else if (kind == 9) {
        // 2^52等の直前・直後を整数のまま扱い、長い和でも丸めを許さない。
        for (int i = 0; i + 1 < n; ++i)
            edge(order[i], order[i + 1], boundary[static_cast<std::size_t>(i) % boundary.size()]);
        for (int i = 0; i + 2 < n; ++i) {
            const auto left = input[order[i]][order[i + 1]];
            const auto right = input[order[i + 1]][order[i + 2]];
            if (left <= bound - right) edge(order[i], order[i + 2], left + right);
        }
    } else if (kind == 10 && n > 1) {
        edge(hub, n - 1, bound);
        if (hub != 0) edge(0, hub, 0);
    }
    return input;
}

std::vector<Case> cases() {
    std::vector<Case> tests;
    std::mt19937_64 random(0x464c4f594453494dULL);
    for (int n : {0,1,2,7,8,15,16,31,32,33,63,64,65,100,127,128,129,256,257}) {
        std::vector<int> kinds;
        if (n <= 33) kinds = {0,1,2,4,6,7};
        else if (n == 256) kinds = {0,2,5,6,9};
        else if (n == 257) kinds = {0,4,10};
        else kinds = {0,1,2,3,4,5,6,7,8,9,10};
        for (int kind : kinds) {
            auto input = make_case(n, kind, random);
            const bool fp = (n == 64 && kind == 0) || (n == 129 && kind == 5);
            tests.push_back({"n=" + std::to_string(n) + ",kind=" + std::to_string(kind),
                             input, reference(input), fp});
        }
    }
    // 小重みの迂回路で境界値を消さない。各直結辺は2辺以上の経路より短い。
    // 全finiteを維持し、distance-onlyのdense専用kernelへ入る入力にする。
    for (int n : {63,64,65,100,127,128,129}) {
        const auto bound = (INF - 1) / n;
        const std::array<std::int64_t,5> requested{
            std::int64_t{1} << 20, std::int64_t{1} << 32,
            std::int64_t{1} << 52, std::int64_t{1} << 53, bound - 1};
        for (std::size_t index = 0; index < requested.size(); ++index) {
            const auto base = std::min(requested[index], bound - 1);
            Matrix input(n, std::vector<std::int64_t>(n));
            for (int from = 0; from < n; ++from)
                for (int to = 0; to < n; ++to)
                    input[from][to] = from == to ? 0 : base + (from + 2 * to) % 3 - 1;
            const bool fp = (n == 64 || n == 65 || n == 127 || n == 128) &&
                            (index == 2 || index == 4);
            tests.push_back({"dense boundary n=" + std::to_string(n) +
                             ",base=" + std::to_string(base), input, reference(input), fp});
        }
    }
    // 変換中のINF検出で戻る位置。先頭lane・次のvector・部分行・末尾行・tailを分ける。
    for (int n : {64,65,127,129}) {
        const auto dense = make_case(n, 0, random);
        const std::array<std::pair<int,int>,6> positions{{
            {0,1}, {0,4}, {1,0}, {1,n - 1}, {n - 1,n - 2}, {n - 2,n - 1}}};
        for (const auto& [from,to] : positions) {
            auto input = dense;
            input[from][to] = INF;
            const bool fp = (n == 65 && from == 1 && to == n - 1) ||
                            (n == 129 && from == n - 2 && to == n - 1);
            tests.push_back({"rollback n=" + std::to_string(n) + ",INF=" +
                             std::to_string(from) + ',' + std::to_string(to),
                             input, reference(input), fp});
        }
    }
    tests.push_back({"single INF diagonal", {{INF}}, {{0}}, false});
    tests.push_back({"single positive diagonal", {{INF - 1}}, {{0}}, false});
    return tests;
}

#if defined(SIRKIT_FLOYD_TEST_MXCSR)
std::size_t floating_state_cases(const std::vector<Case>& tests) {
    // x86-64/SSE2のMXCSR。例外をmaskし、丸め4通り×FTZ×DAZ×sticky有無。
    const unsigned original = _mm_getcsr();
    struct Restore {
        unsigned value;
        ~Restore() { _mm_setcsr(value); }
    } restore{original};
    constexpr unsigned sticky = 0x3f, rounding = 0x6000, ftz = 0x8000, daz = 0x40;
    const unsigned base = (original | 0x1f80) & ~(sticky | rounding | ftz | daz);
    std::size_t count = 0;
    for (const auto& test : tests) if (test.check_fp)
        for (unsigned round : {0u,0x2000u,0x4000u,0x6000u})
            for (unsigned flush : {0u,ftz})
                for (unsigned denormal : {0u,daz})
                    for (unsigned flags : {0u,sticky}) {
                        auto actual = test.input;
                        const unsigned before = base | round | flush | denormal | flags;
                        _mm_setcsr(before);
                        const bool negative = FloydWarshall(actual);
                        const unsigned after = _mm_getcsr();
                        _mm_setcsr(original);
                        if (after != before)
                            throw std::runtime_error(test.name + ": MXCSR changed, before=" +
                                std::to_string(before) + ", after=" + std::to_string(after));
                        compare(test, actual, negative);
                        ++count;
                    }
    return count;
}
#else
std::size_t floating_state_cases(const std::vector<Case>&) { return 0; }
#endif
}

int main() {
    try {
        const auto tests = cases();
        for (const auto& test : tests) {
            auto actual = test.input;
            compare(test, actual, FloydWarshall(actual));
        }
        const auto fp = floating_state_cases(tests);
        std::cout << tests.size() << " nonnegative Dijkstra graphs and " << fp
                  << " MXCSR configurations passed\n";
    } catch (const std::exception& error) {
        std::cerr << "Floyd nonnegative SIMD test failed: " << error.what() << '\n';
        return 1;
    }
}
