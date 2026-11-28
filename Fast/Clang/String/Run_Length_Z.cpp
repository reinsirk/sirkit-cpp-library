// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Run_Length_Z.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_length_z.md
//
// API一覧
// RunLengthZ::Run{byte,count}: byteがcount回連続する区間。countはuint64_t。
// RunLengthZ(runs): 連長圧縮された入力からO(n+1)時間・領域で構築。0回のrunは飛ばし、隣接同値は結合。
// size(): 展開後の長さ。O(1)。合計長はUINT64_MAX以下、結合後の区間数はINT_MAX以下。
// prefix_at(position): 展開後の0始まりの位置から始まる接尾辞との共通接頭辞長。position==size()なら0。O(log(n+2))。
// sum_prefix_lengths(modulus=1000000007): 全開始位置のZ値の総和を正のuint32_tの法で返す。O(n+1)、照会追加O(1)。
// nは入力区間数。同梱のZ AlgorithmはDocument/z_algorithm.mdを参照。
//
// [同梱のZ Algorithm]
// z_algorithm(const std::string&)/z_algorithm<T>(const std::vector<T>&): ACL形式のZ配列を返す。
// 追加のz_algorithm(sequence)は任意の添字アクセス可能な列に対応。空なら{}、非空ならz[0]=n。
// PrefixMatches(pattern,text): textの各開始位置とpatternの共通接頭辞長を返す。
// ZMatches(pattern,text): 完全一致する開始位置を昇順に返す。空patternは0..text.size()。
// size()/添字アクセスと同値関係の==が使える列が前提。比較O(1)ならZはO(n)、後2者はO(n+m)時間。
// 各列長はINT_MAX未満。Z/PrefixMatchesの領域は返却値とpattern長、ZMatchesはこれに一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_Z_ALGORITHM_IMPL_DEFINED
#define SIRKIT_Z_ALGORITHM_IMPL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class Sequence>
std::vector<int> z_algorithm_impl(const Sequence& s) {
    if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("z_algorithm: sequence too long");
    const int n = static_cast<int>(s.size());
    std::vector<int> z(s.size());
    if (n == 0) return z;
    z[0] = n;
    int i = 1, length = 0;
    while (i < n) {
        if (length == 0) {
            // 未処理位置のz[i]は零初期化済み。不一致なら書き直さない。
            if (!static_cast<bool>(s[0] == s[i])) { ++i; continue; }
            length = 1;
        }
        while (length < n - i && s[length] == s[i + length]) ++length;
        z[i] = length;
        // 末尾まで一致した箱では、残りも確定済みの値だけで埋められる。
        if (length == n - i) {
            if (i == 1) {
                for (++i; i < n; ++i) z[i] = n - i;
            } else {
                const int left = i;
                for (++i; i < n; ++i) z[i] = std::min(n - i, z[i - left]);
            }
            break;
        }
        // 箱内で不一致まで確定する位置を写し、次の位置へ一致長を持ち越す。
        int advance = 1;
        while (advance < length && z[advance] < length - advance) {
            z[i + advance] = z[advance];
            ++advance;
        }
        i += advance;
        length = std::max(0, length - advance);
    }
    return z;
}
}
#endif

