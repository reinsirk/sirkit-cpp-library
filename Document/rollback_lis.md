# 巻き戻せる最長増加部分列

末尾に値を足すたびに最長増加部分列の長さを更新します。木を深さ優先でたどって、根から各頂点までの列に対する答えを求める用途に使えます。

`sirkit::RollbackLIS<T, Strict>` は空の列から始まります。`Strict` の既定値は `true` で、`false` なら同じ値を続けて選べます。`T` のムーブ構築と破棄は `noexcept` で、`operator<` による比較は狭義弱順序を満たすことが前提です。

## `push_back`

```text
void push_back(T value)
```

列の末尾に値を追加します。比較と値の操作を $O(1)$、直前の最長増加部分列長を $L$ とすると、償却 $O(1+\log(L+1))$ 時間です。

## `pop_back`

```text
void pop_back()
```

列の末尾を1要素削除します。列は空でないことが前提です。$O(1)$ 時間です。

## `rollback`

```text
void rollback(std::size_t prefix_size)
```

先頭の `prefix_size` 要素を残します。残す長さは `size()` 以下です。削除する1要素あたり $O(1)$ 時間です。

## `indices`

```text
std::vector<std::size_t> indices() const
```

最長増加部分列の一例を、現在の列の0始まりの添字で返します。$O(L+1)$ 時間・領域です。

## `length`

```text
std::size_t length() const noexcept
```

最長増加部分列の長さを $O(1)$ で返します。

## `size`

```text
std::size_t size() const noexcept
```

現在の列の長さを $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept
```

列が空かを $O(1)$ で調べます。

## `reserve`

```text
void reserve(std::size_t capacity)
```

`capacity` 要素までを保持する領域を用意します。確保が必要なら、現在の列長に比例する時間がかかります。

## `clear`

```text
void clear() noexcept
```

作業領域を保ったまま列を空にします。時間は現在の列長に比例します。

## 使用例

```cpp
sirkit::RollbackLIS<int> lis;
lis.push_back(3);
lis.push_back(1);
lis.push_back(2);
assert(lis.length() == 2);
lis.rollback(1);
assert(lis.length() == 1);
```

[実装](../Algorithm/Rollback_LIS.cpp)・[使用例の全体](../Examples/rollback_lis_example.cpp)
