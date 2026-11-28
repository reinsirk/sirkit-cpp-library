# 二次元パターン検索

文字のグリッドから、指定した長方形と一致する部分をすべて探します。文字はバイト単位で比較します。

## `find_grid_pattern`

```cpp
template <class Text, class Pattern>
std::vector<std::pair<std::size_t, std::size_t>>
find_grid_pattern(const Text &text, const Pattern &pattern);
```

一致した長方形の左上座標を、行と列の昇順で返します。座標は0始まりの `{行, 列}` です。重なり合う一致も列挙します。

`std::vector<std::string>` など、`size()` と添字アクセスを備え、各行を `std::string_view` に変換できるグリッドを渡せます。同じグリッド内の行はすべて同じ幅にしてください。検査が有効なときは不揃いな行に対して `std::invalid_argument` を送出します。高さや幅が0のグリッドでは一致を返しません。

Aho–Corasick 法で各位置をパターンの行と照合し、KMP 法で縦に並ぶ行の一致を調べます。盤面を $H\times W$、パターンを $R\times C$、パターンの異なるバイト数を $\sigma$、一致数を $K$ とすると、$O(H+R+\sigma(HW+RC)+K)$ 時間、入力を除いて $O(RC+W+K)$ 領域です。

## 使用例

```cpp
const std::vector<std::string> text{"ababa", "babab", "ababa"};
const std::vector<std::string> pattern{"aba", "bab"};
const auto matches = sirkit::find_grid_pattern(text, pattern);
// matches は {{0, 0}, {0, 2}, {1, 1}}。
```

[実装](../String/Grid_Pattern_Matching.cpp)・[使用例の全体](../Examples/grid_pattern_matching_example.cpp)・[AtCoder Verify](../Verify/current/submissions/grid_pattern_matching_seek_grid.cpp)・[AOJ Verify](../Verify/current/submissions/grid_pattern_matching_all.cpp)
