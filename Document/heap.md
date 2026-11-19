# 最小ヒープ

`DataStructure/heap.cpp`の`Heap`は、`int`を所有する最小ヒープです。外部ライブラリに依存せず貼り付けられます。使用例は[heap_example.cpp](../Examples/heap_example.cpp)です。

`SIRKIT_HEAP_FORCE_QUATERNARY`を定義すると、処理系による既定の選択にかかわらず、根が2子で他の内部節点が最大4子の構成を使います。

## API

### Heap

```text
Heap();
explicit Heap(std::vector<int> values);
Heap(std::initializer_list<int> values);
template<class Iterator> Heap(Iterator first, Iterator last);
```

空／指定した値の最小ヒープを構築します。vectorは値で受け取り所有します。

**制約:** iterator範囲が有効でintへ変換できること。

**計算量:** 空はO(1)、N要素からの構築はO(N)。

### empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### top

```text
int top() const noexcept;
```

最小値を返し、空なら-1です。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### top_or

```text
int top_or(int fallback) const noexcept;
```

最小値を返し、空ならfallbackです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### push

```text
void push(int value);
```

値を1個追加します。

**制約:** 追加の制約はありません。

**計算量:** 償却O(log(N+1))。

### pop

```text
void pop() noexcept;
```

最小値を1個削除します。空なら無操作です。

**制約:** 追加の制約はありません。

**計算量:** O(log(N+1))。

### pop_value

```text
int pop_value();
```

最小値を削除して返します。

**制約:** 空でないこと。検査時の違反はout_of_range。

**計算量:** O(log(N+1))。

### replace_top

```text
int replace_top(int value);
```

最小値をvalueに置換し、旧最小値を返します。

**制約:** 空でないこと。検査時の違反はout_of_range。

**計算量:** O(log(N+1))。

### push_pop

```text
int push_pop(int value) noexcept;
```

追加後の最小値を削除して返します。空ならvalueです。

**制約:** 追加の制約はありません。

**計算量:** O(log(N+1))。

### clear

```text
void clear() noexcept;
```

空にして容量を保持します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### reserve

```text
void reserve(std::size_t n);
```

n要素分の容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再確保時O(N)、確保済みならO(1)。


`pop_value()`と`replace_top()`は、空なら`std::out_of_range`を送出します。`SKIP_RUNTIME_CHECKS`を定義するとこの検査を除き、非空が前提となります。`top()`の`-1`は空の目印と値の区別がつかないため、必要なら`empty()`を併用します。

コピーは独立した配列を持ち、ムーブは配列の所有権を移します。負値・重複値・`int`の両端値も格納できます。

[完全な使用例](../Examples/heap_example.cpp)
