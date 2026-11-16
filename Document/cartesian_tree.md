# Cartesian Tree

`CartesianTree`は、数列の最小元を根にして左右の区間を部分木に分ける二分木です。頂点番号は入力の添字で、「左の子→自分→右の子」とたどると元の順序を復元できます。同値なら左の添字を祖先にします。[本体](../Graph/Cartesian_Tree.cpp)・[使用例](../Examples/cartesian_tree_example.cpp)を参照してください。

## API

### CartesianTree

```text
CartesianTree();
template<class T, std::size_t Extent, class Compare = std::less<>> explicit CartesianTree(std::span<T, Extent> values, Compare&& compare = Compare{});
template<class T, class Allocator, class Compare = std::less<>> requires (!std::is_same_v<T, bool>) explicit CartesianTree(const std::vector<T, Allocator>& values, Compare&& compare = Compare{});
CartesianTree(const CartesianTree&);
CartesianTree(CartesianTree&& other) noexcept;
```

空、またはspan・vectorの値列から木を構築します。比較器は省略時std::less<>です。

**制約:** 入力長はINT32_MAX以下で、比較器は一貫した狭義弱順序です。

**計算量:** 空の構築・ムーブはO(1)、コピーはO(N)。入力からはO(N)時間・領域、比較O(N)回。

### build

```text
template<class T, std::size_t Extent, class Compare = std::less<>> void build(std::span<T, Extent> values, Compare&& compare = Compare{});
template<class T, class Allocator, class Compare = std::less<>> requires (!std::is_same_v<T, bool>) void build(const std::vector<T, Allocator>& values, Compare&& compare = Compare{});
```

spanまたはvectorから構築／再構築する。

**制約:** 入力長はINT32_MAX以下で、比較器は一貫した狭義弱順序です。

**計算量:** O(旧N+新N)時間、O(新N)追加領域。比較はO(新N)回。

### root

```text
std::int32_t root() const noexcept;
```

根の頂点番号を返します。空なら-1です。

**制約:** なし。

**計算量:** O(1)。

### parent

```text
std::int32_t parent(std::int32_t vertex) const;
```

親の頂点番号を返します。根の親は-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### left_child

```text
std::int32_t left_child(std::int32_t vertex) const;
```

左の子の添字を返します。存在しなければ-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### right_child

```text
std::int32_t right_child(std::int32_t vertex) const;
```

右の子の添字を返します。存在しなければ-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### children

```text
std::pair<std::int32_t, std::int32_t> children(std::int32_t vertex) const;
```

左の子と右の子の添字をpairで返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### parents

```text
std::span<const std::int32_t> parents() const noexcept;
```

親番号の配列を読み取り専用で返します。

**制約:** なし。

**計算量:** O(1)。

### left_children

```text
std::span<const std::int32_t> left_children() const noexcept;
```

左の子の配列への読み取り専用spanを返します。

**制約:** なし。

**計算量:** O(1)。

### right_children

```text
std::span<const std::int32_t> right_children() const noexcept;
```

右の子の配列への読み取り専用spanを返します。

**制約:** なし。

**計算量:** O(1)。

### size

```text
std::size_t size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

頂点数が0ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** O(N)。


```cpp
std::vector<std::int64_t> a{4,2,2,5,1};
CartesianTree tree(a); // tree.root() == 4。
auto children = tree.children(4); // 左の子は1、右の子は-1。
CartesianTree maximum(a,std::greater<>{}); // 最大元を優先する木。
```

## 比較と利用前提

既定はstd::less<>です。compare(a,b)がtrueならaを優先し、両方向ともfalseなら同値として左の添字を優先します。比較器はstrict weak ordering（標準の大小比較と同様の整合した順序）を満たし、呼び出し中に入力や本体を変更しないことが前提です。比較器は構築中だけ使い、保存しません。

配列長はINT32_MAX以下。std::arrayや生配列にはstd::span(values)を使います。vector<bool>は対象外ですが、連続したbool配列のspanは使えます。入力を変更・保持せず、値のコピーも不要です。

単調スタックで構築し、比較O(N)回・O(N)時間・所有O(N)・追加作業O(N)です。比較器自体の費用は別に数えます。再帰を使わず、鎖になる入力も扱います。取得はO(1)、コピーはO(N)、move・swapはO(1)。再構築は旧木の破棄も含めO(旧N+新N)です。

コピーは独立。build・コピー代入の確保や比較に失敗しても旧木とviewを維持しますが、比較器の外部状態は戻しません。自身の配列spanもbuildの入力に使えます。成功したbuild・clear・代入・所有権移動後は古いspanを使わないでください。move元は空でroot=-1です。

頂点は0<=v<size()です。頂点番号の入力検査は`SKIP_RUNTIME_CHECKS`で除き、容量・添字の保護は維持します。比較器の順序を検査するための追加走査はしません。区間[l,r]の両端のLCAは、その区間の左優先の最小元に対応します。Library Checkerの出力では、根の親を根自身に変換します。

## 構築・状態と補助型

### operator=

```text
CartesianTree& operator=(const CartesianTree& other);
CartesianTree& operator=(CartesianTree&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。

### swap

```text
void swap(CartesianTree& other) noexcept;
void swap(CartesianTree& a, CartesianTree& b) noexcept;
```

保持する状態を交換します。

**制約:** なし。

**計算量:** O(1)。
