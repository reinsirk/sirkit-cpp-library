# 接頭辞と部分文字列のLCS

固定した文字列 `s` と `t` に対して、`s` の接頭辞と `t` の連続区間を指定し、最長共通部分列の長さを繰り返し求めます。

```cpp
const sirkit::PrefixSubstringLCS lcs("abc", "zbcab");
auto answer = lcs.length(2, 1, 5);  // "ab" と "bcab" のLCS長は2
```

## `PrefixSubstringLCS`

```text
PrefixSubstringLCS(std::string_view s, std::string_view t);
```

引数は `std::string_view` です。比較はバイト単位で、空文字列も渡せます。$M=|s|$ と $N=|t|$ が正なら、前処理は $O(MN\log(N+1))$ 時間です。保存領域は64ビットのブロックを単位として $O(M(\lfloor N/64\rfloor+1)\log(N+1))$、前処理に使う作業領域は $O(N)$ です。

## `length`

```text
std::size_t length(std::size_t a, std::size_t b, std::size_t c) const;
```

`s[0,a)` と `t[b,c)` のLCSの長さを返します。添字は0始まりで、右端は区間に含みません。片方でも空区間なら0です。通常版で添字が範囲外、または `b > c` の場合は `std::out_of_range` を送出します。

開始位置ごとのDPの差をしきい値として記録し、区間に含まれるしきい値をウェーブレット行列で数えます。問い合わせ1回は $O(1+\log(N+1))$ です。

[使用例](../Examples/prefix_substring_lcs_example.cpp) ／ [Verify](../Verify/current/submissions/prefix_substring_lcs.cpp) ／ [原論文](https://arxiv.org/abs/0707.3619)
