// SPDX-License-Identifier: CC0-1.0
// 数学的参考: J. H. Conway, On Numbers and Games, Chapter 6; R. K. Guy, Impartial Games.
// https://www.cambridge.org/core/books/abs/games-of-no-chance/impartial-games/FAC3E206D7C0A86C60615EF8F669EABB
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/nim_product.md
//
// API一覧
// nim_product(a, b): uint64_tのニム積を返す。ニム和はa^b。0は吸収元、1は単位元。
// 初回に8ビット積表など69 KiBを生成し、以後は3段の分割で定数時間・追加確保なし。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NIM_PRODUCT_DEFINED
#define SIRKIT_NIM_PRODUCT_DEFINED

namespace detail
{
template <int Half, class Multiply, class Scale>
constexpr std::uint64_t NimProductSplit(std::uint64_t a, std::uint64_t b, Multiply multiply, Scale scale)
{
    constexpr auto kMask = (std::uint64_t{1} << Half) - 1;
    const auto a_high = a >> Half;
    const auto b_high = b >> Half;
    const auto a_low = a & kMask;
    const auto b_low = b & kMask;
    const auto high = multiply(a_high, b_high);
    const auto low = multiply(a_low, b_low);
    const auto mixed = multiply(a_high ^ a_low, b_high ^ b_low);
    // F=2^Half。F⊗F=F xor (F/2) なので、上位係数は mixed xor low。
    return ((mixed ^ low) << Half) ^ low ^ scale(high);
}

template <int Bits>
constexpr std::uint64_t SmallNimProduct(std::uint64_t a, std::uint64_t b)
{
    if constexpr (Bits == 1)
    {
        return a & b;
    }
    else
    {
        constexpr int kHalf = Bits / 2;
        return NimProductSplit<kHalf>(a, b, [](std::uint64_t x, std::uint64_t y)
                                      {
                                          return SmallNimProduct<kHalf>(x, y);
                                      },
                                      [](std::uint64_t x)
                                      {
                                          return SmallNimProduct<kHalf>(x, std::uint64_t{1} << (kHalf - 1));
                                      });
    }
}

class NimProductTable
{
public:
    NimProductTable()
    {
        for (std::size_t a = 0; a < byte_product_.size(); ++a)
        {
            for (std::size_t b = 0; b < byte_product_[a].size(); ++b)
            {
                byte_product_[a][b] = static_cast<std::uint8_t>(SmallNimProduct<8>(a, b));
            }
        }
        FillScale(scale16_);
        FillScale(scale32_);
    }

    template <int Bits>
    std::uint64_t multiply(std::uint64_t a, std::uint64_t b) const
    {
        if constexpr (Bits == 8)
        {
            return byte_product_[a][b];
        }
        else
        {
            constexpr int kHalf = Bits / 2;
            return NimProductSplit<kHalf>(a, b, [this](std::uint64_t x, std::uint64_t y)
                                          {
                                              return multiply<kHalf>(x, y);
                                          },
                                          [this](std::uint64_t x)
                                          {
                                              return Scale<kHalf>(x);
                                          });
        }
    }

private:
    std::array<std::array<std::uint8_t, 256>, 256> byte_product_;
    std::array<std::array<std::uint16_t, 256>, 2> scale16_;
    std::array<std::array<std::uint32_t, 256>, 4> scale32_;

    template <class T, std::size_t Bytes>
    void FillScale(std::array<std::array<T, 256>, Bytes> &table)
    {
        constexpr std::size_t kBits = Bytes * 8;
        constexpr auto kFactor = std::uint64_t{1} << (kBits - 1);
        for (std::size_t byte = 0; byte < Bytes; ++byte)
        {
            for (std::size_t value = 0; value < table[byte].size(); ++value)
            {
                table[byte][value] = static_cast<T>(multiply<kBits>(std::uint64_t{value} << (8 * byte), kFactor));
            }
        }
    }

    template <int Bits>
    std::uint64_t Scale(std::uint64_t value) const
    {
        // 定数倍はxorに対して線形なので、各バイトの寄与を足す。
        if constexpr (Bits == 8)
        {
            return byte_product_[128][value];
        }
        else if constexpr (Bits == 16)
        {
            return scale16_[0][value & 255] ^ scale16_[1][value >> 8];
        }
        else
        {
            return scale32_[0][value & 255] ^ scale32_[1][(value >> 8) & 255] ^
                   scale32_[2][(value >> 16) & 255] ^ scale32_[3][value >> 24];
        }
    }
};
} // namespace detail

inline std::uint64_t nim_product(std::uint64_t a, std::uint64_t b)
{
    static const detail::NimProductTable table;
    return table.multiply<64>(a, b);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
