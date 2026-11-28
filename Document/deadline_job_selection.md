# 締切付きの仕事を選んで報酬を最大化

仕事ごとに所要時間・締切・報酬があります。時刻0から仕事を一つずつ進め、締切までに完了する仕事から得られる報酬を最大にします。

## `DeadlineJob`

`{duration, deadline, reward}` で所要時間、完了できる最後の時刻、報酬を指定します。所要時間0や負の報酬も指定できます。

## `DeadlineJobSchedule`

`reward` は最大報酬です。`jobs` には選んだ仕事の0始まりの添字が実行順に入ります。

## `maximize_deadline_job_reward(jobs)`

仕事を締切順に並べ、各終了時刻までに得られる最大報酬を動的計画法で求めます。全件を見送る場合の報酬は0です。最大報酬は `std::int64_t` に収めてください。

$n$ を件数、$D$ を最大締切と所要時間の総和の小さい方とします。$O(1+n\log(n+1)+n(D+1))$ 時間、$O(n+D+1)$ 語と復元用の $O(n(D+1))$ ビットを使います。

## 使用例

```cpp
const std::array<sirkit::DeadlineJob, 3> jobs{{
    {3, 7, 20},
    {2, 3, 10},
    {3, 5, 15},
}};
const auto result = sirkit::maximize_deadline_job_reward(jobs);
assert(result.reward == 35);
assert((result.jobs == std::vector<std::size_t>{2, 0}));
```

[実装](../Algorithm/Deadline_Job_Selection.cpp)・[使用例の全体](../Examples/deadline_job_selection_example.cpp)・[Verify](../Verify/current/submissions/deadline_job_selection.cpp)
