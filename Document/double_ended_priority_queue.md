# 両端優先度付きキュー

要素を追加し、最小値と最大値のどちらからでも取り出せます。同じ値を複数追加できます。

比較関数 `Compare` には狭義弱順序を与えます。要素数を $n$ とし、比較と要素の移動を定数時間と数えます。空のキューの両端を参照・削除すると、入力検査が有効な場合は `std::out_of_range` を送出します。

## `DoubleEndedPriorityQueue`

```text
template <class T, class Compare = std::less<T>>
class DoubleEndedPriorityQueue;

DoubleEndedPriorityQueue() = default;
explicit DoubleEndedPriorityQueue(std::vector<T> values, Compare compare = {});
```

空のキュー、または `values` をすべて入れたキューを作ります。空の構築は $O(1)$、要素列からの構築は $O(n+1)$ 時間・領域です。

## `empty`

```text
bool empty() const noexcept;
```

空なら `true` を返します。 $O(1)$ です。

## `size`

```text
std::size_t size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## `push`

```text
void push(T value);
```

要素を1個追加します。 償却 $O(1+\log(n+1))$ です。

## `min`

```text
const T &min() const;
```

空でないキューの最小値への読み取り専用参照を返します。追加・削除後は参照を取得し直します。 $O(1)$ です。

## `max`

```text
const T &max() const;
```

空でないキューの最大値への読み取り専用参照を返します。追加・削除後は参照を取得し直します。 $O(1)$ です。

## `pop_min`

```text
T pop_min();
```

空でないキューから最小値を1個取り出して返します。 $O(1+\log(n+1))$ です。

## `pop_max`

```text
T pop_max();
```

空でないキューから最大値を1個取り出して返します。 $O(1+\log(n+1))$ です。

## 使用例

```cpp
sirkit::DoubleEndedPriorityQueue<int> queue({3, 5, 1});
assert(queue.pop_min() == 1);
assert(queue.pop_max() == 5);
```

[使用例](../Examples/double_ended_priority_queue_example.cpp) ／ [Verify](../Verify/current/submissions/double_ended_priority_queue.cpp)
