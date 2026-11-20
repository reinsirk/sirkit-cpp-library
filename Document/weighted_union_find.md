# 重み付きUnion Find

`potential(b)-potential(a)=w`という差の制約を管理します。頂点は`0<=v<n<=INT_MAX`です。

## API

### UnionFindW

```text
explicit UnionFindW(std::size_t n = 0);
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
bool merge(int a, int b, T w);
```

未接続の成分を結合したときだけtrueを返します。既接続なら差が矛盾していてもfalseです。

**制約:** 頂点番号は0以上n未満。かつ、途中の重み演算がTの範囲内。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### unite

```text
void unite(int a, int b, T w);
```

成分を結合します。既接続の矛盾は無視します。

**制約:** 頂点番号は0以上n未満。かつ、途中の重み演算がTの範囲内。

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

### weight

```text
T weight(int i);
```

代表の重みを0とした頂点iの重みを返します。

**制約:** 頂点番号は0以上n未満。かつ、途中の重み演算がTの範囲内。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### diff

```text
T diff(int a, int b);
```

potential(b)-potential(a)を返します。

**制約:** 頂点番号は0以上n未満。 aとbが同じ成分。かつ、途中の重み演算がTの範囲内。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### try_diff

```text
std::optional<T> try_diff(int a, int b);
```

差を返し、別成分ならnulloptを返します。

**制約:** 頂点番号は0以上n未満。かつ、途中の重み演算がTの範囲内。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

### constrain

```text
bool constrain(int a, int b, T w);
```

未接続なら結合してtrue、既接続なら差がwと一致する場合だけtrueを返します。

**制約:** 頂点番号は0以上n未満。 Tに==が必要。かつ、途中の重み演算がTの範囲内。

**計算量:** 償却O(α(n+1))、1回の最悪O(log(n+1))。

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
void swap(UnionFindW<T>& other) noexcept;
void swap(UnionFindW<T>& left, UnionFindW<T>& right) noexcept;
```

内容を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


同じサイズの結合では第1引数の代表を残します。`constrain`はその制約の整合性だけを返し、矛盾フラグは保存しません。

```cpp
UnionFindW<std::int64_t> uf(4);
uf.unite(0, 1, 10);
uf.merge(1, 2, -3);
assert(uf.diff(0, 2) == 7);
assert(!uf.constrain(0, 2, 8));
assert(!uf.try_diff(0, 3));
```

## 重みの型・制約

Tは`T{}`を零とする可換加法群で、コピー・ムーブ、`+`・`-`・単項`-`・`+=`・`==`が必要です。通常操作の算術・代入は非送出が前提で、浮動値の許容誤差判定はありません。符号なし整数は型幅の剰余です。

符号付き整数は、重みの累積・根間の補正・符号反転を含む**途中結果もTに収めます**。最終的な差だけが収まる場合や、値を変えてから例外を投げる独自型の状態維持は保証しません。

## 計算量・所有

Tの操作をO(1)、頂点数をnとすると、代表探索・差・結合・整合判定は償却O(α(n+1))、1回の最悪はO(log(n+1))です。`groups`は償却O(1+n α(n+1))時間・O(n+1)追加領域。構築とコピーはO(n+1)。保持領域は確保済み容量Cに対しO(C+1)です。

引数なし`size`・`empty`・`component_count`・`swap`・ムーブ構築はO(1)。`reset(m)`とコピー代入は旧長nも含めO(n+m+1)、`clear`とムーブ代入はO(n+1)の上界です。

コピーは独立し、ムーブ元は空です。コピー代入・`reset`の確保や値の構築に失敗しても成分・成分数・差を保ちますが、容量は増える場合があります。照会・`groups`でも経路圧縮を行い、`groups`の確保失敗でも論理的な成分・差を保ちます。

頂点範囲外は`out_of_range`、別成分への`diff`は`invalid_argument`です。`SKIP_RUNTIME_CHECKS`でこれらを省きますが、容量保護は維持します。追加SIMDは`SIRKIT_DISABLE_SIMD`で除外できます。

[完全な使用例](../Examples/union_find_w_example.cpp)
