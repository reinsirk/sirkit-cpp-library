# 両端更新できる回文木

文字列の先頭・末尾に1文字を加えたり取り除いたりしながら、相異なる回文の個数と最長の回文接頭辞・接尾辞の長さを調べます。

```cpp
sirkit::PalindromesInDeque palindromes;
palindromes.push_back('a');
palindromes.push_front('b');
palindromes.push_back('b');  // "bab"
auto count = palindromes.distinct_palindrome_count();  // 3
palindromes.pop_front();  // "ab"
```

## `PalindromesInDeque`

```text
explicit PalindromesInDeque(std::size_t expected_pushes = 0);
```

空の文字列から始めます。`expected_pushes` に追加回数の見込みを渡すと回文頂点を予約します。予約を使わないとき、追加の総回数 $U$ に対して領域は $O(26U+1)$ です。

追加に渡せる文字は英小文字です。通常版で対象外の文字を追加すると `std::invalid_argument`、空の文字列から削除すると `std::out_of_range` を送出します。

## `push_front`

```text
void push_front(char character);
```

先頭に1文字加えます。償却 $O(1)$ です。

## `push_back`

```text
void push_back(char character);
```

末尾に1文字加えます。償却 $O(1)$ です。

## `pop_front`

```text
void pop_front();
```

先頭の1文字を $O(1)$ で取り除きます。

## `pop_back`

```text
void pop_back();
```

末尾の1文字を $O(1)$ で取り除きます。

## `distinct_palindrome_count`

```text
std::size_t distinct_palindrome_count() const noexcept;
```

空でない相異なる回文部分文字列の個数を $O(1)$ で返します。

## `longest_prefix_length`

```text
std::size_t longest_prefix_length() const noexcept;
```

回文である最長の接頭辞の長さを $O(1)$ で返します。空文字列では0です。

## `longest_suffix_length`

```text
std::size_t longest_suffix_length() const noexcept;
```

回文である最長の接尾辞の長さを $O(1)$ で返します。空文字列では0です。

## `size`

```text
std::size_t size() const noexcept;
```

現在の文字数を $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept;
```

現在の文字列が空なら `true` を返します。$O(1)$ です。

[使用例](../Examples/palindromes_in_deque_example.cpp) ／ [Verify](../Verify/current/submissions/palindromes_in_deque.cpp) ／ [原論文](https://arxiv.org/abs/2210.02292)
