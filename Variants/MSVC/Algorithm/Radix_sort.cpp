// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Algorithm/Radix_sort.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/radix_sort.md
//
// API一覧
// [再利用作業領域]
// RadixSortWorkspace<T>(): boolを除く8/16/32/64bit整数用の再利用作業領域。
// sort(values): 符号付きも全域で安定昇順ソート。B=sizeof(T)としてO(Bn)、追加領域O(n)。
// sort(values,maximum): 同じ処理。全要素<=maximumが前提で、通常処理中に検査する。
// reserve(n): n要素分の作業領域を予約。capacity(): 現在の予約要素数。
// release(): 作業領域を解放する。sortはvaluesの容量・要素の保管先を変えない。
//
// [値の整列]
// RadixSort(values): 符号付きも全域で安定昇順ソート、O(sizeof(T)*n)時間・O(n)追加領域。
// RadixSort(values,maximum): 同じ処理。全要素<=maximumが前提（負の値・負のmaximumも可）。
// valuesの容量・要素の保管先は不変。型Tはboolを除く8/16/32/64bit整数。
//
// [添字の整列]
// RadixSortOrder(values): 昇順の元添字列を返す。同値は元の順、入力不変。
// boolを除く8/16/32/64bit整数。O(sizeof(T)*n)時間・O(n)追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>



// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RADIX_SORT_DETAIL_DEFINED
#define SIRKIT_RADIX_SORT_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class T>
concept RadixInteger = std::integral<T> && !std::same_as<T, bool> &&
    !std::is_const_v<T> && !std::is_volatile_v<T> &&
    (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8) &&
    std::numeric_limits<std::make_unsigned_t<T>>::digits == sizeof(T) * 8;

template <RadixInteger T> struct RadixSortDetail {
    using Unsigned = std::make_unsigned_t<T>;
    static constexpr std::size_t bytes = sizeof(T);
    static constexpr std::size_t small_limit = 64;
    using Histogram = std::array<std::array<std::size_t, 256>, bytes>;

    static Unsigned key(T value) noexcept {
        Unsigned result = static_cast<Unsigned>(value);
        if constexpr (std::is_signed_v<T>) result ^= Unsigned{1} << (bytes * 8 - 1);
        return result;
    }
    template <bool HasMaximum> static void check(T value, [[maybe_unused]] T maximum) {
#ifndef SKIP_RUNTIME_CHECKS
        if constexpr (HasMaximum) {
            if (value > maximum) throw std::out_of_range("RadixSort: value exceeds maximum");
        }
#else
        (void)value;
#endif
    }
    template <bool HasMaximum>
    static Unsigned histogram(const std::vector<T>& values, Histogram& counts, T maximum, bool& ordered, bool& reversed) {
        if constexpr (bytes < 8) {
            counts = {};
            const Unsigned first = key(values.front());
            Unsigned different = 0;
            T previous = values.front();
            ordered = reversed = true;
            for (T value : values) {
                check<HasMaximum>(value, maximum);
                ordered &= previous <= value;
                reversed &= previous >= value;
                previous = value;
                Unsigned encoded = key(value);
                different |= encoded ^ first;
                {
                    for (std::size_t byte = 0; byte < bytes; ++byte) {
                        ++counts[byte][encoded & Unsigned{255}];
                        if constexpr (bytes > 1) encoded >>= 8;
                    }
                }
            }
            return different;
        } else {
            const Unsigned first = key(values.front());
            Unsigned different = 0;
            {
                T previous = values.front();
                ordered = reversed = true;
                for (T value : values) {
                    check<HasMaximum>(value, maximum);
                    ordered &= previous <= value;
                    reversed &= previous >= value;
                    previous = value;
                    different |= key(value) ^ first;
                }
            }
            // 整列済みなら集計しない。散布する上位バイト数は差分から決める。
            if (ordered || reversed) return different;
            counts = {};
            if constexpr (bytes > 4) {
                const std::size_t active = (std::bit_width(different) + 7) / 8;
                if (active <= 4) {
                    for (T value : values) {
                        const Unsigned encoded = key(value);
                        ++counts[0][encoded & Unsigned{255}];
                        if (active > 1) ++counts[1][(encoded >> 8) & Unsigned{255}];
                        if (active > 2) ++counts[2][(encoded >> 16) & Unsigned{255}];
                        if (active > 3) ++counts[3][(encoded >> 24) & Unsigned{255}];
                    }
                    return different;
                }
            }
            for (T value : values) {
                Unsigned encoded = key(value);
                for (std::size_t byte = 0; byte < bytes; ++byte) {
                    ++counts[byte][encoded & Unsigned{255}];
                    if constexpr (bytes > 1) encoded >>= 8;
                }
            }
            return different;
        }
    }
    static void prefix(std::array<std::size_t, 256>& counts) noexcept {
        std::size_t offset = 0;
        for (auto& count : counts) {
            const auto current = count;
            count = offset;
            offset += current;
        }
    }
    template <bool HasMaximum>
    static void sort(std::vector<T>& values, std::vector<T>& buffer, T maximum) {
        const std::size_t n = values.size();
        if (n <= small_limit) {
            if (n) check<HasMaximum>(values[0], maximum);
            for (std::size_t i = 1; i < n; ++i) {
                const T value = values[i];
                check<HasMaximum>(value, maximum);
                std::size_t j = i;
                for (; j && value < values[j - 1]; --j) values[j] = values[j - 1];
                values[j] = value;
            }
            return;
        }
        Histogram counts;
        bool ordered, reversed;
        const Unsigned different = histogram<HasMaximum>(values, counts, maximum, ordered, reversed);
        if (ordered) return;
        if (reversed) {
            std::reverse(values.begin(), values.end());
            return;
        }
        buffer.resize(n);
        T* source = values.data();
        T* destination = buffer.data();
        for (std::size_t byte = 0; byte < bytes; ++byte) {
            const std::size_t shift = byte * 8;
            if (((different >> shift) & Unsigned{255}) == 0) continue;
            auto& offsets = counts[byte];
            prefix(offsets);
            for (std::size_t i = 0; i < n; ++i) {
                const T value = source[i];
                const auto bucket = static_cast<std::size_t>((key(value) >> shift) & Unsigned{255});
                destination[offsets[bucket]++] = value;
            }
            std::swap(source, destination);
        }
        if (source != values.data()) std::copy_n(source, n, values.data());
    }
};
} // namespace reinsirk::detail
#endif

