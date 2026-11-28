# 順位を引ける多重集合

値を追加・削除しながら、指定した値より前にある要素の個数や、指定順位の値を求めます。キーの候補を事前に集めなくても使えます。

## `OrderedMultiset<T, Compare>`

```cpp
explicit OrderedMultiset(Compare compare = {});
```

空のAVL木を作ります。比較は既定で昇順です。`Compare` は `const` のまま呼べる狭義弱順序とします。コピー不可です。比較器が例外を投げずに移動できる場合は移動でき、移動元の集合は空になります。

## `size`

重複込みの要素数を `std::uint64_t` で返します。

## `empty`

空かどうかを返します。`size` と `empty` は $O(1)$ 時間です。

## `insert`

`insert(value, copies = 1)` で値を追加します。同値なキーがあれば、先に登録した値をそのまま保持します。要素数の合計が `UINT64_MAX` を超える場合は `std::length_error` を送出します。

## `erase`

`erase(value, copies = 1)` で同値な要素を最大 `copies` 個削除し、実際に削除した個数を返します。

## `count`

指定値と同値な個数を返します。

## `count_less`

指定値より比較順で前の要素を数えます。

## `count_less_equal`

指定値より前または同値の要素を数えます。

## `kth`

比較順で0始まりの順位を指定し、その値を `const T *` で返します。範囲外は `nullptr` です。ポインタが指す値は、同値な最後の1個を削除するか所有する木を破棄するまで有効です。

異なるキーの数を $u$ として、`insert` から `kth` まで最悪 $O(1+\log(u+1))$ 時間、木の領域は $O(u+1)$ です。比較と値の操作は $O(1)$ とします。

## 使用例

```cpp
sirkit::OrderedMultiset<int> values;
values.insert(2);
values.insert(5, 2);
assert(values.count_less(5) == 1);
assert(*values.kth(2) == 5);
assert(values.erase(5) == 1 && values.size() == 2);
```

[実装](../DataStructure/Ordered_Multiset.cpp)・[使用例の全体](../Examples/ordered_multiset_example.cpp)・[追加・削除のVerify](../Verify/current/submissions/ordered_multiset_extrema.cpp)・[順位のVerify](../Verify/current/submissions/ordered_multiset_ranks.cpp)
