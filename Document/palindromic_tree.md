# 回文木

[PalindromicTree](../String/Palindromic_Tree.cpp)は、文字列にバイトを追加しながら、異なる回文を1つずつ頂点として保持します。既定の`PalindromicTree<>`は英小文字、`PalindromicTree<256,0>`はNULを含む全バイト、一般形`<Alphabet,Base>`は[Base,Base+Alphabet)です。Unicodeの文字単位ではありません。[使用例](../Examples/palindromic_tree_example.cpp)を参照してください。


以下、nは現在の文字列長、mは追加列または照会パターンの長さです。

## API

### PalindromicTree

```text
PalindromicTree();
explicit PalindromicTree(std::string_view text);
PalindromicTree(const PalindromicTree&);
PalindromicTree(PalindromicTree&&other)noexcept;
```

空／指定文字列／独立コピー／所有権移動で構築します。

**制約:** 1<=Alphabet<=256、Base+Alphabet<=256、文字列は指定バイト範囲内。

**計算量:** 空とmoveはO(1)、文字列長mにO(Alphabet*m)、コピーは保持領域に比例。

### operator=

```text
PalindromicTree&operator=(const PalindromicTree&other);
PalindromicTree&operator=(PalindromicTree&&other)noexcept;
```

独立コピー／所有権移動で代入し、自身への参照を返します。

**制約:** 同じ型の個体。

**計算量:** コピーは新旧保持領域に比例。move代入は旧状態の破棄を含みます。

### append

```text
std::int32_t append(std::uint8_t byte);
void append(std::string_view more);
```

末尾へバイト／列を追加します。1バイト版は最長回文接尾辞の頂点番号を返します。

**制約:** 追加するバイトは[Base,Base+Alphabet)。番号・長さ・容量の上限以下。

**計算量:** 1バイトは償却O(Alphabet)、最悪O(n+Alphabet)。mバイトの追加はO(Alphabet*m)。

### size

```text
std::size_t size()const noexcept;
```

