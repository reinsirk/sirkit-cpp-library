# Z配列と文字列検索

`String/Z_Algorithm.cpp`は、列の各接尾辞と先頭部分が何要素一致するかを線形時間で求める。グローバルの`z_algorithm`を、[ACL](https://atcoder.github.io/ac-library/production/document_ja/string.html)と同じ引数・戻り値・呼び出し形式で使用できる。文字列を整数vectorへ変換する必要はなく、入力を直接比較する。

| API | 引数と戻り値 | 計算量 |
| --- | --- | --- |
| `z_algorithm(const std::string& s)` | `z[i]`が先頭と位置`i`の共通接頭辞長である`vector<int>` | O(n)時間・領域 |
| `z_algorithm<T>(const std::vector<T>& s)` | 整数等の列から同じZ配列を返す。`T`は推論可 | O(n)時間・領域 |
| `z_algorithm(sequence)` | `size()`・添字アクセス・等値比較を持つ列への追加対応 | O(n)時間・領域 |
| `PrefixMatches(pattern,text)` | textの各開始位置とpatternの共通接頭辞長を返す | O(n+m)時間・領域 |
| `ZMatches(pattern,text)` | 完全一致の開始位置を昇順の`vector<int>`で返す | O(n+m)時間・領域 |

`n`はtextまたは入力列、`m`はpatternの長さ。各要素の比較がO(1)の場合の計算量。空列のZ配列は空、非空なら`z[0] = n`。追加の列対応では`std::string_view`・`std::array`なども使える。

`PrefixMatches(pattern,text)`はtextの各位置とpatternの共通接頭辞長、`ZMatches(pattern,text)`は完全一致する開始位置を返す。文字列を連結せず、patternのZ配列と一致範囲を使うため、特殊な区切り文字は必要ない。全バイト値や任意の整数を扱える。時間はO(n+m)、補助領域はpattern長と返却配列に比例する。空patternは全境界0..nに一致する。

列長はINT_MAX未満が前提で、上限確認は検査省略時にも残す。元の列は変更しない。整数・バイト値の範囲に制約はなく、等値比較は一貫した同値関係とする。検査専用の全走査は行わない。

```cpp
const std::string text = "ababa", pattern = "aba";
const auto z = z_algorithm(text);             // {5, 0, 3, 0, 1}
const auto positions = ZMatches(pattern,text); // {0, 2}
const auto lengths = PrefixMatches(pattern,text); // {3, 0, 3, 0, 1}
```

[完全な使用例](../Examples/z_algorithm_example.cpp) · [Verify提出コード](../Verify/current/submissions/z_algorithm.cpp) · [Library Checker](https://judge.yosupo.jp/problem/zalgorithm)
