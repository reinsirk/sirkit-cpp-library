# Project Selection

`TwoVariableSubmodularOpt<COST>`は、二値変数の劣モジュラなコストの総和を最小化します。`true`を始点側、`false`を終点側とする最小カットとして解きます。[本体](../Graph/Project_Selection.cpp)・[使用例](../Examples/project_selection_example.cpp)。

## API

nは変数数、Vは補助頂点込みの頂点数、Eは順方向辺数です。単一項の追加は償却O(1)、集合項はO(|xs|)。項は既存のコストへ加算され、利得Pはコスト−Pを表します。

### TwoVariableSubmodularOpt

```text
TwoVariableSubmodularOpt();
TwoVariableSubmodularOpt(const TwoVariableSubmodularOpt&);
TwoVariableSubmodularOpt(TwoVariableSubmodularOpt&& other);
explicit TwoVariableSubmodularOpt(int n);
TwoVariableSubmodularOpt(int n, COST );
```

n変数・コスト項なしのモデルを作ります。旧inf引数の値は使用しません。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(n)時間・領域。コピーは保持領域に比例、ムーブはO(1)。

### init

```text
void init(int n, COST = 0);
```

n変数のモデルに作り直します。initはO(n+旧V+旧E)です。変数数を保って全項を消すにはclearを使います。clearはO(V+E)で、追加の確保はありません。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(n+旧V+旧E)時間。

### clear

```text
void clear() noexcept;
```

変数数と容量を残して全ての項を消します。

**制約:** なし。

**計算量:** O(V+E)時間。確保しません。

### add_single_cost

```text
void add_single_cost(int xi, COST false_cost, COST true_cost);
```

xiがfalse／trueの場合にF／Tを加える。

**制約:** iは有効な変数番号。F,Tと分解途中のコストもCOSTに収まること。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_psp_penalty

```text
void add_psp_penalty(int xi, int xj, COST cost);
```

xi=trueかつxj=falseに非負費用Cを加える。

**制約:** 変数番号は有効で、C>=0。数値範囲は共通の前提に従います。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_psp_constraint

```text
void add_psp_constraint(int xi, int xj);
```

xi=trueかつxj=falseを禁止する。

**制約:** 変数番号は0..n-1。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### force

```text
void force(int xi, bool value);
```

xiを固定／xi=xjを強制する。

**制約:** 変数番号は0..n-1。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_equality

```text
void add_equality(int xi, int xj);
```

2変数の値が等しいことを強制します。

**制約:** 変数番号は0..n-1。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_submodular_function

```text
void add_submodular_function(int xi, int xj, COST A, COST B, COST C, COST D);
```

FF/FT/TF/TTのコストを加える。i≠jではB+C≥A+D、i=jはA/Dだけ使う。

**制約:** i,jは有効な変数番号。i≠jではB+C≥A+D、i=jではA,Dだけを使います。数値は共通の範囲内です。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_both_true_profit

```text
void add_both_true_profit(int xi, int xj, COST profit);
```

両変数がtrueのとき非負利得Pを加えます。

**制約:** 変数番号は有効で、P>=0。数値範囲は共通の前提に従います。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_both_false_profit

```text
void add_both_false_profit(int xi, int xj, COST profit);
```

両変数がfalseのとき非負利得Pを加えます。

**制約:** 変数番号は有効で、P>=0。数値範囲は共通の前提に従います。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### add_all_true_profit

```text
void add_all_true_profit(const std::vector<int>& xs, COST profit);
```

指定した変数が全てtrueのとき非負利得Pを加えます。空集合もPを得ます。

**制約:** 変数番号は有効で、P>=0。数値範囲は共通の前提に従います。

**計算量:** O(|xs|)時間。

### add_all_false_profit

```text
void add_all_false_profit(const std::vector<int>& xs, COST profit);
```

指定した変数が全てfalseのとき非負利得Pを加えます。空集合もPを得ます。

**制約:** 変数番号は有効で、P>=0。数値範囲は共通の前提に従います。

**計算量:** O(|xs|)時間。

### add_any_true_cost

```text
void add_any_true_cost(const std::vector<int>& xs, COST cost);
```

指定した変数の1個以上がtrueなら非負費用Cを加えます。空集合は0です。

**制約:** 変数番号は有効で、C>=0。数値範囲は共通の前提に従います。

**計算量:** O(|xs|)時間。

### add_any_false_cost

```text
void add_any_false_cost(const std::vector<int>& xs, COST cost);
```

