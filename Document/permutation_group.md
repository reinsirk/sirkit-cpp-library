# 置換群

複数の置換を繰り返し合成して作れる置換の個数や、指定した置換を作れるかを調べます。`p[i]` は $i$ の移動先です。

## API

| 操作 | 戻り値・計算量 |
|---|---|
| `PermutationGroup(n, generators)` | $0,\ldots,n-1$ の置換から群を構築します。生成元が $g$ 個なら $O(n^5+gn^2)$ 時間、$O(n^3)$ 領域です。 |
| `degree()` | 置換の長さ $n$。$O(1)$。 |
| `contains(p)` | `p` が群に属するか。$O(n^2)$ 時間、$O(n)$ 補助領域。 |
| `stabilizer_indices()` | $i$ 未満の点をすべて固定したとき、$i$ が移れる点の個数を各 $i$ について返します。全要素の積が群の位数です。$O(n)$。 |
| `order_mod(mod)` | 群の位数を `mod` で割った余り。$O(n)$。 |

各置換は長さ $n$ の `std::vector<int>` で、要素は $0,\ldots,n-1$ の並べ替えです。$n=0$ や生成元が空の場合も恒等置換を含みます。`mod` は正の `std::uint32_t` です。通常版では、長さや要素が不正な置換、負の $n$、`mod=0` に `std::invalid_argument` を送出します。

```cpp
const std::vector<sirkit::PermutationGroup::Permutation> generators{{1, 2, 0}};
const sirkit::PermutationGroup group(3, generators);
const auto count = group.order_mod(998244353); // 3
const bool has_swap = group.contains(std::array{1, 0, 2}); // false
```

[使用例](../Examples/permutation_group_example.cpp) ／ [Verify](../Verify/current/submissions/permutation_group_order.cpp) ／ [参考論文](https://arxiv.org/abs/math/9201304)
