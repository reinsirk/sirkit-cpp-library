# 並列二分探索

イベントを先頭から順に適用したとき、各照会の条件が初めて成立する時点を求めます。例えば辺を順に加えるグラフで、二つの頂点がいつ連結になるかを繰り返し照会できます。

## `parallel_binary_search`

```text
template<class Reset, class Apply, class Predicate>
std::vector<std::size_t> parallel_binary_search(std::size_t event_count, std::size_t query_count,
                                                Reset &&reset, Apply &&apply, Predicate &&predicate)
```

照会ごとの答えを元の照会番号順に返します。答えは処理済みのイベント数です。初期状態で条件を満たせば0、最後まで満たさなければ `event_count + 1` です。

`reset()` で状態を初期化し、`apply(i)` で0始まりの `i` 番目のイベントを適用します。イベントは初期化のたびに先頭から順に適用します。`predicate(q)` は現在の状態で0始まりの照会 `q` の条件を調べます。一度成立した条件は以後も成立することが前提です。照会の呼び出し順は決まっていません。

イベント数を $M$、照会数を $Q$ とします。呼び出し先の実行時間を除き $O((M+Q+1)\log(M+2))$ 時間・$O(M+Q+1)$ 領域です。各関数の呼び出しは `reset` が $O(\log(M+2))$ 回、`apply` が $O(M\log(M+2))$ 回、`predicate` が $O(Q\log(M+2))$ 回です。照会が0件なら空の配列を返します。

## 使用例

```cpp
const std::array events{2, 4, 1};
const std::array targets{0, 4, 8};
int sum = 0;
const auto steps = sirkit::parallel_binary_search(
    events.size(), targets.size(),
    [&] { sum = 0; },
    [&](std::size_t i) { sum += events[i]; },
    [&](std::size_t q) { return sum >= targets[q]; });
// steps = {0, 2, 4}。8には到達しないので3+1。
```

[実装](../Algorithm/Parallel_Binary_Search.cpp)・[使用例の全体](../Examples/parallel_binary_search_example.cpp)