指定した変数の1個以上がfalseなら非負費用Cを加えます。空集合は0です。

**制約:** 変数番号は有効で、C>=0。数値範囲は共通の前提に従います。

**計算量:** O(|xs|)時間。

### add_constant

```text
void add_constant(COST cost);
```

全割当に定数Cを加える。O(1)。

**制約:** Cと累積定数がCOSTに収まること。負のCも使えます。

**計算量:** 償却O(1)。定数項だけの追加はO(1)です。

### solve

```text
COST solve();
```

最小コスト。初回・項追加後は最悪O(V²E)、変更なしの再照会はO(1)。

**制約:** 共通の数値範囲を満たすこと。制約が矛盾すればdomain_errorです。

**計算量:** 変更後は最悪O(V²E)時間・O(V+E)領域。変更なしの再照会はO(1)。

### reconstruct

```text
std::vector<bool> reconstruct();
```

最適な割当をvector<bool>で返します。必要に応じてsolveを呼び、割当の復元に追加でO(n)かかります。同じ最小コストを与える割当のうちどれを選ぶかは未規定です。

**制約:** 共通の数値範囲を満たすこと。制約が矛盾すればdomain_errorです。

**計算量:** solveの費用＋O(n)時間・領域。

### solve_with_assignment

```text
std::pair<COST, std::vector<bool>> solve_with_assignment();
```

{最小コスト,最適割当}。solve＋O(n)。

**制約:** 共通の数値範囲を満たすこと。制約が矛盾すればdomain_errorです。

**計算量:** solveの費用＋O(n)時間・領域。

### empty

```text
bool empty() const noexcept;
```

変数数が0ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### variable_count

```text
int variable_count() const noexcept;
```

変数数を返します。

**制約:** なし。

**計算量:** O(1)。

### node_count

```text
int node_count() const noexcept;
```

補助頂点を含む頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### reserve_nodes

```text
void reserve_nodes(std::size_t count);
```

頂点容量を予約する。O(V)。辺容量の予約ではない。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### swap

```text
void swap(TwoVariableSubmodularOpt& other) noexcept;
void swap(TwoVariableSubmodularOpt& a, TwoVariableSubmodularOpt& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。

### operator<<

```text
std::ostream& operator<<(std::ostream& out, const TwoVariableSubmodularOpt& optimizer);
```

現在の順方向辺をfrom→to(flow/capacity)形式で出力する。O(E)。

**制約:** 変数番号、非負費用・利得、劣モジュラ条件と数値範囲は下記の前提に従います。

**計算量:** O(E)。


集合内の変数番号は重複しても構いません。費用・利得0の集合項は無操作で、未参照の添字は検査しません。

```cpp
TwoVariableSubmodularOpt<std::int64_t> model(3);
model.add_single_cost(0, 0, -8); // 選ぶと8の利得。
model.add_psp_constraint(0, 1);  // 0を選ぶなら1も選ぶ。
model.add_any_true_cost({0, 1}, 3);
model.add_any_false_cost({0, 1, 2}, 2);
auto [cost, assignment] = model.solve_with_assignment();
```

## 前提・所有・失敗

COSTは64ビット以下の符号付き整数です。入力・分解途中のコスト・定数項・最適値もCOSTに収まり、有限辺容量の総和は`std::make_unsigned_t<COST>`の最大値未満とします。超過は`overflow_error`、制約矛盾はsolve時に`domain_error`です。領域はO(V+E)。

項を追加した後のsolveは残余から再開し、定数の追加だけならフローを再計算しません。制約が矛盾した場合はclearまたはinitで作り直します。最適値が型の範囲を超えた場合は、定数を調整した後に取得し直せます。

コピーは独立、move元は空の0変数モデルです。項追加・init・コピー代入の失敗時は変数数・項・最適値を保ちますが、容量は変わり得ます。solveの確保失敗後も再試行できます。clearは容量を再利用し、initは隣接配列を作り直します。

`SKIP_RUNTIME_CHECKS`で添字・非負性・劣モジュラ性の検査を除きます。内部容量・算術の上限は維持します。[内部処理・出典・検証の記録](../Audit/Changes/graph-document-details-2026-11-15.md)を参照してください。

## 構築・状態と補助型

### operator=

```text
TwoVariableSubmodularOpt& operator=(const TwoVariableSubmodularOpt& other);
TwoVariableSubmodularOpt& operator=(TwoVariableSubmodularOpt&& other);
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
