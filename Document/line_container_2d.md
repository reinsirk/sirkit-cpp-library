# 2次元の内積最適化

`LineContainer2D`は点を追加し、方向`(a,b)`に対する`a*x+b*y`の最大値・最小値と、その点を照会します。削除には対応しません。[使用例](../Examples/line_container_2d_example.cpp)を参照してください。

以下、hは両包絡に保持する点数の合計です。保持領域はO(h)、追加は償却O(log h)、照会はO(log h)です。多数の点を包絡から除く1回の追加にはO(h)かかる場合があります。

## API

### LineContainer2D

```text
LineContainer2D();
explicit LineContainer2D(std::int64_t inf);
```

空で構築します。省略時のinfは2^60です。

**制約:** inf>0。

**計算量:** O(1)。

### infinity

```text
std::int64_t infinity() const noexcept;
```

設定した制限値を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### add

```text
void add(long double x, long double y);
```

点(x,y)を追加します。重複も追加件数に含みます。

**制約:** 座標が有限で、後述の数値条件を満たすこと。

**計算量:** 償却O(log(h+1))、1回の最悪O(h)。

### max_ld

```text
long double max_ld(long double a, long double b) const;
template<class A, class B> long double max_ld(A a, B b) const;
```

最大内積値を返します。整数型の専用オーバーロードも浮動小数点で計算します。

**制約:** 非空。浮動係数は有限。整数A/Bは型全体がint64_t範囲内。

**計算量:** O(log(h+1))。

### max_ll

```text
std::int64_t max_ll(std::int64_t a, std::int64_t b) const;
```

最大内積値を±infに制限し、0方向へ切り捨てます。

**制約:** 非空。

**計算量:** O(log(h+1))。

### max_rounded_ll

```text
std::int64_t max_rounded_ll(std::int64_t a, std::int64_t b) const;
```

最大内積値を±infに制限し、最も近い整数へ丸めます。半整数は0から遠い側です。

**制約:** 非空。

**計算量:** O(log(h+1))。

### argmax

```text
std::pair<long double,long double> argmax(long double a, long double b) const;
```

最大値を与える元の点を返します。同点の選択は未規定です。

**制約:** 非空、係数は有限。

**計算量:** O(log(h+1))。

### max_with_point

```text
Result max_with_point(long double a, long double b) const;
```

点を1回選び、最大値とその点をResult{value,x,y}で返します。

**制約:** 非空、係数は有限。

**計算量:** O(log(h+1))。

### max_optional

```text
std::optional<long double> max_optional(long double a, long double b) const;
```

最大内積値を返し、空ならnulloptです。

**制約:** 係数は有限。

**計算量:** O(log(h+1))。

### argmax_optional

```text
std::optional<std::pair<long double,long double>> argmax_optional(long double a, long double b) const;
```

最大値を与える点を返し、空ならnulloptです。

**制約:** 係数は有限。

**計算量:** O(log(h+1))。

### min_ld

```text
long double min_ld(long double a, long double b) const;
template<class A, class B> long double min_ld(A a, B b) const;
```

最小内積値を返します。整数型の専用オーバーロードも浮動小数点で計算します。

**制約:** 非空。浮動係数は有限。整数A/Bは型全体がint64_t範囲内。

**計算量:** O(log(h+1))。

### min_ll

```text
std::int64_t min_ll(std::int64_t a, std::int64_t b) const;
```

最小内積値を±infに制限し、0方向へ切り捨てます。

**制約:** 非空。

**計算量:** O(log(h+1))。

### min_rounded_ll

```text
std::int64_t min_rounded_ll(std::int64_t a, std::int64_t b) const;
```

最小内積値を±infに制限し、最も近い整数へ丸めます。半整数は0から遠い側です。

**制約:** 非空。

**計算量:** O(log(h+1))。

### argmin

```text
std::pair<long double,long double> argmin(long double a, long double b) const;
```

