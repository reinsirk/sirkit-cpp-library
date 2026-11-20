# 暗黙Treap

`DataStructure/implicit_treap.cpp`の`lazy_implicit_treap`は挿入・削除・区間操作を行う可変長列です。公開APIは`namespace sirkit`内にあります。[使用例](../Examples/implicit_treap_example.cpp)。

`S`は要素と集約の型、`op`は結合演算、`e()`は単位元、`F`は更新です。`mapping`は更新を適用し、`composition(f,g)`はgの後にfを適用、`id()`は更新なしを表します。更新は集約に分配し、非可換な集約も使えます。`Commutative=true`で可換を指定し、既定の最小値・和では自動判定します。

以下、Nは現在の要素数、Pは保持するノード枠数、MはID索引の枠数、kは削除要素数です。S/Fのコピー・演算・述語をO(1)として記します。木の高さは期待O(log N)、最悪O(N)です。

## API

### lazy_implicit_treap

```text
lazy_implicit_treap();
template<class I> requires std::convertible_to<I,S>
explicit lazy_implicit_treap(I&& identity);
explicit lazy_implicit_treap(const std::vector<S>& values);
lazy_implicit_treap(const std::vector<S>& values, S identity);
lazy_implicit_treap(const lazy_implicit_treap& other);
lazy_implicit_treap(lazy_implicit_treap&& other) noexcept;
```

引数を省略すると空列になります。単位元を指定した空列やvectorからも構築できます。コピーは独立し、ムーブでは所有権を移します。

**制約:** op・単位元・作用は冒頭の契約を満たすこと。

**計算量:** 空はO(1)、vectorはO(N)時間・領域、コピーはO(P+M)、moveはO(1)。

### operator=

```text
lazy_implicit_treap& operator=(const lazy_implicit_treap& other);
lazy_implicit_treap& operator=(lazy_implicit_treap&& other) noexcept;
```

独立コピー／所有権移動で代入し、自身への参照を返します。

**制約:** 追加の制約はありません。

**計算量:** コピーは新旧のP+Mに比例、move代入は旧状態の破棄を含め旧P+旧M+1の上界です。

### identity

```text
S identity() const;
```

設定した単位元を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### size

```text
int size() const;
```

