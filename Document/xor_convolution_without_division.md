# 除算なしXOR畳み込み

- `xor_convolution_without_division(a,b)` は、各 $k$ について $i\mathbin{\oplus}j=k$ を満たす $a_i b_j$ の和を返します。
- 入力の長さは同じ2冪か、両方とも0です。係数型には加法単位元 `T{}` と `+=`、`-=`、`*=` が必要です。
- すべての係数について $x+x=0$ が成り立つ環では、`xor_convolution_without_division<true>(a,b)` で計算を省けます。

たとえば各係数を4で割った余りとして扱う場合も、除算なしで計算できます。長さを $N$ とすると、時間は $O(1+N\log^2(N+1))$、領域は $O(N\log(N+1))$ です。

[使用例](../Examples/xor_convolution_without_division_example.cpp) ／ [Verify](../Verify/current/submissions/xor_convolution_without_division_yuki1901.cpp)