最小値を与える元の点を返します。同点の選択は未規定です。

**制約:** 非空、係数は有限。

**計算量:** O(log(h+1))。

### min_with_point

```text
Result min_with_point(long double a, long double b) const;
```

点を1回選び、最小値とその点をResult{value,x,y}で返します。

**制約:** 非空、係数は有限。

**計算量:** O(log(h+1))。

### min_optional

```text
std::optional<long double> min_optional(long double a, long double b) const;
```

最小内積値を返し、空ならnulloptです。

**制約:** 係数は有限。

**計算量:** O(log(h+1))。

### argmin_optional

```text
std::optional<std::pair<long double,long double>> argmin_optional(long double a, long double b) const;
```

最小値を与える点を返し、空ならnulloptです。

**制約:** 係数は有限。

**計算量:** O(log(h+1))。

### size

```text
std::size_t size() const noexcept;
```

重複を含む追加点数を返します。包絡に残る点数とは異なります。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

登録点がなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

全点を削除し空にします。

**制約:** 追加の制約はありません。

**計算量:** O(h)。

### swap

```text
void swap(LineContainer2D& other) noexcept;
```

制限値・点・確保領域を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


同点で選ばれる点は規定しません。`Result`は3つの値を所有し、表の要素への参照を持ちません。内積値と選択点が両方必要な場合に使います。
`value`は返した点の内積です。浮動計算の順序により、別に呼んだ`max_ld`・`min_ld`とビット単位で一致する保証はありません。

```cpp
LineContainer2D points(1000000); // ll版を±1000000に制限。
points.add(1, 2);
points.add(3, -1);
auto result = points.max_with_point(2, 1);
// result.value == 5, result.x == 3, result.y == -1
```

## 数値と入力の前提

座標と浮動係数は有限値が前提です。型本来の`long double`精度と既定の丸めモードで使います。内積・交点には丸め誤差があり、整数引数やll版でも厳密な整数内積を保証しません。丸め前に精度を失った値は、rounded版でも復元できません。ゼロの符号は規定しません。

非空の照会で、有限な非零`a`に対して`b/a`が表現範囲を超える場合は、`SKIP_RUNTIME_CHECKS`の有無によらず`overflow_error`を投げます。内積が浮動小数点範囲を超える場合は無限大になり得ます。巨大な有限積同士の相殺でNaNになった場合は、指数を調整して再評価します。ll版は内積を±infへ制限するため、範囲外の数学的な値をそのまま返すAPIではありません。

検査が有効な場合は非有限の入力を拒否し、点がない状態でoptionalを返さない照会を呼ぶと`out_of_range`を投げます。`SKIP_RUNTIME_CHECKS`を定義すると、これらの入力検査を除外します。追加点数の上限、比の表現範囲、相殺の回復は維持します。`SIRKIT_LINE_CONTAINER_FORCE_PORTABLE`は同梱1D部品の整数演算の標準代替経路を指定するもので、2D演算を厳密整数へ切り替える設定ではありません。

`SIRKIT_LINE_CONTAINER_STANDARD_HINTS`を定義すると、補助キャッシュに座標を複製せず、直線への参照で照会します。

## 所有権と公開メンバー

コピーはO(h)で独立した包絡を持ち、内部の選択点参照も組み直します。ムーブはO(1)で、元は空として再利用できます。コピー・move・clearでもinfを保持し、swapでは設定も交換します。自己代入は内容を保持します。追加途中の確保失敗では、両包絡と登録件数を空に戻して例外を伝えます。コピー代入は一時コピーが完成してから交換します。

公開の`minlc`・`maxlc`・`Xmin`・`Xmax`・`Ymin`・`Ymax`は読み取り用です。包絡の直線は元点の`k=y,m=x`を保持します。外から書き換えたり、包絡だけに追加・削除したりすると、件数・座標範囲・内部参照の不変条件が壊れます。
