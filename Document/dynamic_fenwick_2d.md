# 動的2次元Fenwick Tree

`DynamicFenwickTree2D<T>`は零配列への単点加算と矩形和を扱います。更新位置の事前登録は不要です。寸法は非負のint、添字は0始まり、区間は半開区間です。

## API

### DynamicFenwickTree2D

```text
explicit DynamicFenwickTree2D(int n = 0, int m = 0);
```

n行m列の零配列を作ります。

**制約:** n,m>=0。

**計算量:** O(1)。

### add

```text
void add(int i, int j, const T& x);
```

位置(i,j)へxを加算します。

**制約:** 0<=i<rows()、0<=j<cols()。

**計算量:** 通常は期待O(log(n+1) log(m+1))。初回準備・変換・再確保の追加費用は共通節を参照。

### set

```text
void set(int i, int j, const T& x);
```

位置(i,j)をxに置換します。

**制約:** 0<=i<rows()、0<=j<cols()。

**計算量:** 通常は期待O(log(n+1) log(m+1))。初回準備・変換・再確保の追加費用は共通節を参照。

### get

```text
T get(int i, int j) const;
```

位置(i,j)の値を返します。

**制約:** 0<=i<rows()、0<=j<cols()。

**計算量:** 通常は期待O(log(n+1) log(m+1))。初回準備・変換・再確保の追加費用は共通節を参照。

### sum

```text
T sum(int n, int m) const;
T sum(int nl, int ml, int nr, int mr) const;
```

[0,n)×[0,m)／[nl,nr)×[ml,mr)の和を返します。

**制約:** 各端点は0以上で寸法以下、各左端<=右端。

**計算量:** 通常は期待O(log(n+1) log(m+1))。初回準備・変換・再確保の追加費用は共通節を参照。

### sum_all

```text
T sum_all() const;
```

全体の和を返します。

**制約:** 途中の演算もTに収まること。

**計算量:** 通常は期待O(log(n+1) log(m+1))。初回準備・変換・再確保の追加費用は共通節を参照。

### rows

```text
int rows() const noexcept;
```

行数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### cols

```text
int cols() const noexcept;
```

列数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

どちらかの寸法が0ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear();
```

寸法と容量を保ったまま、値を零に戻します。

**制約:** 追加の制約はありません。

**計算量:** 行索引長I・疎行索引容量HにO(I+H)。

### reset

```text
void reset(int n, int m);
```

寸法と値を設定し直し、容量を再利用します。

**制約:** n,m>=0。

**計算量:** O(I+H)。行の値配列は即座に走査しません。

### reserve

```text
void reserve(std::size_t nodes);
```

行プールと疎索引の容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再配置時は予約前後の全保持容量C/C′に期待O(C+C′)。

### stored_nodes

```text
std::size_t stored_nodes() const noexcept;
```

更新経路上の仮想Fenwickセル数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### swap

```text
void swap(DynamicFenwickTree2D& other) noexcept;
```

寸法・値・確保済み領域を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


Tは零の既定構築と加減算に対応し、途中結果も表現範囲に収めます。`get`・`set`・`add`の座標は配列内、和の端点は各寸法以下で、左端<=右端が前提です。

```cpp
DynamicFenwickTree2D<std::int64_t> tree(1000, 1000);
tree.add(10, 20, 7);
auto total = tree.sum(0, 0, 11, 21); // 7。
tree.set(10, 20, 3);
auto value = tree.get(10, 20);       // 3。
```

## 計算量・所有

通常の更新・矩形和・単点取得は期待O(log(n+1) log(m+1))です。初回更新の行索引準備はn<=100万でO(n)、直接配列を使う行の初期化・変換はO(m)です。容量再確保にも保持量に比例する費用が別途かかります。メモリは行索引と各行が保持する点・セル・配列に依存します。

読み取り時にノードや領域は増えません。`stored_nodes`が数えるのは更新経路上の仮想Fenwickセルです。実際の木ノード数とは異なり、零を加算したセルや値が相殺されたセルも数えます。

コピーは独立し、move・swapは領域の所有権を移します。`clear`は寸法を保ち、`reset`は寸法も変え、対応する領域を再利用します。同梱の[1次元版](dynamic_fenwick.md)も使えます。

既定の寸法・添字・区間検査は`SKIP_RUNTIME_CHECKS`で除けます。容量・添字の内部上限は維持します。

[完全な使用例](../Examples/dynamic_fenwick_2d_example.cpp)