要素数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const;
```

空ならtrueを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### get

```text
S get(int i);
```

位置iの値を返します。

**制約:** 0<=i<size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### set

```text
void set(int i, S value);
```

位置iをvalueに置換します。

**制約:** 0<=i<size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### front

```text
S front();
```

先頭の値を返します。

**制約:** 非空。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### back

```text
S back();
```

末尾の値を返します。

**制約:** 非空。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### insert_at

```text
void insert_at(int pos, S value);
```

位置posの直前に挿入します。

**制約:** 0<=pos<=size()。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。

### push_front

```text
void push_front(S value);
```

先頭へ追加します。

**制約:** 要素数の上限以下。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。

### push_back

```text
void push_back(S value);
```

末尾へ追加します。

**制約:** 要素数の上限以下。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。

### erase_at

```text
void erase_at(int i);
```

位置iの要素を削除します。

**制約:** 0<=i<size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### pop_front

```text
void pop_front();
```

先頭を削除します。空なら無操作です。

**制約:** 追加の制約はありません。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### pop_back

```text
void pop_back();
```

末尾を削除します。空なら無操作です。

**制約:** 追加の制約はありません。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### erase

```text
void erase(int l, int r);
```

区間[l,r)を削除します。検査有効時の不正区間は無操作です。

**制約:** 0<=l<=r<=size()。

**計算量:** 削除数kに期待O(k+log(N+1))。

### erase_prod

```text
S erase_prod(int l, int r);
```

区間の集約値を返して削除します。空区間は単位元です。

**制約:** 0<=l<=r<=size()。

**計算量:** 削除数kに期待O(k+log(N+1))。

### prod

```text
S prod(int l, int r);
```

順序を保った区間集約を返します。空区間は単位元です。

**制約:** 0<=l<=r<=size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### all_prod

```text
S all_prod() const;
```

全体の集約を返し、空なら単位元です。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### apply

```text
void apply(int l, int r, F f);
```

区間へ作用fを適用します。空区間は無操作です。

**制約:** 0<=l<=r<=size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### reverse

```text
void reverse(int l, int r);
```

区間を反転します。

**制約:** 0<=l<=r<=size()。

**計算量:** 期待O(log(N+1))、全体反転はO(1)。

### rotate

```text
void rotate(int l, int m, int r);
```

[l,m)と[m,r)の順序を入れ替えます。

**制約:** 0<=l<=m<=r<=size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### move_range

```text
void move_range(int l, int r, int pos);
```

[l,r)を移動前の隙間posの直前へ移します。l<=pos<=rや空区間は無操作です。

**制約:** 0<=l<=r<=size()。 0<=pos<=size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### to_vector

```text
std::vector<S> to_vector();
```

現在の列をコピーして返します。

**制約:** Sのコピーが可能。

**計算量:** O(N)時間・領域。

### lower_bound

```text
int lower_bound(S value);
```

value以上の最初の添字。なければNを返します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### upper_bound

```text
int upper_bound(S value);
```

valueより大きい最初の添字。なければNを返します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### get_rank

```text
int get_rank(S value);
```

valueより小さい要素数を返します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### count

```text
int count(S value);
```

valueと<で等価な要素数を返します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### contains

```text
bool contains(S value);
```

一致する要素があるかを返します。==が使える場合は==で比較します。使えなければ<で同値かを判定します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### insert_sorted

```text
void insert_sorted(S value);
```

昇順を保って値を挿入します。

**制約:** 列全体がoperator<による昇順。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。

### erase_val

```text
void erase_val(S value);
```

containsと同じ判定で一致する要素を1個削除します。なければ無操作です。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### erase_max

```text
void erase_max();
```

最大値を1個削除します。空なら無操作です。

**制約:** 列全体がoperator<による昇順。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### max_right

```text
template<class G> int max_right(int l, G g);
template<bool (*g)(S)> int max_right(int l);
```

g(prod(l,r))が真となる最大の右端rを返します。

**制約:** 0<=l<=size()、g(identity())が真で、区間伸長に対し単調であること。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### min_left

```text
template<class G> int min_left(int r, G g);
template<bool (*g)(S)> int min_left(int r);
```

g(prod(l,r))が真となる最小の左端lを返します。

**制約:** 0<=r<=size()、g(identity())が真で、区間伸長に対し単調であること。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### reserve

```text
void reserve(std::size_t n);
```

ノード容量を予約します。

**制約:** n<=INT_MAX、内部容量の上限以下。

**計算量:** 再確保時O(P)、足りていればO(1)。

### clear

```text
void clear();
```

全要素とID登録を消し、確保済み容量を保持します。

**制約:** 追加の制約はありません。

**計算量:** O(P+M)。

### swap

```text
void swap(lazy_implicit_treap& other) noexcept;
void swap(lazy_implicit_treap& a, lazy_implicit_treap& b) noexcept;
```

列・単位元・ID索引の所有権を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


添字は0始まり、区間は半開区間です。昇順用操作は列全体が`operator<`で昇順、境界探索は`g(identity())`が真で、区間を伸ばすと単調であることが前提です。これらを確認する全列走査はしません。`contains`・`erase_val`は`==`があればそれを使い、なければ`<`による同値を使います。

既定の`RangeAddMin`はint64_tの区間加算・最小値で、単位元は2^60です。`t(1000)`や`t(values,1000)`の丸括弧で変更します。従来の`t({7})`は1要素、`t{{}}`は空列です。空列は`t{}`と書くと明確です（`t({})`は従来から曖昧）。各要素と加算後の値は単位元以下にします。`RangeAddSum`は`S{sum,size}`の加算・和で、各要素に`size=1`を持たせます。全演算が型の範囲に収まることが前提です。

```cpp
using namespace sirkit;
lazy_implicit_treap<> a(std::vector<std::int64_t>{10, 20, 30}, 1000000);
a.insert_at(1, 5);           // [10,5,20,30]
a.apply(0, 2, 3);            // [13,8,20,30]
auto minimum = a.prod(0, 4); // 8
// 空区間では設定した1000000を返す。
a.move_range(1, 3, 4);       // [13,30,8,20]
```

## ID付き操作

`indexed_implicit_treap`は先頭8テンプレート引数を共有し、`TrackIds=true`にする別名です。IDは非負intで、生存要素間で一意とします。通常の挿入・vector構築はIDなしです。領域はO(P+M)で、大きく飛び飛びのIDは利用側で圧縮してください。

### insert_at_with_id

```text
void insert_at_with_id(int pos, int id, S value);
```

位置posの直前へID付き要素を追加します。

**制約:** TrackIds=true、0<=pos<=size()。idは非負で未使用。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。 ID索引の拡張費用は別。

### insert_after

```text
void insert_after(int id, int new_id, S value);
```

既存idの直後へ追加します。

**制約:** TrackIds=true、idは存在しnew_idは非負で未使用。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。 ID索引の拡張費用は別。

### insert_before

```text
void insert_before(int id, int new_id, S value);
```

既存idの直前へ追加します。

**制約:** TrackIds=true、idは存在しnew_idは非負で未使用。

**計算量:** 償却期待O(log(N+1))、ノード再確保時O(P)。 ID索引の拡張費用は別。

### contains_id

```text
bool contains_id(int id) const;
```

そのIDが生存中ならtrueです。

**制約:** TrackIds=true。

**計算量:** O(1)。

### position_of

```text
int position_of(int id);
```

現在の添字を返し、未登録なら-1です。

**制約:** TrackIds=true。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### get_id

```text
int get_id(int pos);
```

その位置のIDを返し、IDなし要素なら-1です。

**制約:** TrackIds=true、0<=pos<size()。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### get_by_id

```text
S get_by_id(int id);
```

IDに対応する値を返します。

**制約:** TrackIds=true、idが存在すること。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### set_by_id

```text
void set_by_id(int id, S value);
```

IDに対応する値を置換します。

**制約:** TrackIds=true、idが存在すること。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### erase_id

```text
bool erase_id(int id);
```

存在すれば削除してtrue、なければfalseです。

**制約:** TrackIds=true。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### prod_between

```text
S prod_between(int first_id, int second_id);
```

両方のIDの間にある要素を列の順に集約します。両端の要素は含めません。端点のIDを渡す順は任意です。

**制約:** TrackIds=true、両IDが存在すること。

**計算量:** 期待O(log(N+1))。木の高さの最悪はO(N)。

### erase_between

```text
S erase_between(int first_id, int second_id);
```

両IDを除く間の集約を返して削除します。端点は残します。

**制約:** TrackIds=true、両IDが存在すること。

**計算量:** 削除数kに期待O(k+log(N+1))。

### reserve_ids

```text
void reserve_ids(std::size_t n);
```

ID索引をn枠まで確保します。

**制約:** TrackIds=true、n<=INT_MAX+1。

**計算量:** 追加枠数に比例。再確保時は旧Mも含むO(M+n)上界。


端点IDは逆順でも渡せ、同じID・隣接IDの間は単位元です。set・作用・並べ替えはIDを保持し、削除したIDは再利用できます。[ID付き使用例](../Examples/implicit_treap_erase_between_example.cpp)では挿入と区間和・削除を示しています。

コピーは独立した列・ID索引を所有し、代入失敗時は元の列を保ちます。指定した単位元は不変値として保持し、コピー・move・clear後も設定を保ちます。move元は空列として再利用できます。ノードの空き枠は再利用し、clearも確保済み容量を保持します。

既定では添字・区間・必要なIDを低コストで検査します。`erase(l,r)`の不正区間は無操作、他の不正位置・空のfront/back・不存在IDへの参照は例外です。負の挿入ID・重複IDは拒否します。`SKIP_RUNTIME_CHECKS`時は有効な入力が前提ですが、不在へのposition_of/contains_id/erase_idの戻り値と容量保護は維持します。

`erase_prod`は集約値の取得に失敗した場合、削除を行いません。

[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#implicit-treap)