#ifndef SIRKIT_Z_ALGORITHM_DEFINED
#define SIRKIT_Z_ALGORITHM_DEFINED
template<class T>
std::vector<int> z_algorithm(const std::vector<T>& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
inline std::vector<int> z_algorithm(const std::string& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> z_algorithm(const Sequence& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
#endif

#ifndef SIRKIT_Z_MATCHES_DEFINED
#define SIRKIT_Z_MATCHES_DEFINED
template <class Pattern, class Text>
std::vector<int> PrefixMatches(const Pattern& pattern, const Text& text) {
    if (text.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("PrefixMatches: text too long");
    const auto z = z_algorithm(pattern);
    const int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
    std::vector<int> matches(text.size());
    int left = 0, right = 0;
    for (int i = 0; i < n; ++i) {
        int length = i < right ? std::min(right - i, z[i - left]) : 0;
        while (length < m && length < n - i && pattern[length] == text[i + length]) ++length;
        matches[i] = length;
        if (i + length > right) { left = i; right = i + length; }
    }
    return matches;
}

template <class Pattern, class Text>
std::vector<int> ZMatches(const Pattern& pattern, const Text& text) {
    auto lengths = PrefixMatches(pattern, text);
    std::vector<int> positions;
    const int m = static_cast<int>(pattern.size());
    for (int i = 0; i < static_cast<int>(lengths.size()); ++i)
        if (lengths[i] == m) positions.push_back(i);
    if (m == 0) positions.push_back(static_cast<int>(text.size()));
    return positions;
}
#endif

#ifndef SIRKIT_RUN_LENGTH_Z_DEFINED
#define SIRKIT_RUN_LENGTH_Z_DEFINED

class RunLengthZ
{
public:
    struct Run
    {
        std::uint8_t byte;
        std::uint64_t count;
        friend constexpr bool operator==(const Run &left, const Run &right) noexcept
        {
            return left.byte == right.byte && left.count == right.count;
        }
    };

    explicit RunLengthZ(std::span<const Run> input)
    {
        std::vector<Run> packed;
        std::uint64_t total = 0;
        for (const auto run : input)
        {
            if (run.count == 0)
            {
                continue;
            }
            if (run.count > std::numeric_limits<std::uint64_t>::max() - total)
            {
                throw std::length_error("RunLengthZ: expanded length overflows");
            }
            total += run.count;
            if (!packed.empty() && packed.back().byte == run.byte)
            {
                packed.back().count += run.count;
            }
            else
            {
                packed.push_back(run);
            }
        }
        if (packed.empty())
        {
            return;
        }
        runs_.reserve(packed.size());
        std::uint64_t end = 0;
        for (const auto run : packed)
        {
            end += run.count;
            runs_.push_back({run, end, 0});
        }

        const auto first = packed.front();
        const auto equal_runs = z_algorithm(std::span<const Run>(packed).subspan(1));
        for (std::size_t i = 0; i < equal_runs.size(); ++i)
        {
            if (packed[i].byte != first.byte || packed[i].count < first.count)
            {
                continue;
            }
            // 最初の連続区間の長さが同じとなる開始位置だけ、次の区間へ進める。
            const auto matched = static_cast<std::size_t>(equal_runs[i]);
            auto matched_bytes = runs_[matched].end - first.count;
            const auto next = i + matched + 1;
            if (next < packed.size() && packed[next].byte == packed[matched + 1].byte)
            {
                matched_bytes += std::min(packed[next].count, packed[matched + 1].count);
            }
            runs_[i].extension = matched_bytes;
        }
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return runs_.empty() ? 0 : runs_.back().end;
    }

    [[nodiscard]] std::uint64_t prefix_at(std::uint64_t position) const
    {
        if (position == size())
        {
            return 0;
        }
        const auto it = std::upper_bound(runs_.begin(), runs_.end(), position,
                                         [](std::uint64_t value, const Summary &run) { return value < run.end; });
        const auto first = runs_.front().run;
        if (it->run.byte != first.byte)
        {
            return 0;
        }
        const auto remaining = it->end - position;
        return remaining == first.count ? first.count + it->extension : std::min(remaining, first.count);
    }

    [[nodiscard]] std::uint32_t sum_prefix_lengths(std::uint32_t modulus = 1'000'000'007) const
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("RunLengthZ: modulus must be positive");
        }
        const auto product = [&](std::uint64_t left, std::uint64_t right) {
            return (left % modulus) * (right % modulus) % modulus;
        };
        std::uint64_t result = 0;
        for (const auto &summary : runs_)
        {
            const auto first = runs_.front().run;
            if (summary.run.byte != first.byte)
            {
                continue;
            }
            const auto common = std::min(summary.run.count, first.count);
            const auto triangle = product(common / 2 + (common % 2), common | std::uint64_t{1});
            const auto plateau = product(summary.run.count - common, common);
            result = (result + triangle + plateau + summary.extension % modulus) % modulus;
        }
        return static_cast<std::uint32_t>(result);
    }

private:
    struct Summary
    {
        Run run;
        std::uint64_t end;
        std::uint64_t extension;
    };
    std::vector<Summary> runs_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
