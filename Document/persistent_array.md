# 永続配列

`sirkit::PersistentArray<T, shift>` は更新前の配列も読み取れる固定長配列です。保存した版から更新すると、その版を起点に新しい版を作れます。添字は0始まりです。

```cpp
sirkit::PersistentArray<int> a(3, 0);
auto *initial = a.root;
auto *first = a.update(0, 5);
auto *second = a.update(initial, 0, 8);
assert(a.get(first, 0) == 5 && a.get(second, 0) == 8);
```

`root` と更新の戻り値は、版を識別する `Node *` 型のポインターです。`nullptr` は全要素が初期値の版を表します。ポインターは保存するか、この配列の関数に渡して使います。配列をコピーすると保存領域を共有し、選択中の版は配列ごとに保持します。ポインターが有効なのは、同じ保存領域を持つ配列が残っている間です。並行更新には外部同期が必要です。

`B = 2^shift`、`N` を要素数、`H = 1 + log_B(N + 1)` とします。`1 <= shift <= 8` で既定値は4です。`T` はコピー構築・代入できる型を使います。

## `PersistentArray`

```text
explicit PersistentArray(std::int64_t n = 0, T id = T{});
explicit PersistentArray(const std::vector<T> &values, T id = T{});
```

`n >= 0` なら長さ `n` の配列を初期値 `id` で作ります。構築は `O(B + H)` 時間です。配列から構築する場合の時間は `O(B + N + (N/B + 1)H)`、領域は `O(N + B)` です。

## `get`

```text
T get(std::int64_t k) const;
T get(Node *version, std::int64_t k) const;
```

現在の版または指定した版の `k` 番目を返します。`0 <= k < size()` を満たす添字を渡します。時間は `O(H)` です。

## `get_two`

```text
std::pair<T, T> get_two(std::int64_t a, std::int64_t b) const;
std::pair<T, T> get_two(Node *version, std::int64_t a, std::int64_t b) const;
```

同じ版の `a`、`b` 番目をその順に返します。両方とも有効な添字を渡します。時間は `O(H)` です。

## `update`

```text
Node *update(std::int64_t k, const T &value);
Node *update(Node *version, std::int64_t k, const T &value);
```

現在の版または指定した版から、`k` 番目を置き換えた版を作ります。作った版に切り替え、そのポインターを返します。`0 <= k < size()` を満たす添字を渡します。償却時間・追加領域は `O(BH)` です。

## `update_two`

```text
Node *update_two(std::int64_t a, const T &x, std::int64_t b, const T &y);
Node *update_two(Node *version, std::int64_t a, const T &x, std::int64_t b, const T &y);
```

有効な添字 `a`、`b` を同時に更新し、作った版に切り替えてそのポインターを返します。`a == b` なら `y` が残ります。償却時間・追加領域は `O(BH)` です。

## `restore`

```text
void restore(Node *version);
```

指定した版に切り替えます。時間は `O(1)` です。

## `size`

```text
std::int64_t size() const noexcept;
```

版に共通する配列の長さを返します。時間は `O(1)` です。

[使用例](../Examples/persistent_array_example.cpp) · [Verifyコード](../Verify/current/submissions/persistent_array_queue.cpp) · [Persistent Queue](https://judge.yosupo.jp/problem/persistent_queue)
