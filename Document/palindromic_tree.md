# 回文木

[PalindromicTree](../String/Palindromic_Tree.cpp)は、文字列にバイトを追加しながら、異なる回文を1つずつ頂点として保持します。既定の`PalindromicTree<>`は英小文字、`PalindromicTree<256,0>`はNULを含む全バイト、一般形`<Alphabet,Base>`は[Base,Base+Alphabet)です。Unicodeの文字単位ではありません。[使用例](../Examples/palindromic_tree_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `PalindromicTree(string_view)` / `append(byte)` / `append(string_view)` | 構築 / 末尾へ追加する。1バイト版は追加後の最長回文接尾辞の頂点番号を返す。 |
| `size()` / `empty()` / `distinct_palindrome_count()` | 文字列長 / 空か / 空を除く異なる回文数。O(1)。 |
| `last_node()` / `longest_suffix_length()` | 最後の頂点番号 / 最長回文接尾辞長。O(1)。 |
| `longest_palindrome()` / `text()` | 最長回文のMatch{start,length} / 保存文字列のview。O(1)。同長なら最初の出現。 |
| `length(id)` / `parent(id)` / `suffix_link(id)` / `first_position(id)` | 回文長 / 両端を除いた頂点 / 最長の真の回文接尾辞 / 最初の開始位置。O(1)。 |
| `find(pattern)` / `contains(pattern)` | 頂点番号のoptional / 存在判定。O(m)。不一致・非回文はnullopt。 |
| `occurrences(pattern)` | 重なり込みの出現回数。空はn+1。 |
| `node_info(id)` / `palindromes()` | NodeInfo{id,length,parent,suffix_link,start,occurrences} / 空でない全頂点を初出順のvectorで返す。 |
| `reserve(text_capacity)` / `reset(text={})` / `clear()` / `swap(other)` | 容量予約 / 新しい文字列へ置換 / 全解放して空へ / O(1)で交換。 |

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
