# 周期文字列への部分列埋め込み

元の文字列を何回も連結した列に、目標の各文字を同じ回数ずつ増やした部分列があるか調べます。繰り返した文字列は作りません。

## `RepeatedSubsequenceIndex`

```cpp
explicit RepeatedSubsequenceIndex(std::string_view cycle);
```

長さ $n$ の `cycle` をバイト単位で前処理します。$O(n+256)$ 時間・領域を使います。コピーした索引は独立し、移動元は空です。

## `contains_expanded`

```cpp
bool contains_expanded(std::uint64_t copies, std::string_view target,
                       std::uint64_t repetitions) const;
```

`cycle` を `copies` 回連結した列の中に、`target` の各バイトをその場で `repetitions` 個ずつに増やした部分列があれば `true` を返します。例えば `target = "ab"` と `repetitions = 2` なら、探す部分列は `"aabb"` です。目標が空、または `repetitions` が0なら常に成立します。目標の長さを $m$ として $O(1+m\log(n+2))$ 時間です。

## `max_repetition`

成立する最大の `repetitions` を `limit` 以下から求めます。`limit` の既定値は `UINT64_MAX` です。目標が空なら `limit` を返します。$O(1+m\log(n+2)\log(\mathrm{limit}+2))$ 時間です。

## 使用例

```cpp
const sirkit::RepeatedSubsequenceIndex index("abc");
assert(index.contains_expanded(3, "ab", 2)); // abcabcabc に aabb がある
assert(index.max_repetition(3, "ab") == 2);
```

[実装](../String/Repeated_Subsequence.cpp)・[使用例の全体](../Examples/repeated_subsequence_example.cpp)・[Verify](../Verify/current/submissions/repeated_subsequence.cpp)
