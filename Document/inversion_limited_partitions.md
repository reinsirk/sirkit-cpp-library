# 各区間の転倒数を制限する

整数列を空でない連続区間に分割するとき、各区間の転倒数を `K` 以下にします。転倒とは、左側の値が右側より大きい組です。

## `inversion_limited_starts(values, K)`

各右端 `r` について、`r` を含み転倒数が `K` 以下となる最小の左端を返します。添字は0始まりです。その左端から `r` までのどこで始めても条件を満たします。

## `count_inversion_limited_partitions<Count>(values, K)`

すべての区間が条件を満たす分割の個数を返します。空列の答えは1です。`Count{}` と `Count{1}` で0と1を作れ、加減算できる型を指定します。途中の累計も型に収めてください。剰余が必要なら modint が使えます。

尺取り法で区間を動かし、Fenwick 木で転倒数を更新します。列長 `n` は `INT_MAX` 未満です。どちらも $O(1+n\log(n+1))$ 時間、$O(n+1)$ 領域です。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{3, 1, 4, 2};
const auto starts = sirkit::inversion_limited_starts(values, 0);
assert((starts == std::vector<std::size_t>{0, 1, 1, 3}));
assert(sirkit::count_inversion_limited_partitions(values, 0) == 2);
```

[実装](../Algorithm/Inversion_Limited_Partitions.cpp)・[使用例の全体](../Examples/inversion_limited_partitions_example.cpp)・[Verify](../Verify/current/submissions/inversion_limited_partitions.cpp)
