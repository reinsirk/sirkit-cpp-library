# 分数

[実装](../Math/Fraction.cpp)・[使用例](../Examples/fraction_example.cpp)

## API

### Fraction::Fraction

```text
constexpr Fraction();
constexpr Fraction(std::int64_t n, std::int64_t d = 1);
```

整数またはn/dから既約分数を構築します。既定構築は0/1です。

**制約:** 分母は非零。約分後の分子と正分母がint64_tに収まること。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::numerator

```text
constexpr std::int64_t numerator() const noexcept;
```

既約分子を返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::denominator

```text
constexpr std::int64_t denominator() const noexcept;
```

正の既約分母を返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::sign

```text
constexpr int sign() const noexcept;
```

値の符号を-1/0/1で返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::is_integer

```text
constexpr bool is_integer() const noexcept;
```

整数ならtrueを返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::trunc

```text
constexpr std::int64_t trunc() const noexcept;
```

0方向へ丸めた整数を返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::floor

```text
constexpr std::int64_t floor() const noexcept;
```

床関数で丸めた整数を返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::ceil

```text
constexpr std::int64_t ceil() const noexcept;
```

天井関数で丸めた整数を返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::inverse

```text
constexpr Fraction inverse() const;
```

逆数。

**制約:** 分母となる値は非零。既約結果がint64_tの分子・正分母で表せること。

**計算量:** O(1)。

### Fraction::abs

```text
constexpr Fraction abs() const;
```

絶対値を返します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** O(1)。

### Fraction::operator T

```text
template<std::floating_point T> explicit constexpr operator T() const noexcept;
```

浮動小数点型Tへ明示的に変換します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::operator+

```text
constexpr Fraction operator+() const noexcept;
friend constexpr Fraction operator+(const Fraction& a, const Fraction& b);
```

和を返します。単項版は同じ値を返します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator-

```text
constexpr Fraction operator-() const;
friend constexpr Fraction operator-(const Fraction& a, const Fraction& b);
```

差を返します。単項版は符号を反転します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator*

```text
friend constexpr Fraction operator*(const Fraction& a, const Fraction& b);
```

積を返します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator/

```text
friend constexpr Fraction operator/(const Fraction& a, const Fraction& b);
```

商を返します。

**制約:** 分母となる値は非零。既約結果がint64_tの分子・正分母で表せること。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator+=

```text
constexpr Fraction& operator+=(const Fraction& b);
```

加算して自身を更新します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator-=

```text
constexpr Fraction& operator-=(const Fraction& b);
```

減算して自身を更新します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator*=

```text
constexpr Fraction& operator*=(const Fraction& b);
```

乗算して自身を更新します。

**制約:** 表現可能な結果を渡すこと。範囲外はoverflow_error。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator/=

```text
constexpr Fraction& operator/=(const Fraction& b);
```

除算して自身を更新します。

**制約:** 分母となる値は非零。既約結果がint64_tの分子・正分母で表せること。

**計算量:** Mを整数の絶対値の上限としてO(log(M+1))。既定構築と単項演算はO(1)。

### Fraction::operator==

```text
constexpr bool operator==(const Fraction&) const;
```

値と形が等しいかを返します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::operator<=>

```text
constexpr std::strong_ordering operator<=>(const Fraction& b) const;
```

大小を三方比較します。

**制約:** 有効なFractionなら追加制約なし。

**計算量:** O(1)。

### Fraction::operator<<

```text
friend std::ostream& operator<<(std::ostream& out, const Fraction& x);
```

ストリームへ値を出力します。

**制約:** 有効なFraction。ストリームの例外設定に従う。

**計算量:** 入出力の費用。読込みは約分のO(log(M+1))を加える。

### Fraction::operator>>

```text
friend std::istream& operator>>(std::istream& in, Fraction& x);
```

整数またはn/dを読み、成功時だけ代入します。分母0・範囲外等はfailbitを立て、代入先を保ちます。

**制約:** 「/」は分子の直後。ストリームの例外設定に従う。

**計算量:** 入出力の費用。読込みは約分のO(log(M+1))を加える。


`Fraction` は分子・分母を `std::int64_t` で持つ厳密な分数です。構築と演算の結果を必ず約分し、分母は正、0は `0/1` に揃えます。無限大やNaNは扱いません。値の保持と正常系の算術では動的なメモリ確保を行いません。

```cpp
Fraction a(1, 3), b(-3, 4);
auto c = a + b;                  // -5/12
bool less = a < b;               // false
auto integer = b.floor();        // -1
double approximate = static_cast<double>(a);
```


Mは扱う整数の絶対値の上限です。固定幅の乗算・比較・除算を定数時間、ユークリッド互除法をO(log M)として記しています。構築・算術・比較・丸めは定数式でも使えます。

## 表現範囲と失敗

**約分後**の分子が `[-2^63, 2^63-1]`、正の分母が `[1, 2^63-1]` に収まる必要があります。最終既約結果がこの範囲外なら `std::overflow_error` を送出します。中間の交差積や和だけが64bitを超える場合も計算でき、単に中間値が大きいという理由では拒否しません。

分母0、0による除算、0の逆数は `std::domain_error` です。分子最小値の絶対値や符号反転など、表現できない結果にもオーバーフロー例外を使います。複合代入は失敗しても元の値を保持します。

入力の `/` は分子の直後に置きます。入力の整数部分がint64_tで読めない場合、分母が0の場合、約分結果が範囲外の場合はストリームを失敗状態にし、代入先を保持します。丸め・比較は浮動小数点を使いませんが、明示的な浮動変換は対象型の丸めを伴います。

これらは数学的な0判定と内部表現の保護なので、`SKIP_RUNTIME_CHECKS` を定義しても維持します。省略対象となる追加の検査状態はありません。入力はint64_tを前提とし、それを超える符号なし整数を渡す前の型変換は利用側で確認してください。

`SIRKIT_FRACTION_DISABLE_INT128`で標準C++の演算経路を選べます。設定マクロは貼り付け前に指定し、複数の翻訳単位で揃えてください。

[完全な使用例](../Examples/fraction_example.cpp)・[演算方法と検証の記録](../Audit/library-docs-concise-math-geometry-2026-11-15.json)
