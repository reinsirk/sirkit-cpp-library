# 1文字ずつ更新できる部分文字列検索

文字列の長さを固定したまま、1バイトの置換とパターン検索を繰り返せます。同じパターンが重なって現れる場合も、開始位置ごとに区別します。

## `DynamicSubstringMatching`

```text
explicit DynamicSubstringMatching(std::string_view text)
```

入力のコピーと各バイトの出現位置を保持します。入力長を $n$ として、構築時間と使用領域は $O(n+1)$ です。コピーを更新しても元には影響せず、移動元は空になります。

## `size`

```text
std::size_t size() const noexcept
```

元のバイト列の長さを $O(1)$ 時間で返します。

## `set`

```text
void set(std::size_t position, char replacement)
```

0始まりの `position` にあるバイトを $O(1)$ 時間で置き換えます。`position < size()` が前提です。

## `contains`

```text
bool contains(std::string_view pattern) const
```

`pattern` と完全一致する連続部分文字列があれば `true` です。

## `count`

```text
std::size_t count(std::string_view pattern) const
```

`pattern` が現れる回数を返します。

## `occurrences`

```text
std::vector<std::size_t> occurrences(std::string_view pattern) const
```

一致した0始まりの開始位置を昇順で返します。空パターンは末尾を含む $n+1$ 箇所に一致します。元の列より長いパターンに一致する位置はありません。

検索は開始位置の候補を64個ずつ絞り込みます。パターン長 $m$ が $1\le m\le n$ なら、$O(m(1+\lceil(n-m+1)/64\rceil))$ 時間・$O(m)$ 追加領域です。空パターンの存在判定と個数は $O(1)$ 時間で求まり、位置を返す場合は結果の個数に比例する時間と領域が加わります。

## 使用例

```cpp
sirkit::DynamicSubstringMatching text("ababa");
assert(text.count("aba") == 2);
text.set(2, 'x');
assert(!text.contains("aba"));
```

[実装](../String/Dynamic_Substring_Matching.cpp)・[使用例の全体](../Examples/dynamic_substring_matching_example.cpp)
