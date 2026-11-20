# Floyd–Warshall

[実装](../Graph/Floyd-Warshall.cpp)・[使用例](../Examples/floyd_warshall_example.cpp)

有向グラフの距離行列から、全点対最短距離と負閉路の影響を求めます。
中継頂点を順に追加して距離を更新します。

## API

V=頂点数、L=返す経路の頂点数です。

### FloydWarshall

```text
inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances, std::int64_t inf = (std::int64_t{1} << 60));
inline bool FloydWarshall(std::vector<std::vector<std::int64_t>> &distances, std::vector<std::vector<int>> &next, std::int64_t inf = (std::int64_t{1} << 60));
```

距離行列を全点対の最短距離で上書きし、負閉路があれば`true`を返します。次頂点表`next`を指定すると、その表も作ります。

**制約:** 行列・番兵・有限距離の前提は下記の通りです。

**計算量:** O(V³)時間。追加領域は通常O(V)。より広い範囲の整数演算を使う場合や、次頂点表を作る版ではO(V²)。

### FloydWarshallPath

```text
inline std::vector<int> FloydWarshallPath(const std::vector<std::vector<int>> &next, int from, int to);
```

経路の頂点列を返します。到達できない場合や負閉路の影響がある場合は空です。

**制約:** distanceは正方行列。nextには本関数で作った表を渡し、from/toは有効な頂点番号です。

**計算量:** O(L+1)時間・領域。


`distance`は`std::vector<std::vector<std::int64_t>>`の正方行列です。
正の`std::int64_t inf`を未接続値として指定し、省略時は`kInf = std::int64_t{1} << 60`を使います。
入力行列も同じ`inf`で初期化してください。有限重みは`(-inf, inf)`で、対角は入力値と0の小さい方を使います。
出力は未到達が`inf`、負閉路を経由して到達できる組だけが`-inf`になります。

```cpp
const std::int64_t inf = 1'000'000'000'000;
std::vector distance(4, std::vector<std::int64_t>(4, inf));
distance[0][1] = 4;
distance[1][2] = -2;
std::vector<std::vector<int>> next;
const bool negative = FloydWarshall(distance, next, inf);
auto path = FloydWarshallPath(next, 0, 2);
// negative=false, distance[0][2]=2, path={0,1,2}
// distance[0][3]=inf、distance[i][i]=0
```

## 前提・例外

頂点数は`INT_MAX`以下とし、`next`はこの関数で作った表を渡します。
有限な最短距離が`(-inf, inf)`を外れる場合は`overflow_error`です。
負閉路の影響による`-inf`とは区別します。途中の和だけが範囲外でも、答えが範囲内なら正常に返ります。
`inf<=0`は`invalid_argument`、容量超過は`length_error`です。

既定では行列の形状・入力値・経路復元の頂点を検査します。
不正な形状は`invalid_argument`、値や頂点は`out_of_range`です。
`SKIP_RUNTIME_CHECKS`で入力検査を省く場合も、容量・算術範囲・`inf`の正値の保護は維持します。
例外時の距離行列の内容は未規定です。

## 計算量

時間O(V³)、通常版の追加領域O(V)、大きな重みで広幅演算が必要な場合はO(V²)です。
経路版はさらにO(V²)の表を作り、経路復元はO(L+1)時間・領域です。
既定の番兵では対応CPU上でSIMDを使う場合があります。無効化は[SIMDの指定](simd.md)を参照してください。

Verify先は[ABC012 D](https://atcoder.jp/contests/abc012/tasks/abc012_4)です。
負辺・負閉路・未到達・経路復元・大きな重みとカスタム番兵は独立参照でも照合します。
