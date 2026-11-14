// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Xor_Basis.cpp
// 参考（数学的構成）: https://ei1333.github.io/library/math/matrix/binary-basis.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_basis.md
//
// API一覧
// [XorBasis]
// XorBasis(): 空の入力集合から生成されるGF(2)線形空間。固定64bit、追加確保なし。
// insert(uint64_t x): 入力を1個追加し、rankが増えればtrue。O(64)。重複・0も入力として記録する。
// merge(other): 相手の入力集合と結合。O(64²)。同じオブジェクトとの結合も各入力を重複して追加した意味になる。
// contains(x): 空部分集合を許してXOR=xを作れるか。contains_nonempty(x): 非空部分集合で作れるか。O(64)。
// max_xor(seed=0)/min_xor(seed=0): seed XOR 部分集合XORの最大/最小。空部分集合を許す。O(64)。
// kth(k): 作れる相異なるXORを昇順に並べた0始まりk番目、なければnullopt。0を含む。O(64²)。
// rank(): 次元、0..64。has_nonempty_zero(): 非空部分集合のXORが0になるか。O(1)。
// clear()/swap(other): 空の入力集合へ戻す/全内容交換。O(64)。コピー・移動は固定配列の値コピー（移動元も保持）。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_XOR_BASIS_DEFINED
#define SIRKIT_XOR_BASIS_DEFINED
class XorBasis {
    std::array<std::uint64_t, 64> basis_{};
    unsigned rank_ = 0;
    bool dependent_ = false;
public:
    bool insert(std::uint64_t value) noexcept {
        for (unsigned bit = 64; bit != 0;) {
            --bit;
            if (((value >> bit) & 1) == 0) continue;
            if (basis_[bit] != 0) value ^= basis_[bit];
            else { basis_[bit] = value; ++rank_; return true; }
        }
        dependent_ = true;
        return false;
    }
    void merge(const XorBasis& other) noexcept {
        // 自己結合でも非零基底の値は変化せず、従属性だけが増える。
        for (auto value : other.basis_) if (value != 0) insert(value);
        dependent_ = dependent_ || other.dependent_;
    }
    bool contains(std::uint64_t value) const noexcept {
        for (unsigned bit = 64; bit != 0;) {
            --bit;
            if (((value >> bit) & 1) != 0) value ^= basis_[bit];
        }
        return value == 0;
    }
    bool contains_nonempty(std::uint64_t value) const noexcept { return value == 0 ? dependent_ : contains(value); }
    std::uint64_t max_xor(std::uint64_t seed = 0) const noexcept {
        for (unsigned bit = 64; bit != 0;) { --bit; seed = std::max(seed, seed ^ basis_[bit]); }
        return seed;
    }
    std::uint64_t min_xor(std::uint64_t seed = 0) const noexcept {
        for (unsigned bit = 64; bit != 0;) { --bit; seed = std::min(seed, seed ^ basis_[bit]); }
        return seed;
    }
    std::optional<std::uint64_t> kth(std::uint64_t k) const noexcept {
        if (rank_ < 64 && k >= (std::uint64_t{1} << rank_)) return std::nullopt;
        auto reduced = basis_;
        for (unsigned bit = 0; bit < 64; ++bit) {
            if (reduced[bit] == 0) continue;
            for (unsigned lower = 0; lower < bit; ++lower)
                if (((reduced[bit] >> lower) & 1) != 0) reduced[bit] ^= reduced[lower];
        }
        std::uint64_t result = 0;
        unsigned position = 0;
        for (auto value : reduced) {
            if (value == 0) continue;
            if (((k >> position) & 1) != 0) result ^= value;
            ++position;
        }
        return result;
    }
    unsigned rank() const noexcept { return rank_; }
    bool has_nonempty_zero() const noexcept { return dependent_; }
    void clear() noexcept { basis_.fill(0); rank_ = 0; dependent_ = false; }
    void swap(XorBasis& other) noexcept { basis_.swap(other.basis_); std::swap(rank_, other.rank_); std::swap(dependent_, other.dependent_); }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
