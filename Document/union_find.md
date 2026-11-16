# Union Find

連結成分の結合と、同じ成分かの照会を行います。頂点は`0<=v<n<=INT_MAX`です。

## API

### UnionFind

```text
explicit UnionFind(std::size_t n = 0);
```

n頂点を独立な成分として構築します。

**制約:** n<=INT_MAX。

**計算量:** O(n+1)時間・領域。

### root

```text
int root(int i);
```

頂点iの代表を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### leader

```text
int leader(int i);
```

頂点iの代表を返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### merge

```text
bool merge(int a, int b);
```

未接続の成分を結合したときだけtrueを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### unite

```text
void unite(int a, int b);
```

成分を結合します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### same

```text
bool same(int a, int b);
```

同じ成分ならtrueを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### issame

```text
bool issame(int a, int b);
```

同じ成分ならtrueを返します。

**制約:** 頂点番号は0以上n未満。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### size

```text
int size(int i);
int size() const noexcept;
```

引数付きは所属成分の頂点数、引数なしは全頂点数を返します。

**制約:** 引数付きは頂点番号は0以上n未満。

**計算量:** 引数付きは償却O(α(n+1))、1回の最悪O(log(n+1))。 引数なしはO(1)。

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
std::vector<std::vector<int>> groups();
```

代表番号順の成分一覧を返し、各成分内の頂点は昇順です。

**制約:** 追加の制約はありません。

**計算量:** 償却O(1+n α(n+1))時間・O(n+1)追加領域。

### reset

```text
void reset(std::size_t n);
void reset();
```

指定した頂点数／現在の頂点数で全成分を独立にし、容量を再利用します。

**制約:** 新頂点数m<=INT_MAX。

**計算量:** 旧長n・新長mにO(n+m+1)。

### clear

```text
void clear() noexcept;
```

空に戻し、確保済み容量を保持します。

**制約:** 追加の制約はありません。

**計算量:** O(n+1)の上界。

### swap

```text
void swap(UnionFind& other) noexcept;
void swap(UnionFind& left, UnionFind& right) noexcept;
```

内容を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


代表は最小頂点とは限りません。同じサイズの結合では第1引数の代表を残し、`groups`は代表番号順、各成分内は頂点番号順です。

```cpp
UnionFind uf(5);
uf.merge(0, 1);
uf.unite(1, 2);
assert(uf.same(0, 2) && uf.size(0) == 3);
auto groups = uf.groups();
```

## 計算量

nは頂点数、Cは確保済み容量です。`root`・`leader`・結合・同一成分判定・`size(v)`は償却O(α(n+1))、1回の最悪はO(log(n+1))。構築・コピーはO(n+1)、`groups`は償却O(1+n α(n+1))時間・O(n+1)追加領域です。

引数なし`size`・`empty`・`component_count`・`swap`・ムーブ構築はO(1)。`reset(m)`・コピー代入は旧長nと新長mに対しO(n+m+1)、`clear`・ムーブ代入はO(n+1)の上界です。保持領域はO(C+1)。

## 所有・検査

コピーは独立し、ムーブ元は空です。`reset`・コピー代入の確保失敗では連結状態と成分数を保ちます。`groups`の失敗では経路圧縮が残る場合がありますが、連結成分・代表・サイズは保ちます。照会でも内部の経路圧縮を行います。

範囲外の頂点は既定で`out_of_range`です。`SKIP_RUNTIME_CHECKS`で頂点検査を省きますが、頂点数・容量の上限は維持します。

[完全な使用例](../Examples/union_find_example.cpp)
