# ID付き優先度付きキュー

各IDの優先度を更新できるヒープです。一つのIDにつき一つの優先度を保持します。`Key` は整数型です。`Largest` を `true` にすると大きい優先度から取り出せます。同じ優先度では小さいIDが先です。

## IndexedPriorityQueue

```text
explicit IndexedPriorityQueue(std::size_t id_limit);
```

IDを `0` 以上 `id_limit` 未満に指定します。空で構築する時間と領域は `O(id_limit+1)` です。`Key` の既定値は `int`、`Largest` は `false` です。

## set

```text
void set(std::size_t id, Key value);
```

未登録のIDを追加するか、登録済みの優先度を置き換えます。登録数を `n` として `O(log(n+1))` 時間です。

## erase

```text
bool erase(std::size_t id);
```

IDを削除し、削除できた場合に `true` を返します。`O(log(n+1))` 時間です。

## top

```text
std::pair<std::size_t, Key> top() const;
```

先頭のIDと優先度を返します。`O(1)` 時間です。`top()` と `pop()` は非空が前提です。

## pop

```text
std::pair<std::size_t, Key> pop();
```

先頭のIDと優先度を取り出します。`O(log(n+1))` 時間です。

## priority

```text
Key priority(std::size_t id) const;
```

登録済みの優先度を返します。`O(1)` 時間です。

## contains

```text
bool contains(std::size_t id) const;
```

IDが登録されているかを判定します。`O(1)` 時間です。

- `id_limit()`、`size()`、`empty()` はIDの上限、登録数、空判定です。各 `O(1)` 時間です。
- `clear()` は登録をすべて取り除きます。`O(n)` 時間です。

検査有効時は範囲外のID、空の `top()` と `pop()`、未登録の `priority()` で `std::out_of_range` を送出します。

## 使用例

```cpp
sirkit::IndexedPriorityQueue<int> queue(5);
queue.set(2, 7);
queue.set(4, 4);
queue.set(2, 1);
const auto [id, priority] = queue.pop();
assert(id == 2 && priority == 1);
```

[完全な使用例](../Examples/indexed_priority_queue_example.cpp)
