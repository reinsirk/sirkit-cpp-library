// SPDX-License-Identifier: CC0-1.0
// Moのorder()だけを測る。MO_SOURCEに比べる版のMo.cppを指定する。
// 例: g++ -std=c++23 -O2 -DSKIP_RUNTIME_CHECKS -I. -DMO_SOURCE='"Algorithm/Mo.cpp"' Audit/Benchmarks/mo_order.cpp
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#include MO_SOURCE
using namespace sirkit;

static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }

int main() {
    // 長さ5e5の一様な区間。更新版は照会と同数の変更を記録し、時刻は記録順。
    std::mt19937_64 random(11);
    for (int count : {16, 256, 2048, 4096, 16384, 65536, 262144, 500000}) {
        constexpr int kLength = 500000;
        Mo mo(kLength);
        MoWithUpdates timed(kLength);
        for (int i = 0; i < count; ++i) {
            int left = static_cast<int>(random() % (kLength + 1)), right = static_cast<int>(random() % (kLength + 1));
            if (left > right) std::swap(left, right);
            mo.add_query(left, right);
            timed.add_update(static_cast<std::int32_t>(random() % kLength));
            timed.add_query(left, right);
        }
        const int repetitions = std::max(5, 2000000 / count);
        double fastest = 1e9, fastest_timed = 1e9;
        std::size_t sink = 0;
        for (int r = 0; r < repetitions; ++r) {
            double start = now();
            const auto order = mo.order();
            fastest = std::min(fastest, now() - start);
            start = now();
            const auto timed_order = timed.order();
            fastest_timed = std::min(fastest_timed, now() - start);
            sink += order[0] + timed_order[0];
        }
        std::printf("Q=%7d static %9.1fus  updates %9.1fus  (sink %zu)\n", count, fastest * 1e6, fastest_timed * 1e6, sink % 7);
    }
}
