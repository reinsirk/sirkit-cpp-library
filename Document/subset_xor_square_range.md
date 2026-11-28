# 区間内の部分集合XORの2乗和

`SubsetXorSquareRange<Bits, Mod>(values)` は、非負32ビット整数列を受け取ります。各値は $2^{\mathrm{Bits}}$ 未満です。ビット幅の既定値は30、法の既定値は998244353です。

- `set(i, x)` は添字 `i` の値を `x` に置き換えます。
- `sum_squares(l, r)` は半開区間 `[l,r)` の添字を任意に選び、選んだ値のXORを2乗して全通り足し、法で割った余りを返します。同じ値でも添字が違えば区別します。空区間の答えは0です。

たとえば `{1,2}` 全体では、XORが `0,1,2,3` なので答えは14です。

構築時間と領域は $O(n\cdot\mathrm{Bits})$、更新・取得時間は $O(\mathrm{Bits}\log(n+1))$ です。

[使用例](../Examples/subset_xor_square_range_example.cpp) ／ [Verify](../Verify/current/submissions/subset_xor_square_range_yuki3792.cpp)
