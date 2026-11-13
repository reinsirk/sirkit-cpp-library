# 接尾辞配列

[suffix_array](../String/Suffix_Array.cpp)は、文字列の各位置から始まる接尾辞を辞書順に並べ、その開始位置を返します。グローバルの`suffix_array`・`lcp_array`は[ACL](https://atcoder.github.io/ac-library/production/document_ja/string.html)の引数・戻り値・呼び出し形式に合わせて使えます。空接尾辞は含まず、文字列は符号なしバイト順です。[使用例](../Examples/suffix_array_example.cpp)を参照してください。

## API

| API | 動作・計算量 |
| --- | --- |
| `suffix_array(const std::string&)` | 開始位置のvectorを返す。O(n)時間・領域。 |
| `suffix_array(vector<int>,upper)` | 値0..upperの列を扱う。O(n+upper)時間・領域。 |
| `suffix_array<T>(const vector<T>&)` | Tの`<`で圧縮して扱う。負数・UINT64_MAXも可。O(n log n)時間・O(n)領域。 |
| `lcp_array(s,sa)` | 辞書順で隣り合う接尾辞の最長共通接頭辞長をvectorで返す。O(n)。 |
| `SuffixArrayEqualRange(s,sa,pattern)`（文字列・整数列） | patternで始まる接尾辞の、sa内の半開区間を返す。O((m+1)log(n+1))時間・追加O(1)領域。 |

```cpp
const std::string text = "banana";
auto sa = suffix_array(text); // {5,3,1,0,4,2}。
auto lcp = lcp_array(text,sa); // {1,3,0,0,2}。
auto [first,last] = SuffixArrayEqualRange(text,sa,"ana");
// [first,last) == [1,3)。一致位置はsa[1]=3とsa[2]=1。
auto words = suffix_array(std::vector<std::string>{"cat","dog","cat"}); // {2,0,1}
```

`string_view`・C文字列（終端あり、nullptr不可）のSA構築、`string_view`・`array`等のLCP構築も同名で使えます。Tの`<`は一貫した順序が前提です。比較・添字アクセスがO(1)の場合の計算量です。空列のLCPは空配列を返します。

upperは0以上INT_MAX-1未満。upperが大きい場合は、上限なしの圧縮版を使います。内部はSA-IS法で、短い列の順序から元の列を誘導整列します。列長はINT_MAX-1未満で、上限の保護は検査省略時も維持します。

lcp_arrayと検索へ渡すsaは正しい接尾辞配列が前提です。lcp_arrayはrank構築中にサイズ・添字・重複を検査しますが、辞書順は再検査しません。検索は実際に参照する添字だけを検査し、未参照のsaまで検査する保証はありません。空patternは全接尾辞に一致します。

全APIは入力を変更せず、返却vectorは独立した所有です。数学的参考は[SA-ISの論文](https://doi.org/10.1109/DCC.2009.42)、Verifyは[Library Checker](https://judge.yosupo.jp/problem/suffixarray)です。
