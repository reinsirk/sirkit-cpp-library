#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/longest_increasing_subsequence
// Submission: https://judge.yosupo.jp/submission/414289

// Library: Algorithm/LIS.cpp
// SPDX-License-Identifier: CC0-1.0
// Original Source: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/lis
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lis.md

// API一覧
// [長さ・接頭辞]
// 比較はoperator<の狭義弱順序。浮動NaNは検査時に拒否、±infは許可。
// LIS/Length/EndingLengthsはTのコピー構築・代入、Sequenceはコピー構築が必要。
// LIS<Strict>(v): 各接頭辞のLIS長を返す。trueは狭義、falseは広義。空入力は空。
// LISLength<Strict=true>(v): LIS長だけを返す。入力は変更せず、比較はoperator<を使う。
// 時間O(N log(L+1))、追加領域はLISがO(N+L)、LISLengthがO(L)。整列済みの増加列はO(N)。
//
// [復元]
// LISIndicesは汎用型をコピーせず比較し、コピー不能・非default構築の型も扱う。
// 組込み数値型では、末尾値を内部へ複製する場合がある。
// LISIndices<Strict=true>(v): LISを1つ復元し、元配列の添字を昇順で返す。同長の解の選択は任意。
// LISSequence<Strict=true>(v): LISを1つ値の列としてコピーする。空なら空。いずれもO(N log(L+1))時間・O(N)追加領域。
//
// [各末尾の長さ]
// LISEndingLengths<Strict=true>(v): 各要素を末尾とする最長長。狭義/広義、空なら空、O(N log(L+1))時間・O(N+L)領域。
// https://miscalc.hatenablog.com/entry/2024/07/25/212618

// GCC 15.2/x86-64では対応CPUでAVX2を使用。SIRKIT_DISABLE_SIMDで専用処理・CPU判定を除く。

#include <algorithm>
#include <array>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
#include <bit>
#include <cstring>
#include <immintrin.h>
#endif

#ifndef SKIP_RUNTIME_CHECKS
#ifndef SIRKIT_LIS_INPUT_CHECK_DEFINED
#define SIRKIT_LIS_INPUT_CHECK_DEFINED
namespace reinsirk::detail {
template <class T>
void check_lis_value(const T& value) {
    if constexpr (std::is_floating_point_v<T>)
        if (std::isnan(value)) throw std::invalid_argument("LIS: NaN input");
}
} // namespace reinsirk::detail
#endif
#endif

#ifndef SIRKIT_LIS_LENGTHS_DEFINED
#define SIRKIT_LIS_LENGTHS_DEFINED
namespace reinsirk::detail {
template <bool Strict, bool Prefix, class T>
auto LISLengths(const std::vector<T>& v) {
    std::vector<T> tails;
    tails.reserve(std::min(v.size(), std::size_t{64}));
    std::vector<std::size_t> lengths;
    if constexpr (Prefix) lengths.resize(v.size());
    std::size_t i = 0;
    for (const auto& value : v) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(value);
#endif
        const bool extend = tails.empty() || (Strict ? tails.back() < value : !(value < tails.back()));
        if (extend) tails.push_back(value);
        else {
            if constexpr (Strict) *std::lower_bound(tails.begin(), tails.end(), value) = value;
            else *std::upper_bound(tails.begin(), tails.end(), value) = value;
        }
        if constexpr (Prefix) lengths[i++] = tails.size();
    }
    if constexpr (Prefix) return lengths;
    else return tails.size();
}
} // namespace reinsirk::detail
template <bool Strict, class T>
std::vector<std::size_t> LIS(const std::vector<T>& v) {
    return reinsirk::detail::LISLengths<Strict, true>(v);
}
template <bool Strict = true, class T>
std::size_t LISLength(const std::vector<T>& v) {
    return reinsirk::detail::LISLengths<Strict, false>(v);
}
#endif

