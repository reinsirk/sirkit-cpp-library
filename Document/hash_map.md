# HashMap

`HashMap<Key,Val,Hasher,KeyEqual>`はキーと値を管理するハッシュ表です。キーは順序不定で列挙され、キーと値のオブジェクトは表が所有します。[使用例](../Examples/hash_map_example.cpp)を参照してください。

検索・追加・次要素を探さないキー削除は期待償却O(1)です。ハッシュ計算は別途必要で、列キーでは列長に比例します。衝突が多い入力では時間が増えます。

以下、size_typeはstd::size_t、Nは登録数、Cは現在の容量、C′は再配置後の容量です。ハッシュ・比較・値操作の費用は別途加わります。

## コンストラクタ

```text
HashMap() = default;
explicit HashMap(size_type expected, Hasher hash = {}, KeyEqual equal = {});
HashMap(const HashMap&) = default;
HashMap(HashMap&& other);
```

空表を構築します。expected指定形はその件数分を予約し、hash・equalを設定します。コピーは独立です。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** 空の構築はO(1)。予約・範囲構築の費用はreserveとinsert_rangeに準じます。 ムーブ構築はO(1)、コピーは保持領域に比例。

## operator[]

```text
Val& operator[](const Key& key);
```

登録値への参照を返します。未登録ならVal{}で追加します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## find

```text
iterator find(const Key& key);
const_iterator find(const Key& key) const;
```

登録値を指すiteratorを返します。不在ならend()で、追加しません。

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

登録されていれば1、不在なら0を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## at

```text
Val& at(const Key& key);
const Val& at(const Key& key) const;
```

登録値への参照を返します。不在ならout_of_rangeです。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## get_or_default

```text
Val get_or_default(const Key& key, Val fallback = Val{}) const;
```

登録値またはfallbackのコピーを返します。不在でも追加しません。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

## insert

```text
iterator insert(const value_type& entry);
iterator insert(value_type&& entry);
```

未登録ならキーと値を追加し、対応するiteratorを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## emplace

```text
iterator emplace(const Key& key, const Val& value);
```

未登録ならキーと値を追加し、対応するiteratorを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## try_emplace

```text
template <class... Args> std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args);
```

未登録時だけargsから値を構築し、{iterator,追加したか}を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## insert_or_assign

```text
template <class V> std::pair<iterator, bool> insert_or_assign(const Key& key, V&& value);
```

追加または上書きし、{iterator,追加したか}を返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待償却O(1)。再配置や衝突集中時の上界は共通の計算量を参照。

## erase

```text
iterator erase(const_iterator it, bool get_next = true);
iterator erase(iterator it, bool get_next = true);
iterator erase(const Key& key);
iterator erase(const Key& key, bool get_next);
```

指定要素を削除して次iteratorを返します。get_next=falseなら次を探さずend()を返します。不在/end()は無操作です。

**制約:** iterator版は同じ表の有効なiteratorまたはend()を渡すこと。

**計算量:** 次iteratorを探す形は最悪O(C)。false指定はキー版が期待償却O(1)、iterator版がO(1)。

## begin

```text
iterator begin() noexcept;
const_iterator begin() const noexcept;
```

先頭のiteratorを返します。列挙順は規定しません。

**制約:** 共通の所有・iteratorの有効期間に従います。

**計算量:** 最悪O(C)、空表はO(1)。全走査はO(C)。

## end

```text
iterator end() noexcept;
const_iterator end() const noexcept;
```

終端iteratorを返します。

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

## clear

```text
void clear() noexcept;
```

容量を保ち、全要素と墓石を削除します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(C)。要素・墓石がなければO(1)。

## swap

```text
void swap(HashMap& other);
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

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

## contain

```text
bool contain(const Key& key) const;
```

指定したキーが登録されていればtrueを返します。

**制約:** 共通のキー・ハッシュ・同値判定の前提を満たすこと。

**計算量:** 期待O(1)、最悪O(C)。ハッシュ・比較・値のコピー費用は別です。

```cpp
HashMap<int, int> counts;
counts[7] = 3;
auto value = counts.get_or_default(9, 0); // 0。キー9は追加しない。
counts.erase(7, false);                  // 次iteratorが不要ならfalse。
for (auto it = counts.begin(); it != counts.end();) it = counts.erase(it);
```

## 削除・列挙の計算量

Cは現在のバケット数、C'は再配置後のバケット数、Nは登録数です。ハッシュ・同値判定・要素操作をO(1)とした費用で、衝突が集中すると再配置は最悪O(C+C'+N²)です。

未登録キーやend()の削除は何もしません。次iteratorを返す削除は空きバケットを走査するため、1回の最悪時間はO(capacity())です。返されたiteratorを順に使う全削除では、表全体の走査はO(capacity())に収まります。

次要素が不要ならeraseの第2引数をfalseにします。キー版は期待償却O(1)、有効なiterator版はO(1)で削除します。空表のbegin()はO(1)。削除で残る墓石はrehashやclearで除かれます。

## 所有と前提

コピーは独立した表を作ります。Key/Valはコピー可能、またはvalue_typeのムーブ構築がnoexceptであることが必要です。再配置・clear・swap・ムーブでiteratorと参照が無効になり、eraseでは対象だけが無効になります。再配置を伴う追加に同じ表の要素参照を渡す場合は、先にコピーしてください。

HasherとKeyEqualは、同値なキーに同じハッシュ値を返し、登録中に規則を変えないことが前提です。`GenericHash`は独立に使え、整数・tuple・列・std::hash対応型などを扱います。独自の同値関係には対応するHasherも用意します。

string_viewなどの借用キーでは、登録中に参照先の寿命と内容を保ちます。削除後は参照先を解放できます。swapはHasher/KeyEqualの交換が例外を投げないことを要求します。`SKIP_RUNTIME_CHECKS`でiteratorの所有元・世代・有効性の検査を省き、容量保護と要素の破棄は維持します。

GCCの対応環境では再配置や状態走査にSIMDなどを使います。`SIRKIT_DISABLE_SIMD`で専用命令を除きます。標準経路との使い分けや測定の詳細は[Audit](../Audit/)、集合版は[HashSet](hash_set.md)を参照してください。主Verifyは[Associative Array](../Verify/current/submissions/hash_map.cpp)です。

## operator=

```text
HashMap& operator=(HashMap other);
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
