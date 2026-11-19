# 二部グラフの最大マッチング

`BipartiteMatching(L,R)`は、左L頂点・右R頂点の間で、端点を共有しない辺を最大何本選べるか求めます。左右は別々に0始まりで番号を付けます。[本体](../Graph/Bipartite_Matching.cpp)・[使用例](../Examples/bipartite_matching_example.cpp)を参照してください。

## API

### add_edge

```text
void add_edge(int left, int right);
```

左lと右rを結ぶ辺を追加します。多重辺を許します。

**制約:** l,rは各側の有効な頂点番号。

**計算量:** 償却O(1)。再確保時は現在の次数に比例します。

### solve

```text
int solve();
```

最大マッチングの総本数を返す。辺追加後も現在の対応から続けて計算する。対応が空なら次数1の辺を先に確定し、最大次数2以下のパス・サイクルも直接解く。

対応が空で次数1の頂点がなく、次数2の右頂点が多い場合は、一部を縮約して小さいグラフで解く。

残りは全辺の次数二乗和を比較し、2倍超の差があれば負荷が小さい側、差が小さければ非孤立頂点が少ない側から再割当する。返す番号は元の左右のまま。再割当の総作業量を線形予算で打ち切り、残りはHopcroft–Karp法で求める。

**制約:** 構築済みの入力が共通の前提を満たすこと。

**計算量:** O((V+M)√(V+1))時間・O(V+M)追加領域。対応が空で最大次数2以下ならO(V+M)時間。最大性が既知ならO(1)。

### match_left

```text
int match_left(int left) const;
```

左頂点に対応する右頂点を返します。未対応なら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### match_right

```text
int match_right(int right) const;
```

右頂点に対応する左頂点を返します。未対応なら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### pairs

```text
std::vector<std::pair<int,int>> pairs() const;
```

左番号順の対応辺をvectorで返す。

**制約:** なし。

**計算量:** O(L)時間・O(対応本数)領域。

### left_matches

```text
std::span<const int> left_matches() const noexcept;
```

左頂点から右頂点への現在の対応表を読み取り専用spanで返します。

**制約:** なし。

**計算量:** O(1)。

### right_matches

```text
std::span<const int> right_matches() const noexcept;
```

右頂点から左頂点への現在の対応表を読み取り専用spanで返します。

**制約:** なし。

**計算量:** O(1)。

### min_vertex_cover

```text
VertexSet min_vertex_cover() const;
```

最小頂点被覆をVertexSet{left,right}で返します。

**制約:** is_maximum()==trueであること。辺追加後はsolveし直してください。

**計算量:** O(V+M)時間・領域。

### max_independent_set

```text
VertexSet max_independent_set() const;
```

最大独立集合をVertexSet{left,right}で返します。

**制約:** is_maximum()==trueであること。辺追加後はsolveし直してください。

**計算量:** O(V+M)時間・領域。

### left_size

```text
int left_size() const noexcept;
```

左頂点の数を返します。

**制約:** なし。

**計算量:** O(1)。

### right_size

```text
int right_size() const noexcept;
```

右頂点の数を返します。

**制約:** なし。

**計算量:** O(1)。

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### matching_size

```text
int matching_size() const noexcept;
```

現在のマッチングの本数を返します。

**制約:** なし。

**計算量:** O(1)。

### is_maximum

```text
bool is_maximum() const noexcept;
```

現在のマッチングが最大と確認済みならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### neighbors

```text
std::span<const int> neighbors(int left) const;
```

指定頂点の隣接列を読み取り専用で返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### reserve_neighbors

```text
void reserve_neighbors(int left, std::size_t capacity);
```

指定頂点の隣接列の容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### clear_matching

```text
void clear_matching() noexcept;
```

辺を残してマッチングだけを消します。

**制約:** なし。

**計算量:** O(V)。

### clear_edges

```text
void clear_edges() noexcept;
```

頂点数と容量を残し、辺とマッチングを消します。

**制約:** なし。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧V+旧M+新V)。

### reset

```text
void reset(int left = 0, int right = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧V+旧M+新V)。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧V+旧M+新V)。

### swap

```text
void swap(BipartiteMatching& other) noexcept;
void swap(BipartiteMatching& a, BipartiteMatching& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


```cpp
BipartiteMatching graph(3,4);
graph.add_edge(0,2);
graph.add_edge(0,1);
graph.add_edge(1,2);
graph.add_edge(2,0);
auto count = graph.solve(); // 3。
auto pairs = graph.pairs(); // 左右の対応辺。
auto cover = graph.min_vertex_cover(); // cover.size() == 3。
```

## 被覆・独立集合と最大性

結果のleft・rightは左右別の昇順vectorで、size()が合計件数です。被覆の最小件数は最大マッチングの本数に等しく、補集合が最大独立集合になります。

被覆・独立集合の取得は`is_maximum()==true`が前提です。辺追加後はfalseとなるため、先にsolveしてください。初期状態はtrueです。対応表とpairsは最大性未確認でも現在の有効な対応を返します。empty()は左右とも0頂点かを表します。

## 計算量と参照

V=L+R、M=登録辺数として、solveはO((V+M)√(V+1))時間・追加O(V+M)領域。最大性が既知ならO(1)です。被覆・独立集合はO(V+M)時間・領域、pairsはO(L)時間・O(対応本数)領域です。件数・対応・span取得はO(1)、辺追加は償却O(1)で、再確保時は現在の次数に比例します。

clear_matchingはO(V)、clear_edgesと再構築は旧状態の破棄も含めO(V+M)。コピーはグラフと対応を独立所有し、move元は0頂点、swapはO(1)です。辺追加・コピー代入・resetとsolveの作業領域確保に失敗しても以前の内容を保ちます。

対応表の値はsolveやclear_matchingで更新されます。隣接spanはその左頂点への辺追加・予約で無効化し得ます。reset・clear・代入後は以前のviewを使わないでください。既定では端点と被覆取得時の最大性を検査し、`SKIP_RUNTIME_CHECKS`で除きます。件数・容量の上限保護は維持します。

出典と検証は[記録](../Audit/graph-document-details-2026-11-15.md)を参照してください。

## 構築・状態と補助型

### BipartiteMatching::VertexSet::size

```text
std::size_t size() const noexcept;
```

leftとrightの頂点数の合計を返します。

**制約:** 結果のVertexSetに対して呼びます。

**計算量:** O(1)。

### BipartiteMatching

```text
explicit BipartiteMatching(int left = 0, int right = 0);
BipartiteMatching(const BipartiteMatching&);
BipartiteMatching(BipartiteMatching&& other) noexcept;
```

左右L,R頂点の空グラフを作ります。コピーは独立、ムーブ元は空です。

**制約:** L,Rは非負で、有効な頂点数です。

**計算量:** 構築O(L+R)、コピーは保持量に比例、ムーブO(1)。

### operator=

```text
BipartiteMatching& operator=(const BipartiteMatching& other);
BipartiteMatching& operator=(BipartiteMatching&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。

### BipartiteMatching::empty

```text
bool empty() const noexcept;
```

左右とも0頂点ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。
