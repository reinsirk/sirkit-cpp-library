# 整数の基数ソート

`bool`を除く8・16・32・64ビットの標準整数型を、安定な昇順に整列します。符号付きの最小値・最大値も扱います。

## API

### RadixSort

```text
template<class T> void RadixSort(std::vector<T>& values);
template<class T> void RadixSort(std::vector<T>& values, T maximum);
```

安定な昇順に直接整列します。入力配列の容量・要素の保管先は変えません。

**制約:** Tは冒頭の整数型。maximumを指定するときは全要素<=maximum。

**計算量:** O(Bn)時間・O(n)追加領域。

### RadixSortOrder

```text
template<class T>
std::vector<std::size_t> RadixSortOrder(const std::vector<T>& values);
```

昇順の元添字列を返します。同値は元の順で、入力は不変です。

**制約:** Tは冒頭の整数型。

**計算量:** O(Bn)時間・O(n)追加領域。

### RadixSortWorkspace::sort

```text
void sort(std::vector<T>& values);
void sort(std::vector<T>& values, T maximum);
```

作業領域を再利用して安定昇順に整列します。

**制約:** Tは冒頭の整数型。上限付きでは全要素<=maximum。

**計算量:** O(Bn)時間・O(n)作業領域。

### RadixSortWorkspace::reserve

```text
void reserve(std::size_t n);
```

n要素分の作業領域を予約します。

**制約:** 容量上限以下。

**計算量:** 再確保時は保持要素数に比例、確保済みならO(1)。

### RadixSortWorkspace::capacity

```text
std::size_t capacity() const noexcept;
```

予約済み要素数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### RadixSortWorkspace::release

```text
void release() noexcept;
```

作業領域を解放します。

**制約:** 追加の制約はありません。

**計算量:** 保持要素数に線形の上界。


入力長n、整数型のバイト数Bに対し、整列・添字列の取得はO(Bn)時間・O(n)追加領域です。同値の添字は元の順に並びます。

reserveは再確保時に保持要素数に比例し、capacityはO(1)、releaseは保持要素数に線形の上界です。

```cpp
std::vector<int> values{3,-1,3,2};
auto order = RadixSortOrder(values); // {1,3,0,2}
RadixSort(values);                  // {-1,2,3,3}
```

## 前提と所有

`sort`は元配列の容量・要素の保管先を変えません。作業領域のコピーは独立し、移動元も再利用できます。

上限指定は負の値・負の上限にも対応します。`SKIP_RUNTIME_CHECKS`で上限の検査を省けます。上限違反の例外までに配列が部分的に整列されている場合があります。容量限界の保護は検査省略時も残ります。

[完全な使用例](../Examples/radix_sort_example.cpp) · [実装・測定・検証の記録](../Audit/document-detail-archive-algorithm-string-2026-11-15.json)
