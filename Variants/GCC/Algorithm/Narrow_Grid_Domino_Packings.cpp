// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Narrow_Grid_Domino_Packings.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/narrow_grid_domino_packings.md
//
// API一覧
// narrow_grid_domino_packings(h,w,mode,mod): h行w列のドミノ配置の数をmodで返す。0<=h<=5、wはuint64_t、mod>0。
// DominoPackingMode::kPerfectは全マスを覆う。kMaximalはドミノを追加できない配置を数える。
// 空の盤面には配置が1通りある。縦・横のドミノは各々区別せず、回転して置ける。
// 到達可能な境界状態数をSとすると、O(S*3^h + S^3 log(w+1))時間、O(S^2 + 4^h)領域。
// Sは完全配置で2^h以下、極大配置で3^h以下。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NARROW_GRID_DOMINO_PACKINGS_DEFINED
#define SIRKIT_NARROW_GRID_DOMINO_PACKINGS_DEFINED

enum class DominoPackingMode
{
    kPerfect,
    kMaximal
};

namespace detail
{
inline std::vector<std::uint32_t> MultiplyDominoTransfers(const std::vector<std::uint32_t> &left, int rows,
                                                          const std::vector<std::uint32_t> &right, int order,
                                                          std::uint32_t modulus)
{
    std::vector<std::uint32_t> answer(rows * order);
    for (int i = 0; i < rows; ++i)
    {
        for (int k = 0; k < order; ++k)
        {
            if (left[i * order + k] == 0)
            {
                continue;
            }
            const std::uint64_t a = left[i * order + k];
            for (int j = 0; j < order; ++j)
            {
                if (right[k * order + j] != 0)
                {
                    auto &cell = answer[i * order + j];
                    cell = static_cast<std::uint32_t>((cell + a * right[k * order + j]) % modulus);
                }
            }
        }
    }
    return answer;
}
}

[[nodiscard]] inline std::uint32_t narrow_grid_domino_packings(int height, std::uint64_t width,
                                                               DominoPackingMode mode,
                                                               std::uint32_t modulus = 1000000007)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (height < 0 || height > 5 || modulus == 0)
    {
        throw std::invalid_argument("narrow_grid_domino_packings: invalid height or modulus");
    }
#endif
    if (height == 0 || width == 0)
    {
        return 1 % modulus;
    }
    const unsigned row_mask = (1U << height) - 1;
    std::vector<int> index(1U << (2 * height), -1);
    std::vector<unsigned> states{0};
    index[0] = 0;
    std::vector<std::vector<int>> edges;
    for (std::size_t i = 0; i < states.size(); ++i)
    {
        const unsigned reserved = states[i] & row_mask;
        const unsigned previous_empty = states[i] >> height;
        auto &destinations = edges.emplace_back();
        const auto fill_column = [&](const auto &self, int row, unsigned outgoing, unsigned empty) -> void
        {
            if (row == height)
            {
                const unsigned key = outgoing | (empty << height);
                if (index[key] < 0)
                {
                    index[key] = static_cast<int>(states.size());
                    states.push_back(key);
                }
                destinations.push_back(index[key]);
                return;
            }
            const unsigned bit = 1U << row;
            if ((reserved & bit) != 0)
            {
                self(self, row + 1, outgoing, empty);
                return;
            }
            self(self, row + 1, outgoing | bit, empty);
            if (row + 1 < height && (reserved & (bit << 1)) == 0)
            {
                self(self, row + 2, outgoing, empty);
            }
            // 極大配置では、未被覆マスが上下左右に隣接するとドミノを追加できてしまう。
            if (mode == DominoPackingMode::kMaximal && (previous_empty & bit) == 0 && (empty & (bit >> 1)) == 0)
            {
                self(self, row + 1, outgoing, empty | bit);
            }
        };
        fill_column(fill_column, 0, 0, 0);
    }

    const int order = static_cast<int>(states.size());
    std::vector<std::uint32_t> transfer(order * order), ways(order);
    for (int i = 0; i < order; ++i)
    {
        for (int j : edges[i])
        {
            // 突出と未被覆の位置が決まると、残りの縦ドミノの配置は一意。
            auto &cell = transfer[i * order + j];
            cell = 1 % modulus;
        }
    }
    ways[0] = 1 % modulus;
    while (width != 0)
    {
        if ((width & 1) != 0)
        {
            ways = detail::MultiplyDominoTransfers(ways, 1, transfer, order, modulus);
        }
        width >>= 1;
        if (width != 0)
        {
            transfer = detail::MultiplyDominoTransfers(transfer, order, transfer, order, modulus);
        }
    }
    std::uint64_t answer = 0;
    for (int i = 0; i < order; ++i)
    {
        if ((states[i] & row_mask) == 0)
        {
            answer += ways[i];
        }
    }
    return static_cast<std::uint32_t>(answer % modulus);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
