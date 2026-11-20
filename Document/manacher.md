# 回文半径

奇数長・偶数長の回文半径を構築します。文字列や整数列を扱い、元の列は保持しません。列の長さをnとし、比較・添字アクセスをO(1)とします。

### PalindromeRadii

```text
PalindromeRadii();
template <class Sequence> explicit PalindromeRadii(const Sequence& s);
```

空／指定した列の半径を構築します。

**制約:** 列にsize・添字アクセス・一貫した同値関係の==があり、n<INT_MAX。

**計算量:** 空はO(1)、列からはO(n+1)時間・領域。

### assign

```text
template <class Sequence> void assign(const Sequence& s);
```

新しい列の半径へ置換します。確保・比較の例外でも既存結果を保ちます。

**制約:** 構築と同じ列の要件。

**計算量:** O(n+1)時間・領域。

### odd / even（公開データ）

```text
std::vector<int> odd, even;
```

odd[i]は中心を含む奇数回文の半径、even[i]はi-1とiの間を中心とする偶数回文の半分の長さです。

**制約:** 変更せず参照だけに使います。

**計算量:** 取得O(1)。

### size

```text
int size() const noexcept;
```

元の列長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

列が空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### is_palindrome

```text
bool is_palindrome(int l, int r) const;
```

半開区間[l,r)が回文ならtrueです。空区間も回文です。

**制約:** 0<=l<=r<=size()。

**計算量:** O(1)。

### longest

```text
std::pair<int, int> longest() const noexcept;
```

最長回文の半開区間を返します。同長なら最小の左端、空なら{0,0}です。

**制約:** 追加の制約はありません。

**計算量:** O(n+1)。

### count

```text
std::uint64_t count() const noexcept;
```

非空の連続回文区間の総数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(n+1)。

### PalindromicLengths

```text
template <class Sequence> std::vector<int> PalindromicLengths(const Sequence& s);
```

文字上／文字間の2n-1中心の最長回文長を交互に返します。空なら空配列です。

**制約:** PalindromeRadiiの構築と同じ列の要件。

**計算量:** O(n+1)時間・領域。

`SKIP_RUNTIME_CHECKS`でも添字と列の前提は同じです。返却値と半径配列は独立して所有します。

[使用例](../Examples/manacher_example.cpp)・[Library Checker](https://judge.yosupo.jp/problem/enumerate_palindromes)
