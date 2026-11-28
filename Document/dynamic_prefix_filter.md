# 禁止接頭辞付きの文字列集合

単語を登録し、禁止された接頭辞で始まる単語を除いた個数を調べます。たとえば `app` を禁止すると `apple` は集計から外れ、解除すると元に戻ります。

## `DynamicPrefixFilter`

```cpp
DynamicPrefixFilter();
```

空の単語多重集合と空の禁止接頭辞集合を作ります。文字列はバイト単位で、空文字列も使えます。

以下の文字列操作は引数の長さを $L$ とすると $O(1+256L)$ 時間です。ノード確保を伴う操作は償却時間です。領域は一度でも追加した接頭辞の種類数に比例します。

## `insert`

```cpp
std::size_t insert(std::string_view word, std::size_t copies = 1);
```

`word` を `copies` 個追加し、その単語の追加後の個数を返します。禁止中の単語も保持します。総個数が `SIZE_MAX` を超える追加は `std::length_error` です。

## `erase`

```cpp
bool erase(std::string_view word, std::size_t copies = 1) noexcept;
```

`word` を指定個数だけ削除します。個数が足りなければ状態を保って `false` を返します。

## `block`

```cpp
bool block(std::string_view prefix);
```

`prefix` を禁止し、設定が変わったかを返します。空の接頭辞を禁止すると全単語が集計から外れます。

## `unblock`

```cpp
bool unblock(std::string_view prefix) noexcept;
```

`prefix` の禁止を解除し、設定が変わったかを返します。禁止中のほかの接頭辞はそのまま有効です。

## `is_blocked`

```cpp
bool is_blocked(std::string_view word) const noexcept;
```

`word` が禁止接頭辞を一つでも持つかを調べます。未登録の単語にも使えます。

## `count`

```cpp
std::size_t count(std::string_view word) const noexcept;
```

`word` の登録個数を返します。

## `size`

```cpp
std::size_t size() const noexcept;
```

登録した単語の総個数を $O(1)$ 時間で返します。

## `allowed_count`

```cpp
std::size_t allowed_count() const noexcept;
```

禁止接頭辞を持たない登録単語の総個数を $O(1)$ 時間で返します。

## 使用例

```cpp
sirkit::DynamicPrefixFilter filter;
filter.insert("apple", 2);
filter.insert("banana");
filter.block("app");
assert(filter.allowed_count() == 1);
filter.unblock("app");
assert(filter.allowed_count() == 3);
```

[実装](../String/Dynamic_Prefix_Filter.cpp)・[使用例の全体](../Examples/dynamic_prefix_filter_example.cpp)・[Verify](../Verify/current/submissions/dynamic_prefix_filter.cpp)
