# 完全永続Union Find

過去の版を保持します。保存したどの版からでも、結合を行って新しい版を作れます。
永続配列と成分サイズによる結合を使い、経路圧縮は行いません。代表の取得・連結判定はO(log²(n+1))、結合は償却O(log²(n+1))です。
全操作を先読みできる問題では、版の分岐を木で表し、rollback UFで処理する方が速い場合があります。このライブラリでは先読みせずに保存版を照会・更新できます。

## Union FindのAPI

`Version`は`PersistentUnionFind::Version`を指し、実体は非所有の`Node*`です（`Node = PersistentArray<int>::Node`）。代表頂点番号の`int`とは区別します。頂点は`0<=i<n`、指定版は同じ保存領域の有効なハンドルを渡します。

### PersistentUnionFind

```text
explicit PersistentUnionFind(std::size_t n = 0);
```

n頂点を独立した成分として構築します。

**制約:** n<=INT_MAX。

**計算量:** B=16としてO(B+log_B(n+1))時間。

### find

```text
int find(int i) const;
int find(int i, Version version) const;
```

現在版／指定版で代表頂点を返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。

### leader

```text
int leader(int i) const;
int leader(int i, Version version) const;
```

現在版／指定版で代表頂点を返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。

### root

```text
int root(int i) const;
int root(int i, Version version) const;
```

現在版／指定版で代表頂点を返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。

### find_with_size

```text
std::pair<int,int> find_with_size(int i) const;
std::pair<int,int> find_with_size(int i, Version version) const;
```

現在版／指定版で{代表頂点, 負の成分サイズ}を返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。

### size

```text
int size(int i) const;
int size(int i, Version version) const;
int size() const noexcept;
```

現在版／指定版で頂点iの成分サイズ。引数なしは全頂点数を返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。引数なしはO(1)。

### same

```text
bool same(int i, int j) const;
bool same(int i, int j, Version version) const;
```

現在版／指定版で同じ成分かを返します。

**制約:** 指定する頂点は0以上n未満、版は同じ保存領域の有効なハンドル。

**計算量:** O(log²(n+1))。

### issame

```text
bool issame(int i, int j) const;
bool issame(int i, int j, Version version) const;
```

`same`の別名です。同じ引数で現在版／指定版を照会し、戻り値・前提も同じです。計算量はO(log²(n+1))です。

### unite

```text
bool unite(int i, int j);
bool unite(int i, int j, Version version);
```

現在版または指定版から分岐して結合し、結果を現在版にします。新たに結合したときだけtrueを返します。結果の版のハンドルは`version()`で取得します。

**制約:** 頂点は範囲内、版は同じ保存領域の有効なハンドル。

**計算量:** 償却O(log²(n+1))時間、成功時O(log(n+1))ノードを追加。

### merge

```text
bool merge(int i, int j);
bool merge(int i, int j, Version version);
```

現在版／指定版から分岐して結合し、結果を現在版にします。新規結合したときだけtrueです。

**制約:** 頂点は範囲内、版は同じ保存領域の有効なハンドル。

**計算量:** 償却O(log²(n+1))時間、成功時O(log(n+1))ノードを追加。

### version

```text
Version version() const noexcept;
```

現在版の非所有ハンドルを返します。

**制約:** 保持する間は同じ保存領域の所有者を存続させます。

**計算量:** O(1)。

### restore

```text
void restore(Version version);
```

現在版を選択します。

**制約:** 同じ保存領域の有効な版。非空UFではnullを指定できません。

**計算量:** O(1)。

### component_count

```text
int component_count() const;
int component_count(Version version) const;
```

現在版／指定版の成分数を返します。

**制約:** 同じ保存領域の有効な版。

**計算量:** O(n log(n+1))。

### groups

```text
std::vector<std::vector<int>> groups() const;
std::vector<std::vector<int>> groups(Version version) const;
```

現在版／指定版の成分一覧を返します。各成分内の頂点は昇順です。

**制約:** 同じ保存領域の有効な版。

**計算量:** O(n log²(n+1))時間・O(n+1)返却領域。


`[,version]`は版を末尾に指定するオーバーロードです。頂点は`0<=i<n`。結合不要でも、指定版が現在版になります。

```cpp
using UF = PersistentUnionFind;
UF uf(4);
auto initial = uf.version();
uf.unite(0, 1);
UF::Version left = uf.version();
std::vector<UF::Version> versions{initial, left};
uf.unite(2, 3, versions[0]);
assert(!uf.same(0, 1) && uf.issame(0, 1, versions[1]));
```

