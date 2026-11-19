// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/RollingHash.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rolling_hash.md
//
// API一覧
// [RollingHash]
// RollingHash(s, base): 符号なしバイト列をO(|s|+1)時間・空間で構築。base<2は10、base>modulus-2はmodulus-10に補正。
// modulus: ハッシュの法2^61-1。ハッシュ値は常に[0,modulus)。
// get(l,r): 半開区間[l,r)のハッシュ、whole(): 全体のハッシュをO(1)で返す。空区間は0。
// size(), empty(), base(): 長さ・空判定・補正後の底をO(1)で返す。
// concat(left,right,right_length): 同じ底の正規化済みハッシュを連結してO(1)で返す。right_length<=size()。
// equal(l,r,other,ol,orr): 同じ底の2区間の長さとハッシュをO(1)で比較する。
// lcp(l,other,ol,limit): 同じ底の接尾辞の最長共通接頭辞長をlimit以下でO(log(min(残長,limit)+1))で返す。
// compare(l,r,other,ol,orr): 同じ底の2区間を符号なしバイト辞書順で比較し-1/0/1をO(log(短い方の長さ+1))で返す。
// 比較にはハッシュ衝突による誤判定の可能性がある。検査有効時は不正な範囲にout_of_range、底・ハッシュにinvalid_argument。
// assign(s): 現在の底で再構築し容量・利用可能な累乗表を再利用、O(旧長+|s|+1)上界、強い例外保証。clear(): 空へ戻して底・容量を保持、O(旧長+1)上界、noexcept。
// AVX2対応x86_64では長さ4096以上の構築を並列化。SIRKIT_DISABLE_SIMDで専用処理とCPU判定を除外。
// 元文字列は保持しない。コピーはO(n+1)で独立、代入は容量を再利用し強い例外保証。ムーブ元は空（自己ムーブ代入は保持）。swapはO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROLLING_HASH_DEFINED
#define SIRKIT_ROLLING_HASH_DEFINED

namespace detail {
inline void CheckRollingHashCapacity(std::size_t size, std::size_t maximum) {
    if (size >= maximum / 2) throw std::length_error("RollingHash: capacity overflow");
}
}

class RollingHash {
public:
    static constexpr std::uint64_t modulus = (std::uint64_t{1} << 61) - 1;

    RollingHash(std::string_view s, std::uint64_t base)
        : base_(normalize_base(base)) {
        detail::CheckRollingHashCapacity(s.size(), data_.max_size());
        data_.resize(s.size() + 1);
        power_data_.resize(s.size() + 1);
        const std::size_t offset = s.size() + 1;
        offset_ = offset;
        Entry* const prefixes = data_.data();
        Entry* const powers = power_data_.data();
        powers_ = powers;
        const std::uint64_t factor = base_;
        std::uint64_t hash = 0;
        std::uint64_t power = 1;
        prefixes[0].value = hash;
        powers[0].value = power;
        for (std::size_t i = 0; i < s.size(); ++i) {
            const auto byte = static_cast<unsigned char>(s[i]);
            hash = multiply_add_byte(hash, factor, byte);
            power = multiply(power, factor);
            prefixes[i + 1].value = hash;
            powers[i + 1].value = power;
        }
    }

    void assign(std::string_view s) {
        detail::CheckRollingHashCapacity(s.size(), data_.max_size());
        const std::size_t count = s.size() + 1;
        const bool reuse_powers = power_data_.size() >= count;
        // 両方の確保を先に終え、失敗時も内容と非所有ポインタを保つ。
        data_.reserve(count);
        power_data_.reserve(count);
        data_.resize(count);
        power_data_.resize(count);
        offset_ = count;
        powers_ = power_data_.data();
        Entry* const prefixes = data_.data();
        Entry* const powers = power_data_.data();
        const std::uint64_t factor = base_;
        std::uint64_t hash = 0;
        prefixes[0].value = hash;
        powers[0].value = 1;
        if (reuse_powers) {
            for (std::size_t i = 0; i < s.size(); ++i) {
                hash = multiply_add_byte(hash, factor, static_cast<unsigned char>(s[i]));
                prefixes[i + 1].value = hash;
            }
        } else {
            std::uint64_t power = 1;
            for (std::size_t i = 0; i < s.size(); ++i) {
                hash = multiply_add_byte(hash, factor, static_cast<unsigned char>(s[i]));
                power = multiply(power, factor);
                prefixes[i + 1].value = hash;
                powers[i + 1].value = power;
            }
        }
    }

    void clear() noexcept {
        data_.clear(); power_data_.clear();
        offset_ = 0; powers_ = nullptr;
    }

    RollingHash(const RollingHash& other)
        : data_(other.data_), power_data_(other.power_data_),
          base_(other.base_), offset_(other.offset_),
          powers_(data_.empty() ? nullptr : power_data_.data()) {}

