# バイト列のRolling Hash

バイト列の区間ハッシュ、区間の一致、最長共通接頭辞を照会します。法は`RollingHash::modulus = 2^61-1`、各文字は符号なしバイトです。**衝突があるため、`equal`・`lcp`・`compare`は厳密比較を保証しません。**


以下、nは現在または構築する列の長さです。

## 構築

### RollingHash

```text
RollingHash(std::string_view s, std::uint64_t base);
RollingHash(const RollingHash& other);
RollingHash(RollingHash&& other) noexcept;
```

バイト列を指定するか、コピーまたは所有権の移動で構築します。コピーは元の状態から独立します。NULを含む列も扱い、元の文字列は保持しません。底を自動乱数化する機能はありません。

**制約:** 比較対象間で補正後の底を共有します。base<2は10、base>modulus-2はmodulus-10へ補正します。

**計算量:** 列からとコピーはO(n+1)、move構築はO(1)。

### operator=

```text
RollingHash& operator=(const RollingHash& other);
RollingHash& operator=(RollingHash&& other) noexcept;
```

コピー／移動代入し、自身への参照を返します。

**制約:** 同じ型の個体。

**計算量:** コピーは旧長n・新長mにO(n+m+1)、move代入は旧長nにO(n+1)の上界。

## ハッシュの取得・連結

### get

```text
std::uint64_t get(std::size_t l, std::size_t r) const;
```

半開区間[l,r)のハッシュを返します。空区間は0です。

**制約:** 0<=l<=r<=size()。

**計算量:** O(1)。

### whole

```text
std::uint64_t whole() const noexcept;
```

列全体のハッシュを返します。空なら0です。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### concat

```text
std::uint64_t concat(std::uint64_t left, std::uint64_t right, std::size_t right_length) const;
```

左右を連結したハッシュを返します。

**制約:** 両ハッシュは[0,modulus)、right_length<=size()。底・長さとの対応は利用側で保ちます。

**計算量:** O(1)。

### size

```text
std::size_t size() const noexcept;
```

列の長さを返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### base

```text
std::uint64_t base() const noexcept;
```

補正後の底を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


空区間のハッシュは0です。`concat`の底・ハッシュ・長さの対応は利用側で保ちます。ハッシュ単体は列の長さを表しません。

## 区間の比較

### equal

```text
bool equal(std::size_t l, std::size_t r, const RollingHash& other, std::size_t ol, std::size_t orr) const;
```

2区間の長さとハッシュが等しければtrueです。

**制約:** 各区間は有効な半開区間、底は同じ。衝突による誤判定があります。

**計算量:** O(1)。

### lcp

```text
std::size_t lcp( std::size_t l, const RollingHash& other, std::size_t ol, std::size_t limit = std::numeric_limits<std::size_t>::max()) const;
```

2つの接尾辞の共通接頭辞長を推定します。結果は両方の接尾辞の長さとlimitのいずれも超えません。

**制約:** l<=size()、ol<=other.size()、底は同じ。衝突時の探索結果は保証しません。

**計算量:** mを両残長とlimitの最小値としてO(1+log(m+1))。

### compare

```text
int compare(std::size_t l, std::size_t r, const RollingHash& other, std::size_t ol, std::size_t orr) const;
```

符号なしバイトの辞書順を-1/0/1で推定します。

**制約:** 各区間は有効な半開区間、底は同じ。衝突による誤判定があります。

**計算量:** 短い方の区間長mにO(1+log(m+1))。


区間は各個体の`[0,size()]`内とし、底は揃えます。lcpのmは2つの接尾辞長とlimitの最小値、compareのmは短い方の区間長です。衝突時のlcp探索結果は保証しません。

## 更新・所有

### assign

```text
void assign(std::string_view s);
```

底を保って列を置換し、容量と利用可能な累乗表を再利用します。

**制約:** sは有効な長さ付きバイト列。NULも扱います。

**計算量:** 旧長n・新長mにO(n+m+1)の上界。

### clear

```text
void clear() noexcept;
```

空に戻し、底と確保済み容量を保持します。

**制約:** 追加の制約はありません。

**計算量:** O(n+1)の上界。

### swap

```text
void swap(RollingHash& other) noexcept;
void swap(RollingHash& left, RollingHash& right) noexcept;
```

底・内容・確保済み容量を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


コピー構築はO(n+1)、コピー代入は旧長n・新長mにO(n+m+1)で、内容は独立します。コピー代入とassignは確保失敗でも内容・底を保ちますが、容量は増える場合があります。ムーブ構築はO(1)、代入は旧長nにO(n+1)の上界です。移動元は空で底を保持し、自己ムーブ代入は内容を保ちます。保持領域は確保済み容量CにO(C+1)です。

```cpp
constexpr std::uint64_t base = 911382323;
RollingHash text("abracadabra", base), word("abra", base);
assert(text.equal(0,4,word,0,4));
assert(text.lcp(0,text,7) == 4);
assert(text.concat(text.get(0,4),text.get(4,11),7) == text.whole());
```

不正な区間・連結長は`out_of_range`、底の不一致・未正規化ハッシュは`invalid_argument`です。`SKIP_RUNTIME_CHECKS`でこれらを省いても、容量の上限保護は残ります。Unicodeの復号・正規化は行いません。

[完全な使用例](../Examples/rolling_hash_example.cpp)
