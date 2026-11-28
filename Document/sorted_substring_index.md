# 一点置換付きの昇順部分文字列判定

文字列を一点ずつ置換しながら、指定区間が昇順か、文字列全体を並べ替えた結果の連続部分になるかを調べます。

## `SortedSubstringIndex<Alphabet, First>`

```cpp
SortedSubstringIndex();
explicit SortedSubstringIndex(std::string_view text);
```

文字列をコピーします。バイトの範囲は `[First, First + Alphabet)` で、既定は英小文字です。$A$ を文字種数、$n$ を長さとして $O(n+A)$ 時間・領域で構築します。$n$ は `INT_MAX` 以下です。

## `size`

文字列の長さを $O(1)$ 時間で返します。

## `empty`

空かどうかを $O(1)$ 時間で返します。

## `get`

位置を指定して1バイトを $O(1)$ 時間で取得します。

## `set`

```cpp
void set(int position, char byte);
```

位置 `position` の値を `byte` に $O(\log(n+1))$ 時間で置換します。

## `is_nondecreasing`

半開区間 `[left, right)` が符号なしバイト順で昇順なら `true` を返します。等しいバイトも続けて並べられます。$O(\log(n+1))$ 時間です。

## `is_sorted_slice`

元の半開区間 `[left, right)` が文字列全体を昇順に並べ替えた結果に連続して現れるかを判定します。区間が昇順で、両端の間の値を持つバイトがすべて区間内にあれば成立します。$O(A+\log(n+1))$ 時間です。

添字は0始まりで、どちらの判定も空区間なら `true` です。検査が有効なら、位置・区間・バイトが範囲外の場合に `std::out_of_range` を送出します。

## 使用例

```cpp
sirkit::SortedSubstringIndex<> index("zacef");
assert(index.is_sorted_slice(1, 5));
index.set(0, 'b');
assert(index.is_nondecreasing(1, 5)); // acef は昇順
assert(!index.is_sorted_slice(1, 5)); // 整列結果の abcef では途中に b が入る
```

[実装](../String/Sorted_Substring_Index.cpp)・[使用例の全体](../Examples/sorted_substring_index_example.cpp)・[Verify](../Verify/current/submissions/sorted_substring_index.cpp)
