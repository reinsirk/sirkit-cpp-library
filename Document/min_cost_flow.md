# 最小費用流

`MinCostFlow<Capacity,Cost>`は、有向グラフで指定量までの流れを最小費用で追加します。頂点は0..N-1、容量は非負整数で、費用は負でも構いません。既定は両方std::int64_tです。[本体](../Graph/Min_Cost_Flow.cpp)・[使用例](../Examples/min_cost_flow_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `MinCostFlow(n=0)` / `add_edge(u,v,capacity,cost)` | n頂点で構築／辺を追加してsize_tの辺IDを返す。自己辺・多重辺可。 |
| `flow(s,t,limit)` | limit以下の追加流量と追加費用のpairを返す。limit省略時はCapacityの最大値。 |
| `slope(s,t,limit)` | {0,0}からの追加流量・費用の折れ線を返す。限界費用が同じ区間はまとめる。 |
| `get_edge(id)` / `edges()` | Edge{from,to,capacity,flow,cost}のコピー／元辺順の一覧。 |
| `change_edge(id,capacity,flow)` / `change_cost(id,cost)` | 指定辺の状態／費用を変える。 |
| `reset_flow()` / `min_cut(s)` | 全元辺の流量を0へ／現在の残余で到達可能な頂点のbool配列。 |
| `size()` / `edge_count()` / `reserve_edges(m)` | 頂点数／元辺数／元辺位置表の予約。 |
| `reset(n=0)` / `swap(other)` | 空のn頂点へ戻す／交換。 |

```cpp
MinCostFlow<> graph(2);
graph.add_edge(0,1,2,-3);
graph.add_edge(0,1,4,5);
auto [amount,cost] = graph.flow(0,1,1); // 追加量1、費用-3。
auto [more,extra] = graph.flow(0,1);    // 追加量5、費用17。
```

戻り値は**その呼び出しで追加した量と費用**で、累積値ではありません。同じ始点・終点で続けて呼べます。s==tまたはlimit==0では流しません。始点・終点の変更やchange_edge後も続ける場合は、0<=flow<=capacityと既存流の流量保存を利用側で保証してください。

## 負費用・計算量

全頂点へ到達する仮想始点からBellman–Ford法でポテンシャルを求め、換算した非負費用でDijkstra法を繰り返します。正の残余容量を持つ辺の負閉路は、sから到達できなくてもdomain_errorで拒否します。有限容量の負閉路でもcycle cancelingは行いません。検出前に費用範囲を超えればoverflow_errorになります。

初期化は負費用辺がなければO(N+M)、一般にはO(NM)。増流A回でO(NM+A(N+M)log(N+M+1))時間、領域O(N+M)です。再帰せず、作業領域は再利用します。辺の変更はポテンシャルを無効にし、次回に再計算します。辺追加は償却O(1)、単点取得・変更はO(1)、一覧・reset_flowはO(M)、min_cutはO(N+M)です。

## 数値・所有と例外

Capacityはbool以外の整数、Costは符号付き整数で、いずれも64ビット以下です。辺費用は[-max(Cost),max(Cost)]。Costの最小値は逆辺の符号反転ができないため、検査省略時もoverflow_errorです。ポテンシャル・換算費用・距離候補・限界費用・追加費用の**途中計算もCostに収まること**が必要です。最終解だけが収まる条件では足りません。

GCC/Clangの128ビット整数拡張が利用可能なら、費用の積和等の範囲判定に使います。これは標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準C++の代替演算へ切り替えられます。費用範囲・負閉路の判定は`SKIP_RUNTIME_CHECKS`でも維持し、端点・負容量等の入力検査だけを省きます。

コピーは残余状態を独立所有し、move元は空、swapはO(1)です。flow・slopeの例外時にも、完了した増流が残る場合があります。途中の増流を片方向だけ更新せず、ポテンシャル更新の失敗時はキャッシュを無効化します。slopeの結果配列の確保失敗も、完了済み増流は取り消しません。

[独立参照](../Verify/current/tests/new_min_cost_flow_reference_test.cpp)・[AtCoder practice2 E](https://atcoder.jp/contests/practice2/tasks/practice2_e)
