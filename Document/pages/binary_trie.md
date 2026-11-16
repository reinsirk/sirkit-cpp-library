# 整数の二分トライ

`Binary_Trie.cpp`は`BinaryTrie<T,MAX_LOG,NODES>`を提供します。非負整数の多重集合に、値の追加・削除、順位、全体XOR、XOR値の最小・最大を照会できます。このファイルには圧縮版の`CompressedBinaryTrie`は含まれません。

以下、B=MAX_LOG+1、size_type=std::size_tです。FindResultは{件数,ID一覧のconst参照}、ElementResultは{値,ID一覧のconst参照}です。

## コンストラクタ

```text
BinaryTrie();
BinaryTrie(const BinaryTrie &) = delete;
BinaryTrie(BinaryTrie &&) = delete;
```

空の多重集合を作ります。コピー・ムーブはできません。

**制約:** 型・ビット幅・容量の共通前提を満たすこと。

**計算量:** 通常版は最初のノードブロック確保を含むO(1)。

## size

```text
size_type size() const noexcept;
```

要素数を返します。

**制約:** 型・容量・返却参照の有効期間は共通の説明に従います。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 型・容量・返却参照の有効期間は共通の説明に従います。

**計算量:** O(1)。

## add

```text
void add(T x, int id = -1);
```

xを1個追加します。id=-1は匿名で、ID一覧には含みません。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。

## insert_unique

```text
bool insert_unique(T x, int id = -1);
```

未登録のときだけxを追加してtrueを返します。登録済みなら件数・IDを変更せずfalseです。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。

## del

```text
bool del(T x, int id = -1);
```

xを1個削除してtrueを返します。値や指定IDがなければfalseです。ID省略時は匿名を優先します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。ID指定時はその値のID数に比例する探索も加わります。

## find

```text
FindResult find(T x) const;
```

{xの件数,ID一覧のconst参照}を返します。未登録なら{0,空配列}です。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## count

```text
size_type count(T x) const;
```

xの個数を匿名・重複も含めて返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## contains

```text
bool contains(T x) const;
```

xが1個以上あればtrueを返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## min_element

```text
ElementResult min_element() const;
```

{最小値,ID一覧のconst参照}を返します。

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## max_element

```text
ElementResult max_element() const;
```

{最大値,ID一覧のconst参照}を返します。

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## get_kth

```text
ElementResult get_kth(std::int64_t k) const;
```

昇順・重複込み・1始まりk番目の{値,ID一覧のconst参照}を返します。

**制約:** 重複込み・1始まりで1<=k<=size()。

**計算量:** O(B)。

## operate_xor

```text
void operate_xor(T x);
```

全要素へXORを一括適用します。後から追加する公開値の意味は変えません。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(1)。

## count_less

```text
template <class Bound> size_type count_less(Bound upper) const noexcept;
```

upper未満の要素数を返します。整数型のupper<=0なら0、キー範囲を超えればsize()です。

**制約:** upperは整数型。型・容量・返却参照の有効期間は共通の説明に従います。

**計算量:** O(B)。

## min_xor

```text
ElementResult min_xor(T x) const;
```

{x XOR yの最小値,yのID一覧のconst参照}を返します。

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## max_xor

```text
ElementResult max_xor(T x) const;
```

{x XOR yの最大値,yのID一覧のconst参照}を返します。

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## count_xor_less

```text
template <class Bound> size_type count_xor_less(T x, Bound upper) const;
```

x XOR y<upperとなる要素数を返します。upperの境界動作はcount_lessと同じです。

**制約:** upperは整数型。キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## 使用例

```cpp
BinaryTrie<std::uint32_t, 29> values;
values.add(7);
values.add(3);
values.add(7);
const auto count = values.count(7);      // 2。
const auto third = values.get_kth(3).first; // 7。順位は1始まり。
values.operate_xor(2);
const auto nearest = values.min_xor(0).first; // min(5,1,5)=1。
```

## 前提と計算量

`T`は`bool`・`const`・`volatile`を除く整数型です。`B=MAX_LOG+1`として、キーとXORマスクは`0 <= x < 2^B`、符号付き型では非負の範囲を扱います。`MAX_LOG`省略時は`std::numeric_limits<T>::digits-1`です。空集合の最小・最大照会はできません。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)、追加は償却O(B)です。指定IDを削除する場合は、その値のID数に比例する探索も加わります。`NODES`は内部でまとめた分岐数ではなく、二分木に展開した根込みのノード上限です。

値・ノード・ID一覧はライブラリが所有します。コピーとムーブはできません。IDを指定しない要素は匿名で、`find`等から返るID一覧に含まれません。返したID一覧への参照は次の更新または破棄まで有効です。通常版には圧縮版専用の`reserve`はありません。

既定の入力検査は`SKIP_RUNTIME_CHECKS`で除けますが、容量・添字・要素数の保護は残ります。追加失敗時は値・個数・IDを保ちます。詳しい共通契約と方式の比較は[二分トライの説明](../binary_trie.md)、完全な例は[通常版の使用例](../../Examples/binary_trie_example.cpp)を参照してください。
