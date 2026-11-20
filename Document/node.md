# 単方向ノードと連結列

`Node`は値を持たず、`std::shared_ptr<Node> n`で次のノードを所有します。[API一覧](../DataStructure/Node.cpp)と[使用例](../Examples/node_example.cpp)を参照してください。

コピーは後続ノードを共有します。共有されていない長い後続列は反復処理で解放し、連結に沿った深い再帰を避けます。循環参照は自動で回収しません。

以下、kは走査または解放するノード数です。各ノードはshared_ptrで次のノードを所有します。

## コンストラクタ

```text
explicit Node(std::shared_ptr<Node> next = {}) noexcept;
```

nextを次ノードとして所有するノードを構築します。コピーは後続を共有します。

**制約:** nextはNodeのshared_ptr。循環参照を自動回収する機能はありません。

**計算量:** O(1)。Nodeはmake_nodeで作ると1回確保します。

## make_node

```text
std::shared_ptr<Node> make_node(std::shared_ptr<Node> next = {});
```

nextを後続に持つ新しいノードを返します。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** O(1)。追加時はノードを1個確保します。

## make_chain

```text
std::shared_ptr<Node> make_chain(std::size_t count, std::shared_ptr<Node> tail = {});
```

tailの前にcount個の新しいノードを追加して先頭を返します。0個ならtailです。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** O(count)時間、count個のノードを確保。

## chain_size

```text
std::size_t chain_size(const std::shared_ptr<Node>& head);
```

ノード数を返します。

**制約:** 循環のない列を渡すこと。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## chain_tail

```text
std::shared_ptr<Node> chain_tail(const std::shared_ptr<Node>& head);
```

末尾のshared_ptrを返します。空ならnullptrです。

**制約:** 循環のない列を渡すこと。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## chain_nth

```text
std::shared_ptr<Node> chain_nth(const std::shared_ptr<Node>& head, std::size_t index);
```

0始まりでindex番目のshared_ptrを返します。該当するノードがなければnullptrです。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** 非循環はO(min(index+1,k))、循環ありはO(index+1)時間・O(1)追加領域。

## push_front_node

```text
void push_front_node(std::shared_ptr<Node>& head);
```

新しい先頭を作り、headを更新します。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** O(1)。追加時はノードを1個確保します。

## pop_front_node

```text
bool pop_front_node(std::shared_ptr<Node>& head) noexcept;
```

headを次へ移してtrueを返します。空ならfalseです。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** O(1)。追加時はノードを1個確保します。

## reverse_chain

```text
std::shared_ptr<Node> reverse_chain(std::shared_ptr<Node> head) noexcept;
```

接続を反転し、新しい先頭を返します。同じノードを共有している側からも、変更後の接続を参照できます。

**制約:** 循環のない列を渡すこと。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## clone_chain

```text
std::shared_ptr<Node> clone_chain(const std::shared_ptr<Node>& head);
```

同じ長さの独立した列を複製して返します。

**制約:** 循環のない列を渡すこと。

**計算量:** kノードにO(k)時間・k回確保。

## cycle_entry

```text
Node* cycle_entry(const std::shared_ptr<Node>& head) noexcept;
```

循環入口の非所有ポインタを返します。循環がなければnullptrです。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## has_cycle

```text
bool has_cycle(const std::shared_ptr<Node>& head) noexcept;
```

循環があればtrueを返します。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## break_cycle

```text
bool break_cycle(const std::shared_ptr<Node>& head) noexcept;
```

循環末尾から入口への辺を切断したらtrueを返します。共有者にも変更が見えます。

**制約:** headやtailはNodeを所有するshared_ptrです。循環の扱いは各説明に従います。

**計算量:** 対象kノードにO(k)時間・O(1)追加領域。

## clear_chain

```text
void clear_chain(std::shared_ptr<Node>& head) noexcept;
```

headを空にし、共有されていない後続を反復解放します。

**制約:** 循環を回収する場合は先にbreak_cycleで切断すること。

**計算量:** 実際に解放するkノードにO(k)時間・O(1)追加領域。

`chain_size`、`chain_tail`、`clone_chain`は既定で循環を検出し、`invalid_argument`を送出します。`SKIP_RUNTIME_CHECKS`時は循環のない入力が前提です。

`reverse_chain`や`break_cycle`で接続を変えると、同じノードを共有する側からも変更後の接続を参照できます。`pop_front_node`は共有された元の先頭ノードの`n`を切断しません。

`push_front_node`は新ノードの確保に成功してから接続を移すため、確保に失敗しても元の先頭と接続を保持します。

`clear_chain`は解放したノード数に比例します。循環は`break_cycle`で切ってから解放します。

[完全な使用例](../Examples/node_example.cpp)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#node)
## operator=

```text
Node& operator=(const Node&) noexcept = default;
Node& operator=(Node&&) noexcept = default;
```

次ノードのshared_ptrをコピーまたはムーブし、自身への参照を返します。コピーは後続を共有します。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 参照カウント操作はO(1)、旧後続を解放する場合はそのノード数に比例。
