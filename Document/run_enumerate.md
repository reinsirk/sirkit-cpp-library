# Run Enumerate

文字列に含まれる、同じ周期で左右どちらにも延長できない繰り返し区間を列挙します。

```cpp
auto runs = sirkit::run_enumerate("abababa");
// {{2, 0, 7}}
```

## `run_enumerate`

```text
std::vector<StringRun> run_enumerate(std::string_view text);
```

引数は `std::string_view` です。結果は `std::vector<StringRun>` で、各要素は `{period, left, right}` の順に最小周期と0始まりの半開区間を持ちます。区間の長さは最小周期の2倍以上です。例では `ab` が3回続く範囲を、同じ周期が保たれる最後の `a` まで含めています。

同じ区間は一度だけ現れ、結果は `(period, left, right)` の昇順です。文字はバイト単位で扱い、空入力では空の配列を返します。

長さ $N$ の文字列に対して $O(N\log(N+1))$ 時間・領域です。Lyndon語から周期の候補を見つけ、等しい部分文字列の長さを求めて両側へ延長します。

[使用例](../Examples/run_enumerate_example.cpp) ／ [Verify](../Verify/current/submissions/run_enumerate.cpp) ／ [The “Runs” Theorem](https://arxiv.org/abs/1406.0263)
