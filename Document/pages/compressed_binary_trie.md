# 整数の圧縮二分トライ

`Compressed_Binary_Trie.cpp`は`CompressedBinaryTrie<T,MAX_LOG,NODES>`を提供します。値が分かれるビットだけを保持する圧縮木で、非負整数の多重集合・順位・全体XOR・XOR値の最小と最大を扱います。このファイルに通常版の`BinaryTrie`は含まれません。

以下、B=MAX_LOG+1、size_type=std::size_tです。FindResultは{件数,ID一覧のconst参照}、ElementResultは{値,ID一覧のconst参照}です。

## コンストラクタ

```text
CompressedBinaryTrie() = default;
CompressedBinaryTrie(const CompressedBinaryTrie &) = delete;
CompressedBinaryTrie(CompressedBinaryTrie &&) = delete;
```

空の多重集合を作ります。コピー・ムーブはできません。

**制約:** 型・ビット幅・容量の共通前提を満たすこと。

**計算量:** O(1)。

## reserve

```text
void reserve(size_type distinct);
```

異なる値distinct個分の圧縮ノード領域を上限内で予約します。

**制約:** 型・容量・返却参照の有効期間は共通の説明に従います。

**計算量:** 再配置時は保持ノード数に比例。未圧縮のNODES上限は緩めません。

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

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。 再配置時は保持ノード数に比例します。

## insert_unique

```text
bool insert_unique(T x, int id = -1);
```

未登録のときだけxを追加してtrueを返します。登録済みなら件数・IDを変更せずfalseです。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。 再配置時は保持ノード数に比例します。

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
CompressedBinaryTrie<std::uint32_t, 29> values;
values.reserve(100); // 異なる値100個分の領域を予約。
values.add(7);
values.add(3);
values.add(7);
const auto count = values.count(7);        // 2。
const auto smallest = values.get_kth(1).first; // 3。
values.operate_xor(2);
const auto nearest = values.min_xor(0).first; // 1。
```

## 前提と計算量

`T`は`bool`・`const`・`volatile`を除く整数型です。`B=MAX_LOG+1`として、キーとXORマスクは`0 <= x < 2^B`、符号付き型では非負の範囲です。`MAX_LOG`省略時は`std::numeric_limits<T>::digits-1`です。順位は重複込み・1始まりで、空集合の最小・最大照会はできません。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)、追加は償却O(B)です。ノード配列の再配置時には保持ノード数に比例する処理が発生します。指定IDの削除は、その値のID数に比例する探索も必要です。

非空で異なる値がD個なら、生存する実ノードは`2D-1`個です。確保した領域は削除で縮めず再利用します。`reserve(n)`は異なる値n個分を予約し、再配置時は保持ノード数に比例します。`NODES`は圧縮後のノード数ではなく、未圧縮の二分木に展開した根込みのノード上限です。

領域とID一覧はライブラリが所有し、コピー・ムーブはできません。IDを省略した要素は匿名で、ID一覧には含めません。ID一覧への参照は次の更新または破棄まで有効です。追加に失敗しても値・個数・IDを保ちます。

既定の入力検査は`SKIP_RUNTIME_CHECKS`で除けますが、容量・オーバーフローの保護は残ります。共通契約と方式の比較は[二分トライの説明](../binary_trie.md)、完全な例は[圧縮版の使用例](../../Examples/compressed_binary_trie_example.cpp)を参照してください。
