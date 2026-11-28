# MeldableHeap

要素を追加・削除するほか、二つの優先度付きキューを結合できます。左偏木（leftist heap）で実装しています。

## `MeldableHeap`

```text
template<class T, class Compare = std::less<T>>
MeldableHeap()
explicit MeldableHeap(Compare compare)
```

空のヒープを作ります。既定は最大ヒープで、`std::greater<T>` を指定すると最小ヒープになります。`Compare` は二つの `const T &` を受け取り、`const` で呼び出せる狭義弱順序です。

各ヒープが要素を所有し、コピーはできません。比較関数のムーブ構築・ムーブ代入が `noexcept` なら、ヒープでも対応する移動操作を使えます。移動元は空です。比較関数が移動後も使える型なら、移動元へ再び追加できます。移動構築は $O(1)$、移動代入は代入先の元の要素数を $k$ として $O(k+1)$ 時間です。

## `size`

```text
std::size_t size() const noexcept
```

要素数を $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept
```

空なら `true` を $O(1)$ で返します。

## `top`

```text
const T &top() const
```

最優先の要素への参照を $O(1)$ で返します。参照の有効期間は要素が破棄されるまでです。空のヒープでは、入力検査が有効な場合は `std::out_of_range` を送出します。

## `push`

```text
void push(const T &value)
void push(T &&value)
```

コピーまたは移動で要素を追加します。追加前の要素数を $n$ とすると、$O(\log(n+1)+1)$ 時間です。

## `emplace`

```text
template<class... Args>
void emplace(Args &&...args)
```

引数から要素を直接構築して追加します。$O(\log(n+1)+1)$ 時間です。

## `pop`

```text
void pop()
```

最優先の要素を削除します。$O(\log(n+1)+1)$ 時間です。空の場合は `top` と同じ入力検査を行います。

## `meld`

```text
void meld(MeldableHeap &other)
```

`other` の全要素を取り込み、`other` を空にします。要素のコピーや移動は行いません。両方の比較関数は同じ順序を定める必要があります。自分自身を渡すと何も変わりません。二つのヒープの要素数が $n,m$ なら $O(\log(n+m+1)+1)$ 時間です。

追加・削除・結合中に要素の構築や比較が例外を送出しても、操作前のヒープの要素は保たれます。

## `clear`

```text
void clear() noexcept
```

全要素を破棄します。$O(n+1)$ 時間、追加領域 $O(1)$ です。ヒープ全体の領域は $O(n+1)$ で、各計算量は要素の構築・破棄・比較を $O(1)$ として数えます。

## 使用例

```cpp
sirkit::MeldableHeap<int> a, b;
a.push(7);
a.push(2);
b.push(9);
a.meld(b);
assert(b.empty() && a.top() == 9);
a.pop();
assert(a.top() == 7);
```

[使用例](../Examples/meldable_heap_example.cpp) ／ [Verify](../Verify/current/submissions/meldable_heap_tree_game.cpp)
