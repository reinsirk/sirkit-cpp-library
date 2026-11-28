# 接尾辞木

文字列の接尾辞を、共通する先頭部分でまとめた圧縮トライです。辺をたどると部分文字列の共通接頭辞と、その出現位置を確認できます。

## `SuffixTree`

```text
explicit SuffixTree(std::string_view text)
```

文字列をコピーして木を作ります。根は頂点0です。空接尾辞は登録せず、空文字列の木には根だけがあります。文字列長を $n$ として、$O(n+1)$ 時間・領域です。長さの上限は `(INT_MAX-1)/2` です。コピーした個体は同じ木を所有します。

取得したビューは参照先の木を所有する個体が生きている間に使えます。文字の比較には符号なしバイトの値を使い、位置と頂点番号は0始まりです。

## `nodes`

```text
std::span<const Node> nodes() const noexcept
```

頂点番号順の一覧を $O(1)$ 時間で参照します。各頂点は次の値を持ちます。

- `parent` は親の番号で、根では `-1` です。
- `depth` は根からの文字数です。
- `suffix_start` はその頂点で終わる接尾辞の開始位置です。終わるものがなければ `-1` です。
- `first_suffix` と `last_suffix` は接尾辞配列での半開区間です。自分と子孫で終わる接尾辞を含みます。

## `children`

```text
std::span<const int> children(int vertex) const
```

子の頂点番号を辺ラベルの辞書順で参照します。ビューの取得は $O(1)$ 時間です。

## `suffixes`

```text
std::span<const int> suffixes(int vertex) const
```

その頂点で終わる接尾辞と子孫で終わる接尾辞の開始位置を、接尾辞の辞書順で参照します。ビューの取得は $O(1)$ 時間です。

## `edge_label`

```text
std::string_view edge_label(int vertex) const
```

親からその頂点へ向かう辺に対応する文字列を参照します。根では空のビューを返します。ビューの取得は $O(1)$ 時間です。

## `longest_nonoverlapping_repeat`

```text
Repeat longest_nonoverlapping_repeat() const
```

重ならずに2回現れる最長部分文字列を $O(n+1)$ 時間・追加領域で求めます。`first_start` と `second_start` は出現位置、`length` は長さです。該当する非空文字列がなければすべて0を返します。

## 使用例

```cpp
const sirkit::SuffixTree tree("bananabanana");
const auto repeat = tree.longest_nonoverlapping_repeat();
assert(repeat.length == 6);
```

[実装](../String/Suffix_Tree.cpp)・[使用例の全体](../Examples/suffix_tree_example.cpp)・[同梱する接尾辞配列のAPI](suffix_array.md)
