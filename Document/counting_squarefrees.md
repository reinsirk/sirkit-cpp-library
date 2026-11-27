# 無平方数の個数

正整数のうち、2以上の整数の平方で割り切れない数の個数を求めます。

```cpp
auto count = sirkit::count_squarefrees(10);  // 7
```

## `count_squarefrees`

```text
std::uint64_t count_squarefrees(std::uint64_t n);
```

$1$ 以上 $n$ 以下の無平方数を数えます。$n=0$ では $0$ を返します。線形篩で扱う部分と、メルテンス関数でまとめる部分に分けます。 固定幅整数演算で $O(n^{2/5})$ 時間・$O(n^{2/5})$ 領域です。

[参考論文（Pawlewicz、Algorithm 2）](https://arxiv.org/abs/1107.4890)

[使用例](../Examples/counting_squarefrees_example.cpp) ／ [Verify](../Verify/current/submissions/counting_squarefrees.cpp)
