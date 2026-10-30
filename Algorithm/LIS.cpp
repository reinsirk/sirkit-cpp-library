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
                while (count >= 32) {
                    const auto quarter = count / 4;
                    const auto p0 = lo + quarter, p1 = p0 + quarter, p2 = p1 + quarter;
                    const unsigned segment = static_cast<unsigned>(values[p0] < v[i]) +
                        static_cast<unsigned>(values[p1] < v[i]) + static_cast<unsigned>(values[p2] < v[i]);
                    lo += segment * quarter + (segment != 0);
                    count = segment == 3 ? count - 3 * quarter - 1 : quarter - (segment != 0);
                }
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
