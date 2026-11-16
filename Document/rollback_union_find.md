# 巻き戻せるUnion Find

成分の結合を履歴へ残し、直前または保存位置まで戻せます。頂点は`0<=v<n<=INT_MAX`です。

## API

### RollbackUnionFind

```text
explicit RollbackUnionFind(std::size_t n = 0);
```

空／n頂点の独立した成分を構築します。

**制約:** 0<=n<=INT_MAX。

**計算量:** O(n+1)時間・領域。

### find

```text
int find(int v) const;
```

代表頂点を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### root

```text
int root(int v) const;
```

代表頂点を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### leader

```text
int leader(int v) const;
```

代表頂点を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### same

```text
bool same(int x, int y) const;
```

同じ成分ならtrueを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### issame

```text
bool issame(int x, int y) const;
```

同じ成分ならtrueを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### size

```text
int size(int v) const;
int size() const noexcept;
```

引数付きは所属成分の頂点数、引数なしは全頂点数を返します。

**制約:** 引数付きは頂点番号は0以上n未満。

**計算量:** 引数付きはO(log(n+1))、引数なしはO(1)。

### unite

```text
bool unite(int x, int y);
```

新規結合ならtrueです。冗長な呼出しも履歴を1回進めます。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(log(n+1))。

### merge

```text
bool merge(int x, int y);
```

新規結合ならtrueです。冗長な呼出しも履歴を1回進めます。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(log(n+1))。

### undo

```text
void undo();
```

直前の追加呼出しを1回取り消します。

**制約:** 履歴が空でないこと。

**計算量:** O(1)。

### get_state

```text
int get_state() const noexcept;
```

現在の履歴位置を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### snapshot

```text
void snapshot() noexcept;
```

現在の履歴位置を1個だけ保存します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### rollback

```text
void rollback(int state = -1);
```

指定位置、または-1なら保存位置まで戻します。

**制約:** 0<=state<=get_state()、またはstate=-1。現在の枝で得た位置を使います。

**計算量:** 戻す回数kにO(k+1)。

### clear_history

```text
void clear_history() noexcept;
```

成分を保ち、履歴・保存位置を消します。容量は保持します。

**制約:** 古い履歴位置は以後使えません。

**計算量:** O(h+1)の上界。

### reserve_history

```text
void reserve_history(std::size_t k);
```

k回分の履歴容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再確保時O(h+1)、予約済みならO(1)。

### empty

```text
bool empty() const noexcept;
```

頂点数が0ならtrueを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### component_count

```text
int component_count() const noexcept;
```

成分数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### groups

```text
std::vector<std::vector<int>> groups() const;
```

成分ごとの頂点昇順一覧を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1+n log(n+1))時間・O(n+1)追加領域。

### reset

```text
void reset(std::size_t n);
void reset();
```

指定／現在の頂点数で独立な成分へ戻し、履歴・保存位置を消します。

**制約:** 新頂点数mは0以上INT_MAX以下。

**計算量:** 旧頂点数n・旧履歴長hにO(n+h+m+1)。

### clear

```text
void clear() noexcept;
```

頂点と履歴を空にし、確保済み容量を保持します。

**制約:** 古い履歴位置は以後使えません。

**計算量:** O(n+h+1)の上界。

### swap

```text
void swap(RollbackUnionFind& other) noexcept;
void swap(RollbackUnionFind& a, RollbackUnionFind& b) noexcept;
```

履歴・保存位置も含めて交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


同じ成分への結合も履歴を1回進め、`undo`の対象になります。位置は現在の枝だけに有効で、捨てた枝や別インスタンスを選ぶ版番号ではありません。`snapshot`の保存枠は1個で、そこより手前へ戻すと保存位置も切り詰めます。

```cpp
RollbackUnionFind uf(4);
const auto state = uf.get_state();
uf.unite(0, 1);
uf.unite(1, 2);
uf.rollback(state);
assert(!uf.same(0, 2));
```

## 計算量

nは頂点数、hは履歴長、Hは確保済み履歴容量です。構築はO(n+1)、保持領域はO(n+H+1)。代表・同一成分・成分サイズはO(log(n+1))、結合は履歴確保を含め償却O(log(n+1))です。

`undo`はO(1)、`rollback`は戻す回数kに対しO(k+1)、`clear_history`はO(h+1)の上界。引数なし`size`・`empty`・`component_count`・`snapshot`・`get_state`・`swap`はO(1)。`groups`はO(1+n log(n+1))時間・O(n+1)追加領域で、成分は代表順、内部は頂点順です。

`reserve_history`は再確保時O(h+1)、予約済みならO(1)。コピー構築はO(n+h+1)、コピー代入は新旧の頂点数・履歴長の和に比例します。`reset(m)`はO(n+h+m+1)、`clear`はO(n+h+1)の上界です。

## 所有・履歴・検査

コピーは履歴・保存位置も含め独立し、ムーブ元は空です。結合時の確保、コピー、`reset`の失敗では連結状態・履歴・保存位置を保ちますが、容量は増える場合があります。`groups`の戻り値も独立した値です。

`clear_history`は成分を残して履歴の開始点を作り直します。`reset`・`clear`は成分も消し、いずれも古い履歴位置を無効にします。容量は保持します。

`undo`は非空履歴、`rollback`は`0<=state<=get_state()`が前提です。頂点・履歴回数の上限はINT_MAX。`SKIP_RUNTIME_CHECKS`は入力検査を省きますが、容量保護は残します。追加SIMDは`SIRKIT_DISABLE_SIMD`で除外できます。

[完全な使用例](../Examples/rollback_union_find_example.cpp)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#rollback-union-find)
