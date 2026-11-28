# 長さを指定した辞書順最小部分列

元の並び順を保ったまま、指定した個数の要素を選びます。たとえば `atcoder` から3文字を選ぶと、辞書順最小の部分列は `acd` です。

## `lexicographic_subsequence_indices`

```cpp
auto indices = sirkit::lexicographic_subsequence_indices(values, k);
```

長さ `k` の辞書順最小部分列を表す添字列を、0始まりで返します。`values` は長さが分かるランダムアクセス範囲です。第3引数に `std::greater<>{}` を渡すと辞書順最大になります。要素列が比較上同じなら、添字列が辞書順で最小の候補を選びます。

## `lexicographic_subsequence`

```cpp
std::string lexicographic_subsequence(std::string_view text, std::size_t k);
```

符号なしバイト順で最小の部分列を文字列で返します。位置が必要なら `lexicographic_subsequence_indices` を使います。

どちらも $O(n+1)$ 時間・$O(k+1)$ 領域です。比較1回を定数時間とします。長さ `k` の有効範囲は0以上、入力の長さ `n` 以下です。検査が有効なとき、`k > n` なら `std::invalid_argument` を送出します。

## 使用例

```cpp
assert(sirkit::lexicographic_subsequence("atcoder", 3) == "acd");
const std::array values{3, 1, 1, 2};
const std::vector<std::size_t> expected{1, 2};
assert(sirkit::lexicographic_subsequence_indices(values, 2) == expected);
```

[実装](../Algorithm/Lexicographic_Subsequence.cpp)・[使用例の全体](../Examples/lexicographic_subsequence_example.cpp)・[Verify](../Verify/current/submissions/lexicographic_subsequence.cpp)
