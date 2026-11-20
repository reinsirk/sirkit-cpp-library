# 整数の二分トライ

[BinaryTrie](../DataStructure/Binary_Trie.cpp)と[CompressedBinaryTrie](../DataStructure/Compressed_Binary_Trie.cpp)は、非負整数の多重集合を扱います。全要素へのXOR、順位、XOR値の最小・最大を照会できます。[通常版の使用例](../Examples/binary_trie_example.cpp)と[圧縮版の使用例](../Examples/compressed_binary_trie_example.cpp)を参照してください。

以下、B=MAX_LOG+1です。size_typeはstd::size_t、FindResultは{件数,ID一覧のconst参照}、ElementResultは{値,ID一覧のconst参照}です。

## 型と範囲

`T`は`bool`・`const`・`volatile`を除く整数型です。`MAX_LOG`は最高位ビット番号で、B=`MAX_LOG+1`とします。キーとXORマスクは`0 <= x < 2^B`の範囲とし、符号付きTでは非負の範囲に収めます。例えば`std::uint32_t,29`は30bit、`std::uint64_t,63`は64bitすべてを使えます。

`MAX_LOG`は省略でき、既定値は`std::numeric_limits<T>::digits - 1`です。両クラスとも、符号なし型では全ビット、符号付き型では符号ビットを除く非負の範囲を扱います。第2テンプレート引数による手動指定もできます。

```cpp
BinaryTrie<std::uint32_t> full;                  // 32bit（MAX_LOG=31）。
BinaryTrie<std::uint32_t, 29> limited;           // 手動で30bitに限定。
CompressedBinaryTrie<std::uint64_t> compressed; // 64bit（MAX_LOG=63）。
CompressedBinaryTrie<std::uint64_t, 29> small;   // 手動で30bitに限定。
```

通常版は最大2bitずつ木をたどります。値の範囲が分かっている場合は`MAX_LOG`を小さく指定すると、探索する段数と必要なノード数を抑えられます。

`NODES`は根を含む未圧縮の二分トライのノード上限です。圧縮版も同じ制限を使い、圧縮した実ノードの個数だけで判定するものではありません。どちらもコピー・ムーブはできません。ノードとID配列はライブラリが所有し、再確保や破棄に利用者の処理は不要です。

## 操作

以下は両方に共通します。`size_type`は`std::size_t`、ID一覧は`std::vector<int>`への読み取り専用参照です。

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

xを1個追加します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。

## insert_unique

```text
bool insert_unique(T x, int id = -1);
```

未登録の値を追加してtrueを返します。登録済みなら個数もIDも変えず、falseを返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** 償却O(B)。IDの追加には配列拡張の費用もかかります。

## del

```text
bool del(T x, int id = -1);
```

該当する要素を1個削除してtrueを返します。値や指定IDがなければfalseを返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。ID指定時はその値のID数に比例する探索も加わります。

## find

```text
FindResult find(T x) const;
```

指定したキーを検索します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## count

```text
size_type count(T x) const;
```

指定した値の個数を返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## contains

```text
bool contains(T x) const;
```

指定したキーが登録されていればtrueを返します。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

## min_element

```text
ElementResult min_element() const;
```

`{最小値/最大値, その値のID一覧}`。空でないこと

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## max_element

```text
ElementResult max_element() const;
```

`{最小値/最大値, その値のID一覧}`。空でないこと

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## get_kth

```text
ElementResult get_kth(std::int64_t k) const;
```

重複込み・1始まりの`{k番目の値, ID一覧}`。`1 <= k <= size()`

**制約:** 重複込み・1始まりで1<=k<=size()。

**計算量:** O(B)。

## operate_xor

```text
void operate_xor(T x);
```

全要素にXORを適用します。以後`add(y)`で追加する要素の公開値はyです。

**制約:** キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(1)。

## count_less

```text
template <class Bound> size_type count_less(Bound upper) const noexcept;
```

upper未満の要素数を返します。整数型のupperが0以下なら0、キーの範囲を超えればsize()です。

**制約:** upperは整数型。型・容量・返却参照の有効期間は共通の説明に従います。

**計算量:** O(B)。

## min_xor

```text
ElementResult min_xor(T x) const;
```

`{x ^ yの最小値/最大値, 対応するyのID一覧}`。空でないこと

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## max_xor

```text
ElementResult max_xor(T x) const;
```

`{x ^ yの最小値/最大値, 対応するyのID一覧}`。空でないこと

**制約:** 空でないこと。XOR引数は0<=x<2^B。

**計算量:** O(B)。

## count_xor_less

```text
template <class Bound> size_type count_xor_less(T x, Bound upper) const;
```

`x ^ y < upper`となる要素数。upperの扱いはcount_lessと同じ

**制約:** upperは整数型。キー・XORマスクは0<=x<2^B。符号付き型では非負。

**計算量:** O(B)。

IDは重複でき、`-1`は匿名要素を表します。匿名要素はID一覧に含めません。ID省略の削除は匿名要素を優先し、匿名がなければ任意のID付き要素を1個削除します。一覧の順序は規定せず、返された参照は次の更新または破棄まで有効です。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)です。指定IDの削除にはその値のID数に比例する探索も加わります。通常版も圧縮版も追加は償却O(B)です。圧縮版ではノード配列を再配置すると、保持ノード数に比例する処理も発生します。ID追加は配列の拡張を伴うことがあります。

圧縮版の`reserve(n)`は異なる値n個分の領域を予約し、再配置時は保持ノード数に比例します。`NODES`の上限は緩和しません。

## 検査と失敗

既定では負のキー、キー範囲外、空集合の最小・最大照会、不正な順位で例外を投げます。`SKIP_RUNTIME_CHECKS`を定義するとこれらの入力検査を除き、利用者が前提を満たす必要があります。ノード容量・添字・要素数のオーバーフロー保護は維持します。

追加に失敗した場合も値・個数・IDを保持しますが、確保済み容量が増える場合はあります。存在しない値やIDの削除はfalseを返し、内容を変えません。

[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#binary-trie)
