# 傾きが単調な直線の包絡

傾き順に直線 $y=ax+b$ を追加し、与えた座標で最小値か最大値を求めます。傾き・切片・座標は符号付き64 bit整数です。[本体](../DataStructure/Monotone_Line_Container.cpp)

- `MonotoneMinLineContainer` は最小化用です。傾きを非増加順に追加します。
- `MonotoneMaxLineContainer` は最大化用です。傾きを非減少順に追加します。

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `add(a, b)` | 直線を追加する | 償却 $O(1)$ |
| `query(x)` | 任意の座標で最適値と係数を求める | $O(\log(h+1))$ |
| `query_monotone(x)` | 非減少順の照会。途中で直線を追加できる | 追加との合計で償却 $O(1)$ |
| `size()`, `empty()` | 保持する直線の本数、空かどうかを返す | $O(1)$ |
| `clear()` | 空に戻し、傾きと座標の順序をリセットする | $O(h)$ |
| `reserve(n)` | 直線 $n$ 本分の領域を予約する | $O(h)$ |

$h$ は保持している直線の本数です。どちらの照会も空なら `std::nullopt` を返します。結果の `value` が最適値、`slope` と `intercept` がその直線の係数です。最適値が `int64_t` の範囲を超えると `std::overflow_error` を送出します。

```cpp
sirkit::MonotoneMinLineContainer hull;
hull.add(3, 1);
hull.add(1, 5);
assert(hull.query_monotone(0)->value == 1);
assert(hull.query_monotone(3)->value == 8);
```

[実行できる使用例](../Examples/monotone_line_container_example.cpp)・[AtCoder Verify](../Verify/current/submissions/monotone_line_container_frog.cpp)
