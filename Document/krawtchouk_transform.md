# Krawtchouk変換

- `krawtchouk_transform(f)` は$\sum_{i=0}^N f_i(1-x)^i(1+x)^{N-i}$ の係数列を返します。
- `inverse_krawtchouk_transform(g)` は同じ長さの変換を元に戻します。
- 法は $998244353$ です。$N$ は入力の長さから1を引いた数です。末尾の0も長さに含めます。入力長の上限は $2^{22}$ です。

たとえば長さ3の列 `{1,0,0}` の変換結果は$(1+x)^2$ の係数 `{1,2,1}` です。ビットを1つ選んで反転する操作を繰り返すとき、1の個数の分布を求めるのに使えます。

入力長 $d$ に対して、時間は $O(1+d\log(d+1))$、領域は $O(d)$ です。

[使用例](../Examples/krawtchouk_transform_example.cpp) ／ [Verify](../Verify/current/submissions/krawtchouk_transform_arc133_f.cpp)
