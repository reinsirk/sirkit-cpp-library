# 連長圧縮キュー

同じ値が続く部分を「値と個数」の組で保存します。値を一括で末尾に追加し、先頭から指定個数を取り出せます。

## `RunLengthQueue<T>`

空のキューを作ります。`T` は移動構築と `const` 値どうしの `==` 比較ができる型です。コピー可能な値型ならキューもコピーできます。移動後、元のキューは空になります。

## `size`

格納した要素の総数を `std::uint64_t` で返します。

## `empty`

空かどうかを返します。

## `run_count`

同じ値が連続するグループの数を返します。

## `front`

先頭の値を `const T &` で返します。空の場合、検査が有効なら `std::out_of_range` を送出します。

## `front_count`

先頭に連続する同じ値の個数を返します。空なら0です。`size` から `front_count` まで各 $O(1)$ 時間です。

## `push_back`

`push_back(value, count = 1)` で値を `count` 個追加します。末尾と等しければ同じグループに入ります。$O(1)$ 時間です。検査が有効なら、総数が `UINT64_MAX` を超える追加に `std::length_error` を送出します。

## `pop_front`

`pop_front(count = 1)` で先頭から `count` 個を削除します。

## `consume_front`

`consume_front(count, visitor)` で削除するグループごとに `visitor(const T &value, std::uint64_t copies)` を呼びます。先頭から順に呼び、グループの一部を取り出したときはその個数を渡します。コールバックから同じキューを変更しないでください。コールバックが例外を送出すると、正常に戻った呼び出しの分だけ削除が済んだ状態になります。

削除はいずれも触れたグループ数を $r$ として $O(r+1)$ 時間です。空からの追加と削除だけを繰り返した場合、合計時間は操作回数に比例します。保存する値はグループにつき一つで、内部領域は同時に保持したグループ数の最大に比例します。計算量は値の移動・比較とコールバックを $O(1)$ とした場合です。

検査が有効なら、要素数を超える削除に `std::out_of_range` を送出します。

## 使用例

```cpp
sirkit::RunLengthQueue<int> queue;
queue.push_back(3, 2);
queue.push_back(5, 4);
std::uint64_t sum = 0;
queue.consume_front(3, [&](int value, std::uint64_t count)
{
    sum += static_cast<std::uint64_t>(value) * count;
});
assert(sum == 11 && queue.front_count() == 3);
```

[実装](../DataStructure/Run_Length_Queue.cpp)・[使用例の全体](../Examples/run_length_queue_example.cpp)・[Verify](../Verify/current/submissions/run_length_queue.cpp)
