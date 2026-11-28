# 禁止部分文字列を避ける列の個数

指定した語を連続部分に持たない文字列を数えます。たとえば `a` と `b` を使い、`aa` を避ける長さ3の列は5個です。

## `count_strings_avoiding_patterns`

```cpp
template <std::size_t Alphabet = 26, unsigned First = 'a', class Field = std::uint32_t>
Field count_strings_avoiding_patterns(std::uint64_t length,
                                       std::span<const std::string_view> forbidden);
```

使う文字は符号なしバイトの範囲 `[First, First + Alphabet)` です。`1 <= Alphabet <= 256`、`First + Alphabet <= 256` が必要で、禁止語にもこの範囲のバイトを使います。既定の返却型は998244353で割った余りです。別の型を渡す場合は、正確な体の四則演算と比較、0・1からの構築が必要です。禁止語に空文字列があれば答えは0で、禁止語の重複は個数に影響しません。

禁止語の合計長を $P$、禁止語に触れずに到達できる状態数を $S$、文字種数を $A$ とします。$t=\min(\mathrm{length}+1,2S)$ 項までの計算は $O(A(P+1)+ASt)$ 時間です。さらに長い長さでは $O(S^2)$ の漸化式復元と第 `length` 項の計算を行います。漸化式の次数を $d$、$L=\mathrm{bit\_ceil}(2d+1)$ とすると、第 `length` 項は既定型で $O(L\log L\log(\mathrm{length}+2))$ 時間、別の型で $O(d^2\log(\mathrm{length}+2))$ 時間です。領域は $O(A(P+1))$ です。

## 使用例

```cpp
const std::array<std::string_view, 1> forbidden{"aa"};
assert(sirkit::count_strings_avoiding_patterns<2>(3, forbidden) == 5);
```

[実装](../String/Pattern_Avoidance.cpp)・[使用例の全体](../Examples/pattern_avoidance_example.cpp)・[Verify](../Verify/current/submissions/pattern_avoidance.cpp)
