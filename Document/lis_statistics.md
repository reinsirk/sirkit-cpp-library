# 最長増加部分列の個数と共通要素

最長増加部分列（LIS）の個数を数えたり、すべてのLISに共通する添字を調べたりできます。

## `count_lis`

```text
template<bool Strict = true, class T>
LISCount count_lis(const std::vector<T> &values, std::uint64_t modulus = 1'000'000'007)
```

戻り値の `length` は最長の長さ、`count` はその長さの部分列の個数を法 `modulus` で割った余りです。値が同じでも、選んだ添字が異なるものは別に数えます。法は正の整数で、空列には長さ0の部分列が一つあります。

## `lis_membership`

```text
template<bool Strict = true, class T>
std::vector<LISMembership> lis_membership(const std::vector<T> &values)
```

添字ごとに、どのLISにも含まれなければ `kNone`、一部だけに含まれれば `kSome`、すべてに含まれれば `kAll` を返します。

どちらも既定は狭義増加です。広義増加を扱う場合は `<false>` を指定します。値の比較とコピーを $O(1)$ として、長さ $n$ の列を $O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域で処理します。

## 使用例

```cpp
const std::vector values{1, 4, 3, 2, 5};
const auto [length, count] = sirkit::count_lis(values); // {3, 3}
const auto membership = sirkit::lis_membership(values); // {kAll, kSome, kSome, kSome, kAll}
```

[実装](../Algorithm/LIS_Statistics.cpp)・[使用例の全体](../Examples/lis_statistics_example.cpp)
