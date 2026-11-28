# Monge 最短路

頂点 `0, …, n-1` の有向グラフで、`j < i` を満たす各辺 `j → i` に重みを設定します。重みが Monge 性を満たすとき、頂点0からの最短距離を順に求めます。

## `monge_shortest_paths`

```text
template<class Cost, class T = std::remove_cvref_t<std::invoke_result_t<Cost &, int, int>>>
MongeShortestPathResult<T> monge_shortest_paths(int n, Cost cost,
                                               std::type_identity_t<T> initial = {})
```

辺 `j → i` の重みを `cost(j, i)` で返します。任意の $0\le a<b<c<d<n$ で次の不等式を満たすことが必要です。

$$
\operatorname{cost}(a,c)+\operatorname{cost}(b,d)
\le \operatorname{cost}(a,d)+\operatorname{cost}(b,c).
$$

`n` は非負です。始点の値は `initial` で、既定では `T{}` を使います。

$$
\mathrm{dp}[i]=\min_{0\le j<i}\{\mathrm{dp}[j]+\operatorname{cost}(j,i)\}.
$$

最短距離 $\mathrm{dp}[i]$ は `distances[i]` に格納します。`predecessors[i]` は最短路の直前の頂点で、同値の場合は最小の頂点番号を選びます。始点の直前は `-1` です。`n = 0` なら両方の配列を空にして返します。

`cost(j, i)` を呼ぶ時点で、`j < i` かつ頂点 `j` の距離は確定しています。比較のために先の終点 `i` を調べることがあるので、終点の呼び出し順に依存しない重みを返してください。負の重みも使えます。値型 `T` は加算と大小比較を正確に行え、両辺に同じ値を加えても大小関係が変わらない型にします。

重みの計算・加算・比較が $O(1)$ なら、Larmore–Schieber のオンライン行列探索により $O(n+1)$ 時間と領域で求まります。

## 使用例

```cpp
const auto result = sirkit::monge_shortest_paths(5, [](int from, int to) {
    const auto length = static_cast<std::int64_t>(to - from);
    return length * length + 3;
});
assert(result.distances[4] == 14);
assert(result.predecessors[4] == 2);
```

[使用例](../Examples/monge_shortest_paths_example.cpp) ／ [Verify](../Verify/current/submissions/monge_shortest_paths_baseball.cpp) ／ [全要素が先に分かる行列の最適値](monge_optimization.md)
