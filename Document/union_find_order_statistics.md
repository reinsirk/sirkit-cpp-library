# UnionFindOrderStatistics

頂点のラベルを固定したまま成分を結合し、成分内でラベルの個数や順位を照会します。同じラベルを持つ頂点も別々に数えます。

```cpp
sirkit::UnionFindOrderStatistics tree(std::vector{30, 10, 30, 20});
tree.merge(0, 1);
tree.merge(2, 1);
tree.kth(0, 0);         // 10
tree.count(0, 30);      // 2
tree.count_less(2, 20); // 1
```

[完全な使用例](../Examples/union_find_order_statistics_example.cpp)

## `UnionFindOrderStatistics`

```text
explicit UnionFindOrderStatistics(const std::vector<T> &labels)
```

`T` はコピーできる全順序型です。頂点の番号は0始まりです。ラベルの比較・コピーを `O(1)` とし、頂点数を `n`、異なるラベルの数を `m` とすると、構築は `O(n log(n+1))` 時間、`O(n log(m+1))` 領域です。頂点数は `INT_MAX` 以下です。

## `merge`

```text
bool merge(int a, int b)
```

2頂点を含む成分を結合し、結合が起きたときに `true` を返します。成分探索は1回あたり償却 `O(α(n))`、ラベルの木を併合する時間は全結合を通して `O(n log(m+1))` です。

## `kth`

```text
std::optional<T> kth(int vertex, int k)
```

成分内のラベルの昇順で0始まりの `k` 番目を返します。範囲外は `std::nullopt` です。償却 `O(log(m+1)+α(n))` 時間です。

## `count` と `count_less`

```text
int count(int vertex, const T &label)
int count_less(int vertex, const T &label)
```

成分内で指定したラベルと等しい個数、または指定したラベル未満の個数を返します。どちらも償却 `O(log(m+1)+α(n))` 時間です。

## 連結成分の情報

```text
int root(int vertex)
bool issame(int a, int b)
int size(int vertex)
int size() const noexcept
int component_count() const noexcept
```

代表、同一成分かどうか、成分の頂点数はそれぞれ償却 `O(α(n))` です。全頂点数と成分数は `O(1)` で返します。
