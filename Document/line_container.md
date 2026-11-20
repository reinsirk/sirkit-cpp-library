# 直線の追加と最適値照会

`LineContainer<T, objective>`は直線`y=k*x+m`を追加し、指定した`x`での最小値または最大値を返します。傾きや照会順の制約はありません。`MinLineContainer<T>`と`MaxLineContainer<T>`を使うと目的を省略できます。[使用例](../Examples/line_container_example.cpp)を参照してください。

保持直線数をhとすると、領域O(h)、追加は償却O(log(h+1))、照会はO(log(h+1))です。1回の追加で多数の直線を除く場合はO(h)かかります。

## API

### LineContainer

```text
LineContainer();
```

空の包絡を構築します。

**制約:** Tとobjectiveは冒頭の型・目的の条件を満たすこと。

**計算量:** O(1)。

### add

```text
void add(T k, T m);
```

直線y=k*x+mを追加します。常に劣る直線や重複する直線を除くことがあります。

**制約:** 整数係数はT内、浮動係数は有限。

**計算量:** 償却O(log(h+1))、1回の最悪O(h)。

### query

```text
T query(T x) const;
```

最適値を返します。同じ最適値を与える直線のうちどれを選ぶかは未規定です。

**制約:** 非空で、浮動xは有限。整数の最適値がT外なら常にoverflow_error。

**計算量:** O(log(h+1))。

### query_optional

```text
std::optional<T> query_optional(T x) const;
```

最適値を返します。空ならnulloptです。

**制約:** 浮動xは有限。整数の最適値がT外なら常にoverflow_error。

**計算量:** O(log(h+1))。

### query_line

```text
std::pair<T,T> query_line(T x) const;
```

選択した直線の元の係数{k,m}を返します。同じ最適値を与える直線のうちどれを選ぶかは未規定です。

**制約:** 非空で、浮動xは有限。値の評価は行いません。

**計算量:** O(log(h+1))。

### query_line_optional

```text
std::optional<std::pair<T,T>> query_line_optional(T x) const;
```

選択した元係数{k,m}を返します。空ならnulloptです。

**制約:** 浮動xは有限。値の評価は行いません。

**計算量:** O(log(h+1))。

### query_with_line

```text
Result query_with_line(T x) const;
```

Result{value,k,m}を返します。同点の直線の選択は未規定です。

**制約:** 非空で、浮動xは有限。整数の最適値がT外なら常にoverflow_error。

**計算量:** O(log(h+1))。

### query_with_line_optional

```text
std::optional<Result> query_with_line_optional(T x) const;
```

Result{value,k,m}を返します。空ならnulloptです。

**制約:** 浮動xは有限。整数の最適値がT外なら常にoverflow_error。

**計算量:** O(log(h+1))。

### size

```text
size_type size() const noexcept;
```

包絡に保持する直線数を返します。追加回数とは異なります。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

包絡が空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

全直線を破棄します。

**制約:** 追加の制約はありません。

**計算量:** O(h)。

### swap

```text
void swap(super& other) noexcept;
```

同じ型の基底multisetと内容を交換します。superは公開される基底型の別名です。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### lc_div

```text
template<class T> T lc_div(T a, T b);
```

整数では床除算、浮動小数点では通常の除算を返します。

**制約:** b!=0。整数結果がT外なら常にoverflow_error。

**計算量:** O(1)。

### lc_inf

```text
template<class T> constexpr T lc_inf();
```

整数ではTの最大値、浮動小数点では正の無限大を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


### begin

```text
super::iterator begin() noexcept;
super::const_iterator begin() const noexcept;
```

傾き順の先頭iteratorを返します。

**制約:** 削除された直線へのiteratorは保持しません。

**計算量:** O(1)。

### end

```text
super::iterator end() noexcept;
super::const_iterator end() const noexcept;
```

末尾の次を表すiteratorを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### lower_bound

```text
super::iterator lower_bound(T x);
super::const_iterator lower_bound(T x) const;
```

右端境界pがx以上の最初の直線を指すiteratorを返します。

**制約:** 正しい包絡状態で使います。

**計算量:** O(log(h+1))。

### insect

```text
bool insect(super::iterator x, super::iterator y);
```

旧互換の交点更新を行い、更新後の境界関係をboolで返します。通常の更新にはaddを使います。

**制約:** 包絡の正しいiteratorだけを渡す内部用の操作です。

**計算量:** O(1)。

同じ最適値を与える直線のうちどれを選ぶかは未規定です。`Result`は値と係数を所有し、コンテナ内の要素への参照を持ちません。最適値とそれを与える直線が両方必要な場合に使います。

```cpp
MinLineContainer<std::int64_t> lines;
lines.add(2, 1);
lines.add(-1, 4);
auto result = lines.query_with_line(3);
// result.value == 1, result.k == -1, result.m == 4
```

## 数値と入力の前提

整数Tは64bit以下の符号付き型です。傾き・切片・問い合わせはTに収めます。交点は整数の床除算で求め、型の範囲へ制限した右端を保持します。係数差の符号付きオーバーフローを避けて計算し、型の最小値ちょうどで選択される直線も扱います。

整数の最適値は数学的な`k*x+m`を評価し、Tの範囲外なら`SKIP_RUNTIME_CHECKS`の有無によらず`overflow_error`を投げます。積だけがTを超えても、切片との相殺で最終値がTに収まれば返します。利用可能なGCC/Clangでは128bit整数拡張を使い、`SIRKIT_LINE_CONTAINER_FORCE_PORTABLE`で標準C++の代替計算へ切り替えられます。拡張型がない環境も代替経路を使います。`query_line`系は最適値の評価をしないため、この値域検査を行いません。

浮動Tでは有限の係数・問い合わせを渡します。交点や値には丸め誤差があり、厳密な実数の最適値を保証しません。値の表現範囲を超える計算は無限大やNaNになり得ます。整数版と異なり、浮動値の計算結果を範囲外例外へ変換しません。

検査有効時は非有限の浮動入力を`invalid_argument`、非optional照会の空表を`out_of_range`で拒否します。`SKIP_RUNTIME_CHECKS`はこれらを除外します。`lc_div`のゼロ除算は`domain_error`、整数の商がTに収まらない場合は`overflow_error`を常に投げます。

## 所有権と参照

包絡のノードは`std::multiset`が所有します。コピーは独立した包絡を持ち、ムーブ後の元も有効な状態で再利用できます。追加の確保失敗では元の包絡を保持します。コピー代入の例外保証は基底の`std::multiset`に従い、失敗時の元内容保持は保証しません。

公開の`begin()`・`end()`・`lower_bound(x)`は包絡の参照に使います。最小化でも`k,m`は元の符号で保存し、傾き順は最大化が昇順、最小化が降順です。整数の`p`は型の範囲へ制限されるため、真の交点そのものとは限りません。追加で削除された直線へのiteratorや参照は無効になります。

基底の`insert`・`erase`を呼び出したり、mutable係数を直接変更したりすると、隣接交点との整合性が失われます。更新には`add`と`clear`を使います。`insect(x,y)`は旧実装との互換性のために残している内部の交点更新です。

同梱部品は2次元版と共有ガードで併用できます。
