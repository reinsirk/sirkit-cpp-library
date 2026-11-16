# 接尾辞配列

[suffix_array](../String/Suffix_Array.cpp)は、文字列の各位置から始まる接尾辞を辞書順に並べ、その開始位置を返します。`sirkit::suffix_array`・`sirkit::lcp_array`は[ACL](https://atcoder.github.io/ac-library/production/document_ja/string.html)の引数・戻り値・呼び出し形式に合わせて使えます。空接尾辞は含まず、文字列は符号なしバイト順です。[使用例](../Examples/suffix_array_example.cpp)を参照してください。

## API

### suffix_array

```text
inline std::vector<int> suffix_array(const std::string& s);
template<class View> requires std::same_as<View, std::string_view> std::vector<int> suffix_array(View s);
template<class Char> requires (std::same_as<Char, char> || std::same_as<Char, const char>) std::vector<int> suffix_array(Char* s);
inline std::vector<int> suffix_array(const std::vector<int>& s, int upper);
template <class T> std::vector<int> suffix_array(const std::vector<T>& s);
```

接尾辞を辞書順に並べ、空接尾辞を除いた開始位置のvectorを返します。

**制約:** 長さnはINT_MAX-1未満。C文字列は終端ありでnullptr不可、upper指定時は0<=値<=upperかつ0<=upper<INT_MAX-1。汎用Tの<は一貫した順序。

**計算量:** バイト列はO(n)、上限付きはO(n+upper)、汎用vectorはO(n log(n+1))時間。領域は各O(n)、上限付きO(n+upper)。

### lcp_array

```text
template<class T> std::vector<int> lcp_array(const std::vector<T>& s, const std::vector<int>& sa);
inline std::vector<int> lcp_array(const std::string& s, const std::vector<int>& sa);
template<class Sequence> requires requires(const Sequence& s) { s.size(); s[0] == s[0]; } std::vector<int> lcp_array(const Sequence& s, const std::vector<int>& sa);
```

隣り合う接尾辞の最長共通接頭辞長を返します。空列なら空配列です。

**制約:** saはsの正しい接尾辞配列。比較・添字アクセスはO(1)、等値比較は一貫した関係。

**計算量:** O(n+1)時間・領域。

### SuffixArrayEqualRange

```text
inline std::pair<int, int> SuffixArrayEqualRange(std::string_view s, const std::vector<int>& sa, std::string_view pattern);
template <std::integral T> std::pair<int, int> SuffixArrayEqualRange(const std::vector<T>& s, const std::vector<int>& sa, const std::vector<T>& pattern);
```

patternで始まる接尾辞の、sa内の半開区間を返します。空patternは全接尾辞です。

**制約:** saは正しい接尾辞配列。値の比較・添字アクセスがO(1)であること。

**計算量:** pattern長mにO((m+1)log(n+1))時間・O(1)追加領域。


```cpp
const std::string text = "banana";
auto sa = suffix_array(text); // {5,3,1,0,4,2}。
auto lcp = lcp_array(text,sa); // {1,3,0,0,2}。
auto [first,last] = SuffixArrayEqualRange(text,sa,"ana");
// [first,last) == [1,3)。一致位置はsa[1]=3とsa[2]=1。
auto words = suffix_array(std::vector<std::string>{"cat","dog","cat"}); // {2,0,1}
```

`string_view`・C文字列（終端あり、nullptr不可）のSA構築、`string_view`・`array`等のLCP構築も同名で使えます。Tの`<`は一貫した順序が前提です。比較・添字アクセスがO(1)の場合の計算量です。空列のLCPは空配列を返します。

upperは0以上INT_MAX-1未満。upperが大きい場合は、上限なしの圧縮版を使います。列長はINT_MAX-1未満で、上限の保護は検査省略時も維持します。

lcp_arrayと検索へ渡すsaは正しい接尾辞配列が前提です。lcp_arrayはrank構築中にサイズ・添字・重複を検査しますが、辞書順は再検査しません。検索は実際に参照する添字だけを検査し、未参照のsaまで検査する保証はありません。空patternは全接尾辞に一致します。

全APIは入力を変更せず、返却vectorは独立した所有です。数学的参考は[SA-ISの論文](https://doi.org/10.1109/DCC.2009.42)、Verifyは[Library Checker](https://judge.yosupo.jp/problem/suffixarray)です。
