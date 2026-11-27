# 順列の強区間分解木

順列で、位置と値の両方が連続する区間を共通区間と呼びます。ほかの共通区間との関係が常に包含か互いに素になる区間が強区間です。各強区間を、それを含む最小の強区間と結んで木にします。

```cpp
const std::array permutation{0, 2, 1, 3};
auto tree = sirkit::common_interval_decomposition(permutation);
// 根は [0,4)。その子は [0,1)、[1,3)、[3,4)。
```

## `common_interval_decomposition`

```text
std::vector<CommonIntervalNode> common_interval_decomposition(std::span<const int> permutation);
```

`0` から `N-1` を1回ずつ含む `int` の順列を渡します。戻り値は `std::vector<CommonIntervalNode>` で、親が子より先に現れます。空列の結果は空です。通常版で範囲外や重複値を渡すと `std::invalid_argument` を送出します。節数の上限 $2N-1$ が `int` に収まらない場合は `std::length_error` です。

各節の `left, right` は位置、`value_begin, value_end` は値の半開区間です。`parent` は親の添字で、根では `-1` です。`kind` は1要素の `leaf`、子の値区間が昇順または降順になる `increasing` / `decreasing`、それ以外の `prime` です。

右端を伸ばしながら区間内の最大値・最小値の差を管理し、構成した木で同じ向きの線形節の親子をまとめます。要素数 $N$ に対して $O(N\log(N+1))$ 時間・$O(N)$ 領域です。

[使用例](../Examples/common_interval_decomposition_example.cpp) ／ [Verify](../Verify/current/submissions/common_interval_decomposition_tree.cpp) ／ [原論文](https://arxiv.org/abs/1309.7141)
