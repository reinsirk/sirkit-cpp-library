# 文字列の基数ソート

文字列を辞書順に並べたときの元の添字を求めます。入力を保持したまま、付随する値もその添字で取り出せます。

## `string_radix_order`

```cpp
std::vector<std::size_t> string_radix_order(std::span<const std::string> words,
                                            std::string_view alphabet = {});
std::vector<std::size_t> string_radix_order(std::span<const std::string_view> words,
                                            std::string_view alphabet = {});
```

辞書順に並べたときの0始まりの添字を返します。同じ文字列は入力の順番を保ちます。片方がもう一方の接頭辞なら、短い方が先です。

`alphabet` は先に並べたいバイトの列です。指定した順を優先し、残りは符号なし値の小さい順に並べます。空なら全バイトを符号なし値で比較します。検査が有効なら、`alphabet` 内の重複に対して `std::invalid_argument` を送出します。

空文字列や値0のバイトも扱います。文字列数を $n$、全体のバイト数を $L$ とすると、8ビットのバイトを使う固定の基数ソートで $O(n+L+1)$ 時間、$O(n+1)$ 追加領域です。

## 使用例

```cpp
const std::array<std::string_view, 5> words{"ab", "a", "ab", "ba", "b"};
const auto order = sirkit::string_radix_order(words, "ba");
assert((order == std::vector<std::size_t>{4, 3, 1, 0, 2}));
```

[実装](../String/String_Radix_Order.cpp)・[使用例の全体](../Examples/string_radix_order_example.cpp)・[安定ソートのVerify](../Verify/current/submissions/string_radix_order_bookshelf.cpp)・[独自順のVerify](../Verify/current/submissions/string_radix_order_alphabet.cpp)
