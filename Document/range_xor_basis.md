# 区間のXOR基底

各区間から要素を選んで指定したXORを作れるか調べます。最大XORと、異なるXORの個数を表す基底の次元も返します。

```cpp
const std::vector<std::uint64_t> a{3, 6, 9};
const std::vector<sirkit::RangeXorQuery> qs{{0, 2, 5}};
auto ans = sirkit::range_xor_queries(a, qs);  // {true, 6, 2}
```

## `range_xor_queries`

```text
std::vector<RangeXorResult> range_xor_queries(
    std::span<const std::uint64_t> values, std::span<const RangeXorQuery> queries);
```

各クエリは `RangeXorQuery{left, right, x}` です。`[left,right)` から選んだ要素のXORを `y` とすると、結果は `x` を作れるかを示す `can_make`、`x ^ y` の最大値 `maximum`、異なる `y` の個数が $2^{rank}$ となる `rank` です。空集合のXORは0です。入力の順序で答えを返します。$n$ 要素、$q$ 問、$w=64$ ビットとして $O((n+q)w+q\log(q+1))$ 時間・$O(w+q)$ 領域です。

[使用例](../Examples/range_xor_basis_example.cpp) ／ [Verify](../Verify/current/submissions/range_xor_basis.cpp)
