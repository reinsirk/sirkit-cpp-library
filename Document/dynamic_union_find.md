# 動的Union Find

`int`全域のキーを、使った時点で登録するUnion Findです。負の値や`INT_MIN`・`INT_MAX`も使えます。連続した添字への変換は利用側で用意する必要がありません。


以下、Nは登録済みキー数です。

## APIと前提

### DynamicUnionFind

```text
DynamicUnionFind();
explicit DynamicUnionFind(std::size_t expected_nodes);
```

空で構築し、後者は指定件数分を予約します。

**制約:** 予約件数はINT_MAX以下。

**計算量:** 空はO(1)、予約は予約容量に比例。

### add

```text
bool add(int key);
```

単独成分として登録し、新規登録ならtrueを返します。

**制約:** 登録数はINT_MAX以下。

**計算量:** ハッシュの期待償却O(1)。

### find

```text
int find(int key);
```

代表キーを返します。未知のキーは自動登録します。

**制約:** 登録数はINT_MAX以下。

**計算量:** 期待償却O(α(N+1))。

### same

```text
int same(int x, int y);
```

同じ成分なら1、異なれば0を返します。未知キーは自動登録します。

**制約:** 登録数はINT_MAX以下。

**計算量:** 期待償却O(α(N+1))。

### unite

```text
int unite(int x, int y);
template<class F> int unite(int x, int y, const F& callback);
```

サイズで併合し、新規結合なら1、既接続なら0です。callback版は併合時だけ新旧代表を渡します。

**制約:** 登録数はINT_MAX以下。callbackで本体を破壊しないこと。

**計算量:** 期待償却O(α(N+1))とcallbackの費用。

### size

```text
int size(int key);
```

所属成分の要素数を返します。未知のキーは自動登録します。

**制約:** 登録数はINT_MAX以下。

**計算量:** 期待償却O(α(N+1))。

### data

```text
int data(int key);
```

根なら負のサイズ、非根なら親キーを返します。未知のキーは自動登録します。

**制約:** 負の親キーとの区別には使わず、代表はfindで取得します。

**計算量:** 期待償却O(α(N+1))。

### contains

```text
bool contains(int key) const;
```

登録済みならtrueです。自動登録しません。

**制約:** 追加の制約はありません。

**計算量:** 期待O(1)。

### node_count

```text
std::size_t node_count() const noexcept;
```

登録済みキーの個数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### component_count

```text
std::size_t component_count() const noexcept;
```

登録済み成分数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

登録がなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### groups

```text
std::vector<std::vector<int>> groups();
```

成分ごとのキー一覧を返します。成分間の順は未規定です。

**制約:** 追加の制約はありません。

**計算量:** O(N α(N+1))、全独立／全連結ではO(N+1)。

### reserve

```text
void reserve(std::size_t expected_nodes);
```

登録状態を保ち指定件数分の容量を予約します。

**制約:** 予約件数はINT_MAX以下。

**計算量:** 再確保前後の容量に比例する期待時間。

### clear

```text
void clear();
```

全登録を消し、確保済み容量を再利用します。

**制約:** 追加の制約はありません。

**計算量:** 保持する配列・表の容量に比例。


`find`・`same`・`size`・`data`・`unite`は、未知のキーを単独成分として自動登録します。登録数は`INT_MAX`以下です。内部の容量上限は`SKIP_RUNTIME_CHECKS`でも維持します。

## 方式と計算量

ハッシュ検索は期待`O(1)`、併合・探索は償却`O(α(N))`です。`contains`は期待`O(1)`、件数取得は`O(1)`、`groups`は`O(N α(N))`、全独立または全連結なら`O(N+1)`です。使用領域は登録数と予約容量に比例します。

コピーは独立した複製を作り、コピー代入の確保失敗時は元の内容を維持します。move元は空になり、再利用できます。1キーの登録に失敗したとき、そのキーは登録されず、既存の連結関係は保ちます。複数キーを扱う操作では、それまでに成功した登録は残ります。`groups`の確保失敗時も登録と連結関係は保ちます。成功した`groups`は親を短縮するため、`data(k)`の返す親キーが変わる場合があります。


[完全な使用例](../Examples/dynamic_union_find_example.cpp)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#dynamic-union-find)