    RollingHash& operator=(const RollingHash& other) {
        if (this != &other) {
            data_.reserve(other.data_.size());
            power_data_.reserve(other.power_data_.size());
            data_.resize(other.data_.size());
            power_data_.resize(other.power_data_.size());
            // Entryはtrivially copyable。独立した配列間で、構築済み要素の表現を写す。
            if (!other.data_.empty()) {
                std::memcpy(data_.data(), other.data_.data(), other.data_.size() * sizeof(Entry));
                std::memcpy(power_data_.data(), other.power_data_.data(), other.power_data_.size() * sizeof(Entry));
            }
            base_ = other.base_;
            offset_ = other.offset_;
            powers_ = data_.empty() ? nullptr : power_data_.data();
        }
        return *this;
    }

    RollingHash(RollingHash&& other) noexcept
        : data_(std::move(other.data_)), power_data_(std::move(other.power_data_)),
          base_(other.base_), offset_(other.offset_),
          powers_(other.powers_) {
        other.data_.clear();
        other.power_data_.clear();
        other.offset_ = 0;
        other.powers_ = nullptr;
    }

    RollingHash& operator=(RollingHash&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            power_data_ = std::move(other.power_data_);
            base_ = other.base_;
            offset_ = other.offset_;
            powers_ = other.powers_;
            other.data_.clear();
            other.power_data_.clear();
            other.offset_ = 0;
            other.powers_ = nullptr;
        }
        return *this;
    }

    void swap(RollingHash& other) noexcept {
        data_.swap(other.data_);
        power_data_.swap(other.power_data_);
        std::swap(base_, other.base_);
        std::swap(offset_, other.offset_);
        std::swap(powers_, other.powers_);
    }

    friend void swap(RollingHash& left, RollingHash& right) noexcept {
        left.swap(right);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return offset_ == 0 ? 0 : offset_ - 1;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] std::uint64_t base() const noexcept {
        return base_;
    }

    [[nodiscard]] std::uint64_t whole() const noexcept {
        return offset_ == 0 ? 0 : data_[offset_ - 1].value;
    }

    [[nodiscard]] std::uint64_t get(std::size_t l, std::size_t r) const {
        return get_unchecked(l, r);
    }

    [[nodiscard]] std::uint64_t concat(std::uint64_t left,
                                     std::uint64_t right,
                                     std::size_t right_length) const {
        return add(right_length == 0 ? left
                                    : multiply(left, powers_[right_length].value),
                   right);
    }

    [[nodiscard]] bool equal(std::size_t l, std::size_t r,
                             const RollingHash& other,
                             std::size_t ol, std::size_t orr) const {
        const std::size_t length = r - l;
        if (length != orr - ol) return false;
        if (length == 0) return true;
        const std::uint64_t start = data_[l].value + modulus - other.data_[ol].value;
        const std::uint64_t delta = start >= modulus ? start - modulus : start;
        if (delta == 0) return data_[r].value == other.data_[orr].value;
        // 開始prefix差の1積だけで2区間のハッシュ差を比較する。
        const std::uint64_t difference = data_[r].value + 5 * modulus -
            multiply_raw(delta, powers_[length].value) - other.data_[orr].value;
        return ((difference & modulus) + (difference >> 61)) == modulus;
    }

    [[nodiscard]] std::size_t lcp(
        std::size_t l, const RollingHash& other, std::size_t ol,
        std::size_t limit = std::numeric_limits<std::size_t>::max()) const {
        return lcp_unchecked(l, other, ol,
                             std::min({size() - l, other.size() - ol, limit}));
    }

    [[nodiscard]] int compare(std::size_t l, std::size_t r,
                              const RollingHash& other,
                              std::size_t ol, std::size_t orr) const {
        const std::size_t length = r - l;
        const std::size_t other_length = orr - ol;
        const std::size_t common = lcp_unchecked(
            l, other, ol, std::min(length, other_length));
        if (common == std::min(length, other_length)) {
            return (length > other_length) - (length < other_length);
        }
        const Entry* const left = data_.data() + l;
        const Entry* const right = other.data_.data() + ol;
        const std::uint64_t start = left[0].value + modulus - right[0].value;
        const std::uint64_t delta = start >= modulus ? start - modulus : start;
        // lcp_uncheckedが上限未満なら、その次の長さのハッシュは不一致。
        // 一致したprefixの次1byteの差だけを1回の乗算で求める。
        const std::size_t length_next = common + 1;
        const std::uint64_t difference = left[length_next].value + 5 * modulus -
            multiply_raw(delta, powers_[length_next].value) - right[length_next].value;
        const std::uint64_t folded = (difference & modulus) + (difference >> 61);
        return 1 - static_cast<int>((folded >> 60) & 1U) * 2;
    }

