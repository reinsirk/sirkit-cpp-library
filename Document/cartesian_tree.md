# Cartesian Tree

`CartesianTree`は、数列の最小元を根にして左右の区間を部分木に分ける二分木です。頂点番号は入力の添字で、「左の子→自分→右の子」とたどると元の順序を復元できます。同値なら左の添字を祖先にします。[本体](../Graph/Cartesian_Tree.cpp)・[使用例](../Examples/cartesian_tree_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `CartesianTree()` | 空の木を作る。 |
| `CartesianTree(values[,compare])` / `build(values[,compare])` | spanまたはvectorから構築／再構築する。 |
| `root()` | 根の添字をstd::int32_tで返す。空なら-1。 |
| `parent(v)` / `left_child(v)` / `right_child(v)` | 親／左の子／右の子の添字。存在しなければ-1。 |
| `children(v)` | {左の子,右の子}を返す。 |
| `parents()` / `left_children()` / `right_children()` | 各配列への読み取り専用span。 |
| `size()` / `empty()` / `clear()` | 頂点数／空か／空にする。 |

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