#ifndef SIRKIT_LIS_RESTORE_DEFINED
#define SIRKIT_LIS_RESTORE_DEFINED
namespace reinsirk::detail {
template <bool Strict, class Index, class T>
std::vector<std::size_t> LISIndicesImpl(const std::vector<T>& v) {
    if (v.empty()) return {};
    std::vector<Index> tails;
    auto previous = std::make_unique_for_overwrite<Index[]>(v.size());
    tails.reserve(std::min(v.size(), std::size_t{64}));
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::check_lis_value(v[0]);
#endif
    tails.push_back(0);
    for (std::size_t i = 1; i < v.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(v[i]);
#endif
        if (Strict ? v[tails.back()] < v[i] : !(v[i] < v[tails.back()])) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            continue;
        }
        if (Strict ? !(v[tails.front()] < v[i]) : v[i] < v[tails.front()]) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = Strict ? v[tails[mid]] < v[i] : !(v[i] < v[tails[mid]]);
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    std::vector<std::size_t> result(tails.size());
    auto at = tails.back();
    for (auto i = result.size(); i > 1; --i) {
        result[i - 1] = at;
        at = previous[at];
    }
    result[0] = at;
    return result;
}
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
#ifndef SIRKIT_LIS_PHASED_RESTORE_DEFINED
#define SIRKIT_LIS_PHASED_RESTORE_DEFINED
template<class Index>
__attribute__((optimize("align-loops=32")))
std::vector<std::size_t> LISIndicesPhased(const std::vector<std::int64_t>& v) {
    if (v.empty()) return {};
    std::vector<Index> tails;
    auto previous = std::make_unique_for_overwrite<Index[]>(v.size());
    tails.reserve(std::min(v.size(), std::size_t{64}));
    tails.push_back(0);
    std::size_t i = 1;
    for (; i < v.size(); ++i) {
        if (v[tails.back()] < v[i]) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            if (tails.size() == 34) { ++i; break; }
            continue;
        }
        if (!(v[tails.front()] < v[i])) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = v[tails[mid]] < v[i];
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    for (; i < v.size(); ++i) {
        if (v[tails.back()] < v[i]) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            if (tails.size() == 4099) { ++i; break; }
            continue;
        }
        if (!(v[tails.front()] < v[i])) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        // 長さ34..4098ではcountが32..4096なので、最初の1段だけ4分割する。
        const auto quarter = count / 4;
        const auto p0 = lo + quarter, p1 = p0 + quarter, p2 = p1 + quarter;
        const auto value = v[i];
        const bool b0 = v[tails[p0]] < value, b1 = v[tails[p1]] < value, b2 = v[tails[p2]] < value;
        const auto m0 = std::size_t{0} - static_cast<std::size_t>(b0);
        const auto m1 = std::size_t{0} - static_cast<std::size_t>(b1);
        const auto m2 = std::size_t{0} - static_cast<std::size_t>(b2);
        lo += ((quarter + 1) & m0) + (quarter & m1) + (quarter & m2);
        count = quarter - static_cast<std::size_t>(b0) + ((count % 4) & m2);
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = v[tails[mid]] < v[i];
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    for (; i < v.size(); ++i) {
        if (v[tails.back()] < v[i]) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            continue;
        }
        if (!(v[tails.front()] < v[i])) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = v[tails[mid]] < v[i];
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    std::vector<std::size_t> result(tails.size());
    auto at = tails.back();
    for (auto pos = result.size(); pos > 1; --pos) {
        result[pos - 1] = at;
        at = previous[at];
    }
    result[0] = at;
    return result;
}
#endif
#endif
template <bool Strict, class Index, bool Repeated = false, class T>
std::vector<std::size_t> LISIndicesWithValues(const std::vector<T>& v) {
    if (v.empty()) return {};
    std::vector<Index> tails;
    std::vector<T> values;
    auto previous = std::make_unique_for_overwrite<Index[]>(v.size());
    tails.reserve(std::min(v.size(), std::size_t{64}));
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::check_lis_value(v[0]);
#endif
    tails.push_back(0);
    values.reserve(std::min(v.size(), std::size_t{64}));
    values.push_back(v[0]);
    std::size_t i = 1, extensions = 0;
    for (; i < v.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(v[i]);
#endif
        if (Strict ? values.back() < v[i] : !(v[i] < values.back())) {
            previous[i] = tails.back();
            if (values.size() == values.capacity()) {
                const auto capacity = values.capacity();
                values.reserve(capacity + std::min(capacity, v.size() - capacity));
            }
            if (values.size() == values.capacity()) std::unreachable();
            values.push_back(v[i]);
            tails.push_back(static_cast<Index>(i));
            if constexpr (Repeated) {
                if (++extensions >= 256 && tails.size() >= 4096) { ++i; break; }
            } else {
                if (tails.size() >= 4096 && tails.size() > (i + 1) / 4) { ++i; break; }
            }
            continue;
        }
        if constexpr (Repeated) extensions = 0;
        if (Strict ? !(values.front() < v[i]) : v[i] < values.front()) {
            tails.front() = static_cast<Index>(i);
            values.front() = v[i];
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
        if constexpr (Strict && std::is_same_v<T, std::int64_t>) {
            // 3点を独立に比較し、候補が小さい間だけ4分割する。
            if (count <= 4096) {
                const auto value = v[i];
                const auto* data = values.data();
                // 値と配列の参照は、この探索中だけ保持する。
                const auto step = [&] {
                    const auto quarter = count / 4;
                    const auto p0 = lo + quarter, p1 = p0 + quarter, p2 = p1 + quarter;
                    const bool b0 = data[p0] < value, b1 = data[p1] < value, b2 = data[p2] < value;
                    // 符号なしの減算で、真なら全ビット1、偽なら0にする。
                    const auto m0 = std::size_t{0} - static_cast<std::size_t>(b0);
                    const auto m1 = std::size_t{0} - static_cast<std::size_t>(b1);
                    const auto m2 = std::size_t{0} - static_cast<std::size_t>(b2);
                    lo += ((quarter + 1) & m0) + (quarter & m1) + (quarter & m2);
                    count = quarter - static_cast<std::size_t>(b0) + ((count % 4) & m2);
                };
                // 最初の1段を展開し、残りの段は同じ処理を繰り返す。
                if (count >= 32) step();
                while (count >= 32) step();
            }
        }
#endif
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = Strict ? values[mid] < v[i] : !(v[i] < values[mid]);
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
        values[lo] = v[i];
    }
    std::vector<T>().swap(values);
    for (; i < v.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(v[i]);
#endif
        if (Strict ? v[tails.back()] < v[i] : !(v[i] < v[tails.back()])) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            continue;
        }
        if (Strict ? !(v[tails.front()] < v[i]) : v[i] < v[tails.front()]) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = Strict ? v[tails[mid]] < v[i] : !(v[i] < v[tails[mid]]);
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    std::vector<std::size_t> result(tails.size());
    auto at = tails.back();
    for (auto i = result.size(); i > 1; --i) {
        result[i - 1] = at;
        at = previous[at];
    }
    result[0] = at;
    return result;
}

#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
#ifndef SIRKIT_LIS_AVX_TAIL_WORKER_DEFINED
#define SIRKIT_LIS_AVX_TAIL_WORKER_DEFINED
// 狭義int64の小区間だけを連続SIMD比較する。
template<class Index>
__attribute__((target("avx2")))
std::vector<std::size_t> LISIndicesWithValuesAVXTail(const std::vector<std::int64_t>& v) {
    if (v.empty()) return {};
    std::vector<Index> tails;
    std::vector<std::int64_t> values;
    auto previous = std::make_unique_for_overwrite<Index[]>(v.size());
    tails.reserve(std::min(v.size(), std::size_t{64}));
    values.reserve(std::min(v.size(), std::size_t{64}));
    tails.push_back(0);
    values.push_back(v[0]);
    std::size_t i = 1;
    for (; i < v.size(); ++i) {
        const auto key_value = v[i];
        if (values.back() < key_value) {
            previous[i] = tails.back();
            if (values.size() == values.capacity()) {
                const auto capacity = values.capacity();
                values.reserve(capacity + std::min(capacity, v.size() - capacity));
            }
            if (values.size() == values.capacity()) std::unreachable();
            values.push_back(key_value);
            tails.push_back(static_cast<Index>(i));
            if (tails.size() >= 4096 && tails.size() > (i + 1) / 4) { ++i; break; }
            continue;
        }
        if (!(values.front() < key_value)) {
            tails.front() = static_cast<Index>(i);
            values.front() = key_value;
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        if (count <= 4096) {
            const auto value = v[i];
            const auto* data = values.data();
            // 値と配列の参照は、この探索中だけ保持する。
            const auto step = [&] {
                const auto quarter = count / 4;
                const auto p0 = lo + quarter, p1 = p0 + quarter, p2 = p1 + quarter;
                const bool b0 = data[p0] < value, b1 = data[p1] < value, b2 = data[p2] < value;
                // 符号なしの減算で、真なら全ビット1、偽なら0にする。
                const auto m0 = std::size_t{0} - static_cast<std::size_t>(b0);
                const auto m1 = std::size_t{0} - static_cast<std::size_t>(b1);
                const auto m2 = std::size_t{0} - static_cast<std::size_t>(b2);
                lo += ((quarter + 1) & m0) + (quarter & m1) + (quarter & m2);
                count = quarter - static_cast<std::size_t>(b0) + ((count % 4) & m2);
            };
            // 最初の1段を展開し、残りの段は同じ処理を繰り返す。
            if (count >= 32) step();
            while (count >= 32) step();
        }
        while (count >= 16) {
            const auto half = count / 2, mid = lo + half;
            const bool before = values[mid] < key_value;
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        // 先頭と末尾の実在する区間を重ね、残候補をすべて比較する。
        if (count >= 4) {
            const auto key = _mm256_set1_epi64x(key_value);
            if (count >= 8) {
                __m256i first0, first1, last0, last1;
                std::memcpy(&first0, values.data() + lo, sizeof(first0));
                std::memcpy(&first1, values.data() + lo + 4, sizeof(first1));
                std::memcpy(&last0, values.data() + lo + count - 8, sizeof(last0));
                std::memcpy(&last1, values.data() + lo + count - 4, sizeof(last1));
                const auto first_mask0 = static_cast<unsigned>(_mm256_movemask_pd(
                    _mm256_castsi256_pd(_mm256_cmpgt_epi64(key, first0))));
                const auto first_mask1 = static_cast<unsigned>(_mm256_movemask_pd(
                    _mm256_castsi256_pd(_mm256_cmpgt_epi64(key, first1))));
                const auto last_mask0 = static_cast<unsigned>(_mm256_movemask_pd(
                    _mm256_castsi256_pd(_mm256_cmpgt_epi64(key, last0))));
                const auto last_mask1 = static_cast<unsigned>(_mm256_movemask_pd(
                    _mm256_castsi256_pd(_mm256_cmpgt_epi64(key, last1))));
                const auto first_mask = first_mask0 | (first_mask1 << 4);
                const auto last_mask = last_mask0 | (last_mask1 << 4);
                const auto first_before = static_cast<std::size_t>(std::countr_one(first_mask));
                const auto last_before = static_cast<std::size_t>(std::countr_one(last_mask));
                lo += first_mask == 255 ? count - 8 + last_before : first_before;
                count = 0;
            }
            else {
                __m256i samples;
                std::memcpy(&samples, values.data() + lo, sizeof(samples));
                const auto comparison = _mm256_cmpgt_epi64(key, samples);
                const auto mask = static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(comparison)));
                const auto advance = static_cast<std::size_t>(std::countr_one(mask));
                lo += advance;
                count = advance == 4 ? count - 4 : 0;
            }
        }
        while (count != 0) {
            const auto half = count / 2, mid = lo + half;
            const bool before = values[mid] < key_value;
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
        values[lo] = key_value;
    }
    std::vector<std::int64_t>().swap(values);
    for (; i < v.size(); ++i) {
        if (v[tails.back()] < v[i]) {
            previous[i] = tails.back();
            tails.push_back(static_cast<Index>(i));
            continue;
        }
        if (!(v[tails.front()] < v[i])) {
            tails.front() = static_cast<Index>(i);
            continue;
        }
        std::size_t lo = 1, count = tails.size() - 2;
        while (count != 0) {
            const auto half = count / 2, mid = lo + half;
            const bool before = v[tails[mid]] < v[i];
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        previous[i] = tails[lo - 1];
        tails[lo] = static_cast<Index>(i);
    }
    std::vector<std::size_t> result(tails.size());
    auto at = tails.back();
    for (auto pos = result.size(); pos > 1; --pos) {
        result[pos - 1] = at;
        at = previous[at];
    }
    result[0] = at;
    return result;
}
#endif
#endif
template<bool Strict, class T>
unsigned LISValuesUseful(const std::vector<T>& v) {
    std::array<T, 256> tails{};
    std::size_t length = 0, searches = 0, equal_neighbors = 0;
    for (std::size_t i = 0; i < tails.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(v[i]);
#endif
#if defined(__GNUC__) && !defined(__clang__)
        if constexpr (!Strict) if (i != 0 && !(v[i - 1] < v[i]) && !(v[i] < v[i - 1])) ++equal_neighbors;
#endif
        std::size_t lo = 0, count = length;
        while (count != 0) {
            const auto half = count / 2;
            const auto mid = lo + half;
            const bool before = Strict ? tails[mid] < v[i] : !(v[i] < tails[mid]);
            lo = before ? mid + 1 : lo;
            count = before ? count - half - 1 : half;
        }
        if (lo != 0 && lo != length) ++searches;
        tails[lo] = v[i];
        length += lo == length;
    }
    if (length >= 16 && length <= 64 && searches >= 128) return 1;
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
    if constexpr (Strict && std::is_same_v<T, std::int64_t>)
        // 先頭ですでに長い場合は、添字だけの段階探索を選ぶ。
        if (length > 64) return 3;
#endif
#if defined(__GNUC__) && !defined(__clang__)
    if constexpr (!Strict) if (length <= 192 && searches >= 64 && equal_neighbors >= 16) return 2;
#endif
    (void)equal_neighbors;
    return 0;
}
template <bool Strict, class Index, class T>
std::vector<std::size_t> LISIndicesSelected(const std::vector<T>& v) {
    if constexpr (sizeof(Index) < sizeof(std::size_t) && std::is_arithmetic_v<T> && sizeof(T) <= 8 && !std::is_same_v<T,bool>) {
        if (v.size() >= 65536) {
            const auto choice = LISValuesUseful<Strict>(v);
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 15 && __GNUC_MINOR__ == 2 && defined(__x86_64__)
            if constexpr (Strict && std::is_same_v<T, std::int64_t>) {
                if (choice == 3) return LISIndicesPhased<Index>(v);
                if (choice != 0 && __builtin_cpu_supports("avx2"))
                    return LISIndicesWithValuesAVXTail<Index>(v);
            }
#endif
#if defined(__GNUC__) && !defined(__clang__)
            if constexpr (!Strict) if (choice == 2) return LISIndicesWithValues<Strict,Index,true>(v);
#endif
            if (choice != 0) return LISIndicesWithValues<Strict,Index>(v);
        }
    }
    return LISIndicesImpl<Strict,Index>(v);
}
} // namespace reinsirk::detail
template <bool Strict = true, class T>
std::vector<std::size_t> LISIndices(const std::vector<T>& v) {
    if (v.size() <= std::numeric_limits<std::uint32_t>::max())
        return reinsirk::detail::LISIndicesSelected<Strict, std::uint32_t>(v);
    return reinsirk::detail::LISIndicesSelected<Strict, std::size_t>(v);
}
template <bool Strict = true, class T>
std::vector<T> LISSequence(const std::vector<T>& v) {
    const auto indices = LISIndices<Strict>(v);
    std::vector<T> result;
    result.reserve(indices.size());
    for (auto i : indices) result.push_back(v[i]);
    return result;
}
#endif

#ifndef SIRKIT_LIS_ENDING_LENGTHS_DEFINED
#define SIRKIT_LIS_ENDING_LENGTHS_DEFINED
template <bool Strict = true, class T>
std::vector<std::size_t> LISEndingLengths(const std::vector<T>& v) {
    std::vector<T> tails;
    tails.reserve(std::min(v.size(), std::size_t{64}));
    std::vector<std::size_t> result;
    result.reserve(v.size());
    for (const auto& value : v) {
#ifndef SKIP_RUNTIME_CHECKS
        reinsirk::detail::check_lis_value(value);
#endif
        if (tails.empty() || (Strict ? tails.back() < value : !(value < tails.back()))) {
            tails.push_back(value);
            result.push_back(tails.size());
        } else {
            auto it = Strict ? std::lower_bound(tails.begin(), tails.end(), value)
                             : std::upper_bound(tails.begin(), tails.end(), value);
            const auto length = static_cast<std::size_t>(it - tails.begin()) + 1;
            *it = value;
            result.push_back(length);
        }
    }
    return result;
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

#include <cstdint>
#include <vector>
int main(){
    int n{};verify_in>>n;std::vector<std::int64_t>a(n);for(auto&x:a)verify_in>>x;
    const auto answer=LISIndices<true>(a);verify_out<<answer.size()<<'\n';
    for(std::size_t i=0;i<answer.size();++i)verify_out<<(i?" ":"")<<answer[i];
    verify_out<<'\n';
}
