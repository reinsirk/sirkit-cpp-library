# 同じ要素数の部分集合マスク

符号なし整数の1ビットを1要素とし、同じ個数を選ぶ組合せをマスクの数値昇順に列挙します。UIntの既定値は`std::uint64_t`です。

## API

### next_combination

```text
template<std::unsigned_integral UInt>
constexpr UInt next_combination(UInt mask) noexcept;
constexpr int next_combination(int mask);
template<std::unsigned_integral UInt>
constexpr bool next_combination(UInt& mask, int n);
```

1引数版は1の個数が同じ次のマスクを返します。入力が0か、次のマスクがなければ0を返します。幅指定版は下位nビット内で次のマスクに更新します。次がなければfalseを返し、maskは変えません。

**制約:** UIntはbool以外。int版は非負。幅指定版は0<=n<=型幅、maskが下位nビット内。

**計算量:** O(1)時間・追加領域。

### first_combination

```text
template<std::unsigned_integral UInt = std::uint64_t>
constexpr UInt first_combination(int k);
```

下位kビットが1の最小マスクを返します。

**制約:** UIntはbool以外、0<=k<=型幅。

**計算量:** O(1)時間・追加領域。

### last_combination

```text
template<std::unsigned_integral UInt = std::uint64_t>
constexpr UInt last_combination(int n, int k);
```

下位nビット内で1がk個の最大マスクを返します。

**制約:** UIntはbool以外、0<=k<=n<=型幅。

**計算量:** O(1)時間・追加領域。

### for_each_combination

```text
template<std::unsigned_integral UInt = std::uint64_t, class Visitor>
constexpr void for_each_combination(int n, int k, Visitor&& visit);
```

各マスクを値の小さい順にvisit(mask)へ渡します。k=0の場合もvisitを1回呼びます。

**制約:** UIntはbool以外、0<=k<=n<=型幅。callbackの例外は伝わります。

**計算量:** O(C(n,k))回のcallback・O(1)追加領域。callback自身の費用は別。


追加領域はO(1)、列挙のcallbackの費用は別です。callbackの例外は呼出元へ伝わります。

## 前提

`0<=k<=n<=UIntの型幅`です。1引数版にはint版もあり、maskは非負、次が正のintに収まらなければ0を返します。全幅シフトは起こしません。

入力検査は`SKIP_RUNTIME_CHECKS`で省けます。列挙回数の上限は設けないため、C(n,k)の大きさは利用側で確認します。

[完全な使用例](../Examples/next_combination_example.cpp)
