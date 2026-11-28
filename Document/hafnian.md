# ハフニアン

完全マッチングごとに選んだ辺の重みを掛け、それらの積を足します。辺の重みを 0 または 1 にすると、無向グラフの完全マッチングを数えられます。

## `hafnian`

```text
template <class T>
T hafnian(const std::vector<std::vector<T>> &matrix)
```

$N\le40$ の正方行列を渡します。重みは上三角で指定します。$T$ は可換環の型で、`T{}` が 0、`T{1}` が 1 で、加減乗算と `+=` が使える必要があります。整数型では中間値も型の範囲に収めます。空行列の答えは 1、奇数次の答えは 0 です。$O(N^2 2^{N/2})$ 回の $T$ 演算と $O(N^4)$ 領域を使います。

```cpp
const std::vector<std::vector<std::int64_t>> weights{
    {0, 1, 2, 3}, {1, 0, 4, 5}, {2, 4, 0, 6}, {3, 5, 6, 0}};
const auto weight_sum = sirkit::hafnian(weights); // 28
```

[使用例](../Examples/hafnian_example.cpp) ／ [Verify](../Verify/current/submissions/hafnian_of_matrix.cpp)
