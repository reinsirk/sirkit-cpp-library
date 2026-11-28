// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: String/Prefix_Bitmap_Table.cpp
// 参考（列一括置換と接頭辞照会の用途）: https://yukicoder.me/problems/no/2761
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_bitmap_table.md
//
// API一覧
// PrefixBitmapTable<Alphabet=26,First='a'>(rows): 等長の文字列を所有して準備。扱うbyteはFirst以上First+Alphabet未満。
// size()/width(): 行数/文字列の長さ。各O(1)。引数なしでは0行、幅0。
// replace(column,from,to): 全行の指定列でfromをtoに置換。列は0始まり。O(1+B)。
// prefix_count(prefix): prefixで始まる行数。重複は別の行として数える。空ならsize()、長すぎるか対象外のbyteを含めば0。
// prefix_countは長さLとしてO(1+L*B)時間・O(B)作業領域。B=ceil(行数/64)。
// 列数Wとして構築O(行数+W*行数+W*Alphabet*B)、保持領域O(1+W*Alphabet*B)ワード。
// 検査有効時、行長が不揃いならinvalid_argument、replaceの範囲外の列と構築/replaceの対象外byteはout_of_range。
// コピーは独立。ムーブ元は行数・幅0。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_BITMAP_TABLE_DEFINED
#define SIRKIT_PREFIX_BITMAP_TABLE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t PrefixBitmapStorageSize(std::size_t width, std::size_t alphabet, std::size_t blocks,
                                           std::size_t maximum)
{
    if (blocks == 0)
    {
        return 0;
    }
    if (width > maximum / alphabet / blocks)
    {
        throw std::length_error("PrefixBitmapTable: table is too large");
    }
    return width * alphabet * blocks;
}
} // namespace reinsirk::detail

template <std::size_t Alphabet = 26, unsigned First = 'a'>
class PrefixBitmapTable
{
    static_assert(Alphabet > 0 && Alphabet <= 256 && First <= 256 - Alphabet);
    using Word = std::uint64_t;
    std::size_t size_ = 0;
    std::size_t width_ = 0;
    std::size_t blocks_ = 0;
    std::vector<Word> bits_;

    [[nodiscard]] static std::size_t digit(unsigned char byte) noexcept
    {
        return static_cast<std::size_t>(byte) - First;
    }

    [[nodiscard]] static std::size_t checked_digit(unsigned char byte)
    {
        const auto code = digit(byte);
#ifndef SKIP_RUNTIME_CHECKS
        if (code >= Alphabet)
        {
            throw std::out_of_range("PrefixBitmapTable: byte outside alphabet");
        }
#endif
        return code;
    }

    [[nodiscard]] std::size_t offset(std::size_t column, std::size_t code) const noexcept
    {
        return (column * Alphabet + code) * blocks_;
    }

    template <class Row>
    void initialize(std::span<const Row> rows)
    {
        size_ = rows.size();
        width_ = rows.empty() ? 0 : rows.front().size();
        blocks_ = size_ == 0 ? 0 : ((size_ - 1) >> 6) + 1;
#ifndef SKIP_RUNTIME_CHECKS
        for (const auto &row : rows)
        {
            if (row.size() != width_)
            {
                throw std::invalid_argument("PrefixBitmapTable: ragged rows");
            }
        }
#endif
        bits_.resize(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PrefixBitmapStorageSize(width_, Alphabet, blocks_, bits_.max_size()));
        for (std::size_t column = 0; column < width_; ++column)
        {
            for (std::size_t row = 0; row < size_; ++row)
            {
                const auto code = checked_digit(rows[row][column]);
                bits_[offset(column, code) + (row >> 6)] |= Word{1} << (row & 63);
            }
        }
    }

public:
    PrefixBitmapTable() = default;

    explicit PrefixBitmapTable(std::span<const std::string> rows)
    {
        initialize(rows);
    }

    explicit PrefixBitmapTable(std::span<const std::string_view> rows)
    {
        initialize(rows);
    }

    PrefixBitmapTable(const PrefixBitmapTable &) = default;

    PrefixBitmapTable(PrefixBitmapTable &&other) noexcept
        : size_(std::exchange(other.size_, 0)), width_(std::exchange(other.width_, 0)),
          blocks_(std::exchange(other.blocks_, 0)), bits_(std::move(other.bits_))
    {
        other.bits_.clear();
    }

    PrefixBitmapTable &operator=(PrefixBitmapTable other) noexcept
    {
        std::swap(size_, other.size_);
        std::swap(width_, other.width_);
        std::swap(blocks_, other.blocks_);
        bits_.swap(other.bits_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::size_t width() const noexcept
    {
        return width_;
    }

    void replace(std::size_t column, unsigned char from, unsigned char to)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (column >= width_)
        {
            throw std::out_of_range("PrefixBitmapTable: column");
        }
#endif
        const auto old_code = checked_digit(from);
        const auto new_code = checked_digit(to);
        if (old_code == new_code)
        {
            return;
        }
        const auto source = offset(column, old_code);
        const auto destination = offset(column, new_code);
        for (std::size_t block = 0; block < blocks_; ++block)
        {
            bits_[destination + block] |= bits_[source + block];
            bits_[source + block] = 0;
        }
    }

    [[nodiscard]] std::size_t prefix_count(std::string_view prefix) const
    {
        if (prefix.empty())
        {
            return size_;
        }
        if (prefix.size() > width_)
        {
            return 0;
        }

        std::vector<Word> active(blocks_, ~Word{0});
        for (std::size_t column = 0; column < prefix.size(); ++column)
        {
            const auto code = digit(prefix[column]);
            if (code >= Alphabet)
            {
                return 0;
            }
            const auto start = offset(column, code);
            Word occupied = 0;
            for (std::size_t block = 0; block < blocks_; ++block)
            {
                active[block] &= bits_[start + block];
                occupied |= active[block];
            }
            if (occupied == 0)
            {
                return 0;
            }
        }

        std::size_t count = 0;
        for (const auto mask : active)
        {
            count += static_cast<std::size_t>(std::popcount(mask));
        }
        return count;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
