# Stern–Brocot 木

すべての正の有理数が1回ずつ現れる二分探索木です。根は `1/1`、左の子へ進むと値は小さく、右の子へ進むと大きくなります。同じ向きに続く移動をまとめて扱います。

```cpp
using Tree = sirkit::SternBrocotTree;
const auto path = Tree::encode({5, 3}); // {{'R', 1}, {'L', 1}, {'R', 1}}
assert((Tree::decode(path) == Tree::Fraction{5, 3}));
```

`Fraction` は `std::uint64_t` の `{分子, 分母}` です。入力の分子・分母は正とし、約分済みでなくても構いません。戻り値は既約です。`Run` は `{方向, 回数}` で、方向は `'L'` または `'R'`、回数は正の `std::uint64_t` です。扱う Run の数を `R` とすると、全操作が `O(R+1)` 時間で、出力以外に `O(1)` 領域を使います。分数からたどる Run の数は分子と分母の大きい方に対して対数です。

## `encode`

```text
static std::vector<Run> encode(Fraction fraction);
```

根から指定した分数までのパスを、同じ方向が続く区間ごとにまとめて返します。根のパスは空です。

## `decode`

```text
static Fraction decode(std::span<const Run> path);
```

根からパスをたどった分数を返します。分子または分母が `std::uint64_t` に収まらなければ、`std::overflow_error` を送出します。

## `ancestor`

```text
static std::optional<Fraction> ancestor(Fraction fraction, std::uint64_t depth);
```

根を深さ0とし、指定した深さの祖先を返します。深さが分数自身より大きければ `std::nullopt` を返します。

## `lca`

```text
static Fraction lca(Fraction a, Fraction b);
```

2つの分数の共通祖先のうち、最も深いものを返します。

## `approximate`

```text
static std::pair<Fraction, Fraction> approximate(Fraction fraction,
    std::uint64_t max_numerator, std::uint64_t max_denominator);
```

分子・分母がそれぞれ上限以下の数のうち、`fraction` 以下の最大値と `fraction` 以上の最小値を返します。`fraction` の既約形が両方の上限に収まる場合、両端が同じ分数になります。下側に候補がないときは `{0, 1}`、上側に候補がないときは `{1, 0}` です。上限のどちらかが0なら両側に候補がありません。

## `range`

```text
static std::pair<Fraction, Fraction> range(Fraction fraction);
```

指定した分数を根とする部分木に含まれる値の開区間を `{下限, 上限}` で返します。下限の0は `{0, 1}`、上限の正の無限大は `{1, 0}` です。

[使用例](../Examples/stern_brocot_tree_example.cpp) ／ [木操作のVerify](../Verify/current/submissions/stern_brocot_tree.cpp) ／ [近似のVerify](../Verify/current/submissions/rational_approximation.cpp) ／ [Nyaan の関連実装](https://nyaannyaan.github.io/library/math/stern-brocot-tree-binary-search.hpp)
