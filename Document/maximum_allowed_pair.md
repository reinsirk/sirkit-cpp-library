# 禁止組を除いた二列の最大和

二つの整数列から値を一つずつ取り、和を最大化します。指定した添字の組は選びません。

## `MaximumAllowedPairResult`

`left` と `right` は元の列における0始まりの添字、`sum` は二値の和です。

## `maximum_allowed_pair`

```cpp
std::optional<MaximumAllowedPairResult> maximum_allowed_pair(
    std::span<const std::int64_t> left, std::span<const std::int64_t> right,
    std::span<const std::pair<std::size_t, std::size_t>> forbidden);
```

最大和を取る組を返します。同点なら `(left, right)` の辞書順最小を選びます。選べる組がなければ `std::nullopt` です。禁止組の添字は0始まりで、同じ組を複数指定できます。

短い列の候補を値の降順に並べ、長い列の各要素について許可された最初の候補を見つけます。列の長さを $n,m$、禁止指定数を $p$ とします。短い列の長さと、長い列の一要素に対する最大禁止指定数に1を足した値の小さい方を $k$ とすると、時間は $O(n+m+p+\min(n,m)\log(k+1))$、領域は $O(n+m+p)$ です。

選ばれた和は `std::int64_t` の範囲に収まる必要があります。検査が有効なら、範囲外の禁止添字には `std::out_of_range`、最大和の範囲超過には `std::overflow_error` を送出します。

## 使用例

```cpp
const std::vector<std::int64_t> left{2, 1}, right{10, 30, 20};
const std::vector<std::pair<std::size_t, std::size_t>> forbidden{{0, 1}, {1, 0}, {1, 2}};
const auto result = sirkit::maximum_allowed_pair(left, right, forbidden);
assert(result && result->sum == 31 && result->left == 1 && result->right == 1);
```

[実装](../Algorithm/Maximum_Allowed_Pair.cpp)・[使用例の全体](../Examples/maximum_allowed_pair_example.cpp)・[Verify](../Verify/current/submissions/maximum_allowed_pair.cpp)
