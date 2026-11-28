# PairXorOrder

列の異なる添字の組 `i<j` について、すべての `a[i] ^ a[j]` を並べたときの順位と個数を調べます。同じ値を持つ要素でも、添字が異なれば別の組として数えます。

## 使い方

```cpp
sirkit::PairXorOrder<unsigned> order({0, 1, 2, 3, 4});
assert(order.pair_count() == 10);
assert(order.kth(6) == 4); // 順位は0始まり
assert(order.count_less(4) == 6);
```

[使用例](../Examples/pair_xor_order_example.cpp)

要素は符号なし整数です。 `kth(k)` には `0 <= k < pair_count()` を満たす順位を渡します。

## 計算量

要素数を `n`、最小値と最大値のXORのビット幅を `B` とすると、構築は `O(n log(n+1))`、各照会は `O(n(1+B))` 時間です。領域は `O(n)` です。
