# 更新できる括弧列

`(` と `)` だけからなる列で、置換や交換の後に範囲内の括弧の対応を調べます。

## `BracketSequence`

```text
explicit BracketSequence(std::string_view text)
```

`text` の内容から構築します。長さ $n$ は `INT_MAX` 以下で、構築時間・領域は $O(n+1)$ です。

## `size`

```text
int size() const noexcept
```

長さを $O(1)$ 時間で返します。

## `get`

```text
char get(int index) const
```

0始まりの `index` にある括弧を $O(1)$ 時間で取得します。

## `set`

```text
void set(int index, char symbol)
```

指定位置を `(` または `)` に置き換えます。$O(\log(n+1))$ 時間です。

## `swap`

```text
void swap(int first, int second)
```

二つの位置の括弧を $O(\log(n+1))$ 時間で交換します。

## `query`

```text
BracketSummary query(int left, int right) const
```

半開区間 `[left, right)` の括弧を照合し、対応した組数 `matched_pairs` と、余った左右の括弧の個数 `unmatched_open`・`unmatched_close` を返します。空区間の各個数は0です。$O(\log(n+1))$ 時間かかります。

## `matched_length`

対応した括弧の総文字数を `BracketSummary` から $O(1)$ 時間で取得します。

## `balanced`

余った括弧が一つもなければ `true` を返す `BracketSummary` の関数です。$O(1)$ 時間かかります。

## `minimum_insertions`

正しい括弧列にするための最小挿入数を `BracketSummary` から $O(1)$ 時間で取得します。

## `first_unmatched_close`

```text
int first_unmatched_close(int left) const
```

`[left, size())` を左から読み、余る最初の `)` の添字を探します。見つからなければ `size()` を返します。$O(\log(n+1))$ 時間です。

## `last_unmatched_open`

```text
int last_unmatched_open(int right) const
```

`[0, right)` を右から読み、余る最後の `(` の添字を探します。なければ `-1` です。$O(\log(n+1))$ 時間かかります。

## 使用例

```cpp
sirkit::BracketSequence brackets("())(");
assert(brackets.query(0, 2).balanced());
assert(brackets.query(0, 4).matched_length() == 2);
assert(brackets.first_unmatched_close(0) == 2);
brackets.swap(2, 3);
assert(brackets.query(0, 4).balanced());
```

[実装](../String/Bracket_Sequence.cpp)・[使用例の全体](../Examples/bracket_sequence_example.cpp)