元の文字列長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty()const noexcept;
```

空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### distinct_palindrome_count

```text
std::size_t distinct_palindrome_count()const noexcept;
```

空を除く異なる回文の個数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### last_node

```text
std::int32_t last_node()const noexcept;
```

最長回文接尾辞の頂点番号を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### longest_suffix_length

```text
std::size_t longest_suffix_length()const noexcept;
```

最長回文接尾辞長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### longest_palindrome

```text
Match longest_palindrome()const noexcept;
```

最長回文のMatch{start,length}を返します。同長なら最初の出現です。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### text

```text
std::string_view text()const noexcept;
```

保存文字列の非所有viewを返します。

**制約:** viewは更新・移動・交換まで。

**計算量:** O(1)。

### length

```text
std::int32_t length(std::int32_t id)const;
```

頂点の回文長を返します。

**制約:** ODD=-1、EVEN=0、または存在する正の頂点番号。

**計算量:** O(1)。

### parent

```text
std::int32_t parent(std::int32_t id)const;
```

回文の両端を除いた頂点番号を返します。

**制約:** 存在する頂点番号。根では-1。

**計算量:** O(1)。

### suffix_link

```text
std::int32_t suffix_link(std::int32_t id)const;
```

最長の真の回文接尾辞の頂点番号を返します。

**制約:** 存在する頂点番号。根では-1。

**計算量:** O(1)。

### first_position

```text
std::size_t first_position(std::int32_t id)const;
```

回文の最初の開始位置を返します。

**制約:** 存在する頂点番号。根では0。

**計算量:** O(1)。

### find

```text
std::optional<std::int32_t> find(std::string_view pattern)const noexcept;
```

対応する頂点番号を返し、非回文・不一致ならnullopt、空語ならEVENです。

**制約:** 範囲外バイトは不一致として扱います。

**計算量:** パターン長mにO(m+1)。

### contains

```text
bool contains(std::string_view pattern)const noexcept;
```

部分文字列である回文として存在すればtrueです。空語も含みます。

**制約:** 範囲外バイトは不一致として扱います。

**計算量:** パターン長mにO(m+1)。

### occurrences

```text
std::uint64_t occurrences(std::string_view pattern)const;
```

重なりを含む出現回数を返します。空語はn+1です。

**制約:** 範囲外バイトは不一致として扱います。同じ個体の並行キャッシュ照会は外部同期が必要です。

**計算量:** 最初／append後の集計はO(n+1)、以後はO(m+1)。

### node_info

```text
NodeInfo node_info(std::int32_t id)const;
```

NodeInfo{id,length,parent,suffix_link,start,occurrences}を返します。

**制約:** 存在する頂点番号。同じ個体の並行キャッシュ照会は外部同期。

**計算量:** 集計が必要ならO(n+1)、以後O(1)。

### palindromes

```text
std::vector<NodeInfo> palindromes()const;
```

空でない全回文のNodeInfoを初出順に返します。

**制約:** 同じ個体の並行キャッシュ照会は外部同期。

**計算量:** O(n+1)時間・返却領域。

### reserve

```text
void reserve(std::size_t text_capacity);
```

指定した文字列長で必要となる最大状態数と関連領域を予約します。

**制約:** 番号・容量の上限以下。

**計算量:** 再確保時は保持する状態・文字列・キャッシュ等の大きさに比例する上界。

### reset

```text
void reset(std::string_view text={});
```

指定した文字列へ置換します。省略すると空です。

**制約:** 追加するバイトは[Base,Base+Alphabet)。番号・長さ・容量の上限以下。

**計算量:** 旧状態の解放と新長mにO(Alphabet*m+旧保持量+1)の上界。

### clear

```text
void clear()noexcept;
```

空へ戻し、全領域を解放します。

**制約:** 既存の借用viewや状態番号を再利用しないこと。

**計算量:** O(n+1)。

### swap

```text
void swap(PalindromicTree&other)noexcept;
void swap(PalindromicTree&a,PalindromicTree&b)noexcept;
```

内容と所有領域を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


```cpp
PalindromicTree<> tree("ababa");
auto distinct = tree.distinct_palindrome_count(); // 5。
auto suffix = tree.longest_suffix_length(); // 5。
auto count = tree.occurrences("aba"); // 2。
```

頂点は長さ-1のODD=-1、空回文EVEN=0、空でない回文は初出順に1以上です。根の開始位置は0、parent/suffix_linkは-1、出現回数はODDが0・EVENがn+1です。空でも根は照会できます。textのviewは更新・移動・交換までの借用として扱います。自分の部分viewをappendへ渡すこともできます。

## 計算量・所有と検査

回文の両側へ同じバイトを付けて遷移し、接尾辞へのリンクで伸ばせる回文を探します。1回の追加で増える異なる回文は高々1つです。固定Alphabetで追加は償却O(1)、1回の最悪はO(n+Alphabet)、領域はO(Alphabet*n+1)です。空なら領域を確保しません。

最初の出現回数照会とappend後の再集計はO(n+1)、以後はパターン探索を除きO(1)です。palindromesの列挙はO(n+1)。const照会もキャッシュを更新するため、同じ個体への並行キャッシュ照会は外部で同期します。キャッシュの確保失敗では木と文字列を保ちます。

コピーは独立、コピー代入・resetは失敗時に元を保ちます。moveはnoexcept・O(1)、移動元は空で再利用可です。1バイト追加は強い例外保証、文字列追加も確保・容量超過では不変ですが、途中の不正バイトでは処理済みの接頭辞を保持します。

`SKIP_RUNTIME_CHECKS`は追加バイト・頂点番号の検査を除きます。範囲外バイトの照会は不一致とし、遷移が途切れた後続まで検査しません。この不一致判定とint32_t番号・長さ・容量の保護は維持します。数学的構成・機能の参考は[LuzhiledのPalindromic Tree](https://ei1333.github.io/library/string/palindromic-tree.hpp)です。