private:
    struct Entry {
        std::uint64_t value;
        // 構築ループで全要素を設定し、不要なゼロ初期化を避ける。
        Entry() noexcept {}
    };


    std::vector<Entry> data_, power_data_;
    std::uint64_t base_;
    std::size_t offset_ = 0;
    const Entry* powers_ = nullptr; // power_data_の累乗領域への非所有ポインタ。

    static constexpr std::uint64_t normalize_base(std::uint64_t base) noexcept {
        return base < 2 ? 10 : (base > modulus - 2 ? modulus - 10 : base);
    }

    static constexpr std::uint64_t add(std::uint64_t a,
                                       std::uint64_t b) noexcept {
        const std::uint64_t sum = a + b;
        return sum >= modulus ? sum - modulus : sum;
    }

    static constexpr std::uint64_t multiply_raw(std::uint64_t a,
                                            std::uint64_t b) noexcept {
        // 32bitの4積を2^61=1、2^64=8として直接折り返す。
        const std::uint64_t a0 = static_cast<std::uint32_t>(a);
        const std::uint64_t b0 = static_cast<std::uint32_t>(b);
        const std::uint64_t a1 = a >> 32;
        const std::uint64_t b1 = b >> 32;
        const std::uint64_t low = a0 * b0;
        const std::uint64_t cross = a0 * b1 + a1 * b0;
        constexpr std::uint64_t mask29 = (std::uint64_t{1} << 29) - 1;
        const std::uint64_t sum = (low & modulus) + (low >> 61) +
                                  ((cross & mask29) << 32) + (cross >> 29) +
                                  ((a1 * b1) << 3);
        const std::uint64_t folded = sum;
        return folded;
    }

    static constexpr std::uint64_t multiply(std::uint64_t a, std::uint64_t b) noexcept {
        std::uint64_t folded = multiply_raw(a, b);
        folded = (folded & modulus) + (folded >> 61);
        return folded >= modulus ? folded - modulus : folded;
    }

    // byteは0..255。乗算結果への加算を正規化前にまとめる。
    static constexpr std::uint64_t multiply_add_byte(
        std::uint64_t a, std::uint64_t b, unsigned char byte) noexcept {
        const std::uint64_t sum = multiply_raw(a, b) + byte;
        const std::uint64_t folded = (sum & modulus) + (sum >> 61);
        return folded >= modulus ? folded - modulus : folded;
    }

    [[nodiscard]] std::uint64_t get_unchecked(std::size_t l,
                                             std::size_t r) const noexcept {
        if (l == r) {
            return 0;
        }
        const std::uint64_t right = data_[r].value;
        const std::uint64_t left = multiply_raw(data_[l].value, powers_[r - l].value);
        // raw積は4p未満、右辺全体は5p未満で64bitに収まる。
        const std::uint64_t difference = right + 4 * modulus - left;
        const std::uint64_t folded = (difference & modulus) + (difference >> 61);
        return folded >= modulus ? folded - modulus : folded;
    }

    [[nodiscard]] std::size_t lcp_unchecked(std::size_t l,
                                           const RollingHash& other,
                                           std::size_t ol,
                                           std::size_t limit) const noexcept {
        if (limit == 0) return 0;
        const Entry* const left = data_.data() + l;
        const Entry* const right = other.data_.data() + ol;
        const Entry* const powers = powers_;
        const std::uint64_t start = left[0].value + modulus - right[0].value;
        const std::uint64_t delta = start >= modulus ? start - modulus : start;
        // 開始接頭辞が同じなら、端点の比較だけで区間ハッシュを比較できる。
        if (delta == 0) {
            if (left[limit].value == right[limit].value) return limit;
            std::size_t low = 0, high = limit - 1;
            while (low < high) {
                const std::size_t middle = high - (high - low) / 2;
                if (left[middle].value == right[middle].value) low = middle;
                else high = middle - 1;
            }
            return low;
        }
        const auto equal_prefix = [&](std::size_t length) noexcept {
            // raw積は4p未満。0<difference<6pで、全中間値は64bitに収まる。
            const std::uint64_t difference = left[length].value + 5 * modulus -
                multiply_raw(delta, powers[length].value) - right[length].value;
            return ((difference & modulus) + (difference >> 61)) == modulus;
        };
        if (!equal_prefix(1)) return 0;
        if (equal_prefix(limit)) return limit;
        // 長さ1だけ一致する照会は、広い範囲を二分探索しない。
        if (!equal_prefix(2)) return 1;
        std::size_t low = 2, high = limit - 1;
        while (low < high) {
            const std::size_t middle = high - (high - low) / 2;
            if (equal_prefix(middle)) low = middle;
            else high = middle - 1;
        }
        return low;
    }

};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
