// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/coordinate_compression.md
//
// API一覧
// CoordinateCompression(values,compare={}): 登録値をcompare順に並べ、等価な値をまとめる。O(n log(n+1))時間、O(n)領域。
// 既定はstd::less<>。valuesにはvector<T>またはspan<const T>を渡す。既定構築は空。
// lower_bound(x)/upper_bound(x): 比較順でxと同じ位置以降/xより後の最初の添字。なければsize()。各O(log(u+1))。
// find(x): 等価な登録値の添字を返す。なければnullopt。O(log(u+1))。
// value(i): 登録値への参照を返す。検査有効時は範囲外でout_of_range。O(1)。
// values(): 登録値のconst span。size()/empty(): 異なる値の数/空か。各O(1)。
// uは等価類の数。コピー・ムーブ構築はO(1)、登録値を共有する。参照は所有するオブジェクトの寿命内で使う。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COORDINATE_COMPRESSION_DEFINED
#define SIRKIT_COORDINATE_COMPRESSION_DEFINED

template <class T, class Compare = std::less<>>
    requires(!std::same_as<T, bool>)
class CoordinateCompression
{
    struct State
    {
        std::vector<T> values;
        [[no_unique_address]] Compare compare;

        State(std::vector<T> source, Compare order) : values(std::move(source)), compare(std::move(order))
        {
            const auto before = std::cref(compare);
            std::ranges::sort(values, before);
            auto kept = values.begin();
            for (auto current = values.begin(); current != values.end(); ++current)
            {
                // 整列済みなので、直前の代表値より後ろの値だけを残す。
                if (kept != values.begin() && !before(*(kept - 1), *current))
                {
                    continue;
                }
                if (kept != current)
                {
                    *kept = std::move(*current);
                }
                ++kept;
            }
            values.erase(kept, values.end());
        }
    };

public:
    CoordinateCompression() = default;

    explicit CoordinateCompression(std::vector<T> values, Compare compare = {})
        : state_(std::make_shared<const State>(std::move(values), std::move(compare)))
    {
    }

    explicit CoordinateCompression(std::span<const T> values, Compare compare = {})
        : CoordinateCompression(std::vector<T>(values.begin(), values.end()), std::move(compare))
    {
    }

    [[nodiscard]] std::span<const T> values() const noexcept
    {
        return state_ ? std::span<const T>(state_->values) : std::span<const T>{};
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values().size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return values().empty();
    }

    template <class Key>
    [[nodiscard]] std::size_t lower_bound(const Key &value) const
    {
        if (!state_)
        {
            return 0;
        }
        const auto first = state_->values.begin(), last = state_->values.end();
        return static_cast<std::size_t>(std::lower_bound(first, last, value, std::cref(state_->compare)) - first);
    }

    template <class Key>
    [[nodiscard]] std::size_t upper_bound(const Key &value) const
    {
        if (!state_)
        {
            return 0;
        }
        const auto first = state_->values.begin(), last = state_->values.end();
        return static_cast<std::size_t>(std::upper_bound(first, last, value, std::cref(state_->compare)) - first);
    }

    template <class Key>
    [[nodiscard]] std::optional<std::size_t> find(const Key &value) const
    {
        const auto id = lower_bound(value);
        if (id == size() || state_->compare(value, state_->values[id]))
        {
            return std::nullopt;
        }
        return id;
    }

    [[nodiscard]] const T &value(std::size_t id) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= size())
        {
            throw std::out_of_range("CoordinateCompression: rank out of range");
        }
#endif
        return values()[id];
    }

private:
    std::shared_ptr<const State> state_;
};

template <class T, std::size_t Extent, class Compare = std::less<>>
CoordinateCompression(std::span<T, Extent>, Compare = {}) -> CoordinateCompression<std::remove_const_t<T>, Compare>;

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
