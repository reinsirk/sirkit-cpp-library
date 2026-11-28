# 木上の動的 Steiner 部分木

マーク済み頂点をすべて含む最小の連結部分木の頂点数を求めます。マークは更新でき、根を指定して探索範囲を部分木に絞れます。[本体](../Graph/Dynamic_Steiner_Subtree.cpp)

## `DynamicSteinerSubtree`

```text
DynamicSteinerSubtree(int n, std::span<const std::pair<int, int>> edges, int root = 0);
```

頂点番号が $0$ 始まりの単純無向木を渡します。各辺は1回だけ指定します。構築直後はすべて未マーク。頂点数 $n$、$B=\lceil n/32\rceil$ に対し、構築は $O(n+B\log(B+1))$ 時間・領域です。

## マークの更新・参照

```text
void set_marked(int v, bool marked = true);
bool toggle(int v);
bool marked(int v) const;
std::size_t size() const noexcept;
```

`set_marked` はマークを設定または解除します。`toggle` は反転後の状態を返します。いずれも $O(\log(n+1))$ 時間です。`marked` はマークの有無、`size` は木全体の頂点数を $O(1)$ で返します。

## `steiner_vertices`

```text
int steiner_vertices() const;
int steiner_vertices(int v, int root) const;
```

引数なしでは全マークを対象とし、$O(1)$ 時間で答えます。引数ありでは `root` を根とみなし、`v` の部分木内のマークだけを対象にします。この照会は $O(\log(n+1))$ 時間です。どちらも対象のマークがなければ $0$、1個なら $1$ を返します。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
sirkit::DynamicSteinerSubtree tree(3, edges);
tree.set_marked(0);
tree.set_marked(2);
const int whole = tree.steiner_vertices();       // 3
const int left = tree.steiner_vertices(1, 2);    // 1
```

[使用例](../Examples/dynamic_steiner_subtree_example.cpp)・[yukicoder Verify](../Verify/current/submissions/dynamic_steiner_subtree.cpp)
