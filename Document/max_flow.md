# 最大流

`mf_graph<Cap>`は、非負整数容量の有向グラフの最大流を求める。型名・辺の型・引数と戻り値・呼び出し形式は[ACLの最大流](https://atcoder.github.io/ac-library/production/document_ja/maxflow.html)に合わせている。独自の名前空間は不要で、`Graph/Max_Flow.cpp`を単独で貼り付けられる。ACL実装の転記ではない。

`Cap`には容量と追加流量が収まる整数型を渡す。ACLの`int`・`std::int64_t`に加え、bool以外の符号なし整数型・小さい整数型も使用できる。省略時は`std::int64_t`。

## API

`n`は頂点数、`m`は追加した元辺の数。頂点は`[0,n)`、元辺番号は追加順の`[0,m)`で、`m <= INT_MAX`。

| 呼び出し | 引数・戻り値 | 計算量 |
| --- | --- | --- |
| `mf_graph<Cap>()` / `mf_graph<Cap>(n)` | 空 / `n`頂点のグラフを作る | O(1) / O(n) |
| `add_edge(from,to,cap)` | 非負容量の元辺を追加し、`int`の辺番号を返す | 償却O(1) |
| `get_edge(int id)` | `edge{int from,to; Cap cap,flow;}`のコピー | O(1) |
| `edges()` | 元辺を追加順に並べた`vector<edge>` | O(m) |
| `change_edge(int id,new_cap,new_flow)` | 元辺の容量・流量を変更する | O(1) |
| `flow(s,t)` | 追加で流せる量を`Cap`で返す | O(n²m) |
| `flow(s,t,flow_limit)` | 上限まで追加で流し、その量を返す | O(n²m) |
| `min_cut(s)` | 残余グラフで`s`から到達可能かを`vector<bool>`で返す | O(n+m) |
| `size()` / `edge_count()` | 頂点数`int` / 元辺数`size_t` | O(1) |
| `reserve_edges(m)` | 元辺の位置表を予約する | O(m)以下の確保・移動 |
| `reset(n=0)` / `swap(other)` | 空の`n`頂点へ戻す / 状態を交換する | O(n+旧領域の破棄) / O(1) |

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

[完全な使用例](../Examples/max_flow_example.cpp) · [Verify提出用コード](../Verify/current/submissions/max_flow_grid.cpp)

## 前提と所有

- 自己辺・多重辺・逆向きの元辺も追加できる。`change_edge`は`0 <= new_flow <= new_cap`が前提で、他の頂点の流量保存は利用側で保証する。
- `flow`の戻り値は累積ではなく今回の追加量。同じ始点・終点で継続でき、指定上限は非負とする。既定の上限は`Cap`の最大値で、総流量が型に収まらない場合は複数回に分け、累積値を十分に広い型で保持する。
- ACLでは`s != t`が前提。この実装は追加仕様として`s == t`を0とし、`flow_limit == 0`も0を返す。異なる端点で継続する場合も、利用側で現在の流量保存を保証する。
- `min_cut`が最小カットを表すのは、最大流を最後まで流して`t`へ到達できなくなった後。上限で途中停止した状態では最小とは限らない。
- 再帰しないDinic法を用い、グラフと作業領域はO(n+m)。コピー構築・代入はO(n+m)で残余を独立所有する。移動構築はO(1)で、移動元は空になる。代入は自身への参照を返し、旧領域の破棄費用が加わる。コピー代入・resetの失敗時は元を保持する。
- 入力検査は操作時に行い、`SKIP_RUNTIME_CHECKS`で除く。内部の辺数・添字上限と安全な容量差の計算は維持する。辺追加の確保失敗で片方向辺を残さず、`flow`の作業領域は残余更新前に確保する。

[独立参照試験](../Verify/current/tests/new_max_flow_reference_test.cpp) · [ACL互換・境界試験](../Verify/current/tests/max_flow_acl_api_test.cpp) · [AtCoder practice2 D](https://atcoder.jp/contests/practice2/tasks/practice2_d)
