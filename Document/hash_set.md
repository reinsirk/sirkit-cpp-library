# HashSet

`HashSet<Key,Hasher,KeyEqual>`は、重複しないキーを所有する順序なし集合です。

以下、size_typeはstd::size_t、Nは登録数、Cは現在の容量、C′は再配置後の容量です。ハッシュ・比較・値操作の費用は別途加わります。

## コンストラクタ

```text
HashSet() = default;
explicit HashSet(size_type expected, Hasher hash = {}, KeyEqual equal = {});
HashSet(std::initializer_list<Key> keys, Hasher hash = {}, KeyEqual equal = {});
template <std::input_iterator It, std::sentinel_for<It> Sentinel> HashSet(It first, Sentinel last, Hasher hash = {}, KeyEqual equal = {});
HashSet(const HashSet&) = default;
HashSet(HashSet&& other);
```

空集合を作成できます。予約容量、初期値リスト、iteratorで表した範囲を指定して構築することもできます。コピーは独立です。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** 空の構築はO(1)。予約・範囲構築の費用はreserveとinsert_rangeに準じます。 ムーブ構築はO(1)、コピーは保持領域に比例。

## insert

```text
iterator insert(const Key& key);
iterator insert(Key&& key);
```

追加した要素または登録済みの同じキーを指すiteratorを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## insert_checked

```text
std::pair<iterator, bool> insert_checked(const Key& key);
std::pair<iterator, bool> insert_checked(Key&& key);
```

キーが未登録なら追加し、{iterator,追加したか}を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## emplace

```text
template <class... Args> std::pair<iterator, bool> emplace(Args&&... args);
```

argsからキーを構築し、{iterator,追加したか}を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## insert_range

```text
template <std::input_iterator It, std::sentinel_for<It> Sentinel> void insert_range(It first, Sentinel last);
```

範囲内のキーを追加します。重複は増やしません。

**制約:** 有効な範囲を渡し、同じ表のiteratorを入力にしないこと。

**計算量:** M個の追加に期待償却O(M)、再配置の費用は共通の計算量に従います。

## find

```text
iterator find(const Key& key) const;
```

指定キーのiteratorを返します。不在ならend()です。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## contain

```text
bool contain(const Key& key) const;
```

指定したキーが登録されていればtrueを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## contains

```text
bool contains(const Key& key) const;
```

指定したキーが登録されていればtrueを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## count

```text
int count(const Key& key) const;
```

登録済みなら1、不在なら0を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## erase

```text
iterator erase(const_iterator it, bool get_next = true);
iterator erase(const Key& key, bool get_next = true);
```

指定キーに対応する要素、またはiteratorが指す要素を削除し、次のiteratorを返します。get_next=falseならend()です。キーが見つからない場合やiteratorがend()の場合は何も変更しません。

**制約:** iterator版は同じ表の有効なiteratorまたはend()を渡すこと。

**計算量:** 次iteratorを探す形は最悪O(C)。false指定はキー版が期待償却O(1)、iterator版がO(1)。

## erase_if

```text
template <class Predicate> size_type erase_if(Predicate predicate);
```

述語を満たす要素を削除し、削除数を返します。

**制約:** 述語から同じ表を変更しないこと。

**計算量:** O(C)、空表はO(1)。述語の費用は別です。

## begin

```text
iterator begin() const noexcept;
friend iterator begin(const HashSet& set) noexcept;
```

先頭のiteratorを返します。列挙順は規定しません。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** 最悪O(C)、空表はO(1)。全走査はO(C)。

## end

```text
iterator end() const noexcept;
friend iterator end(const HashSet& set) noexcept;
```

終端iteratorを返します。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** O(1)。

## cbegin

```text
const_iterator cbegin() const noexcept;
```

先頭のconst_iteratorを返します。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** 最悪O(C)、空表はO(1)。全走査はO(C)。

## cend

```text
const_iterator cend() const noexcept;
```

終端const_iteratorを返します。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** O(1)。

## size

```text
size_type size() const noexcept;
```

要素数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## capacity

```text
size_type capacity() const noexcept;
```

確保済みのバケット数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## load_factor

