# 上位・下位K個の和

値の追加・削除を受けながら、順序の先頭K個の和を求めます。小さい方から選ぶときは `Largest=false`、大きい方から選ぶときは `Largest=true` を指定します。重複する値は個別に数えます。

`T` は値の型、`Sum` は和の型です。どちらにも `bool` を除く整数型を使います。`n` を現在の要素数として使用領域は `O(n+1)` です。

## TopKSum

```text
explicit TopKSum(std::size_t k = 0);
```

空の多重集合を作ります。`T` の既定値は `int`、`Largest` は `false`、`Sum` は `T` です。`O(1)` 時間です。

## insert

```text
void insert(T value);
```

値を1個追加します。`O(log(n+1))` 時間です。

## erase

```text
bool erase(T value);
```

指定した値を1個削除し、削除できた場合は `true` を返します。`O(log(n+1))` 時間です。

## set_k

```text
void set_k(std::size_t k);
```

選ぶ個数を更新します。実際に選ぶ個数の増減を `d` として `O(1+d log(n+1))` 時間です。

## sum

```text
Sum sum() const;
```

先頭 `min(k,n)` 個の和を返します。求める和を `Sum` で表せることが前提です。`O(1)` 時間です。

## kth

```text
std::optional<T> kth() const;
```

指定した順序で `k` 番目の値を返します。`k == 0` または `n < k` なら `std::nullopt` です。`O(1)` 時間です。

## 状態を調べる・初期化する

- `k()` は指定した個数を返します。`O(1)` 時間です。
- `size()` は全要素数を返します。`O(1)` 時間です。
- `empty()` は空かどうかを判定します。`O(1)` 時間です。
- `clear()` は `k` を保ちながら全値を削除します。`O(n)` 時間です。
- `swap(other)` は要素と設定を交換します。`O(1)` 時間です。

## 使用例

```cpp
sirkit::TopKSum<int, true, std::int64_t> largest(2);
for (int value : {3, 7, 5, 7})
{
    largest.insert(value);
}
assert(largest.sum() == 14);
largest.erase(7);
assert(largest.sum() == 12);
```

[完全な使用例](../Examples/top_k_sum_example.cpp)
