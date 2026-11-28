# 出発時刻で変わる経路

乗り物を待てるグラフで、最早到着時刻や終電に間に合う最遅出発時刻を求めます。[本体](../Graph/Time_Dependent_Paths.cpp)

- `earliest_arrival_times(n, edges, source, start_time, arrival)` は始点から各頂点に最も早く着く時刻を返します。`arrival(i, t)` は辺 `i` に時刻 `t` 以降で乗ったときの最早到着時刻です。
- `latest_departure_times(n, edges, target, deadline, departure)` は各頂点から期限までに終点に着ける最遅出発時刻を返します。`departure(i, t)` は辺 `i` で時刻 `t` までに着ける最遅出発時刻です。

有向辺は $0$ 始まりの端点の組です。移動できない辺や到達できない頂点の時刻には `std::nullopt` を使います。早く着けば待てるので、乗車可能な時刻が早いほど最早到着は遅くならず、期限が遅いほど最遅出発は早まりません。この条件でダイクストラ法を適用します。移動関数を1回呼ぶ費用を $T$ として、$O(n+m \log(m+1)+mT)$ 時間、$O(n+m)$ 領域です。

## 使用例

時刻 3 に頂点 0 を発車し、時刻 5 に頂点 1 に着く便だけがあるとします。

```cpp
const std::array edges{std::pair{0, 1}};
const auto board = [](std::size_t, std::int64_t ready) -> std::optional<std::int64_t> {
    return ready <= 3 ? std::optional<std::int64_t>(5) : std::nullopt;
};
const auto arrive = sirkit::earliest_arrival_times(2, edges, 0, 1, board);
// arrive[1] は 5。
```

[使用例](../Examples/time_dependent_paths_example.cpp)・[最早到着の Verify](../Verify/current/submissions/time_dependent_paths_earliest.cpp)・[最遅出発の Verify](../Verify/current/submissions/time_dependent_paths_latest.cpp)
