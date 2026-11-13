# Sliding Window Aggregation

`SlidingWindowAggregation<T,Op>`は、末尾へ追加・先頭から削除するキューと、その全体の集約値を管理します。スライドする区間の和・最小値・写像合成などに使えます。Opはconst呼出可能な結合演算、identityは左右の単位元です。逆演算や左右の交換は要求しません。[使用例](../Examples/sliding_window_aggregation_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `(identity,op=Op{})` | 単位元と演算を指定して空のキューを作る。 |
| `push_back(value)` / `pop_front()` | 末尾に追加 / 先頭を削除する。償却O(1)、1回は最悪O(N)。 |
| `front()` / `back()` | 先頭 / 末尾のconst T&を返す。O(1)。 |
| `all_prod()` | 先頭から順に合成した値を返す。空ならidentity。O(1)。 |
| `size()` / `empty()` / `clear()` | 要素数 / 空か / 全削除。clearはO(N)、ほかはO(1)。 |
| `swap(other)` | 演算・単位元・列をO(1)で交換する。 |

```cpp
SlidingWindowAggregation<std::int64_t,std::plus<std::int64_t>> queue(0);
queue.push_back(2); queue.push_back(5);
auto sum = queue.all_prod(); // 7。
queue.pop_front();
auto first = queue.front(); // 5。
```

## 前提と所有権

挿入側・削除側の2つのスタックに、それぞれの順序の集約値を持ちます。これにより、非可換の演算でも元の列の順序を保ちます。計算量は演算と値のコピーをO(1)としたものです。文字列連結などの費用は別途かかります。保持領域は過去の最大要素数に比例し、削除・clear後も容量を保持する場合があります。

front・back・pop_frontは非空が前提です。既定で空を検査してout_of_rangeを投げ、`SKIP_RUNTIME_CHECKS`でこの検査を除きます。front・backの参照は更新・交換・移動で無効になり得ます。T=boolも使えます。

T・Opはコピー可能で、破棄が例外を投げない型を使います。Opに外部副作用を持たせず、コールバックから同じキューを変更しないでください。push・popでコピー・演算・確保が失敗しても列を保ちます。演算の外部状態は巻き戻しません。

コピーは独立、移動元は空です。代入・swapにはT・Opが交換可能であることが必要です。代入の強い例外保証は、その交換がnoexceptの場合に成立します。交換自体が例外を投げる型では途中状態を保証しません。

構成と問題選定は[NyaanのSWAG](https://nyaannyaan.github.io/library/data-structure/slide-window-aggregation.hpp)を参考にし、実装は独立に組み立てています。
