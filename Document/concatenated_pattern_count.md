# 連結で作る文字列のパターン出現回数

固定したパターンが何回現れるかを、文字列を展開せずに求めます。例えば `ab` と `aba` の要約を連結すると、境界をまたぐ出現も含めて `ababa` の `aba` を2回と数えます。

## `ConcatenatedPatternCounter`

```text
explicit ConcatenatedPatternCounter(std::string_view pattern, std::uint32_t modulus = 998'244'353)
```

空でないパターンと正の法を渡します。文字はバイトとして扱います。長さを $m$、パターンに含まれる異なるバイトの数を $a$ として $O(ma)$ 時間・領域です。個体のコピーは前処理を共有します。

## `summarize`

```text
Summary summarize(std::string_view text) const
```

文字列を連結に使える要約に変換します。空文字列の要約を連結しても結果は変わりません。文字列の長さを $n$ として $O(m(n+1))$ 時間、要約の領域は $O(m)$ です。

## `concatenate`

```text
Summary concatenate(const Summary &left, const Summary &right) const
```

左、右の順に連結した文字列の要約を返します。各要約は同じ個体か、そのコピーで作ったものを使います。$O(m)$ 時間、結果の領域は $O(m)$ です。

## `count`

```text
std::uint32_t count(const Summary &summary) const
```

重なる出現も数え、回数を指定した法で割った余りを返します。要約の制約は `concatenate` と共通です。$O(1)$ 時間・追加領域です。

## 使用例

```cpp
const sirkit::ConcatenatedPatternCounter counter("aba");
const auto left = counter.summarize("ab");
const auto right = counter.summarize("aba");
const auto joined = counter.concatenate(left, right);
assert(counter.count(joined) == 2);
```

[実装](../String/Concatenated_Pattern_Count.cpp)・[使用例の全体](../Examples/concatenated_pattern_count_example.cpp)