nは頂点数です。`find`・`leader`・`root`・`find_with_size`・`size(i)`・`same`・`issame`はO(log²(n+1))、結合は償却O(log²(n+1))時間で、成功時O(log(n+1))ノードを追加します。構築は後述の定数配列と同じ費用です。版の取得・選択・引数なし`size`はO(1)、`component_count`はO(n log(n+1))、`groups`はO(n log²(n+1))時間です。

## 永続配列のAPI

`PersistentArray<T,shift=4>`も単独で使えます。`B=2^shift`、`1<=shift<=8`、添字はint64_t、`0<=n<=INT64_MAX`。Tはコピー構築・代入に対応し、初期値省略時は`T{}`を構築できることが必要です。

### PersistentArray

```text
explicit PersistentArray(std::int64_t n = 0, T ID = T{});
explicit PersistentArray(const std::vector<T>& values, T ID = T{});
```

全要素がIDの疎な配列を作るか、vectorから構築します。

**制約:** 0<=n<=INT64_MAX、1<=shift<=8。Tのコピー・代入が必要です。

**計算量:** 定数配列はO(B+log_B(n+1))時間。vector版はO(B+n+(n/B+1)log_B(n+1))時間・O(n+B)領域。

### get

```text
T get(std::int64_t k) const;
T get(Node* version, std::int64_t k) const;
```

現在版／指定版の値をコピーして返します。

**制約:** 0<=k<size()。同じ保存領域の有効な版、nullは全要素IDの版。

**計算量:** O(log_B(n+1))。

### get_two

```text
std::pair<T,T> get_two(std::int64_t a, std::int64_t b) const;
std::pair<T,T> get_two(Node* version, std::int64_t a, std::int64_t b) const;
```

2要素を指定した順に返します。同じ添字も使えます。

**制約:** 添字は範囲内、版はgetと同じ条件。

**計算量:** O(log_B(n+1))。

### update

```text
Node* update(std::int64_t k, const T& value);
Node* update(Node* version, std::int64_t k, const T& value);
```

1点を変更した新しい版を現在版にし、そのハンドルを返します。

**制約:** 添字は範囲内、版はgetと同じ条件。

**計算量:** 償却O(B log_B(n+1))時間・追加領域。

### update_two

```text
Node* update_two(std::int64_t a, const T& x, std::int64_t b, const T& y);
Node* update_two(Node* version, std::int64_t a, const T& x, std::int64_t b, const T& y);
```

2点を同時更新した版を現在版にし、ハンドルを返します。同じ添字ならyを優先します。

**制約:** 添字は範囲内、版はgetと同じ条件。

**計算量:** 償却O(B log_B(n+1))時間・追加領域。

### restore（PersistentArray）

```text
void restore(Node* version);
```

現在版を選択します。

**制約:** 同じ保存領域の有効なハンドル。nullは全要素IDの版。

**計算量:** O(1)。

### root（公開データ）

```text
Node* root;
```

現在版の非所有ハンドルです。

**制約:** 同じ保存領域の所有者が残る間だけ有効です。

**計算量:** 読取りO(1)。

### size（PersistentArray）

```text
std::int64_t size() const noexcept;
```

配列の要素数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


配列のnullptr版は全要素がIDの版です。Union Findの非空インスタンスでは、公開APIから得た非null版を渡します。

`get`・`get_two`はO(log_B(n+1))、更新は償却O(B log_B(n+1))時間・追加領域です。定数配列の構築はO(B+log_B(n+1))時間、vectorからはO(B+n+(n/B+1)log_B(n+1))時間・O(n+B)領域です。

## 版の寿命・失敗

コピーは保存領域を共有し、現在版を個別に保持します。一方の更新は他方の現在版・過去版を変えません。ムーブ元は空です。

版ハンドルは同じ保存領域を所有するインスタンス（UFでは元のUFまたはそのコピー）が残る間だけ有効です。ハンドルに所有権はありません。`delete`や直接の参照外しは行いません。別の保存領域のハンドルや寿命が切れたハンドルは渡せません。並行更新には外部同期が必要です。最後の所有者が破棄されると全版を解放します。

更新失敗でも現在版・保存版を保ちますが、作成途中のノードや確保済み容量は残る場合があります。公開`arr`の直接変更はUnion Findの不変条件を壊すため、通常操作には使いません。

既定では添字・版の所有元を検査し、非空Union Findのnull版は`invalid_argument`、範囲外は`out_of_range`です。寿命切れ検出の保証はありません。`SKIP_RUNTIME_CHECKS`で入力検査を省いても、サイズ・ノード容量の保護は維持します。

[完全な使用例](../Examples/persistent_union_find_example.cpp)・[実装・検証の記録](../Audit/Changes/document-detail-before-concise-data-structure-2026-11-15.md#persistent-union-find)
