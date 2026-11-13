# 回文半径

`String/Manacher.cpp`の`PalindromeRadii`は、奇数長と偶数長の回文半径をそれぞれO(n)時間・領域で構築する。`odd[i]`は中心iを含む半径、`even[i]`はi-1とiの間を中心とする半分の長さ。文字列や整数列を扱い、元の列を保持しない。

半開区間`[l,r)`の`is_palindrome(l,r)`はO(1)、空区間は回文とする。`longest()`は最長回文区間を返し、同長なら左端が小さいものを選ぶ。`count()`は非空の連続回文区間の総数を返す。後2者はO(n)。`PalindromicLengths(s)`は、文字上と文字間の2n-1個の中心について最長回文長を交互に返す。

列長はINT_MAX未満で、比較は一貫した同値関係とする。公開の半径配列は変更せず参照だけに使う。添字検査は`SKIP_RUNTIME_CHECKS`で除かれ、その場合も`0<=l<=r<=size()`が必要。`assign`は新しい配列を完成してから置き換えるため、確保失敗や比較の例外でも既存の結果を保つ。

[使用例](../Examples/manacher_example.cpp)、[Library Checker](https://judge.yosupo.jp/problem/enumerate_palindromes)を参照。
