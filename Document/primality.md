# 素数判定と隣接素数

符号付き・符号なしの64ビット以下の整数を扱います。`is_prime`は`std::uint64_t`全域で確定的に判定します。

## API

### MillerRabin

```text
template <class Integer, class Bases> bool MillerRabin(Integer n, const Bases &bases) noexcept(::sirkit::detail::miller_rabin_bases_nothrow<Bases>);
template <class Integer> bool MillerRabin(Integer n, std::initializer_list<std::uint64_t> bases) noexcept;
```

指定した底で強擬素数検査に通ればtrueを返します。

**制約:** 64ビット以下の整数の底列。剰余0の底は省く。n<2はfalse、偶数はn==2。奇数n>=3で有効な底が空ならtrue。

**計算量:** is_primeはO(log(n+1))、指定底k個はO(k log(n+1))時間・O(1)領域。

### is_prime

```text
template <class Integer> bool is_prime(Integer n) noexcept;
```

素数ならtrueを返します。

**制約:** 64ビット以下の整数。負数・0・1はfalse。

**計算量:** is_primeはO(log(n+1))、指定底k個はO(k log(n+1))時間・O(1)領域。

### next_prime

```text
template<class Integer> std::optional<std::uint64_t> next_prime(Integer n) noexcept;
```

n以上の最小の素数をoptional<uint64_t>で返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 候補数cとその最大値NにO(1+c log(N+1))時間・O(1)領域。

### prev_prime

```text
template<class Integer> std::optional<std::uint64_t> prev_prime(Integer n) noexcept;
```

n以下の最大の素数を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 候補数cとその最大値NにO(1+c log(N+1))時間・O(1)領域。


```cpp
assert(is_prime(998244353));
assert(!is_prime(-7));
assert(*next_prime(24) == 29);
assert(*prev_prime(24) == 23);
assert(MillerRabin(2047, {2})); // 合成数でも指定底を通る例。
```

動的確保や底列の保持はしません。`is_prime`・隣接素数・整数リスト版は`noexcept`です。汎用底列版はiterator操作などの例外を伝えます。`SKIP_RUNTIME_CHECKS`でも負数・0・1や表現範囲の扱いは同じです。

[完全な使用例](../Examples/primality_example.cpp)・[来歴と固定底の根拠](../Audit/Provenance/provenance-primality-2026-10-22.json)・[内部処理の記録](../Audit/Changes/library-docs-concise-math-geometry-2026-11-15.json)
