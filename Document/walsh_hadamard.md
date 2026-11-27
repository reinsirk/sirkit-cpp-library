# Walsh–Hadamard変換とXOR畳み込み

添字を XOR で組み合わせる畳み込みを $O(n\log(n+1))$ で求めます。

```cpp
std::vector<long long> a{1, 2, 3, 4}, b{0, 1, 0, 0};
auto c = sirkit::bitwise_xor_convolution(a, b); // {2, 1, 4, 3}
```

配列の長さ $n$ は0または2の冪です。逆変換と畳み込みの係数型には、符号付き整数か、$n$ が逆元を持つ型（奇数法の modint など）を使います。整数型では変換途中の積和も型の範囲に収めてください。

## `walsh_hadamard_transform`

```text
template <class T, std::size_t Extent>
void walsh_hadamard_transform(std::span<T, Extent> values);
```

同じ配列に変換結果を書き込みます。入力を $a$ とすると出力の第 $k$ 成分は $\sum_i (-1)^{\mathrm{popcount}(i\mathbin{\&}k)}a_i$ です。

## `inverse_walsh_hadamard_transform`

```text
template <class T, std::size_t Extent>
void inverse_walsh_hadamard_transform(std::span<T, Extent> values);
```

変換をもう一度行って $n$ で割り、元の値を復元します。整数型には割り切れるデータを与えます。

## `bitwise_xor_convolution`

```text
template <class T>
std::vector<T> bitwise_xor_convolution(std::vector<T> left, std::vector<T> right);
```

同じ長さの2列から $c_k=\sum_{i\oplus j=k}\mathtt{left}[i]\mathtt{right}[j]$ を返します。

## `bitwise_xor_square`

```text
template <class T>
std::vector<T> bitwise_xor_square(std::vector<T> values);
```

同じ列同士を畳み込みます。順変換は1回です。

各操作は $O(n\log(n+1))$ 時間です。畳み込みは値渡しの配列を作業に使い、その配列以外の補助領域は $O(1)$ です。通常版では長さの条件に違反すると `std::invalid_argument` を送出します。

参考文献 [Fino・Algazi, Unified Matrix Treatment of the Fast Walsh-Hadamard Transform](https://doi.org/10.1109/TC.1976.1674569)。

[使用例](../Examples/walsh_hadamard_example.cpp) ／ [Verify](../Verify/current/submissions/bitwise_xor_convolution.cpp)