```text
double load_factor() const noexcept;
```

登録数を容量で割った使用率を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## reserve

```text
void reserve(size_type count);
```

指定した要素数まで再確保せず格納できる容量を予約します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 再配置時は期待O(C+C′)、最悪O(C+C′+N²)。予約済みreserveはO(1)。

## rehash

```text
void rehash(size_type requested);
```

指定数以上のバケットへ再配置します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 再配置時は期待O(C+C′)、最悪O(C+C′+N²)。予約済みreserveはO(1)。

## clear

```text
void clear() noexcept;
```

容量を保って全要素と墓石を削除します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(C)。要素・墓石がなければO(1)。

## swap

```text
void swap(HashSet& other);
friend void swap(HashSet& left, HashSet& right) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

## operator==

```text
friend bool operator==(const HashSet& left, const HashSet& right);
```

格納するキー集合が等しければtrueを返します。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** 期待O(両表の容量)。空同士はO(1)、衝突集中で増加します。

## operator!=

```text
bool operator!=(const HashSet& left, const HashSet& right);
```

C++23のoperator==から得られる不等値比較です。

格納するキー集合が異なればtrueを返します。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** 期待O(両表の容量)。空同士はO(1)、衝突集中で増加します。

C/C'は再配置前後の容量、Nは登録数です。ハッシュ・比較・要素操作をO(1)として記します。列キーのハッシュは列長に比例します。衝突集中時の再配置は最悪O(C+C'+N²)。予約済みreserveはO(1)、削除では自動縮小せず`rehash(0)`で縮めます。

次のiteratorを返すeraseは最悪O(C)です。falseを指定すると、キー指定の削除は期待償却O(1)、有効なiteratorでの削除はO(1)です。`erase_if`の述語から表を変更してはいけません。

```cpp
HashSet<int> values;
values.insert(7);
assert(values.contains(7));
values.erase(7, false);
assert(values.empty());
```

## 所有・制約

Keyはコピー可能、またはムーブ構築がnoexceptであることが必要です。コピーはKeyのコピー構築を使って独立し、ムーブ元は空。キーの書き換えは禁止です。再配置・clear・swap・ムーブでiteratorと参照が無効になり、eraseは対象だけを無効にします。

Hasher/KeyEqualは同値なキーへ同じハッシュを返し、登録中に規則を変えません。swapは両比較器の非送出swapが前提です。借用キーの参照先は登録中に寿命・内容を保ち、削除後は解放できます。同じ要素の集合は種・列挙順によらず同じ外側ハッシュになりますが、独自の同値関係には対応する外側Hasherも必要です。

同梱`GenericHash()`は実行時の種、`GenericHash(seed)`は指定種で構築し、`operator()(x)`は整数・列挙・浮動値・tuple・range・std::hash対応型を再帰的にハッシュ化します。unorderedコンテナの列挙順は無視します。

`SKIP_RUNTIME_CHECKS`でiterator検査を除きます。検査有効でも全ての無効な再利用を検出する保証はありません。容量保護は維持し、追加SIMDは`SIRKIT_DISABLE_SIMD`で除外できます。

[完全な使用例](../Examples/hash_set_example.cpp)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#hash-set)

## operator=

```text
HashSet& operator=(HashSet other);
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。

## GenericHash（コンストラクタ）

```text
GenericHash();
explicit constexpr GenericHash(std::uint64_t seed) noexcept;
```

省略形は実行時の種、引数形は指定した種で構築します。同じ個体の規則は一定です。

**制約:** 同値なキーには同じハッシュ値を返すこと。独自の同値関係には対応するHasherを用意します。

**計算量:** O(1)。

## GenericHash::operator()

```text
template<class T> std::uint64_t operator()(const T& value) const;
```

整数・列挙・浮動小数・tuple・range・std::hash対応型を再帰的にハッシュ化します。unorderedコンテナの要素順は無視します。

**制約:** 構成要素もハッシュ化可能であること。std::hash対応型の結果はその実装にも依存します。

**計算量:** 固定幅の整数はO(1)、tupleは各要素の和、列は走査する要素数とそのハッシュ費用の和。
