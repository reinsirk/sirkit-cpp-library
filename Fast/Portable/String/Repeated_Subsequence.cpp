// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Repeated_Subsequence.cpp
// 参考（周期列での照会）: https://atcoder.jp/contests/abc346/tasks/abc346_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/repeated_subsequence.md
//
// API一覧
// RepeatedSubsequenceIndex(cycle): 各バイトの出現位置を記録。O(n+256)時間・領域、nはcycleの長さ。コピーは独立し、移動元は空。
// contains_expanded(copies,target,repetitions): cycleをcopies回並べた列に、targetの各バイトをその場でrepetitions個に増やした部分列があるか。
// max_repetition(copies,target,limit): 条件を満たす0以上limit以下の最大repetitions。limit既定はUINT64_MAX、空targetはlimit。
// 空列・0回の照会可。回数はuint64_t。繰り返した文字列を実体化しない。
// mをtargetの長さとし、contains_expandedはO(1+m*log(n+2))、max_repetitionはO(1+m*log(n+2)*log(limit+2))時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_REPEATED_SUBSEQUENCE_DEFINED
#define SIRKIT_REPEATED_SUBSEQUENCE_DEFINED

class RepeatedSubsequenceIndex
{
    using Count = std::uint64_t;
    std::array<std::vector<std::size_t>, 256> positions_;

public:
    explicit RepeatedSubsequenceIndex(std::string_view cycle)
    {
        for (std::size_t i = 0; i < cycle.size(); ++i)
        {
            positions_[static_cast<unsigned char>(cycle[i])].push_back(i);
        }
    }

    RepeatedSubsequenceIndex(const RepeatedSubsequenceIndex &) = default;

    RepeatedSubsequenceIndex(RepeatedSubsequenceIndex &&other) noexcept
    {
        positions_.swap(other.positions_);
    }

    RepeatedSubsequenceIndex &operator=(RepeatedSubsequenceIndex other) noexcept
    {
        positions_.swap(other.positions_);
        return *this;
    }

    [[nodiscard]] bool contains_expanded(Count copies, std::string_view target, Count repetitions) const
    {
        if (repetitions == 0 || target.empty())
        {
            return true;
        }
        Count copy = 0;
        std::size_t offset = 0;
        for (const char byte : target)
        {
            const auto &positions = positions_[static_cast<unsigned char>(byte)];
            if (positions.empty())
            {
                return false;
            }
            const Count frequency = positions.size();
            const Count start = static_cast<Count>(std::lower_bound(positions.begin(), positions.end(), offset) - positions.begin());
            Count advance = (repetitions - 1) / frequency;
            const Count remainder = (repetitions - 1) % frequency;
            const Count available = frequency - start;
            Count position;
            // 周期内の出現番号と周期を分け、回数と文字列長の積を取らない。
            if (remainder >= available)
            {
                ++advance;
                position = remainder - available;
            }
            else
            {
                position = start + remainder;
            }
            if (advance >= copies - copy)
            {
                return false;
            }
            copy += advance;
            offset = positions[static_cast<std::size_t>(position)] + 1;
        }
        return true;
    }

    [[nodiscard]] Count max_repetition(Count copies, std::string_view target,
                                       Count limit = std::numeric_limits<Count>::max()) const
    {
        if (target.empty())
        {
            return limit;
        }
        if (copies == 0 || limit == 0)
        {
            return 0;
        }
        std::array<Count, 256> needed{};
        for (const char byte : target)
        {
            ++needed[static_cast<unsigned char>(byte)];
        }
        Count high = limit;
        for (std::size_t byte = 0; byte < needed.size(); ++byte)
        {
            if (needed[byte] == 0)
            {
                continue;
            }
            const Count frequency = positions_[byte].size();
            if (frequency <= std::numeric_limits<Count>::max() / copies)
            {
                high = std::min(high, frequency * copies / needed[byte]);
            }
        }
        if (contains_expanded(copies, target, high))
        {
            return high;
        }
        --high;
        Count low = 0;
        while (low < high)
        {
            const Count distance = high - low;
            const Count mid = low + distance / 2 + distance % 2;
            if (contains_expanded(copies, target, mid))
            {
                low = mid;
            }
            else
            {
                high = mid - 1;
            }
        }
        return low;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
