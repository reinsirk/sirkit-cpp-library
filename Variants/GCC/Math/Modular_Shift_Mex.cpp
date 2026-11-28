// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Modular_Shift_Mex.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_mex.md
// 参考（円環上の連続区間と床関数和による差分）: https://atcoder.jp/contests/abc465/editorial/22561
//
// API一覧
// ModularShiftMex(count,modulus,step,start=0): 空集合で初期化。uint32_tのcount/modulus、負のint64_tのstep/startも可。
// insert(value)/erase(value): [0,modulus)の値を追加/削除。集合が変わったときtrue。重複は保持しない。
// sum(): k=0..count-1で、集合の全要素をstart+step*kだけ法modulusでずらしたmexの総和。uint64_tで厳密。
// size()/clear(): 要素数O(1)/集合を空にするO(r+1)。移動元は同じ設定の空集合。自己代入は内容を保つ。
// 一直線に並べたときの占有区間数をrとして、追加削除はO(log(r+2)+log(modulus+1))時間・合計O(r+1)領域。
// 構築とsumはO(1)、コピーはO(r+1)。法0はinvalid_argument、範囲外の値はout_of_range。
//
// [同梱のFloor Sum]
// floor_sum(n,m,a,b): i=0..n-1 の floor((a*i+b)/m) の総和。a,bは負でもよい。
// 0<=n<2^32、1<=m<2^32。ACLと同じく結果は2^64を法とする符号付き64ビット値。
// min_of_mod_of_linear(n,m,a,b): i=0..n-1 の (a*i+b) mod m の最小値。1<=n<2^32。
// 両方ともO(log(m+1))時間・O(1)領域。検査有効時、n,mが範囲外ならstd::invalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <map>
#include <type_traits>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FLOOR_SUM_DEFINED
#define SIRKIT_FLOOR_SUM_DEFINED

inline std::int64_t floor_sum(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n < 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("floor_sum: n or m");
    }
#endif
    if (n == 0)
    {
        return 0;
    }
    const auto count = static_cast<std::uint64_t>(n);
    const auto triangular = count * (count - 1) / 2;
    std::uint64_t sum = 0;
    if (a < 0)
    {
        const auto remainder = a % m;
        const auto quotient = a / m;
        sum += triangular * static_cast<std::uint64_t>(quotient - (remainder != 0));
        a = remainder == 0 ? 0 : remainder + m;
    }
    if (b < 0)
    {
        const auto remainder = b % m;
        const auto quotient = b / m;
        sum += count * static_cast<std::uint64_t>(quotient - (remainder != 0));
        b = remainder == 0 ? 0 : remainder + m;
    }
    auto length = count;
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = static_cast<std::uint64_t>(a);
    auto offset = static_cast<std::uint64_t>(b);
    while (true)
    {
        if (slope >= modulus)
        {
            sum += (length * (length - 1) / 2) * (slope / modulus);
            slope %= modulus;
        }
        if (offset >= modulus)
        {
            sum += length * (offset / modulus);
            offset %= modulus;
        }
        const auto limit = slope * length + offset;
        if (limit < modulus)
        {
            break;
        }
        length = limit / modulus; // 格子点を横方向から数え直す。
        offset = limit % modulus;
        std::swap(modulus, slope);
    }
    return std::bit_cast<std::int64_t>(sum);
}

inline std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m, std::int64_t a, std::int64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::int64_t kLimit = std::int64_t{1} << 32;
    if (n <= 0 || n >= kLimit || m <= 0 || m >= kLimit)
    {
        throw std::invalid_argument("min_of_mod_of_linear: n or m");
    }
#endif
    const auto normalize = [](std::int64_t x, std::int64_t modulus)
    {
        x %= modulus;
        return static_cast<std::uint64_t>(x < 0 ? x + modulus : x);
    };
    auto length = static_cast<std::uint64_t>(n);
    auto modulus = static_cast<std::uint64_t>(m);
    auto slope = normalize(a, m);
    auto offset = normalize(b, m);
    auto minimum = offset;
    while (minimum != 0 && slope != 0)
    {
        if (slope * 2 > modulus) // 逆から走査して次の法を半分以下にする。
        {
            offset = (slope * (length - 1) + offset) % modulus;
            slope = modulus - slope;
        }
        minimum = std::min(minimum, offset);
        const auto wraps = (slope * (length - 1) + offset) / modulus;
        if (wraps == 0)
        {
            break;
        }
        const auto reverse_step = (slope - modulus % slope) % slope; // 次の法は slope。
        length = wraps;
        offset = (offset % slope + reverse_step) % slope; // y回目の折り返しは (offset - modulus*y) mod slope。
        modulus = slope;
        slope = reverse_step;
        minimum = std::min(minimum, offset);
    }
    return static_cast<std::int64_t>(minimum);
}

#endif

#ifndef SIRKIT_MODULAR_SHIFT_MEX_DEFINED
#define SIRKIT_MODULAR_SHIFT_MEX_DEFINED

class ModularShiftMex
{
    struct Progression
    {
        std::uint32_t count;
        std::uint32_t modulus;
        std::uint32_t slope;
        std::uint32_t phase;
    };
    using Runs = std::map<std::uint32_t, std::uint32_t>;

public:
    ModularShiftMex(std::uint32_t count, std::uint32_t modulus, std::int64_t step, std::int64_t start = 0)
        : progression_{count, modulus, Normalize(step, modulus), Normalize(start, modulus)}
    {
    }

