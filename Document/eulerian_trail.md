# オイラー路

`EulerianTrail`はグラフの全辺を1回ずつ通る路を求めます。`EulerianTrail<true>`は有向グラフ用で別名は`DirectedEulerianTrail`、`EulerianTrail<false>`は無向グラフ用で別名は`UndirectedEulerianTrail`です。Nは頂点数、Mは追加した辺数です。

頂点は0..N-1で、自己辺・平行辺を許します。各辺は追加時のIDで区別します。

## EulerianTrail

```text
explicit EulerianTrail(int n = 0);
EulerianTrail(const EulerianTrail&);
EulerianTrail(EulerianTrail&& other) noexcept;
```

空のn頂点グラフを作ります。コピーは独立、ムーブ元は0頂点0辺です。

**制約:** n>=0。

**計算量:** 構築O(1)、コピーO(N+M)、ムーブO(1)。

## add_edge

```text
std::size_t add_edge(int u, int v);
```

辺を追加し、std::size_tの辺IDを返します。

**制約:** u,vは有効な頂点番号。

**計算量:** 償却O(1)。再確保時は現在の保持辺数に比例します。

## find

```text
std::optional<Trail> find(int start = -1, bool circuit_only = false) const;
```

全辺を通る路をoptional<Trail>で返し、存在しなければnulloptです。circuit_only=trueは閉路だけを許します。入力を変更せず繰り返し呼べます。

**制約:** start=-1は自動選択。明示する場合は有効な頂点番号です。

**計算量:** O(N+M)時間・領域。

## edge

```text
std::pair<int, int> edge(std::size_t id) const;
```

追加時の辺の両端をpair<int,int>で返します。

**制約:** idは登録済みの辺ID。

**計算量:** O(1)。

## size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

## edge_count

```text
std::size_t edge_count() const noexcept;
```

辺数を返します。

**制約:** なし。

**計算量:** O(1)。

## reserve_edges

```text
void reserve_edges(std::size_t count);
```

辺記録の容量を予約します。

**制約:** なし。

**計算量:** 再確保する場合は現在の辺数に比例します。

## reset

```text
void reset(int n = 0);
```

空のn頂点グラフへ戻します。

**制約:** n>=0。

**計算量:** 旧領域の破棄に比例します。

## swap

```text
void swap(EulerianTrail& other) noexcept;
```

状態を交換します。

**制約:** なし。

**計算量:** O(1)。

## operator=

```text
EulerianTrail& operator=(EulerianTrail other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーはO(N+M)、旧領域の破棄費用を加えます。

## circuit

```text
bool circuit() const noexcept;
```

結果の頂点列が空、または始点と終点が等しければtrueです。

**制約:** Trailのメンバーとして呼びます。

**計算量:** O(1)。

## 結果と境界

`Trail.vertices[i]`から`vertices[i+1]`へ`Trail.edges[i]`を通ります。結果の配列は独立して所有します。無向辺の通行方向は頂点列から分かります。

辺が0なら開始点だけの路、頂点と辺がともに0なら両配列が空の路です。開始点が次数条件を満たさない場合や、辺を持つ成分が複数ある場合はnulloptです。孤立頂点は路へ加えません。

`SKIP_RUNTIME_CHECKS`で端点・辺ID検査を除きます。次数条件と全辺を使い切る判定は維持します。

[完全な使用例](../Examples/eulerian_trail_example.cpp)・[独立参照](../Verify/current/tests/new_eulerian_reference_test.cpp)
