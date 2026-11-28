# RangeFenwickTree

列の区間全体に同じ値を加え、更新後の区間和を求めます。添字は0始まりで、右端は区間に含めません。

## 使い方

```cpp
std::vector<std::int64_t> a{2, 1, 4, 7};
sirkit::RangeFenwickTree<> bit(a);
bit.add(1, 4, 3); // [2, 4, 7, 10]
assert(bit.sum(0, 3) == 13);
assert(bit.get(3) == 10);
bit.set(0, 5);   // [5, 4, 7, 10]
```

[使用例](../Examples/range_fenwick_tree_example.cpp)

初めはすべて0なら `RangeFenwickTree<>(n)` を使います。型 `T` の既定は `std::int64_t` です。加算の結果は `T` のビット幅で剰余を取るため、数学的な答えが型の範囲に収まる場合はそのまま得られます。

## 計算量

要素数を `n` とすると、構築は `O(n+1)` 時間と領域、区間加算・区間和・一点取得・一点代入は `O(log(n+1))` 時間です。
