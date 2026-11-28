# リングバッファ型の両端キュー

前後への追加・削除と添字アクセスを行います。`RingDeque<T>` は空で始まり、容量が足りなくなると自動で増えます。[本体](../DataStructure/Ring_Deque.cpp)

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `push_front(x)` / `push_back(x)` | 前後へ追加する。`emplace_` も使用可 | 償却 $O(1)$ |
| `pop_front()` / `pop_back()` | 前後の要素を削除する | $O(1)$ |
| `front()` / `back()` / `operator[](i)` | 要素の参照。添字は0始まり | $O(1)$ |
| `at(i)` | 添字を検査して要素の参照を返す | $O(1)$ |
| `reserve(n)` | 容量を予約する | 容量増加時に $O(n)$ |

```cpp
sirkit::RingDeque<int> deque;
deque.push_back(4);
deque.push_front(1);
assert(deque[1] == 4);
deque.pop_front();
assert(deque.front() == 4);
```

容量が増えると要素への参照は無効になります。`clear()` は全要素を破棄し、容量を保ちます。要素はコピー構築できるか、ムーブ構築が例外を送出しない型を使ってください。

[実行できる使用例](../Examples/ring_deque_example.cpp)・[Library Checker Verify](../Verify/current/submissions/ring_deque.cpp)
