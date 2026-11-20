# 最短線形漸化式の推定

`BerlekampMassey(s)`は、有限列sに合う最短次数の係数列を返します。長さdの戻り値cは、`d <= i < s.size()`で` s[i] = c[0]*s[i-1] + ... + c[d-1]*s[i-d]`を満たします。

## API

### BerlekampMassey

```text
template<class T> std::vector<T> BerlekampMassey(const std::vector<T>& sequence);
std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence);
```

有限列に合う最短線形漸化式の係数を返します。

**制約:** 汎用Tは正確な体。uint32_t版は法998244353。

**計算量:** O(n²)時間・O(n)領域。


```cpp
const std::vector<std::uint32_t> s{0,1,1,2,3,5,8,13};
const auto c = sirkit::BerlekampMassey(s); // {1,1}。
```

`std::uint32_t`の配列版は法998244353で計算し、入力を正規化します。空列・全零列の結果は空です。入力は変更しません。nを入力の長さ、係数演算をO(1)とすると、O(n²)時間・O(n)領域です。

汎用型Tには正確な体の四則演算・0/1構築・等値比較が必要です。素数法Mintや有理数は使えますが、通常の整数型・一般の合成数法には使えません。浮動小数点誤差への対策はありません。

有限列への適合だけでは未知の後続項を保証できません。同じ最短次数の係数も一意とは限りません。第k項の照会が必要なら、別の[線形漸化式ライブラリ](../../Math/Linear_Recurrence.cpp)を使います。

[完全な使用例](../../Examples/berlekamp_massey_example.cpp)・[方式と係数の約束](../linear_recurrence.md)
