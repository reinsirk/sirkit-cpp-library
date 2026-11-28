# 木上の Mo 法

固定された木のパスをまとめて調べます。パスに頂点や辺を1個加える操作と、1個取り除く操作が書ける場合に使えます。[本体](../Graph/Tree_Mo.cpp)

## `TreeMo`

```text
TreeMo(int n, std::span<const std::pair<int, int>> edges, int root = 0);
std::size_t size() const noexcept;
```

$0$ 始まりの無向木を渡します。各無向辺は1回だけ渡し、辺番号は入力順です。$B=\lceil n/32\rceil$ として構築は $O(n+B\log(B+1))$ 時間・領域です。`size` は頂点数を $O(1)$ で返します。

## `run`・`run_edges`

```text
run(queries, add, erase, answer, block_size = 0);
run_edges(queries, add, erase, answer, block_size = 0);
```

`queries` は頂点対の列です。`run` の `add` と `erase` は頂点番号を受け取り、`run_edges` では辺番号を受け取ります。集計がパスと一致した時点で `answer` を呼び、元の照会番号を渡します。パス上の順序は保持しません。外部の集計を空にして開始し、正常終了時には空に戻ります。

`block_size=0` で幅を自動選択します。幅 $b$、照会数 $q$ に対し、追加・削除は $O(qb+n(n/b+1)+q)$ 回、整列は $O(q\log(q+1))$ 時間です。作業領域は $O(n+q)$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
const std::array queries{std::pair{0, 2}, std::pair{2, 2}};
sirkit::TreeMo tree(3, edges);
int active = 0;
std::array<int, 2> answer{};
tree.run(queries, [&](int) { ++active; }, [&](int) { --active; },
         [&](std::size_t i) { answer[i] = active; }); // {3, 1}
```

[使用例](../Examples/tree_mo_example.cpp)・[AtCoder Verify](../Verify/current/submissions/tree_mo.cpp)