    ModularShiftMex(const ModularShiftMex &) = default;

    ModularShiftMex(ModularShiftMex &&other) noexcept(std::is_nothrow_default_constructible_v<Runs>)
        : progression_(other.progression_)
    {
        Swap(other);
    }

    ModularShiftMex &operator=(ModularShiftMex other) noexcept
    {
        Swap(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t sum() const noexcept
    {
        return total_;
    }

    [[nodiscard]] std::uint32_t size() const noexcept
    {
        return size_;
    }

    void clear() noexcept
    {
        runs_.clear();
        size_ = 0;
        total_ = 0;
    }

    bool insert(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before != runs_.end() && before->second > value)
        {
            return false;
        }
        const auto modulus = progression_.modulus;
        const auto next_total = size_ + 1 == modulus
                                    ? std::uint64_t{modulus} * progression_.count
                                    : total_ + AddedContribution(value, before, after);
        const auto next_value = value + 1;
        const bool touch_left = before != runs_.end() && before->second == value;
        const bool touch_right = after != runs_.end() && after->first == next_value;
        if (touch_left)
        {
            if (touch_right)
            {
                before->second = after->second;
                runs_.erase(after);
            }
            else
            {
                before->second = next_value;
            }
        }
        else if (touch_right)
        {
            MoveStart(after, value);
        }
        else
        {
            runs_.emplace_hint(after, value, next_value);
        }
        ++size_;
        total_ = next_total;
        return true;
    }

    bool erase(std::uint32_t value)
    {
        Check(value);
        const auto after = runs_.upper_bound(value);
        const auto before = Previous(after);
        if (before == runs_.end() || before->second <= value)
        {
            return false;
        }
        const auto next_value = value + 1;
        if (before->first == value)
        {
            if (before->second == next_value)
            {
                runs_.erase(before);
            }
            else
            {
                MoveStart(before, next_value);
            }
        }
        else
        {
            if (before->second != next_value)
            {
                runs_.emplace_hint(after, next_value, before->second);
            }
            before->second = value;
        }
        --size_;
        if (size_ + 1 == progression_.modulus)
        {
            total_ = MissingOneSum(value);
        }
        else
        {
            const auto next = runs_.upper_bound(value);
            total_ -= AddedContribution(value, Previous(next), next);
        }
        return true;
    }

private:
    void Swap(ModularShiftMex &other) noexcept
    {
        std::swap(progression_, other.progression_);
        runs_.swap(other.runs_);
        std::swap(size_, other.size_);
        std::swap(total_, other.total_);
    }

    static std::uint32_t Normalize(std::int64_t value, std::uint32_t modulus)
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("ModularShiftMex: zero modulus");
        }
        const auto remainder = value % static_cast<std::int64_t>(modulus);
        return static_cast<std::uint32_t>(remainder < 0 ? remainder + modulus : remainder);
    }

    void Check(std::uint32_t value) const
    {
        if (value >= progression_.modulus)
        {
            throw std::out_of_range("ModularShiftMex: residue outside modulus");
        }
    }

    Runs::iterator Previous(Runs::iterator after) noexcept
    {
        return after == runs_.begin() ? runs_.end() : std::prev(after);
    }

    void MoveStart(Runs::iterator run, std::uint32_t start)
    {
        auto node = runs_.extract(run);
        node.key() = start;
        runs_.insert(std::move(node));
    }

    [[nodiscard]] std::uint64_t FloorWithOffset(std::int64_t offset) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        return static_cast<std::uint64_t>(floor_sum(count, modulus, slope, offset + phase));
    }

    [[nodiscard]] std::uint64_t MissingOneSum(std::uint32_t missing) const
    {
        const auto [count, modulus, slope, phase] = progression_;
        const std::uint64_t length = count;
        const auto linear = (length * (length - 1) / 2) * slope + length * (std::uint64_t{phase} + missing);
        return linear - modulus * FloorWithOffset(missing);
    }

    [[nodiscard]] std::uint64_t AddedContribution(
        std::uint32_t value, Runs::const_iterator before, Runs::const_iterator after) const
    {
        auto left = static_cast<std::int64_t>(value);
        auto right = left + 1;
        if (before != runs_.end() && before->second == value)
        {
            left = before->first;
        }
        if (after != runs_.end() && after->first == value + 1)
        {
            right = after->second;
        }
        const auto modulus = progression_.modulus;
        if (!runs_.empty())
        {
            if (left == 0 && runs_.rbegin()->second == modulus)
            {
                left -= modulus - runs_.rbegin()->first;
            }
            if (right == modulus && runs_.begin()->first == 0)
            {
                right += runs_.begin()->second;
            }
        }

        // 左端からvalueの間に0が移動した回だけ、mexの終点がvalueからrightへ動く。
        const auto occurrences = FloorWithOffset(value) - FloorWithOffset(left - 1);
        return occurrences * static_cast<std::uint64_t>(right - value);
    }

    Progression progression_;
    Runs runs_; // [first,second)の占有区間。円環をまたぐときは端の2区間に分ける。
    std::uint32_t size_ = 0;
    std::uint64_t total_ = 0;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
