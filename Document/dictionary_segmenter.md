# 文字列を辞書の語に分割する

文字列全体を辞書にある語だけで分け、使う語が最少になる分割を求めます。同じ辞書で複数の文字列を調べられます。

## `DictionarySegmenter<Alphabet, First>(words)`

`words` は各要素を `std::string_view` に変換できる範囲です。空語は無視し、添字は元の語順で数えます。語の内容は構築時に取り込みます。

既定の文字範囲は `a..z`。辞書と調べる文字列は、バイト値が `First` から `First + Alphabet - 1` までに入る必要があります。辞書の語数を $k$、総文字数を $L$、オートマトンの状態数を $S$ とすると、構築は $O(1+k+L+S\cdot Alphabet)$ 時間、$O(1+k+S\cdot Alphabet)$ 領域です。

## `ByteDictionarySegmenter`

256種類すべてのバイトを扱う版です。

## `minimum_segmentation(text)`

分割に使う語の0始まりの添字を先頭から順に返します。空文字列なら空の分割、作れなければ `std::nullopt` です。

Aho–Corasick で見つけた語の終端ごとに最少語数を更新します。文字列長を $n$、辞書語の総一致数を $R$ とすると、$O(1+n+R)$ 時間、$O(n+1)$ 領域です。

## 使用例

```cpp
constexpr std::array<std::string_view, 3> words{"a", "ab", "ba"};
const sirkit::DictionarySegmenter<> segmenter(words);
const auto pieces = segmenter.minimum_segmentation("aba");
assert(pieces && pieces->size() == 2);
```

[実装](../String/Dictionary_Segmenter.cpp)・[使用例の全体](../Examples/dictionary_segmenter_example.cpp)・[Verify](../Verify/current/submissions/dictionary_segmenter.cpp)
