# エラトステネスの篩

[実装](../Math/Eratosthenes.cpp)・[完全な使用例](../Examples/eratosthenes_example.cpp)

## API

### PrimeSieve::PrimeSieve

```text
explicit PrimeSieve(std::uint64_t limit = 0);
PrimeSieve(const PrimeSieve&);
PrimeSieve(PrimeSieve&& other) noexcept;
```

上限以下の素数表を構築します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 上限NにO(N log log(N+2))時間・O(N) bit保持。

### PrimeSieve::operator=

```text
PrimeSieve& operator=(const PrimeSieve& other);
PrimeSieve& operator=(PrimeSieve&& other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** コピーはO(N/64+1)、移動は旧領域の解放を含む。

### PrimeSieve::is_prime

```text
bool is_prime(std::uint64_t n) const;
```

素数ならtrueを返します。

**制約:** 0<=n<=limit()。

**計算量:** O(1)。

### PrimeSieve::limit

```text
std::uint64_t limit() const noexcept;
```

篩の上限を返します。

**制約:** なし。

**計算量:** O(1)。

### PrimeSieve::prime_count

```text
std::uint64_t prime_count() const noexcept;
```

上限以下の素数の個数を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N/64+1)。

### PrimeSieve::primes

```text
std::vector<std::uint64_t> primes() const;
```

昇順の素数列を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** P個の素数にO(N/64+P+1)時間・O(P)返却領域。

### PrimeSieve::append_to

```text
[[gnu::always_inline]] #endif void append_to(std::vector<std::uint64_t>& ans) const;
```

素数列を昇順に出力先vectorへ追記します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N/64+P+1)時間・出力領域。

### PrimeSieve::for_each_prime

```text
template <class Function> void for_each_prime(Function&& function) const;
```

各素数を昇順に`f(prime)`へ渡します。列挙用vectorは作りません。

**制約:** callback中に同じ表を変更・移動・破棄しない。

**計算量:** O(N/64+P+1)時間とcallback費用。

### PrimeSieve::for_each_prime_stride

```text
template <class Function> void for_each_prime_stride(std::uint64_t first, std::uint64_t step, Function&& function) const;
```

番号`first, first+step, ...`の素数だけを渡します。`step>0`が前提です。該当する番号がなければコールバックを呼びません。

**制約:** step>0。callback中に同じ表を変更・移動・破棄しない。

**計算量:** 最悪O(N/64+P+1)時間とcallback費用。

### eratosthenes

```text
void eratosthenes(std::uint64_t limit, std::vector<std::uint64_t>& ans);
std::vector<std::uint64_t> eratosthenes(std::uint64_t limit);
```

素数列を返す／既存列へ追記する。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 構築と列挙の費用。


`PrimeSieve(limit)`で`[0, limit]`の素数表を作ります。判定を繰り返す場合は表を保持し、列挙だけなら`eratosthenes(limit)`で素数列を取得できます。

## API

上限・素数・素数番号は`std::uint64_t`です。素数番号は`2`を0番目とする昇順の添字です。


```cpp
PrimeSieve sieve(30);
bool yes = sieve.is_prime(29);       // true
const auto primes = sieve.primes(); // {2,3,5,7,11,13,17,19,23,29}
std::vector<std::uint64_t> selected;
sieve.for_each_prime_stride(1, 3, [&](std::uint64_t p) {
    selected.push_back(p);
}); // selected={3,11,19}。素数番号1,4,7。
```

## 前提・所有と計算量

構築は`O(limit log log limit)`時間、保持量は`O(limit)` bitです。奇数1個につき1bitを使い、大きい表は一定範囲ずつ篩います。
`limit()`・`is_prime()`は`O(1)`、`prime_count()`は`O(limit/64)`です。

素数の個数を`P`とすると、列挙は最悪`O(limit/64+P)`です。間引き列挙では、出力対象のないワード内の素数を数えるだけで、個々の値の列挙は省きます。通常列挙・間引き列挙とも追加領域は`O(1)`で、callbackの費用は別です。`primes()`と追記先には出力件数分の領域が必要です。

列挙中に同じ表を代入・移動・破棄しないでください。callbackが例外を送出しても表は変わりません。
コピーは独立所有で、コピー代入の確保失敗時は元の表と上限を保ちます。移動元は上限0の空の表になり、代入して再利用できます。

既定では`is_prime()`の上限と`step=0`を検査し、それぞれ`std::out_of_range`・`std::invalid_argument`を送出します。`SKIP_RUNTIME_CHECKS`で省略する場合も前提は同じです。内部の容量・添字上限は省略時も維持します。

個数集計には対応環境でSIMDを使い、未対応環境は標準C++の経路を使います。無効化などは[SIMDによる内部処理](simd.md)を参照してください。

[Verify提出コード](../Verify/current/submissions/eratosthenes.cpp)・[境界の独立参照](../Verify/current/tests/prime_sieve_boundary_test.cpp)。内部処理・測定・検証の記録は移行前の記録で確認できます。
