# 隣接交換の最小回数

同じ長さの二つの列について、隣り合う要素の交換を繰り返して一方をもう一方にそろえます。値が重複していても、最小回数と要素の対応を求められます。

## `adjacent_swap_permutation`

```text
adjacent_swap_permutation(from, to, compare = {})
```

`from[i]` の移動先を `result[i]` とする0始まりの順列を `std::optional<std::vector<std::size_t>>` で返します。同じ値の要素は出現順に対応させます。

## `adjacent_swap_distance`

```text
adjacent_swap_distance(from, to, compare = {})
```

隣接する要素の交換で `from` を `to` に変える最小回数を `std::optional<std::uint64_t>` で返します。出現順を保つ対応の転倒数が答えです。

どちらも、要素の種類や個数が異なれば `std::nullopt` です。入力には `std::size` と添字アクセスが必要で、長さは `UINT32_MAX` 以下です。`compare` は既定で `<` を使い、どちら向きにも小さくない要素を同じ値として扱います。長さを $n$ とし、比較と添字アクセスを $O(1)$ とすると、どちらも $O((n+1)\log(n+2))$ 時間・$O(n+1)$ 追加領域です。

## 使用例

```cpp
const std::string_view from = "aabb", to = "baba";
const auto order = sirkit::adjacent_swap_permutation(from, to); // {1, 3, 0, 2}
const auto swaps = sirkit::adjacent_swap_distance(from, to);    // 3
```

[実装](../Algorithm/Adjacent_Swap_Distance.cpp)・[使用例の全体](../Examples/adjacent_swap_distance_example.cpp)
