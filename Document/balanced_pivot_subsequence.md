# 中央の左右が同数の最長部分列

固定文字列の指定区間から、左の文字を $k$ 個、中央の文字を1個、右の文字を $k$ 個の順に選び、$k$ を最大化します。区間の中で離れた位置からも選べます。

## `BalancedPivotSubsequenceIndex`

```cpp
BalancedPivotSubsequenceIndex(std::string_view text, char left, char middle, char right);
```

左右と中央の文字をそれぞれ1バイトで指定します。同じ文字を指定しても構いません。文字列の長さを $n$ として $O(n+1)$ 時間・領域で前処理します。コピーした索引は独立し、移動元は空になります。

## `size`

元の文字列のバイト数を $O(1)$ 時間で返します。

## `query`

半開区間 `[left, right)` を $O(1+\log(n+1))$ 時間で調べます。同じ長さの部分列を作れる中央が複数あれば、最も左を選びます。中央の文字が区間内になければ `std::nullopt` です。検査が有効なら、範囲外の区間や左右の逆転には `std::out_of_range` を送出します。

## `BalancedPivotSubsequenceResult`

`pivot` は元の文字列での中央の0始まり位置、`pairs` は中央の左右それぞれから選ぶ文字数です。中央だけを選ぶときは `pairs` が0になります。

## `length`

中央を含む部分列の長さ `2 * pairs + 1` を $O(1)$ 時間で返します。

## 使用例

```cpp
const sirkit::BalancedPivotSubsequenceIndex index("a/a/bb", 'a', '/', 'b');
const auto answer = index.query(0, 6);
assert(answer && answer->pivot == 3 && answer->length() == 5); // aa/bb
```

[実装](../String/Balanced_Pivot_Subsequence.cpp)・[使用例の全体](../Examples/balanced_pivot_subsequence_example.cpp)・[Verify](../Verify/current/submissions/balanced_pivot_subsequence.cpp)