#ifndef SIRKIT_RADIX_SORT_WORKSPACE_DEFINED
#define SIRKIT_RADIX_SORT_WORKSPACE_DEFINED
template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixInteger T> class RadixSortWorkspace {
    std::vector<T> buffer_;
public:
    void sort(std::vector<T>& values) {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixSortDetail<T>::template sort<false>(values, buffer_, T{});
    }
    void sort(std::vector<T>& values, T maximum) {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixSortDetail<T>::template sort<true>(values, buffer_, maximum);
    }
    void reserve(std::size_t count) { buffer_.reserve(count); }
    std::size_t capacity() const noexcept { return buffer_.capacity(); }
    void release() noexcept { std::vector<T>().swap(buffer_); }
};
#endif

#ifndef SIRKIT_RADIX_SORT_DEFINED
#define SIRKIT_RADIX_SORT_DEFINED
// 小配列は確保せず挿入ソート。整列済みは変更せず、一定のbyteは散布を省く。
template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixInteger T> void RadixSort(std::vector<T>& values) {
    RadixSortWorkspace<T> workspace;
    workspace.sort(values);
}
template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixInteger T> void RadixSort(std::vector<T>& values, T maximum) {
    RadixSortWorkspace<T> workspace;
    workspace.sort(values, maximum);
}
#endif

#ifndef SIRKIT_RADIX_SORT_ORDER_DEFINED
#define SIRKIT_RADIX_SORT_ORDER_DEFINED
template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixInteger T> std::vector<std::size_t> RadixSortOrder(const std::vector<T>& values) {
    using Detail = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::RadixSortDetail<T>;
    using Unsigned = typename Detail::Unsigned;
    const std::size_t n = values.size();
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), std::size_t{0});
    if (n <= Detail::small_limit) {
        for (std::size_t i = 1; i < n; ++i) {
            const std::size_t index = order[i];
            std::size_t j = i;
            for (; j && values[index] < values[order[j - 1]]; --j) order[j] = order[j - 1];
            order[j] = index;
        }
        return order;
    }
    typename Detail::Histogram counts;
    bool ordered, reversed;
    const Unsigned different = Detail::template histogram<false>(values, counts, T{}, ordered, reversed);
    if (ordered) return order;
    if (reversed) {
        // 同じ値の区間内は元の添字順を保つ。
        std::size_t output = 0, end = n;
        while (end) {
            std::size_t begin = end - 1;
            while (begin && values[begin - 1] == values[end - 1]) --begin;
            for (std::size_t i = begin; i < end; ++i) order[output++] = i;
            end = begin;
        }
        return order;
    }
    std::vector<std::size_t> buffer(n);
    for (std::size_t byte = 0; byte < Detail::bytes; ++byte) {
        const std::size_t shift = byte * 8;
        if (((different >> shift) & Unsigned{255}) == 0) continue;
        auto& offsets = counts[byte];
        Detail::prefix(offsets);
        for (std::size_t index : order) {
            const auto bucket = static_cast<std::size_t>((Detail::key(values[index]) >> shift) & Unsigned{255});
            buffer[offsets[bucket]++] = index;
        }
        order.swap(buffer);
    }
    return order;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
