# 文字の置換と区間回文判定

固定長の文字列で1文字ずつ置き換え、指定した区間が回文か調べます。二つの素数法のローリングハッシュを使うため、衝突時は回文でない区間を回文と判定する可能性があります。

## `DynamicPalindrome`

```text
explicit DynamicPalindrome(std::string_view text)
DynamicPalindrome(std::string_view text, std::uint64_t seed)
```

文字列をバイト列としてコピーし、乱数でハッシュの基数を選びます。同じ実行環境で基数を再現したい場合は `seed` を指定できます。長さを $n$ として $O(n+1)$ 時間・領域です。

## `set`

```text
void set(std::size_t index, std::uint8_t byte)
```

0始まりの位置 `index` のバイトを変更します。添字は `size()` 未満です。$O(1+\log(n+1))$ 時間です。

## `get`

```text
std::uint8_t get(std::size_t index) const
```

指定位置のバイト値を $O(1)$ 時間で返します。添字の制約は `set` と共通です。

## `is_palindrome`

```text
bool is_palindrome(std::size_t left, std::size_t right) const
```

半開区間 `[left,right)` が回文なら `true` を返します。`left <= right <= size()` が前提です。空区間も回文と判定します。$O(1+\log(n+1))$ 時間です。

## `size`

```text
std::size_t size() const noexcept
```

文字列のバイト数を $O(1)$ 時間で返します。

## 使用例

```cpp
sirkit::DynamicPalindrome text("abcba");
assert(text.is_palindrome(0, 5));
text.set(0, 'x');
assert(!text.is_palindrome(0, 5));
assert(text.is_palindrome(1, 4));
```

[実装](../String/Dynamic_Palindrome.cpp)・[使用例の全体](../Examples/dynamic_palindrome_example.cpp)
