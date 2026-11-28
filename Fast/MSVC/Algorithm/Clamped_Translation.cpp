// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Clamped_Translation.cpp
// 参考（操作の合成）: https://atcoder.jp/contests/abc196/tasks/abc196_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/clamped_translation.md
//
// API一覧
// ClampedTranslation(): 恒等関数。
// then_add(delta): 最後にx+deltaを追加する。
// then_chmax(low): 最後にmax(x,low)を追加する。
// then_chmin(high): 最後にmin(x,high)を追加する。
// then(after) const: この写像を適用した後にafterを適用する新しい写像を返す。
// apply(x) const: 合成した写像の値。
// すべてO(1)時間・領域。有限の境界、合計の加算量、返り値はint64_tに収まること。
// 最終結果を境界内に丸められる場合、x+deltaの途中の範囲超過は許される。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CLAMPED_TRANSLATION_DEFINED
#define SIRKIT_CLAMPED_TRANSLATION_DEFINED

class ClampedTranslation
{
    using Value = std::int64_t;
    using Bound = std::optional<Value>;

    Value shift_ = 0;
    Bound lower_;
    Bound upper_;

    ClampedTranslation(Value shift, Bound lower, Bound upper) noexcept
        : shift_(shift), lower_(lower), upper_(upper)
    {
    }

    static Value evaluate(Value value, Value shift, Bound below, Bound above)
    {
        constexpr Value kMin = std::numeric_limits<Value>::min();
        constexpr Value kMax = std::numeric_limits<Value>::max();
        if (shift > 0 && value > kMax - shift)
        {
            if (above)
            {
                return *above;
            }
        }
        if (shift < 0 && value < kMin - shift)
        {
            if (below)
            {
                return *below;
            }
        }

        // 符号なし加算で表現し、検査を省いた構成でも符号付きオーバーフローを起こさない。
        Value translated = static_cast<Value>(static_cast<std::uint64_t>(value) + static_cast<std::uint64_t>(shift));
        if (below)
        {
            translated = std::max(translated, *below);
        }
        if (above)
        {
            translated = std::min(translated, *above);
        }
        return translated;
    }

public:
    ClampedTranslation() = default;

    [[nodiscard]] Value apply(Value value) const
    {
        return evaluate(value, shift_, lower_, upper_);
    }

    [[nodiscard]] ClampedTranslation then(const ClampedTranslation &after) const
    {
        const Bound lower = lower_ ? Bound{after.apply(*lower_)} : after.lower_;
        const Bound upper = upper_ ? Bound{after.apply(*upper_)} : after.upper_;
        if (lower && lower == upper)
        {
            return {0, lower, upper};
        }
        return {evaluate(shift_, after.shift_, {}, {}), lower, upper};
    }

    ClampedTranslation &then_add(Value difference)
    {
        *this = then({difference, {}, {}});
        return *this;
    }

    ClampedTranslation &then_chmax(Value lower)
    {
        *this = then({0, lower, {}});
        return *this;
    }

    ClampedTranslation &then_chmin(Value upper)
    {
        *this = then({0, {}, upper});
        return *this;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
