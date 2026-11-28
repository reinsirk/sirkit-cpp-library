# 周期文字列の中でパターンを繰り返す

`period` を無限に連ねた列の中に、同じ `pattern` を隙間なく何回続けて置けるか求めます。

## `PeriodicPatternRun`

`start` は周期内の開始位置で、0始まりです。`repetitions` は最大回数を表し、上限がなければ `std::nullopt` になります。同じ回数なら開始位置の最小を返します。

## `longest_periodic_pattern_run(period, pattern)`

1回も置けなければ `{0, 0}`、`pattern` が空なら `{0, std::nullopt}` を返します。比較はバイト単位です。

KMP で一致する開始位置をそろえ、パターン長ずつ進む周期上の経路を調べます。各文字列の長さを $n$, $m$ とすると、$O(1+n+m)$ 時間・領域です。

## 使用例

```cpp
const auto finite = sirkit::longest_periodic_pattern_run("abcabab", "ab");
assert(finite.start == 3 && finite.repetitions == 3);
const auto unbounded = sirkit::longest_periodic_pattern_run("abab", "ab");
assert(unbounded.start == 0 && !unbounded.repetitions);
```

[実装](../String/Periodic_Pattern_Runs.cpp)・[使用例の全体](../Examples/periodic_pattern_runs_example.cpp)・[Verify](../Verify/current/submissions/periodic_pattern_runs.cpp)
