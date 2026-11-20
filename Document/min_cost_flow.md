# 最小費用流

`MinCostFlow<Capacity,Cost>`は、有向グラフで指定量までの流れを最小費用で追加します。頂点は0..N-1、容量は非負整数で、費用は負でも構いません。既定は両方std::int64_tです。[本体](../Graph/Min_Cost_Flow.cpp)・[使用例](../Examples/min_cost_flow_example.cpp)を参照してください。

## API

### MinCostFlow

```text
explicit MinCostFlow(int n = 0);
MinCostFlow(const MinCostFlow&);
MinCostFlow(MinCostFlow&& other) noexcept;
```

空のn頂点グラフを作ります。コピーは残余状態を独立に所有し、ムーブ元は空です。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(N)時間・領域。コピーはO(N+M)、ムーブはO(1)。

### add_edge

```text
std::size_t add_edge(int from, int to, Capacity capacity, Cost cost);
```

容量・費用を指定して有向辺を追加し、std::size_tの辺IDを返します。自己辺・多重辺を許します。

**制約:** 端点は0..N-1、capacity>=0。費用は下記の表現範囲内です。

**計算量:** 償却O(1)。再確保時は現在の保持要素数に比例します。

### flow

```text
std::pair<Capacity, Cost> flow(int source, int sink, Capacity limit = std::numeric_limits<Capacity>::max());
```

limit以下の追加流量と追加費用をpairで返します。limitを省略するとCapacityの最大値を使います。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** 初期化O(N+M)（非負費用）またはO(NM)。増流A回にO(A(N+M)log(N+M+1))、領域O(N+M)。

### slope

```text
std::vector<std::pair<Capacity, Cost>> slope(int source, int sink, Capacity limit = std::numeric_limits<Capacity>::max());
```

{0,0}から始まる追加流量と追加費用の折れ線を返します。限界費用が同じ区間はまとめます。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** 初期化O(N+M)（非負費用）またはO(NM)。増流A回にO(A(N+M)log(N+M+1))時間。作業領域O(N+M)、出力領域O(A+1)。

### get_edge

```text
Edge get_edge(std::size_t id) const;
```

指定した元辺の端点・容量・現在流量などをコピーして返します。

**制約:** idは登録済みの有効な辺番号です。

**計算量:** O(1)。

### edges

```text
std::vector<Edge> edges() const;
```

登録した元辺を追加順で返します。

**制約:** なし。

**計算量:** O(M)時間。edgesの結果領域はO(M)。

### change_edge

```text
void change_edge(std::size_t id, Capacity capacity, Capacity sent);
```

指定した辺の容量と流量を変更します。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(1)。

### change_cost

```text
void change_cost(std::size_t id, Cost cost);
```

指定した辺の費用を変更します。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(1)。

### reset_flow

```text
void reset_flow() noexcept;
```

登録した全ての辺の流量を0に戻します。

**制約:** なし。

**計算量:** O(M)時間。edgesの結果領域はO(M)。

### min_cut

```text
std::vector<bool> min_cut(int source) const;
```

現在の残余グラフでsourceから到達できるかを、頂点ごとにboolで返します。

**制約:** 端点・容量・上限と数値範囲は下記の前提に従い、既存流の流量保存を利用側で保ちます。

**計算量:** O(N+M)時間・O(N)結果領域。

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

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧状態の破棄と新頂点の準備に比例します。

### swap

```text
void swap(MinCostFlow& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


```cpp
MinCostFlow<> graph(2);
graph.add_edge(0,1,2,-3);
graph.add_edge(0,1,4,5);
auto [amount,cost] = graph.flow(0,1,1); // 追加量1、費用-3。
auto [more,extra] = graph.flow(0,1);    // 追加量5、費用17。
```

戻り値は**その呼び出しで追加した量と費用**で、累積値ではありません。同じ始点・終点で続けて呼べます。s==tまたはlimit==0では流しません。始点・終点の変更やchange_edge後も続ける場合は、0<=flow<=capacityと既存流の流量保存を利用側で保証してください。

## 負費用・計算量

正の残余容量を持つ辺の負閉路は、sから到達できなくてもdomain_errorで拒否します。有限容量の負閉路でもcycle cancelingは行いません。検出前に費用範囲を超えればoverflow_errorになります。

初期化は負費用辺がなければO(N+M)、一般にはO(NM)。増流A回でO(NM+A(N+M)log(N+M+1))時間、領域O(N+M)です。辺の変更はポテンシャルを無効にし、次回に再計算します。辺追加は償却O(1)、単点取得・変更はO(1)、一覧・reset_flowはO(M)、min_cutはO(N+M)です。

## 数値・所有と例外

Capacityはbool以外の整数、Costは符号付き整数で、いずれも64ビット以下です。辺費用は[-max(Cost),max(Cost)]。Costの最小値は逆辺の符号反転ができないため、検査省略時もoverflow_errorです。ポテンシャル・換算費用・距離候補・限界費用・追加費用の**途中計算もCostに収まること**が必要です。最終解だけが収まる条件では足りません。

GCC/Clangの128ビット整数拡張が利用可能なら、費用の積和等の範囲判定に使います。これは標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準C++の代替演算へ切り替えられます。費用範囲・負閉路の判定は`SKIP_RUNTIME_CHECKS`でも維持し、端点・負容量等の入力検査だけを省きます。

コピーは残余状態を独立所有し、move元は空、swapはO(1)です。flow・slopeの例外時にも、完了した増流が残る場合があります。slopeの結果配列の確保失敗も、完了済み増流は取り消しません。

[独立参照](../Verify/current/tests/new_min_cost_flow_reference_test.cpp)・[AtCoder practice2 E](https://atcoder.jp/contests/practice2/tasks/practice2_e)

## 構築・状態と補助型

### operator=

```text
MinCostFlow& operator=(MinCostFlow other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
