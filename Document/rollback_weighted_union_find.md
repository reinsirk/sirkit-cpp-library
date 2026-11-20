# 巻き戻せる重み付きUnion Find

`potential(y)-potential(x)=w`という差の制約を追加し、逆順に取り消せます。

## API

nは頂点数、hは履歴長です。Tの演算・コピー・破棄をO(1)として記します。

### RollbackWeightedUnionFind

```text
RollbackWeightedUnionFind();
explicit RollbackWeightedUnionFind(int n);
```

空の状態、またはn頂点がそれぞれ独立した状態を作ります。

**制約:** 0<=n<=INT_MAX。

**計算量:** O(n+1)時間・領域。

### find

```text
int find(int v) const;
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

### size

```text
int size(int v) const;
int size() const noexcept;
```

引数付きは所属成分の頂点数、引数なしは全頂点数を返します。

**制約:** 引数付きは頂点番号は0以上n未満。

**計算量:** 引数付きはO(log(n+1))、引数なしはO(1)。

### root_with_weight

```text
std::pair<int,T> root_with_weight(int v) const;
```

{代表, 代表からvへの差}を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### get_weight

```text
T get_weight(int v) const;
```

代表からvへの重みの差を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### diff

```text
T diff(int x, int y) const;
```

potential(y)-potential(x)を返します。

**制約:** 頂点番号は0以上n未満。 xとyは同じ成分。

**計算量:** O(log(n+1))。

### try_diff

```text
std::optional<T> try_diff(int x, int y) const;
```

差を返し、未連結ならnulloptを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** O(log(n+1))。

### consistent

```text
bool consistent(int x, int y, const T& w) const;
```

未連結または現在の差がwならtrueです。保存可能性までは保証しません。

**制約:** 頂点番号は0以上n未満。 Tに==が必要。

**計算量:** O(log(n+1))。

### unite

```text
bool unite(int x, int y, T w);
```

新たに結合した場合はtrueを返します。すでに同じ成分に属する頂点を指定した場合も履歴を1回進めますが、重みの差は検査しません。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(log(n+1))。

### unite_checked

```text
MergeResult unite_checked(int x, int y, T w);
```

Merged／AlreadyConsistent／Contradictionを返します。どの場合も履歴を1回進めます。

**制約:** 頂点番号は0以上n未満。 Tに==が必要。

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

指定した履歴位置まで戻します。-1を指定すると保存位置まで戻します。

**制約:** 0<=state<=get_state()、またはstate=-1。現在の枝で得た位置を使います。

**計算量:** 戻す回数kにO(k+1)。

### clear_history

```text
void clear_history() noexcept;
```

成分と重みの差を保ち、履歴・保存位置を消します。容量は保持します。

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

成分ごとに頂点を番号の昇順で返します。

**制約:** 追加の制約はありません。

**計算量:** O(1+n log(n+1))時間・O(n+1)追加領域。

### reset

```text
void reset(int n);
void reset();
```

指定／現在の頂点数で独立な成分へ戻し、履歴・保存位置を消します。

**制約:** 新頂点数mは0以上INT_MAX以下。

**計算量:** 旧頂点数n・旧履歴長hにO(n+h+m+1)。

### clear

```text
void clear();
```

頂点と履歴を空にし、確保済み容量を保持します。

**制約:** 古い履歴位置は以後使えません。

**計算量:** O(n+h+1)の上界。

### swap

```text
void swap(RollbackWeightedUnionFind<T>& other) noexcept;
void swap(RollbackWeightedUnionFind<T>& a, RollbackWeightedUnionFind<T>& b) noexcept;
```

履歴・保存位置も含めて交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


冗長または矛盾した制約を追加した場合も、履歴を1回進めます。矛盾した制約を追加しても成分と重みの差は変わらず、矛盾フラグも残りません。

```cpp
RollbackWeightedUnionFind<std::int64_t> uf(3);
uf.unite(0, 1, 4);
const int state = uf.get_state();
uf.unite(1, 2, -7);
assert(uf.diff(0, 2) == -3);
uf.rollback(state);
assert(!uf.same(0, 2));
```

## 型・履歴の制約

Tは零の構築、コピー・ムーブ・代入、加減算を持つ可換加法群です。整合性判定には`==`も必要です。符号なし整数は型幅の剰余、浮動値は許容誤差判定なしです。

標準の符号付き整数では途中和の相殺も扱い、照会の最終値がT外なら`overflow_error`です。結合時は選ばれた根間の差をTに保存できる必要があります。`consistent`は保存可能性までは保証しません。128ビット拡張が使えない場合は標準2語整数を使い、`SIRKIT_FORCE_STANDARD_ARITHMETIC`で選択できます。

頂点は`0<=v<size()`、履歴位置は`0<=state<=get_state()`、頂点数と履歴長はINT_MAX以下です。位置は現在の枝だけに有効です。保存位置より前へ戻すと保存位置も切り詰め、`reset`・`clear`・`clear_history`は古い位置を無効にします。

## 所有・例外・検査

確保済み履歴容量Hに対し保持領域O(n+H+1)、コピーはO(n+h+1)で独立です。ムーブ構築はO(1)で元は空、ムーブ代入には旧状態の破棄も含みます。

`reset`の零構築・確保の失敗では成分・差・履歴を保ちますが、容量は増える場合があります。標準整数・通常のmodintでは結合の履歴確保失敗でも状態を保持します。値を変えてから投げる独自型の代入など、全例外の状態維持は保証しません。有効な履歴の巻き戻しはTの算術・構築・代入を行いません。

頂点、空履歴のundo、不正な位置、未連結のdiffは既定で検査します。`SKIP_RUNTIME_CHECKS`で省く場合も前提は同じで、容量・整数演算の保護は維持します。追加SIMDは`SIRKIT_DISABLE_SIMD`で除外できます。

[完全な使用例](../Examples/rollback_weighted_union_find_example.cpp)・[Verifyの状態](../Verify/current/README.md)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#rollback-weighted-union-find)
