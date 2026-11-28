# 各項が次の項を割る数列の数え上げ

- `divisibility_chain_end_counts(length, max_value)[x]` は、長さ `length` で末項が $x$ の数列の個数を法 $998244353$ で返します。`IntegerSieve` を直接渡して最小素因数表を再利用できます。時間・追加領域ともに $O(1+\mathrm{max\_value})$ です。
- `divisibility_chain_product_count(value)` は初項が2以上で、全項の積が `value` になる数列を正確に数えます。長さは任意です。64ビット素因数分解の期待時間に加え、異なる素因数の個数に比例します。

どちらも正の整数からなる列を数え、同じ値が続いてもかまいません。たとえば積が4になる列は `(4)` と `(2,2)` の2通りです。

[使用例](../Examples/divisibility_chains_example.cpp) ／ [末項のVerify](../Verify/current/submissions/divisibility_chain_ends_arc116_c.cpp) ／ [積のVerify](../Verify/current/submissions/divisibility_chain_product_yuki2798.cpp)
