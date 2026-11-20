# 最長増加部分列

数列の順序を保って選んだ、狭義または広義の最長増加部分列を求めます。`Strict=true`は狭義、`false`は広義です。`LIS`以外の`Strict`は省略時`true`です。

Nを入力長、Lを最長部分列長とし、要素の比較・コピーをO(1)とします。

## 長さ

### LIS

```text
template<bool Strict, class T>
std::vector<std::size_t> LIS(const std::vector<T>& v);
```

各接頭辞の最長部分列長を返します。

**制約:** Strictを指定し、Tは共通の比較・コピー要件を満たすこと。

**計算量:** O(N log(L+1))時間・O(N+L)追加領域。

### LISLength

```text
template<bool Strict = true, class T>
std::size_t LISLength(const std::vector<T>& v);
```

列全体の最長部分列長を返します。空列は0です。

**制約:** Tは共通の比較・コピー要件を満たすこと。

**計算量:** O(N log(L+1))時間・O(L)追加領域。

### LISEndingLengths

```text
template<bool Strict = true, class T>
std::vector<std::size_t> LISEndingLengths(const std::vector<T>& v);
```

各要素を必ず末尾に使う最長部分列長を返します。

**制約:** Tは共通の比較・コピー要件を満たすこと。

**計算量:** O(N log(L+1))時間・O(N+L)追加領域。


接頭辞長はその位置までの最大長、末尾長はその要素を必ず使う列の長さです。空入力では長さ0、各vectorは空です。

## 復元

### LISIndices

```text
template<bool Strict = true, class T>
std::vector<std::size_t> LISIndices(const std::vector<T>& v);
```

最長部分列の各要素が元の数列で持つ添字を昇順で返します。同じ長さの解が複数ある場合、どれを選ぶかは規定しません。

**制約:** Tに一貫した狭義弱順序が必要です。汎用型のコピー・既定構築は不要です。

**計算量:** O(N log(L+1))時間・O(N)追加領域。

### LISSequence

```text
template<bool Strict = true, class T>
std::vector<T> LISSequence(const std::vector<T>& v);
```

解の値をコピーした列を返します。空入力は空列です。

**制約:** Tに一貫した狭義弱順序とコピー構築が必要です。

**計算量:** O(N log(L+1))時間・O(N)追加領域。


同じ長さの解の選択は規定しません。入力は変更せず、戻り値は独立して所有します。

```cpp
const std::vector<int> a{3,1,2,0};
auto length = LISLength(a);        // 2
auto prefix = LIS<true>(a);        // {1,1,2,2}
auto ending = LISEndingLengths(a); // {1,1,2,1}
auto indices = LISIndices(a);      // {1,2}
```

## 型と前提

`v`は`std::vector<T>`です。`operator<`に一貫した狭義弱順序が必要で、`==`・`<=`は不要です。長さ系はTのコピー構築・代入、`LISSequence`はコピー構築を必要とします。`LISIndices`は汎用型をコピーせず、非default構築・コピー不能な型も比較だけで扱います。

標準浮動小数点のNaNは既定で`invalid_argument`です。`SKIP_RUNTIME_CHECKS`ではNaNを含めないことが利用側の前提です。正負の無限大は使えます。比較・コピー・確保の例外でも入力は保持します。

[完全な使用例](../Examples/lis_example.cpp) · [実装・検証の記録](../Audit/document-detail-archive-algorithm-string-2026-11-15.json)
