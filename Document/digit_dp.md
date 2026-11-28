# 桁DP

桁を左から読み、決めた規則で状態を変えます。上限以下の数を、読み終えたときの状態ごとに数えます。

## `digit_dp_counts<Count>(upper, state_count, initial, transition, radix)`

`upper` は2〜36進数の文字列です。`radix` の既定は10です。`0` から `upper` までを含めて数えます。状態番号は `[0,state_count)` で、`initial` から始めます。

`transition(state,digit,leading_zero)` は次の状態番号を返します。その桁を選ばせない場合は `std::nullopt` を返せます。`leading_zero` は先頭に並ぶ0だけ真です。数0では全桁が先頭の0です。

`Count{}` は0、`Count{1}` は1で、`+=` を使える型を指定します。加算と遷移が $O(1)$ なら、桁数 $L$、状態数 $S$、基数 $B$ として $O((L+1)SB)$ 時間、$O(SB)$ 領域です。

```cpp
const auto count = sirkit::digit_dp_counts<std::uint64_t>("25", 2, 0,
    [](std::size_t seen, unsigned digit, bool leading_zero) {
        return seen || (digit == 0 && !leading_zero);
    });
// 先頭の0を除き、0を含むのは10と20。count[1] == 2。
```

[使用例](../Examples/digit_dp_example.cpp) ／ [Verify](../Verify/current/submissions/digit_dp_dp_s.cpp)
