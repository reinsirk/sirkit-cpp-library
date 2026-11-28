# 区間スケジューリング

互いに重ならない区間を選び、個数や重みの総和を最大化します。選んだ区間の添字も取得できます。

## 区間の指定

```text
ScheduleInterval{left, right, weight = 1}
```

区間は半開区間 `[left, right)` で、`left < right` が前提です。端点が接する二つの区間は同時に選べます。端点と重みは符号付き64ビット整数です。

## `interval_schedule_max_count`

```text
std::vector<std::size_t> interval_schedule_max_count(std::span<const ScheduleInterval> intervals)
```

個数が最大になるように選び、元の添字を時刻順に返します。各区間の重みは無視します。

## `interval_schedule_max_weight`

```text
WeightedIntervalSchedule interval_schedule_max_weight(std::span<const ScheduleInterval> intervals)
```

重みの総和を最大化します。戻り値の `weight` は総和で、`indices` は元の添字を時刻順に並べた配列です。0個も選べるので、最大の総和が0なら空の配列を返します。総和が `std::int64_t` の上限を超える場合は例外を送出します。

どちらも区間数を $n$ として $O(n\log(n+2))$ 時間・$O(n+1)$ 領域です。最適解が複数ある場合はそのうちの一つを返します。

## 使用例

```cpp
const std::array intervals{
    sirkit::ScheduleInterval{0, 2, 3},
    sirkit::ScheduleInterval{1, 4, 10},
    sirkit::ScheduleInterval{2, 4, 4},
};
const auto count = sirkit::interval_schedule_max_count(intervals);
assert((count == std::vector<std::size_t>{0, 2}));
const auto best = sirkit::interval_schedule_max_weight(intervals);
assert(best.weight == 10 && best.indices == std::vector<std::size_t>{1});
```

[実装](../Algorithm/Interval_Scheduling.cpp)・[使用例の全体](../Examples/interval_scheduling_example.cpp)
