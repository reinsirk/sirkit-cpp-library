// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Combination/Q_Binomial.cpp
// 参考（q-Lucasの定理）: https://www.combinatorics.org/ojs/index.php/eljc/article/download/v19i3p1/pdf/
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/q_binomial.md
//
// API一覧
// [PrimeQBinomial]
// PrimeQBinomial(prime, q, max_n): 素数primeを法として、0<=q<prime、0<=max_n<primeを指定する。
// COM(n,k): 0<=n<=max_nのq二項係数を返す。k>nなら0。引数と結果はuint32_t。
// 前計算O(max_n+log prime)時間・O(max_n+1)領域。COMはO(1)時間。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PRIME_Q_BINOMIAL_DEFINED
#define SIRKIT_PRIME_Q_BINOMIAL_DEFINED
class PrimeQBinomial
{
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    struct Factorials
    {
        std::vector<u32> forward;
        std::vector<u32> inverse;
    };

    u32 prime_;
    u32 q_;
    [[maybe_unused]] u32 max_n_;
    // order_ == 0のときは、前計算範囲でqの累乗が1に戻らない。
    u32 order_ = 0;
    Factorials q_factorials_;
    Factorials ordinary_;

    u32 multiply(u32 left, u32 right) const noexcept
    {
        return static_cast<u32>(static_cast<u64>(left) * right % prime_);
    }

    u32 invert(u32 base) const noexcept
    {
        u32 result = 1;
        for (u32 exponent = prime_ - 2; exponent > 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                result = multiply(result, base);
            }
            base = multiply(base, base);
        }
        return result;
    }

    // [i]_qの分母(q-1)は二項係数で消えるので、積にはq^i-1を使う。
    void prepare_q(u32 max_n)
    {
        u32 power = 1;
        auto &table = q_factorials_;
        table.forward.reserve(static_cast<std::size_t>(max_n) + 1);
        table.forward.push_back(1);
        for (u32 i = 1; i <= max_n; ++i)
        {
            const auto next = multiply(power, q_);
            if (next == 1)
            {
                order_ = i;
                break;
            }
            power = next;
            table.forward.push_back(multiply(table.forward.back(), power - 1));
        }
        table.inverse.resize(table.forward.size());
        table.inverse.back() = invert(table.forward.back());
        const auto q_inverse = invert(q_);
        for (std::size_t i = table.forward.size() - 1; i != 0; --i)
        {
            table.inverse[i - 1] = multiply(table.inverse[i], power - 1);
            power = multiply(power, q_inverse);
        }
    }

    void prepare_ordinary(u32 limit)
    {
        auto &table = ordinary_;
        table.forward.resize(static_cast<std::size_t>(limit) + 1);
        table.inverse.resize(table.forward.size());
        table.forward[0] = 1;
        for (u32 i = 1; i <= limit; ++i)
        {
            table.forward[i] = multiply(table.forward[i - 1], i);
        }
        table.inverse[limit] = invert(table.forward[limit]);
        for (u32 i = limit; i != 0; --i)
        {
            table.inverse[i - 1] = multiply(table.inverse[i], i);
        }
    }

    u32 choose(const Factorials &table, u32 n, u32 k) const noexcept
    {
        return multiply(multiply(table.forward[n], table.inverse[k]), table.inverse[n - k]);
    }

public:
    PrimeQBinomial(u32 prime, u32 q, u32 max_n) : prime_(prime), q_(q), max_n_(max_n)
    {
        if (q == 0)
        {
            return;
        }
        if (q == 1)
        {
            order_ = 1;
        }
        else
        {
            prepare_q(max_n);
        }
        if (order_ != 0)
        {
            prepare_ordinary(max_n / order_);
        }
    }

    [[nodiscard]] u32 COM(u32 n, u32 k) const
    {
        if (k > n)
        {
            return 0;
        }
        if (q_ == 0)
        {
            return 1;
        }
        if (order_ == 0)
        {
            return choose(q_factorials_, n, k);
        }
        if (order_ == 1)
        {
            return choose(ordinary_, n, k);
        }
        const auto above = n % order_;
        const auto below = k % order_;
        if (below > above)
        {
            return 0;
        }
        return multiply(choose(ordinary_, n / order_, k / order_), choose(q_factorials_, above, below));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
