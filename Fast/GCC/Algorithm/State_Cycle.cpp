// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/State_Cycle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/state_cycle.md
//
// API一覧
// StateCycle{prefix, period}: 周期が始まる0始まりの位置と最短周期。
// find_state_cycle(start, next, transition_limit, equal={}): Brent法でStateCycleを求める。
// nextをtransition_limit回以内に呼び、結果まで求まらなければnullopt。周期を発見できる列でO(min(limit, prefix+period)+1)時間。
// fold_state_sequence(start, count, next, project, combine, identity, equal={}): 先頭count状態を順番に集約する。
// combineは結合的、identityは単位元。周期がある列でO(min(count, prefix+period)+log(count+1)+1)時間。
// 周期がなくても指定した個数まで処理できる。各O(1)追加領域。状態・集約値のコピーと関数呼び出しはO(1)とする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATE_CYCLE_DEFINED
#define SIRKIT_STATE_CYCLE_DEFINED

struct StateCycle
{
    std::uint64_t prefix;
    std::uint64_t period;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template<class State>
class BrentCycleTracker
{
public:
    explicit BrentCycleTracker(const State &initial) : checkpoint_(initial)
    {
    }

    template<class Equal>
    std::uint64_t observe(const State &state, Equal &equal)
    {
        ++length_;
        if (std::invoke(equal, std::as_const(checkpoint_), state))
        {
            return length_;
        }
        if (length_ == window_)
        {
            checkpoint_ = state;
            // 比較区間を倍にする。残りの観測回数で表現できる上限を越えないようにする。
            window_ += std::min(window_, std::numeric_limits<std::uint64_t>::max() - window_);
            length_ = 0;
        }
        return 0;
    }

private:
    State checkpoint_;
    std::uint64_t window_ = 1;
    std::uint64_t length_ = 0;
};
} // namespace reinsirk::detail

template<class State, class Next, class Equal = std::equal_to<>>
std::optional<StateCycle> find_state_cycle(State start, Next next, std::uint64_t transition_limit, Equal equal = {})
{
    detail::BrentCycleTracker<State> tracker(start);
    auto current = start;
    while (transition_limit != 0)
    {
        current = std::invoke(next, std::as_const(current));
        --transition_limit;
        const auto period = tracker.observe(current, equal);
        if (period == 0)
        {
            continue;
        }
        if (transition_limit < period)
        {
            return std::nullopt;
        }

        // 同じ初期状態から、片方だけ周期分先行させると周期の入口で一致する。
        current = start;
        for (std::uint64_t step = 0; step < period; ++step)
        {
            current = std::invoke(next, std::as_const(current));
        }
        transition_limit -= period;
        std::uint64_t prefix = 0;
        while (!std::invoke(equal, std::as_const(start), std::as_const(current)))
        {
            if (transition_limit < 2)
            {
                return std::nullopt;
            }
            start = std::invoke(next, std::as_const(start));
            current = std::invoke(next, std::as_const(current));
            transition_limit -= 2;
            ++prefix;
        }
        return StateCycle{prefix, period};
    }
    return std::nullopt;
}

template<class State, class Next, class Project, class Combine, class Value, class Equal = std::equal_to<>>
Value fold_state_sequence(State start, std::uint64_t count, Next next, Project project, Combine combine, Value identity,
                          Equal equal = {})
{
    detail::BrentCycleTracker<State> tracker(start);
    auto result = identity;
    while (count != 0)
    {
        result = std::invoke(combine, std::as_const(result), std::invoke(project, std::as_const(start)));
        if (--count == 0)
        {
            break;
        }
        start = std::invoke(next, std::as_const(start));
        const auto period = tracker.observe(start, equal);
        if (period == 0)
        {
            continue;
        }

        auto copies = count / period;
        if (copies != 0)
        {
            auto power = identity;
            auto current = start;
            for (std::uint64_t step = 0; step < period; ++step)
            {
                power = std::invoke(combine, std::as_const(power), std::invoke(project, std::as_const(current)));
                current = std::invoke(next, std::as_const(current));
            }
            for (; copies != 0; copies >>= 1)
            {
                if ((copies & 1) != 0)
                {
                    result = std::invoke(combine, std::as_const(result), std::as_const(power));
                }
                if (copies > 1)
                {
                    power = std::invoke(combine, std::as_const(power), std::as_const(power));
                }
            }
        }

        const auto tail = count % period;
        for (std::uint64_t step = 0; step < tail; ++step)
        {
            result = std::invoke(combine, std::as_const(result), std::invoke(project, std::as_const(start)));
            start = std::invoke(next, std::as_const(start));
        }
        break;
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
