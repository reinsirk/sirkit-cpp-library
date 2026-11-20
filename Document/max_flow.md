# 最大流

`mf_graph<Cap>`は、非負整数容量の有向グラフの最大流を求める。型名・辺の型・引数と戻り値・呼び出し形式は[ACLの最大流](https://atcoder.github.io/ac-library/production/document_ja/maxflow.html)に合わせている。型は`sirkit::mf_graph`。`Graph/Max_Flow.cpp`を単独で貼り付けられる。

`Cap`には容量と追加流量が収まる整数型を渡す。ACLの`int`・`std::int64_t`に加え、bool以外の符号なし整数型・小さい整数型も使用できる。省略時は`std::int64_t`。

## API

`n`は頂点数、`m`は追加した元辺の数。頂点は`[0,n)`、元辺番号は追加順の`[0,m)`で、`m <= INT_MAX`。

### mf_graph

```text
mf_graph();
explicit mf_graph(int n);
mf_graph(const mf_graph&);
mf_graph(mf_graph&& other) noexcept;
```

空またはn頂点のグラフを作ります。コピーは残余を独立に保持し、ムーブ元は空になります。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空の構築はO(1)、n頂点の構築はO(n)。コピーはO(n+m)、ムーブはO(1)。

### add_edge

```text
int add_edge(int from, int to, Cap capacity);
```

非負容量の元辺を追加し、`int`の辺番号を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 償却O(1)。

### get_edge

```text
edge get_edge(int id);
edge get_edge(int id) const;
```

指定した元辺の端点・容量・現在流量などをコピーして返します。

**制約:** idは登録済みの有効な辺番号です。

**計算量:** O(1)。

### edges

```text
std::vector<edge> edges();
std::vector<edge> edges() const;
```

登録した元辺を追加順で返します。

**制約:** なし。

**計算量:** O(m)。

### change_edge

```text
void change_edge(int id, Cap capacity, Cap sent);
```

元辺の容量と流量を変更します。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(1)。

### flow

```text
Cap flow(int source, int sink);
Cap flow(int source, int sink, Cap limit);
```

上限以下の流量を追加し、今回の追加量を返します。上限省略時は容量型の最大値です。反復Dinic法から必要に応じてpush-relabel法へ切り替えます。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(n+n²m)時間・O(n)作業領域。上限1ではO(n+m)。

### min_cut

```text
std::vector<bool> min_cut(int source);
std::vector<bool> min_cut(int source) const;
```

各頂点に残余グラフで`s`から到達できるかを`vector<bool>`で返します。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(n+m)。

### size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### reserve_edges

```text
void reserve_edges(std::size_t count);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** O(m)以下の確保・移動。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(n+旧領域の破棄)。

### swap

```text
void swap(mf_graph& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


`get_edge`・`edges`・`min_cut`はACLと同じ非const呼び出しに加え、constオブジェクトからも同名で呼べる。`size`・`edge_count`・`reserve_edges`・`reset`・`swap`は追加機能。

## 使用例

```cpp
mf_graph<std::int64_t> graph(4);
const int id = graph.add_edge(0, 1, 5);
graph.add_edge(1, 3, 5);
const auto first = graph.flow(0, 3, 2); // 2だけ流す
const auto rest = graph.flow(0, 3);     // 残り3を流す
const auto used = graph.get_edge(id);  // used.cap == 5, used.flow == 5
const auto cut = graph.min_cut(0);     // cut[0] == true, cut[3] == false
```

[完全な使用例](../Examples/max_flow_example.cpp) · [Verify提出用コード](../Verify/current/submissions/preferred_max_flow_bipartite.cpp)

## 前提と所有

- 自己辺・多重辺・逆向きの元辺も追加できる。`change_edge`は`0 <= new_flow <= new_cap`が前提で、他の頂点の流量保存は利用側で保証する。
- `flow`の戻り値は累積ではなく今回の追加量。同じ始点・終点で継続でき、指定上限は非負とする。既定の上限は`Cap`の最大値で、総流量が型に収まらない場合は複数回に分け、累積値を十分に広い型で保持する。
- ACLでは`s != t`が前提。この実装は追加仕様として`s == t`を0とし、`flow_limit == 0`も0を返す。異なる端点で継続する場合も、利用側で現在の流量保存を保証する。
- `min_cut`が最小カットを表すのは、流量を追加し終えて残余グラフで`t`に到達できなくなった後。上限に達して途中で止まった場合は最小とは限らない。
- グラフと作業領域はO(n+m)。コピー構築・代入はO(n+m)で残余を独立所有する。移動構築はO(1)で、移動元は空になる。代入は自身への参照を返し、旧領域の破棄費用が加わる。コピー代入・resetの失敗時は元を保持する。
- 入力検査は操作時に行い、`SKIP_RUNTIME_CHECKS`で除く。内部の辺数・添字上限と安全な容量差の計算は維持する。辺追加・flowの作業領域確保の失敗時は残余を保つ。

[独立参照試験](../Verify/current/tests/new_max_flow_reference_test.cpp) · [ACL互換・境界試験](../Verify/current/tests/max_flow_acl_api_test.cpp) · [Library Checker Matching](https://judge.yosupo.jp/problem/bipartitematching)（単位容量）・[旧AtCoder practice2 D](https://atcoder.jp/contests/practice2/tasks/practice2_d)（参考）

## 構築・状態と補助型

### operator=

```text
mf_graph& operator=(mf_graph other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
