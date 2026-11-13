# エラトステネスの篩

[実装](../Math/Eratosthenes.cpp)・[完全な使用例](../Examples/eratosthenes_example.cpp)

`PrimeSieve(limit)`で`[0, limit]`の素数表を作ります。判定を繰り返す場合は表を保持し、列挙だけなら`eratosthenes(limit)`で素数列を取得できます。

## API

上限・素数・素数番号は`std::uint64_t`です。素数番号は`2`を0番目とする昇順の添字です。

| 呼び出し | 動作 |
| --- | --- |
| `PrimeSieve(limit=0)` | 上限以下の素数表を構築する。 |
| `limit()` / `is_prime(n)` | 上限／`0<=n<=limit`の素数判定を返す。 |
| `prime_count()` | 上限以下の素数の個数を返す。 |
| `primes()` | 昇順の`std::vector<std::uint64_t>`を返す。 |
| `append_to(result)` | 既存のvectorの末尾へ素数列を追加する。 |
| `for_each_prime(f)` | 各素数を昇順に`f(prime)`へ渡す。列挙用vectorを作らない。 |
| `for_each_prime_stride(first, step, f)` | 番号`first, first+step, ...`の素数だけを渡す。`step>0`、該当番号がなければ呼ばない。 |
| `eratosthenes(limit)` / `(limit, result)` | 素数列を返す／既存列へ追記する。 |

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

素数数を`P`として列挙は最悪`O(limit/64+P)`です。間引き列挙は、出力対象のないワードを個数だけ数えて飛ばします。通常列挙・間引き列挙とも追加領域は`O(1)`で、callbackの費用は別です。`primes()`と追記先には出力件数分の領域が必要です。

列挙中に同じ表を代入・移動・破棄しないでください。callbackが例外を送出しても表は変わりません。
コピーは独立所有で、コピー代入の確保失敗時は元の表と上限を保ちます。移動元は上限0の空の表になり、代入して再利用できます。

既定では`is_prime()`の上限と`step=0`を検査し、それぞれ`std::out_of_range`・`std::invalid_argument`を送出します。`SKIP_RUNTIME_CHECKS`で省略する場合も前提は同じです。内部の容量・添字上限は省略時も維持します。

個数集計には対応環境でSIMDを使い、未対応環境は標準C++の経路を使います。無効化などは[SIMDによる内部処理](simd.md)を参照してください。

[Verify提出コード](../Verify/current/submissions/eratosthenes.cpp)・[境界の独立参照](../Verify/current/tests/prime_sieve_boundary_test.cpp)。内部処理・測定・検証の記録は`Audit/`に置きます。
