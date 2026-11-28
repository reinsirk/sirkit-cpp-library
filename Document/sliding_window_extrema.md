# スライド窓の最小値・最大値

同じ大きさの窓をずらしながら、窓ごとの最小値や最大値をまとめて求めます。比較関数の既定値は `std::less<>{}` です。最大値には `std::greater<>{}` を渡します。比較関数はconstの状態で呼び出せる狭義弱順序とします。

## `sliding_window_argextrema`

```cpp
std::vector<std::size_t>
sliding_window_argextrema(const Sequence &values, std::size_t width, const Compare &compare = {});
```

幅 `width` の各窓で比較順が先の要素を選び、その添字を0始まりで返します。同順位なら左側を選びます。窓の幅が0か列より長い場合は空の配列です。

`values` は要素数を定数時間で取得できるランダムアクセス範囲です。長さを $n$ とすると、$O(n)$ 時間で求まり、返却値以外に $O(\text{width})$ の領域を使います。

## `sliding_window_extrema`

```cpp
std::vector<T>
sliding_window_extrema(const Sequence &values, std::size_t width, const Compare &compare = {});
```

同じ窓を順に調べ、添字の代わりに選んだ要素を返します。型 `T` は `values` の要素型で、計算量は添字を返す版と同じです。

## `sliding_window_extrema_2d`

```cpp
std::vector<std::vector<T>> sliding_window_extrema_2d(
    const std::vector<std::vector<T>> &grid, std::size_t height, std::size_t width,
    const Compare &compare = {});
```

`result[y][x]` は `(y, x)` を左上とする高さ `height`、幅 `width` の矩形で選んだ値です。同順位なら行が小さい方を取り、同じ行では列が小さい方を選びます。窓の一辺が0か盤面より大きい場合は空の配列です。

`T` はコピー構築とコピー代入が可能とします。各行は同じ長さが必要で、検査が有効な場合は不揃いな行に対して `std::invalid_argument` を送出します。窓が収まる $H\times W$ の盤面で、$O(HW)$ 時間と $O(H(W-\text{width}+1)+\max(\text{height},\text{width}))$ 領域です。

## 使用例

```cpp
const std::vector<int> values{4, 1, 1, 5};
const auto positions = sirkit::sliding_window_argextrema(values, 2);
assert((positions == std::vector<std::size_t>{1, 1, 2}));
```

[実装](../DataStructure/Sliding_Window_Extrema.cpp)・[使用例の全体](../Examples/sliding_window_extrema_example.cpp)・[1次元のVerify](../Verify/current/submissions/sliding_window_extrema.cpp)・[2次元のVerify](../Verify/current/submissions/sliding_window_extrema_stamps.cpp)
