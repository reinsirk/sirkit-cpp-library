# 多倍長整数

64ビット整数に収まらない値を加減乗除したり比較したりできます。10進と16進のどちらかを型で選びます。

## 構築

```text
template <unsigned Radix = 10>
class BigInteger;
BigInteger(std::int64_t value = 0)
explicit BigInteger(std::string_view text)
```

`Radix` は10または16です。文字列では先頭の `+` や `-`、先頭ゼロ、16進の小文字を使えます。`0x` や桁区切りは省いてください。$D$ 桁の文字列からの構築は $O(D)$ 時間です。

## `sign`

```text
int sign() const noexcept
```

負なら -1、ゼロなら 0、正なら 1 です。$O(1)$ 時間です。

## `to_string`

```text
std::string to_string() const
```

同じ基数の文字列を返します。先頭ゼロを除き、16進の英字は大文字です。$O(D)$ 時間です。

以下の計算量では $D$ を入力の合計桁数とします。

## `operator+`

`a + b` は和を返します。$O(D)$ 時間です。

## `operator-`

`a - b` は差、`-a` は符号を変えた値を返します。各 $O(D)$ 時間です。

## `operator+=`

`a += b` は和を `a` に格納します。$O(D)$ 時間です。

## `operator-=`

`a -= b` は差を `a` に格納します。$O(D)$ 時間です。

## `operator<=>`

符号付きの整数として大小を比較します。$O(D)$ 時間です。

## `operator==`

値が等しければ `true` を返します。$O(D)$ 時間です。

## `operator*`

`a * b` は積を返します。桁数を $A,B$ として、小さい方が128桁以下なら $O(AB)$ 時間の筆算を使い、両方が長ければ $O((A+B)\log(A+B))$ 時間のNTTを使います。領域は $O(A+B)$ で、NTTの変換長は $2^{23}$ 以下です。

## `operator*=`

`a *= b` は積を `a` に格納します。計算量は `operator*` と同じです。

## `divmod`

```text
std::pair<BigInteger, std::int64_t> divmod(std::uint32_t divisor) const
std::pair<BigInteger, BigInteger> divmod(const BigInteger &divisor) const
```

商と余りを返します。商は0方向に丸め、余りはゼロか被除数と同じ符号です。除数がゼロなら `std::domain_error` を送出します。

`std::uint32_t` の除数では $O(D)$ 時間かかります。多倍長整数で割る場合、被除数と除数の桁数を $A,B$ として $O(A\log^2(B+1))$ 時間・$O(A+B)$ 領域です。NTTの変換長は乗算時と同じく $2^{23}$ 以下に収まる必要があります。

商が短い場合や除数が小さい場合は筆算を使います。大きい場合は桁を分割して商を見積もり、NTTによる積を使って余りを計算します。[Burnikel・Ziegler の研究報告](https://pure.mpg.de/rest/items/item_1819444_2/component/file_1850582/content)

## `operator/`

`a / b` は商を返します。計算量は多倍長版の `divmod` と同じです。

## `operator%`

`a % b` は余りを返します。計算量は多倍長版の `divmod` と同じです。

## `operator/=`

`a /= b` は商を `a` に格納します。計算量は多倍長版の `divmod` と同じです。

## `operator%=`

`a %= b` は余りを `a` に格納します。計算量は多倍長版の `divmod` と同じです。

## 使用例

```cpp
const sirkit::BigInteger<> a("-1000000000000000000000000");
const sirkit::BigInteger<> b("999999999999999999999999");
const auto [quotient, remainder] = a.divmod(b); // {-1, -1}
```

[実装](../Math/Big_Integer.cpp)・[使用例の全体](../Examples/big_integer_example.cpp)・[同梱NTT](ntt.md)

提出用ソースは[加算](../Verify/current/submissions/addition_of_big_integers.cpp)、[乗算](../Verify/current/submissions/multiplication_of_big_integers.cpp)、[10進の除算](../Verify/current/submissions/division_of_big_integers.cpp)、[16進の除算](../Verify/current/submissions/division_of_hex_big_integers.cpp)です。
