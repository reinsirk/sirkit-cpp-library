# 動的Fenwick Tree

`DynamicFenwickTree<S,T>`は全要素が零の配列に対して単点加算と区間和を扱います。更新位置を事前に登録する必要はありません。Sはbool以外の64bit以下の整数型、Tは零初期化・加減算に対応する値型です。

## API

### DynamicFenwickTree

```text
explicit DynamicFenwickTree(S n = 0);
```

長さnの零配列を作ります。

**制約:** n>=0。

**計算量:** O(1)。

### add

```text
void add(S k, const T& x);
```

位置kへxを加算します。

**制約:** 0<=k<size()。途中の演算もTに収まること。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### set

```text
void set(S k, const T& x);
```

位置kをxに置き換えます。

**制約:** 0<=k<size()。途中の演算もTに収まること。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### sum

```text
T sum(S k) const;
T sum(S l, S r) const;
```

[0,k)／[l,r)の和を返します。

**制約:** 0<=k<=size()、0<=l<=r<=size()。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### sum_all

```text
T sum_all() const;
```

全要素の和を返します。

**制約:** 途中結果もTに収まること。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### get

```text
T get(S k) const;
```

位置kの値を返します。

**制約:** 0<=k<size()。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### operator[]

```text
T operator[](S k) const;
```

位置kの値を返します。書換え用の参照ではありません。

**制約:** 0<=k<size()。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### lower_bound

```text
S lower_bound(T w) const;
```

累積和がw以上となる最初の位置を返し、なければsize()です。w<=0なら0です。

**制約:** 全要素が非負。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### upper_bound

```text
S upper_bound(T w) const;
```

累積和がwより大きくなる最初の位置を返し、なければsize()です。w<0なら0です。

**制約:** 全要素が非負。

**計算量:** 通常は期待償却O(log(n+1))。変換・再確保の追加費用は共通の計算量節を参照。

### size

```text
S size() const noexcept;
```

配列長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

配列長が0ならtrueです。全要素0の判定ではありません。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear();
```

長さ・容量を保ち、全値を零へ戻します。

**制約:** 追加の制約はありません。

**計算量:** 全保持容量Cに最悪O(C)。

### reset

```text
void reset(S n);
```

配列長をnに変え、全要素を零に戻します。

**制約:** n>=0。

**計算量:** 全保持容量Cに最悪O(C)。

### reserve

```text
void reserve(std::size_t nodes);
```

Fenwickセル数を目安に容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再配置時は期待O(C+C′+log(n+1))。C/C′は予約前後の全保持容量。

### stored_nodes

```text
std::size_t stored_nodes() const noexcept;
```

更新経路に含まれるFenwickセルの数を、重複を除いて返します。

**制約:** 追加の制約はありません。

**計算量:** 通常O(1)、連続追加の途中はO(log(n+1))。


添字は0始まり、区間は`[l,r)`です。`lower_bound`・`upper_bound`は全要素非負が前提です。`lower_bound(w<=0)`と`upper_bound(w<0)`は0を返します。演算の途中結果もTの範囲に収めます。

```cpp
DynamicFenwickTree<std::int64_t, std::int64_t> tree(1000000000);
tree.add(10, 3);
tree.add(20, 5);
auto sum = tree.sum(10, 21); // 8。
auto value = tree.get(20);   // 5。
```

## 計算量・所有

通常の更新・和・探索は期待償却O(log(n+1))です。密な領域への変換にはO(n)、保持領域の再確保には保持量に比例する費用が別途かかります。保持形式は更新状況に応じて選び、読み取りは領域を確保せず登録数も増やしません。

`stored_nodes`は更新経路のFenwickセルの和集合の件数で、実際の保持点数・木ノード数ではありません。零の加算や相殺済みセルも数え、reserveでは変わりません。

コピーは独立し、ムーブ元は長さ0です。`clear`は寸法を保って容量を再利用し、`reset`は寸法も変えます。整数専用経路のコピー代入は失敗時に旧内容を保ち、その他の値型では旧長の零状態へ戻します。

入力検査は`SKIP_RUNTIME_CHECKS`で省けますが、有効な寸法・添字・区間の前提は同じです。

[完全な使用例](../Examples/dynamic_fenwick_example.cpp)
