# 文字列の列一括置換と接頭辞の個数

同じ長さの単語を行として並べ、指定した列である文字を別の文字に一括置換します。現在の単語のうち、指定した接頭辞で始まる行の個数を調べられます。

## `PrefixBitmapTable`

```cpp
PrefixBitmapTable();
explicit PrefixBitmapTable(std::span<const std::string> rows);
explicit PrefixBitmapTable(std::span<const std::string_view> rows);
```

文字は連続したバイト値 `[First, First + Alphabet)` から選びます。既定の `Alphabet = 26, First = 'a'` は英小文字向けです。行数を $N$、行の長さを $W$、行のビット集合を保持する64ビット語の数を $B=\lceil N/64\rceil$ とします。構築は $O(1+N+NW+W\cdot\text{Alphabet}\cdot B)$ 時間、保持領域は $O(1+W\cdot\text{Alphabet}\cdot B)$ ワードです。

検査が有効なら、不揃いな行に対して `std::invalid_argument`、範囲外のバイトに対して `std::out_of_range` を送出します。引数なしでは行数・幅ともに0です。

## `size`

```cpp
std::size_t size() const noexcept;
```

行数を $O(1)$ 時間で返します。

## `width`

```cpp
std::size_t width() const noexcept;
```

行の長さを $O(1)$ 時間で返します。

## `replace`

```cpp
void replace(std::size_t column, unsigned char from, unsigned char to);
```

全行の `column` 列で `from` を `to` に置き換えます。列は0始まりで、`column < width()` が前提です。検査が有効な場合の範囲外の列・バイトは `std::out_of_range` です。$O(1+B)$ 時間で更新します。

## `prefix_count`

```cpp
std::size_t prefix_count(std::string_view prefix) const;
```

`prefix` から始まる行を数えます。重複した文字列も別々の行です。空の接頭辞なら全行が一致し、幅より長い接頭辞や対象外のバイトを含む接頭辞には0を返します。接頭辞の長さを $L$ として $O(1+LB)$ 時間・$O(B)$ 作業領域です。

## 使用例

```cpp
const std::array<std::string_view, 4> rows{"aaa", "aba", "bbb", "ccc"};
sirkit::PrefixBitmapTable<> table(rows);
assert(table.prefix_count("aa") == 1);
table.replace(1, 'b', 'a');
assert(table.prefix_count("aa") == 2);
```

[実装](../String/Prefix_Bitmap_Table.cpp)・[使用例の全体](../Examples/prefix_bitmap_table_example.cpp)・[Verify](../Verify/current/submissions/prefix_bitmap_table.cpp)
