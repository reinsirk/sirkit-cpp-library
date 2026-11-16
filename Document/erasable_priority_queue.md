# 削除可能な優先度付きキュー

`ErasablePriorityQueue<T, Minimize>`はC++20以降で利用でき、追加・先頭削除に加えて、指定した値を1個削除できます。`Minimize=true`なら最小値、`false`なら最大値を先頭にします。

## APIと前提

### ErasablePriorityQueue

```text
ErasablePriorityQueue();
explicit ErasablePriorityQueue(std::vector<T> values);
```

空／vectorの要素から構築します。

**制約:** Tの比較と==が共通の同値関係を表すこと。

**計算量:** 空はO(1)、n要素からはO(n)。

### push

```text
void push(const T& value);
void push(T&& value);
```

値を1個追加します。

**制約:** 冒頭のTの比較・コピー要件を満たすこと。

**計算量:** O(log(H+1))。

### emplace

```text
template<class... Args> void emplace(Args&&... args);
```

引数からTを構築して追加します。

**制約:** 引数からTを構築できること。

**計算量:** 構築とO(log(H+1))。

### top

```text
T top();
```

先頭値を返し、必要な遅延削除を回収します。

**制約:** 空でないこと。

**計算量:** 償却O(log(H+1))。

### pop

```text
void pop();
```

先頭を1個削除します。

**制約:** 空でないこと。

**計算量:** 償却O(log(H+1))。

### erase

```text
void erase(const T& value);
```

存在する値を1個分だけ遅延削除します。

**制約:** その値が有効な集合に存在すること。存在・個数は検査しません。

**計算量:** O(log(H+1))。

### size

```text
std::size_t size() const noexcept;
```

有効な要素数を返します。遅延削除の回収はしません。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

有効な要素がなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### try_top

```text
std::optional<T> try_top();
```

先頭値を返し、空ならnulloptです。

**制約:** 追加の制約はありません。

**計算量:** 償却O(log(H+1))。

### extract_top

```text
std::optional<T> extract_top();
```

先頭を削除して値を返し、空ならnulloptです。

**制約:** 追加の制約はありません。

**計算量:** 償却O(log(H+1))。

### swap

```text
void swap(ErasablePriorityQueue& other) noexcept;
```

有効な要素と遅延削除をまとめて交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

全要素と遅延削除を破棄し、容量を解放します。

**制約:** 追加の制約はありません。

**計算量:** O(H)。


`T`は比較・コピーが可能な型を使います。比較は厳密弱順序を満たし、`==`はその同値関係と一致する必要があります。浮動小数点のNaNはこの条件を満たしません。move元は空になり、再利用できます。

既定では空の`top()`・`pop()`・`erase()`を例外で検出します。`SKIP_RUNTIME_CHECKS`を定義すると、この検査を除きます。空でない場合も、存在しない値の`erase()`は利用側の前提違反です。

## 方式と計算量

値を保持するヒープと、削除する値を保持するヒープを使います。先頭の取得・削除時に両方の先頭が一致していれば、両方から取り除きます。削除の記録は、対象が先頭に来るまで残ります。

未回収の削除済み要素を含む保存数を`H`とすると、追加と削除予約は`O(log(H+1))`です。先頭取得・削除は回収を含めて償却`O(log(H+1))`ですが、1回の操作で多数の削除を回収することがあります。`size()`・`empty()`は`O(1)`、一括構築は`O(n)`、使用領域は`O(H)`です。

要素型や比較演算が例外を投げる場合の保証は、内部の`std::priority_queue`の操作に依存します。

[完全な使用例](../Examples/erasable_priority_queue_example.cpp)・[実装・検証の記録](../Audit/document-detail-before-concise-data-structure-2026-11-15.md#erasable-priority-queue)
