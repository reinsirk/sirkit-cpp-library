# 単調最小優先度付きキュー

最小キーから取り出す整数用のキューです。取り出したキーが次の挿入下限になります。Dijkstra法のように、この下限より小さいキーを後から追加しない用途に使えます。

`Key` は `bool` を除く64ビット以下の符号なし整数型、`Value` は例外を送出せずムーブ構築できる型です。`W` をキーのビット幅、`M` を同時に入れた最大要素数として使用領域は `O(W(M+1))` です。

## RadixHeap

```text
explicit RadixHeap(Key lower_bound = 0);
```

指定した下限の空キューを作ります。`O(W)` 時間です。

## emplace

```text
template<class... Args> void emplace(Key key, Args &&...args);
```

引数から値を構築して追加します。`key >= lower_bound()` が前提です。検査有効時は下限を下回ると `std::invalid_argument` を送出します。償却 `O(1)` 時間です。

## top

```text
const std::pair<Key, Value> &top() const;
```

最小キーと値の組を参照します。下限は変わりません。走査するバケットの要素数を `b` として `O(1+b)` 時間です。更新後に参照を保持できる保証はありません。

## pop

```text
std::pair<Key, Value> pop();
```

最小キーと値の組を取り出し、下限をそのキーに進めます。再配置は挿入件数あたり償却 `O(W)` 時間で、1回では最悪 `O(W+b)` 時間です。同じキーの順序は未規定です。

`top()` と `pop()` は非空が前提です。検査有効時は空なら `std::out_of_range` を送出します。

## 状態を調べる・初期化する

- `lower_bound()` は挿入できる最小キーを返します。`O(1)` 時間です。
- `size()` と `empty()` は要素数と空判定を返します。各 `O(1)` 時間です。
- `clear(lower_bound=0)` は全要素を削除して下限を設定します。バケットの容量は再利用します。現在の要素数を `n` として `O(W+n)` 時間です。
- `swap(other)` は二つのキューを交換します。`O(W)` 時間です。

## 使用例

```cpp
sirkit::RadixHeap<unsigned, int> queue;
queue.emplace(10, 0);
queue.emplace(3, 1);
const auto [distance, vertex] = queue.pop();
assert(distance == 3 && vertex == 1);
queue.emplace(distance + 2, 2);
```

[完全な使用例](../Examples/radix_heap_example.cpp)
