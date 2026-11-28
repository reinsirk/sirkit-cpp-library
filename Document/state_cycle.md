# 状態列の周期検出と集約

初期状態 `start` から `next` を繰り返し適用します。巨大な回数の状態を集約したいとき、周期を見つけて繰り返し分をまとめられます。

## `find_state_cycle`

```text
std::optional<StateCycle> find_state_cycle(start, next, transition_limit, equal = {})
```

`prefix` は周期が始まる0始まりの位置、`period` は最短の周期です。位置と周期が `next` の呼び出し上限 `transition_limit` までに確定しなければ `std::nullopt` を返します。周期がある列では $O(\min(\text{transition_limit},\text{prefix}+\text{period})+1)$ 時間です。

## `fold_state_sequence`

```text
fold_state_sequence(start, count, next, project, combine, identity, equal = {})
```

初期状態を含む先頭 `count` 個を `project` で変換し、`combine` で順番に集約します。`combine` は結合則を満たし、`identity` がその単位元です。順序を保つため、文字列連結のような非可換な演算にも使えます。周期がある列では状態を $O(\min(\text{count},\text{prefix}+\text{period})+1)$ 回調べ、周期の集約結果を $O(\log(\text{count}+1))$ 回で合成します。

各関数が保持する状態と集約値は定数個です。`next` と `project` は同じ状態に対して同じ結果を返すように定義します。状態はコピーでき、等価判定は既定で `==` です。`equal` を渡す場合、等価な状態は遷移先も等価であり、変換結果が一致する必要があります。

## 使用例

```cpp
const auto next = [](int state) { return state == 3 ? 1 : state + 1; };
const auto cycle = sirkit::find_state_cycle(0, next, 30); // prefix=1, period=3
const int sum = sirkit::fold_state_sequence(0, 8, next, std::identity{}, std::plus<>{}, 0); // 13
```

[Brent法の原典](https://maths-people.anu.edu.au/brent/pub/pub051.html)・[実装](../Algorithm/State_Cycle.cpp)・[使用例の全体](../Examples/state_cycle_example.cpp)
