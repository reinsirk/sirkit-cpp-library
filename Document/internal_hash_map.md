# 内部整数ハッシュ表

`sirkit::detail::InternalHashMap<Key, Val, Packed, Denominator>`は動的BITと動的Union Findが共有する内部部品です。一般のキー、削除、反復子が必要なら[HashMap](hash_map.md)を使います。[使用例](../Examples/internal_hash_map_example.cpp)も参照してください。

## API

### detail::InternalHashMap::InternalHashMap

```text
InternalHashMap() noexcept(std::is_nothrow_default_constructible_v<Val>);
InternalHashMap(const InternalHashMap&);
InternalHashMap(InternalHashMap&& other) noexcept(std::is_nothrow_default_constructible_v<Val> && std::is_nothrow_swappable_v<Val>);
```

空の表を構築します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 既定構築・移動はO(1)、コピーはO(容量)。値型の操作費用を加える。

### detail::InternalHashMap::operator=

```text
InternalHashMap& operator=(InternalHashMap other);
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 引数のコピー／移動と交換の費用。

### detail::InternalHashMap::swap

```text
void swap(InternalHashMap& other) noexcept(std::is_nothrow_swappable_v<Val>);
```

内容と所有する領域を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 値型のswap以外はO(1)。

### detail::InternalHashMap::reserve

```text
void reserve(std::size_t n);
```

必要な容量を予約します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 再確保時は期待O(新容量)、不要ならO(1)。

### detail::InternalHashMap::reallocate

```text
void reallocate();
```

容量を2倍にします。未確保なら初期容量を確保します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 期待O(新容量)。

### detail::InternalHashMap::operator[]

```text
Val& operator[](Key key);
```

未登録なら設定済み初期値を追加し、値への参照を返します。

**制約:** Keyが対応する整数型。既存添字の制約はない。

**計算量:** 期待償却O(1)。

### detail::InternalHashMap::find_or_emplace

```text
template <class F> Val& find_or_emplace(Key key, F&& factory);
```

未登録のときだけfactoryで値を作って追加し、値への参照を返します。

**制約:** factoryで表を変更せず、表内要素の参照を使わない。

**計算量:** 期待償却O(1)とfactoryの費用。

### detail::InternalHashMap::find

```text
const Val* find(const Key& key) const noexcept;
Val* find(const Key& key) noexcept;
```

値のポインタを返します。未登録ならnullptrです。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 期待O(1)。

### detail::InternalHashMap::find_nonempty

```text
const Val* find_nonempty(const Key& key) const noexcept;
```

容量確保済みの表で値を検索します。

**制約:** 容量が確保済み。

**計算量:** 期待O(1)。

### detail::InternalHashMap::enumerate

```text
std::vector<std::pair<Key, Val>> enumerate() const;
```

キーと値をコピーしたvectorを順序不定で返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 非空はO(容量)、空はO(1)。

### detail::InternalHashMap::for_each

```text
template <class F> void for_each(F&& f) const;
```

各キーと値のconst参照でコールバックを呼びます。

**制約:** callbackで同じ表を変更しない。

**計算量:** 非空はO(容量)、空はO(1)とcallback費用。

### detail::InternalHashMap::contains

```text
bool contains(Key key) const noexcept;
```

対象が存在すればtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 期待O(1)。

### detail::InternalHashMap::contain

```text
bool contain(Key key) const noexcept;
```

キーが登録済みならtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 期待O(1)。

### detail::InternalHashMap::count

```text
int count(Key key) const noexcept;
```

キーが登録済みなら1、なければ0を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 期待O(1)。

### detail::InternalHashMap::clear

```text
void clear() noexcept;
```

内容を空に戻します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 非空はO(容量)、空はO(1)。

### detail::InternalHashMap::release

```text
void release() noexcept;
```

領域と登録値を破棄して未確保の空表に戻します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(容量)。

### detail::InternalHashMap::capacity

```text
std::size_t capacity() const noexcept;
```

確保済みスロット数を返します。

**制約:** なし。

**計算量:** O(1)。

### detail::InternalHashMap::load_factor

```text
double load_factor() const noexcept;
```

登録件数を容量で割った値を返します。未確保なら0です。

**制約:** なし。

**計算量:** O(1)。

### detail::InternalHashMap::size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** なし。

**計算量:** O(1)。

### detail::InternalHashMap::empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### detail::InternalHashMap::set_default

```text
void set_default(const Val& value);
```

今後のoperator[]による追加に使う初期値を設定します。既存値は変えません。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 値型のコピー代入の費用。


キーはboolを除く64ビット以下の非修飾整数型です。値は既定構築・コピー代入可能である必要があり、表のコピーや`enumerate()`にはコピー構築も必要です。キー、値、占有情報を別々の配列に持ち、乗算ハッシュと線形探索を使います。ハッシュの係数はプロセス内で共有する時刻由来の奇数で、暗号学的な安全性はありません。

`Packed=true`は占有情報をビット単位で保持します。`Denominator`は2～8で、容量Cに対してC−floor(C/Denominator)件まで登録できます。既定値はfalseと5です。容量は8以上の2の冪で増やし、空の既定構築では配列を確保しません。容量上限の保護は`SKIP_RUNTIME_CHECKS`にかかわらず有効です。


上の計算量では値型のコピー・構築・破棄やコールバックの費用をO(1)としています。検索と再確保を伴わない追加の最悪計算量はO(容量)、再確保を伴う追加と再ハッシュはO(容量²)です。攻撃的なキー集合に対する計算量の保証はありません。空表を再確保する場合も新配列の初期化費用は残ります。

再確保・`release()`・表の代入で参照やポインタは無効になります。`reserve(n)`の後は登録数nまでの追加で再確保しません。`clear()`でスロットのオブジェクトは残りますが、その参照を登録値として使い続けないでください。`swap`やムーブでは参照先の所有者が移ります。

`find_or_emplace`のfactoryは表を変更せず、表内の要素への参照やポインタを使わないでください。呼び出し前に再確保する場合があります。`for_each`のコールバックも表を変更しないことが前提です。再確保時のコピーや値の代入が失敗しても登録件数は増えません。失敗した未登録スロットに値の資源が残る場合があります。値型自身の代入・swapが例外を投げる場合、初期値の変更を元に戻す保証はありません。

コピーでは配列も複製します。ムーブ後の元の表は既定の初期値を持つ空の表となり、領域を未確保の状態から再利用できます。`clear()`で残るstringやshared_ptrなどの資源も解放したい場合は`release()`を使います。ただし初期値が所有する資源は保持されます。

[完全な使用例](../Examples/internal_hash_map_example.cpp)・[内部処理と検証の記録](../Audit/library-docs-concise-math-geometry-2026-11-15.json)
