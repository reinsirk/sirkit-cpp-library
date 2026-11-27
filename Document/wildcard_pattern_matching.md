# ワイルドカード付き文字列照合

テキストの各位置から、パターンと一致するかを調べます。どちらの文字列でも、`*` は任意の1文字と一致します。

```cpp
auto matches = sirkit::wildcard_pattern_matching("ab*c", "a*");
// {true, false, true}
```

## `wildcard_pattern_matching`

```text
std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern);
```

引数は `std::string_view` で、使える文字は英小文字と `*` です。戻り値は `std::vector<bool>` で、要素 `i` はテキストの位置 `i` からパターンが一致するかを表します。パターンが空なら両端を含むすべての境界で一致し、テキストより長ければ戻り値は空です。

両入力の長さは $2^{19}$ 以下です。上限を超えると `std::length_error`、通常版で対象外の文字を渡すと `std::invalid_argument` を送出します。

ワイルドカードを除いた文字の差の二乗を足し、和が0なら一致と判定します。畳み込みの係数を法998244353未満に抑えているため、剰余から正しい和が得られます。長さをそれぞれ $N,M$ とすると、パターンが32文字以下なら $O(N+M+1)$ 時間、その他は $O((N+M)\log(N+M))$ 時間です。追加領域は $O(N+M+1)$ です。

単独貼り付け版に [NTT](ntt.md) を同梱しています。

[使用例](../Examples/wildcard_pattern_matching_example.cpp) ／ [Verify](../Verify/current/submissions/wildcard_pattern_matching.cpp)
