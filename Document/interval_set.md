# 整数区間集合

連続する整数を半開区間 `[left, right)` にまとめて管理します。負の値も使えます。`T` は `bool` を除く整数型で、型の最大値は区間の右端に使えます。集合に格納できるのは最大値の一つ手前までです。

`m` を現在の区間数として、使用領域は `O(m+1)` です。区間を受け取る操作は `left <= right` が前提です。検査有効時は逆順の端点で `std::invalid_argument` を送出します。

## add

```text
void add(T left, T right);
```

区間内の整数を追加します。重なる区間や隣り合う区間を併合します。空区間は変化を起こしません。変更する区間数を `k` として償却 `O(log(m+1)+k)` 時間です。

## erase

```text
void erase(T left, T right);
```

区間内の整数を削除します。元の区間の一部が残るときは端を縮めるか、二つに分けます。空区間は変化を起こしません。変更する区間数を `k` として償却 `O(log(m+1)+k)` 時間です。

## contains

```text
bool contains(T value) const;
```

整数 `value` が集合に含まれるかを判定します。`O(log(m+1))` 時間です。

## covers

```text
bool covers(T left, T right) const;
```

区間の全要素が集合に含まれるかを判定します。空区間は `true` です。`O(log(m+1))` 時間です。

## mex

```text
T mex(T first = 0) const;
```

`first` 以上で集合に含まれない最小の整数を返します。`O(log(m+1))` 時間です。

## interval_at

```text
std::optional<std::pair<T, T>> interval_at(T value) const;
```

`value` を含む最大の連続区間の両端を返します。`value` が集合になければ `std::nullopt` です。`O(log(m+1))` 時間です。

## 状態を調べる・初期化する

- `IntervalSet<T>()` は空の集合を作ります。`T` の既定値は `int` です。`O(1)` 時間です。
- `size()` は集合内の整数の総数を `T` に対応する符号なし型で返します。`O(1)` 時間です。
- `interval_count()` は区間数を返します。`O(1)` 時間です。
- `empty()` は空集合かを判定します。`O(1)` 時間です。
- `clear()` は全要素を削除します。`O(m)` 時間です。
- `swap(other)` は二つの集合を交換します。`O(1)` 時間です。

## 使用例

```cpp
sirkit::IntervalSet<int> used;
used.add(0, 3);
used.add(3, 5);
assert(used.mex() == 5);
used.erase(2, 4);
assert(used.mex() == 2);
```

[完全な使用例](../Examples/interval_set_example.cpp)
