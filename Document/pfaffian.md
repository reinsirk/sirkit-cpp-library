# パフィアン

交代行列のパフィアンを素数で割った余りを求めます。交代行列は対角成分が `0` で、`A[j][i] = -A[i][j]` を満たす行列です。行列式はパフィアンの二乗に等しくなります。

```cpp
constexpr std::uint32_t p = 998244353;
std::vector<std::vector<std::uint32_t>> matrix{{0, 3}, {p - 3, 0}};
auto pf = sirkit::pfaffian_mod(matrix);  // 3
```

## `pfaffian_mod<Prime=998244353>(matrix)`

```text
template <std::uint32_t Prime = 998244353>
std::uint32_t pfaffian_mod(std::vector<std::vector<std::uint32_t>> matrix);
```

`Prime` は `std::uint32_t` に収まる素数です。法2も使えます。正方行列を渡し、各要素は `0` 以上 `Prime` 未満の剰余で表します。`0×0` の結果は `1`、次元が奇数の場合は `0` です。通常版で行列の形、各要素の値域、交代条件を検査し、満たさない場合は `std::invalid_argument` を送出します。

非零の要素を選んで行と列を同時に入れ替え、先頭の2行・2列ずつを消去します。次元を $D$ として $O(D^3)$ 時間、行列を含め $O(D^2)$ 領域です。元の行列が不要なら `std::move(matrix)` で渡せます。

[使用例](../Examples/pfaffian_example.cpp) ／ [Verify](../Verify/current/submissions/pfaffian_of_matrix.cpp) ／ [計算方法の原論文](https://arxiv.org/abs/1102.3440)
